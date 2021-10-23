/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: playerpool.cpp,v 1.19 2006/05/07 15:35:32 kyeman Exp $

*/

#include "main.h"

//----------------------------------------------------

CPlayerPool::CPlayerPool()
{
	// loop through and initialize all net players to null and slot states to false
	for(WORD wPlayerID = 0; wPlayerID < MAX_PLAYERS; wPlayerID++) {
		m_bPlayerSlotState[wPlayerID] = false;
		m_pPlayers[wPlayerID] = NULL;
		m_bIsAnNPC[wPlayerID] = false;
	}
	m_iPlayerCount = 0;
	m_iPoolSize = -1; // = 0;
	m_fLastTimerTime = 0.0f;
}

//----------------------------------------------------

CPlayerPool::~CPlayerPool()
{	
	for(WORD wPlayerID = 0; wPlayerID < MAX_PLAYERS; wPlayerID++) {
		Delete(wPlayerID,0);
	}
	m_iPlayerCount = 0;
}

//----------------------------------------------------

bool CPlayerPool::New(WORD wPlayerID, PCHAR szPlayerName, char* szSerial, char* szVersion, bool bIsNPC)
{
	if(wPlayerID >= MAX_PLAYERS) return false;
	if(strlen(szPlayerName) >= MAX_PLAYER_NAME) return false;

	m_pPlayers[wPlayerID] = new CPlayer();

	if(m_pPlayers[wPlayerID])
	{
		strcpy(m_szPlayerName[wPlayerID], szPlayerName);

		memset(m_szPlayerSerial[wPlayerID], 0, MAX_PLAYER_SERIAL+1);
		memset(m_szPlayerVersion[wPlayerID], 0, MAX_PLAYER_VERSION+1);
		
		if(szSerial) {
			if (strlen(szSerial) >= MAX_PLAYER_SERIAL)
				return false;
			strcpy(m_szPlayerSerial[wPlayerID], szSerial);
		}

		if (szVersion) {
			if (strlen(szVersion) >= MAX_PLAYER_VERSION)
				return false;
			strcpy(m_szPlayerVersion[wPlayerID], szVersion);
		}

		m_pPlayers[wPlayerID]->SetID(wPlayerID);
		m_bPlayerSlotState[wPlayerID] = true;
		m_iPlayerScore[wPlayerID] = 0;
		m_iPlayerMoney[wPlayerID] = 0;
		m_bIsAnAdmin[wPlayerID] = false;
		m_iVirtualWorld[wPlayerID] = 0;
		
		BYTE byteIsNPC;
		if (bIsNPC) {
			m_bIsAnNPC[wPlayerID] = true;
			byteIsNPC = 1;
		} else {
			m_bIsAnNPC[wPlayerID] = false;
			byteIsNPC = 0;
		}

		// Notify all the other players of a newcommer with
		// a 'ServerJoin' join RPC 
		RakNet::BitStream bsSend;
		bsSend.Write(wPlayerID);
		bsSend.Write((DWORD)0);
		bsSend.Write(byteIsNPC);
		BYTE namelen = (BYTE)strlen(szPlayerName);
		bsSend.Write(namelen);
		bsSend.Write(szPlayerName, namelen);

		pNetGame->BroadcastData(RPC_ServerJoin, &bsSend, wPlayerID, 2);

		RakServerInterface* pRak = pNetGame->GetRakServer();
		PlayerID Player = pRak->GetPlayerIDFromIndex(wPlayerID);
		in_addr in;
		in.s_addr = Player.binaryAddress;

		int iTime = pConsole->GetIntVariable("playertimeout");
		pNetGame->GetRakServer()->SetTimeoutTime(iTime, Player);

		if (bIsNPC)
			logprintf("[npc:join] %s has joined the server (%u:%s)",
				szPlayerName, wPlayerID, inet_ntoa(in));
		else
			logprintf("[join] %s has joined the server (%u:%s)",
				szPlayerName, wPlayerID, inet_ntoa(in));

/*#ifdef RAKRCON
		bsSend.Reset();
		bsSend.Write(bytePlayerID);
		bsSend.Write(szPlayerName,MAX_PLAYER_NAME);

		pRcon->GetRakServer()->RPC( RPC_ServerJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0,
			UNASSIGNED_PLAYER_ID, true, false );
#endif*/

		m_pPlayers[wPlayerID]->UpdateTimer();

		pNetGame->GetFilterScripts()->OnPlayerConnect(wPlayerID);
		CGameMode *pGameMode = pNetGame->GetGameMode();
		if(pGameMode) {
			pGameMode->OnPlayerConnect(wPlayerID);
		}
		
		m_iPlayerCount++;

		UpdatePoolSize();

		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------

bool CPlayerPool::Delete(WORD wPlayerID, BYTE byteReason)
{
	if(!GetSlotState(wPlayerID) || !m_pPlayers[wPlayerID])
	{
		return false; // Player already deleted or not used.
	}

	CFilterScripts* pFilterScripts = pNetGame->GetFilterScripts();
	if(pFilterScripts) {
		pFilterScripts->OnPlayerDisconnect(wPlayerID, byteReason);
	}
	CGameMode *pGameMode = pNetGame->GetGameMode();
	if(pGameMode) {
		pGameMode->OnPlayerDisconnect(wPlayerID, byteReason);
	}

	m_bPlayerSlotState[wPlayerID] = false;
	delete m_pPlayers[wPlayerID];
	m_pPlayers[wPlayerID] = NULL;
	m_bIsAnAdmin[wPlayerID] = false;
	
	// Notify all the other players that this client is quiting.
	RakNet::BitStream bsSend;
	bsSend.Write(wPlayerID);
	bsSend.Write(byteReason);
	pNetGame->BroadcastData(RPC_ServerQuit, &bsSend, wPlayerID, 2);
		
	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	for (WORD i = 0; i < MAX_OBJECTS; i++)
	{
		// Remove all personal objects (checking done by the function)
		pObjectPool->DeleteForPlayer(wPlayerID, i);
	}

	if (m_bIsAnNPC[wPlayerID]) {
		logprintf("[npc:part] %s has left the server (%u:%u)",m_szPlayerName[wPlayerID],wPlayerID,byteReason);
		m_bIsAnNPC[wPlayerID] = false;
	} else {
		logprintf("[part] %s has left the server (%u:%u)",m_szPlayerName[wPlayerID],wPlayerID,byteReason);
	}

/*#ifdef RAKRCON
	pRcon->GetRakServer()->RPC( RPC_ServerQuit, &bsSend, HIGH_PRIORITY, RELIABLE, 0,
		UNASSIGNED_PLAYER_ID, true, false);
#endif*/

	m_iPlayerCount--;

	UpdatePoolSize();

	return true;
}

//----------------------------------------------------

void CPlayerPool::UpdatePoolSize()
{
	int iNewSize = -1; // = 0;
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_bPlayerSlotState[i])
		{
			iNewSize = i;
		}
	}
	m_iPoolSize = iNewSize;
}

//----------------------------------------------------

bool CPlayerPool::Process(float fElapsedTime)
{
	// Process all CPlayers
	for(BYTE bytePlayerID = 0; bytePlayerID < MAX_PLAYERS; bytePlayerID++)
	{
		if(true == m_bPlayerSlotState[bytePlayerID])
		{
			m_pPlayers[bytePlayerID]->Process(fElapsedTime);
		}
	}

	m_fLastTimerTime += fElapsedTime;
	if (m_fLastTimerTime > 30.0f) // 30 sec
	{
		UpdateTimersForAll();
		m_fLastTimerTime = 0.0f;
	}
	return true;
}

//----------------------------------------------------

void CPlayerPool::UpdateTimersForAll()
{
	for (int x = 0; x <= m_iPoolSize; x++)
	{
		if (m_bPlayerSlotState[x] && m_pPlayers[x])
		{
			m_pPlayers[x]->UpdateTimer();
		}
	}
}

//----------------------------------------------------

void CPlayerPool::InitPlayersForPlayer(BYTE bytePlayerID)
{
	WORD lp=0;
	RakNet::BitStream bsExistingClient;
	RakNet::BitStream bsPlayerVW;

	RakServerInterface* pRak = pNetGame->GetRakServer();
	PlayerID Player = pRak->GetPlayerIDFromIndex(bytePlayerID);

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	bool send = false;

	while(lp!=MAX_PLAYERS) {
		if((GetSlotState(lp) == true) && (lp != bytePlayerID)) {
			BYTE namelen = strlen(GetPlayerName(lp));

			bsExistingClient.Write(lp);
			bsExistingClient.Write(namelen);
			bsExistingClient.Write(GetPlayerName(lp), namelen);
			bsExistingClient.Write(GetPlayerScore(lp));

			pNetGame->GetRakServer()->RPC(RPC_ServerJoin,&bsExistingClient,HIGH_PRIORITY,RELIABLE,0,Player,false,false);
			bsExistingClient.Reset();
			
			// Send all the VW data in one lump
			bsPlayerVW.Write(lp);
			bsPlayerVW.Write(GetPlayerVirtualWorld(lp));
			send = true;
		}
		lp++;
	}
	if (send)
	{
		pRak->RPC(RPC_ScrSetPlayerVirtualWorld , &bsPlayerVW, HIGH_PRIORITY, RELIABLE, 0, Player, false, false);
	}
}

//----------------------------------------------------

void CPlayerPool::InitSpawnsForPlayer(BYTE bytePlayerID)
{
	BYTE x=0;
	CPlayer *pSpawnPlayer;

	while(x!=MAX_PLAYERS) {
		if((GetSlotState(x) == true) && (x != bytePlayerID)) {
			pSpawnPlayer = GetAt(x);
			if(pSpawnPlayer->IsActive()) {
				pSpawnPlayer->SpawnForPlayer(bytePlayerID);
			}
		}
		x++;
	}
}

//----------------------------------------------------
// Return constant describing the type of kill.

BYTE CPlayerPool::GetKillType(BYTE byteWhoKilled, BYTE byteWhoDied)
{

	if( byteWhoKilled != INVALID_PLAYER_ID &&
		byteWhoKilled < MAX_PLAYERS &&
		byteWhoDied < MAX_PLAYERS ) {

		if(m_bPlayerSlotState[byteWhoKilled] && m_bPlayerSlotState[byteWhoDied]) {
			if(GetAt(byteWhoKilled)->GetTeam() == NO_TEAM || GetAt(byteWhoDied)->GetTeam() == NO_TEAM) {
				return VALID_KILL;
			}
			if(GetAt(byteWhoKilled)->GetTeam() != GetAt(byteWhoDied)->GetTeam()) {
				return VALID_KILL;
			}
			else {
				return TEAM_KILL;
			}
		}
		return SELF_KILL;
	}

	if(byteWhoKilled == INVALID_PLAYER_ID && byteWhoDied < MAX_PLAYERS)
	{
		return SELF_KILL;
	}
	
	return SELF_KILL;						
}

//----------------------------------------------------

float CPlayerPool::GetDistanceFromPlayerToPlayer(WORD wPlayer1, WORD wPlayer2)
{
	VECTOR	*vecFromPlayer;
	VECTOR	*vecThisPlayer;
	float	fSX,fSY,fSZ;

	CPlayer * pPlayer1 = GetAt(wPlayer1);
	CPlayer * pPlayer2 = GetAt(wPlayer2);

	vecFromPlayer = &pPlayer1->m_vecPos;
	vecThisPlayer = &pPlayer2->m_vecPos;
	
	fSX = (vecThisPlayer->X - vecFromPlayer->X) * (vecThisPlayer->X - vecFromPlayer->X);
	fSY = (vecThisPlayer->Y - vecFromPlayer->Y) * (vecThisPlayer->Y - vecFromPlayer->Y);
	fSZ = (vecThisPlayer->Z - vecFromPlayer->Z) * (vecThisPlayer->Z - vecFromPlayer->Z);

	return (float)sqrt(fSX + fSY + fSZ);	
}

//----------------------------------------------------

float CPlayerPool::GetDistanceSquaredFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2)
{
	VECTOR	*vecFromPlayer;
	VECTOR	*vecThisPlayer;
	float	fSX,fSY,fSZ;

	CPlayer * pPlayer1 = GetAt(bytePlayer1);
	CPlayer * pPlayer2 = GetAt(bytePlayer2);

	vecFromPlayer = &pPlayer1->m_vecPos;
	vecThisPlayer = &pPlayer2->m_vecPos;
	
	fSX = (vecThisPlayer->X - vecFromPlayer->X) * (vecThisPlayer->X - vecFromPlayer->X);
	fSY = (vecThisPlayer->Y - vecFromPlayer->Y) * (vecThisPlayer->Y - vecFromPlayer->Y);
	fSZ = (vecThisPlayer->Z - vecFromPlayer->Z) * (vecThisPlayer->Z - vecFromPlayer->Z);

	return (float)(fSX + fSY + fSZ);
}

//----------------------------------------------------

bool CPlayerPool::IsNickInUse(PCHAR szNick)
{
	int x=0;
	while(x!=MAX_PLAYERS) {
		if(GetSlotState((WORD)x)) {
			//if(!stricmp(GetPlayerName((BYTE)x),szNick)) {
			if (!strcmp(GetPlayerName((WORD)x), szNick)) {
				return true;
			}
		}
		x++;
	}
	return false;
}

//----------------------------------------------------

void CPlayerPool::DeactivateAll()
{
	CGameMode* pGameMode = pNetGame->GetGameMode();
	CFilterScripts* pFilterScripts = pNetGame->GetFilterScripts();
	int x=0;
	while(x<=m_iPoolSize) {
		if(true == m_bPlayerSlotState[x]) {
			pGameMode->OnPlayerDisconnect(x, 1);
			pFilterScripts->OnPlayerDisconnect(x, 1);
			m_pPlayers[x]->Deactivate();
		}
		//m_byteVirtualWorld[x] = 0;
		x++;
	}
}

//----------------------------------------------------

void CPlayerPool::SetPlayerVirtualWorld(WORD wPlayerID, int iVirtualWorld)
{
	if (wPlayerID >= MAX_PLAYERS || !m_bPlayerSlotState[wPlayerID]) return;
	
	if (pArtwork && iVirtualWorld)
	{
		if (m_iVirtualWorld[wPlayerID] == iVirtualWorld)
			return;
		//*(_BYTE *)(CPlayerPool::GetAt(this, a_playerid) + 12469) = 1; TODO: !
	}
	if (m_iVirtualWorld[wPlayerID] != iVirtualWorld)
	{
		m_iVirtualWorld[wPlayerID] = iVirtualWorld;
		// Tell existing players it's changed
		//RakNet::BitStream bsData;
		//bsData.Write(bytePlayerID); // player id
		//bsData.Write(byteVirtualWorld); // vw id
		//RakServerInterface *pRak = pNetGame->GetRakServer();
		//pRak->RPC(RPC_ScrSetPlayerVirtualWorld , &bsData, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true, false);
	}
}
	
//----------------------------------------------------

void CPlayerPool::DestroyActorForPlayers(unsigned short usActorID)
{
	for (int i = 0; i <= m_iPoolSize; i++)
	{
		if (m_bPlayerSlotState[i] && m_pPlayers[i]->IsActorStreamedIn(usActorID))
		{
			m_pPlayers[i]->StreamActorOut(usActorID);
		}
	}
}

int CPlayerPool::GetPlayerCount()
{
	int iPlayerCount=0;
	int x=0;
	while (x <= m_iPoolSize) {
		if (GetSlotState(x) && IsValidID(x) && !m_bIsAnNPC[x]) {
			iPlayerCount++;
		}
	}
	return iPlayerCount;
}

int CPlayerPool::GetNPCCount()
{
	int iNPCCount=0;
	int x=0;
	while (x<=m_iPoolSize) {
		if (GetSlotState(x) && m_bIsAnNPC[x]) {
			iNPCCount++;
		}
	}
	return iNPCCount;
}
