//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: playerpool.h,v 1.10 2006/04/09 09:54:45 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

//----------------------------------------------------
#pragma pack(1)
class CPlayerPool
{
private:

	CLocalPlayer	*m_pLocalPlayer;
	WORD			m_wLocalPlayerID;
	int				m_iLocalPlayerScore;
	DWORD			m_dwLocalPlayerPing;

	bool			m_bPlayerSlotState[MAX_PLAYERS];
	CRemotePlayer	*m_pPlayers[MAX_PLAYERS];
	CPlayerInfo		*m_pPlayerInfos[MAX_PLAYERS];
	//DWORD			m_dwPlayerPings[MAX_PLAYERS];
	//ULONG			m_ulIPAddresses[MAX_PLAYERS];
	//int				m_iPlayerScores[MAX_PLAYERS];

	//CHAR			m_szPlayerNames[MAX_PLAYERS][MAX_PLAYER_NAME+1];
	//CHAR			m_szLocalPlayerName[MAX_PLAYER_NAME+1];
	std::string		m_szLocalPlayerName;

	int				m_iPoolSize;

public:
	// Process All CPlayers
	bool Process();

	void SetLocalPlayerName(PCHAR szName) { m_szLocalPlayerName = szName; };
	PCHAR GetLocalPlayerName() { return (PCHAR)m_szLocalPlayerName.data(); };
	
	PCHAR GetPlayerName(WORD wPlayerID)
	{
		if (wPlayerID == m_wLocalPlayerID)
		{
			return (PCHAR)m_szLocalPlayerName.c_str();
		} else {
			if (wPlayerID > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerID]) { return NULL; }
			return m_pPlayerInfos[wPlayerID]->GetName();
		}	
	};

	void SetPlayerName(WORD wPlayerID, PCHAR szName) {
		if (wPlayerID > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerID]) { return; }
		m_pPlayerInfos[wPlayerID]->SetName(szName);
	}

	CLocalPlayer * GetLocalPlayer() { return m_pLocalPlayer; };
	BYTE FindRemotePlayerIDFromGtaPtr(PED_TYPE * pActor);

	bool New(BYTE bytePlayerID, PCHAR szPlayerName);
	bool Delete(BYTE bytePlayerID, BYTE byteReason);
	void UpdatePoolSize();

	CRemotePlayer* GetAt(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS || !m_pPlayerInfos[wPlayerID]) { return NULL; }
		return m_pPlayerInfos[wPlayerID]->GetRemotePlayer();
	};

	// Find out if the slot is inuse.
	bool GetSlotState(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return false; }
		return m_bPlayerSlotState[wPlayerID];
	};

	void SetLocalPlayerID(WORD wID) {
		//strcpy(m_szPlayerNames[byteID], m_szLocalPlayerName);
		m_wLocalPlayerID = wID;
	};

	WORD GetLocalPlayerID() { return m_wLocalPlayerID; };

	BYTE GetCount();
	int GetCount(bool bIncludeNPCs);

	void UpdateScore(WORD wPlayerId, int iScore)
	{
		if (wPlayerId == m_wLocalPlayerID)
		{
			m_iLocalPlayerScore = iScore;
		} else {
			if (wPlayerId > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerId]) { return; }
			m_pPlayerInfos[wPlayerId]->SetScore(iScore);
		}
	};

	void UpdatePing(WORD wPlayerId, DWORD dwPing) {
		if (wPlayerId == m_wLocalPlayerID)
		{
			m_dwLocalPlayerPing = dwPing;
		} else {
			if (wPlayerId > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerId]) { return; }
			m_pPlayerInfos[wPlayerId]->SetPing(dwPing);
		}
	};

	/*void UpdateIPAddress(BYTE bytePlayerId, ULONG ulIPAddress) {
		if (bytePlayerId > MAX_PLAYERS-1) { return; }
		m_ulIPAddresses[bytePlayerId] = ulIPAddress;
	}*/

	int GetLocalPlayerScore() {
		return m_iLocalPlayerScore;
	};

	DWORD GetLocalPlayerPing() {
		return m_dwLocalPlayerPing;
	};

	int GetPlayerScore(WORD wPlayerId) {
		if (wPlayerId > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerId]) { return 0; }
		return m_pPlayerInfos[wPlayerId]->GetScore();
	};

	DWORD GetPlayerPing(WORD wPlayerId)
	{
		if (wPlayerId > MAX_PLAYERS-1 || !m_pPlayerInfos[wPlayerId]) { return 0; }
		return m_pPlayerInfos[wPlayerId]->GetPing();
	};

	/*ULONG GetPlayerIP(BYTE bytePlayerId) {
		if (bytePlayerId > MAX_PLAYERS-1) { return 0; }
		return m_ulIPAddresses[bytePlayerId];
	};*/

	int GetPoolSize() { return m_iPoolSize; };

	void DeactivateAll();
	void DeleteAll();

	CPlayerPool();
	~CPlayerPool();
};

//----------------------------------------------------