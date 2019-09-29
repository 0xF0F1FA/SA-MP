/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		gamemodes.cpp
	desc:
		GameMode Event Executive.

    Version: $Id: gamemodes.cpp,v 1.27 2006/04/19 11:57:59 spookie Exp $

*/

#include "main.h"

#define CHECK_INIT() { if (!m_bInitialised) return 0; };

extern "C" int amx_CoreInit(AMX* amx);
extern "C" int amx_CoreCleanup(AMX* amx);
extern "C" int amx_FloatInit(AMX* amx);
extern "C" int amx_FloatCleanup(AMX* amx);
extern "C" int amx_StringInit(AMX* amx);
extern "C" int amx_StringCleanup(AMX* amx);
extern "C" int amx_FileInit(AMX* amx);
extern "C" int amx_FileCleanup(AMX* amx);
extern "C" int amx_TimeInit(AMX* amx);
extern "C" int amx_TimeCleanup(AMX* amx);
extern "C" int amx_DGramInit(AMX* amx);
extern "C" int amx_DGramCleanup(AMX* amx);
extern "C" int amx_sampDbInit(AMX *amx);
extern "C" int amx_sampDbCleanup(AMX *amx);

int AMXAPI aux_LoadProgram(AMX* amx, char* filename);
int AMXAPI aux_FreeProgram(AMX *amx);
char * AMXAPI aux_StrError(int errnum);
void AMXPrintError(CGameMode* pGameMode, AMX *amx, int error);
int amx_CustomInit(AMX *amx);

char szGameModeFileName[256];

extern CNetGame* pNetGame;
 
//----------------------------------------------------------------------------------

CGameMode::CGameMode()
{
	m_bInitialised = false;
	m_bSleeping = false;
	//m_pScriptTimers = new CScriptTimers;
}

//----------------------------------------------------------------------------------

CGameMode::~CGameMode()
{
	Unload();
	//SAFE_DELETE(m_pScriptTimers);
	/*if (m_pScriptTimers)
	{
		m_pScriptTimers->EndGMTimers();
		//m_pScriptTimers = null;
	}*/
}

//----------------------------------------------------------------------------------

bool CGameMode::Load(char* pFileName)
{
	if (m_bInitialised)
		Unload();

	FILE* f = fopen(pFileName, "rb");
	if (!f) return false;
	fclose(f);

	memset((void*)&m_amx, 0, sizeof(AMX));
	m_fSleepTime = 0.0f;
	strcpy(szGameModeFileName, pFileName);

	int err = aux_LoadProgram(&m_amx, szGameModeFileName);
	if (err != AMX_ERR_NONE)
	{
		AMXPrintError(this, &m_amx, err);
		logprintf("Failed to load '%s' script.", szGameModeFileName);
		return false;
	}

	amx_CoreInit(&m_amx);
	amx_FloatInit(&m_amx);
	amx_StringInit(&m_amx);
	amx_FileInit(&m_amx);
	amx_TimeInit(&m_amx);
	//amx_DGramInit(&m_amx);
	amx_CustomInit(&m_amx);
	amx_sampDbInit(&m_amx);

	pPlugins->DoAmxLoad(&m_amx);

	m_bInitialised = true;

	// Execute OnGameModeInit callback, if it exists!
	int tmp;
	if (!amx_FindPublic(&m_amx, "OnGameModeInit", &tmp))
		amx_Exec(&m_amx, (cell*)&tmp, tmp);
	pNetGame->GetFilterScripts()->OnGameModeInit();
	// ----------------------------------------------

	cell ret = 0;
	err = amx_Exec(&m_amx, &ret, AMX_EXEC_MAIN);
	if (err == AMX_ERR_SLEEP)
	{
		m_bSleeping = true;
		m_fSleepTime = ((float)ret / 1000.0f);
	}
	else if (err != AMX_ERR_NONE)
	{
		m_bSleeping = false;
		AMXPrintError(this, &m_amx, err);
	}

	return true;
}

//----------------------------------------------------------------------------------

void CGameMode::Unload()
{
	// Execute OnGameModeExit callback, if it exists!
	int tmp;
	if (!amx_FindPublic(&m_amx, "OnGameModeExit", &tmp))
		amx_Exec(&m_amx, (cell*)&tmp, tmp);
	// ----------------------------------------------

	// Call in filterscripts
	pNetGame->GetFilterScripts()->OnGameModeExit();
	pNetGame->GetTimers()->DeleteForMode(&m_amx);

	if (m_bInitialised)
	{
		aux_FreeProgram(&m_amx);
		pPlugins->DoAmxUnload(&m_amx);
		amx_sampDbCleanup(&m_amx);
		//amx_DGramCleanup(&m_amx);
		amx_TimeCleanup(&m_amx);
		amx_FileCleanup(&m_amx);
		amx_StringCleanup(&m_amx);
		amx_FloatCleanup(&m_amx);
		amx_CoreCleanup(&m_amx);
	}
	m_bInitialised = false;
	m_bSleeping = false;
}

//----------------------------------------------------------------------------------

void CGameMode::Frame(float fElapsedTime)
{
	if (!m_bInitialised)
		return;

	//if (m_pScriptTimers)
		//m_pScriptTimers->Process((DWORD)(fElapsedTime * 1000.0f));

	if (!m_bSleeping)
		return;

	if (m_fSleepTime > 0.0f)
	{
		m_fSleepTime -= fElapsedTime;
	}
	else
	{
		cell ret;
		int err = amx_Exec(&m_amx, &ret, AMX_EXEC_CONT);
		if (err == AMX_ERR_SLEEP)
		{
			m_bSleeping = true;
			m_fSleepTime = ((float)ret / 1000.0f);
		}
		else
		{
			m_bSleeping = false;
			AMXPrintError(this, &m_amx, err);
		}
	}
}

//----------------------------------------------------------------------------------

int CGameMode::CallPublic(char* szFuncName)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, szFuncName, &idx))
		amx_Exec(&m_amx, &ret, idx);
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerConnect(playerid);
int CGameMode::OnPlayerConnect(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerConnect", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerDisconnect(playerid, reason);
int CGameMode::OnPlayerDisconnect(cell playerid, cell reason)
{
	//CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerDisconnect", &idx))
	{
		amx_Push(&m_amx, reason);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerSpawn(playerid);
int CGameMode::OnPlayerSpawn(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerSpawn", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerDeath(playerid, killerid, reason);
int CGameMode::OnPlayerDeath(cell playerid, cell killerid, cell reason)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerDeath", &idx))
	{
		amx_Push(&m_amx, reason);
		amx_Push(&m_amx, killerid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleSpawn(vehicleid);
int CGameMode::OnVehicleSpawn(cell vehicleid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnVehicleSpawn", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleDeath(vehicleid, killerid);
int CGameMode::OnVehicleDeath(cell vehicleid, cell killerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnVehicleDeath", &idx))
	{
		amx_Push(&m_amx, killerid);
		amx_Push(&m_amx, vehicleid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerText(playerid, text[]);
int CGameMode::OnPlayerText(cell playerid, unsigned char * szText)
{
	CHECK_INIT();

	int idx;
	cell ret = 1;	// DEFAULT TO 1!
	int orig_strlen = strlen((char*)szText) + 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerText", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)szText, 0, 0);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)szText, phys_addr, 0, orig_strlen);
		amx_Release(&m_amx, amx_addr);
	}

	if (ret && pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid)) {
		pNetGame->GetPlayerPool()->GetAt((BYTE)playerid)->Say(szText, strlen((char*)szText));
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerPrivmsg(playerid, toplayerid, text[]);
int CGameMode::OnPlayerPrivmsg(cell playerid, cell toplayerid, unsigned char * szText)
{
	CHECK_INIT();

	int idx;
	cell ret = 1;	// DEFAULT TO 1!
	int orig_strlen = strlen((char*)szText) + 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerPrivmsg", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)szText, 0, 0);
		amx_Push(&m_amx, toplayerid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)szText, phys_addr, 0, orig_strlen);
		amx_Release(&m_amx, amx_addr);
	}

	if (ret && pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid)) {
		pNetGame->GetPlayerPool()->GetAt((BYTE)playerid)->Privmsg((BYTE)toplayerid, szText, strlen((char*)szText));
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerTeamPrivmsg(playerid, text[]);
int CGameMode::OnPlayerTeamPrivmsg(cell playerid, unsigned char * szText)
{
	CHECK_INIT();

	int idx;
	cell ret = 1;	// DEFAULT TO 1!
	int orig_strlen = strlen((char*)szText) + 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerTeamPrivmsg", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)szText, 0, 0);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)szText, phys_addr, 0, orig_strlen);
		amx_Release(&m_amx, amx_addr);
	}

	if (ret && pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid)) {
		pNetGame->GetPlayerPool()->GetAt((BYTE)playerid)->TeamPrivmsg(szText, strlen((char*)szText));
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerCommandText(playerid, cmdtext[]);
int CGameMode::OnPlayerCommandText(cell playerid, unsigned char * szCommandText)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	int orig_strlen = strlen((char*)szCommandText);

	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerCommandText", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)szCommandText, 0, 0);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
		amx_Release(&m_amx, amx_addr);
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerInfoChange(playerid);
int CGameMode::OnPlayerInfoChange(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerInfoChange", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerRequestClass(playerid, classid);
int CGameMode::OnPlayerRequestClass(cell playerid, cell classid)
{
	CHECK_INIT();

	int idx;
	cell ret = 1;	// DEFAULT TO 1!
	
	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerRequestClass", &idx))
	{
		amx_Push(&m_amx, classid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerRequestSpawn(playerid);
int CGameMode::OnPlayerRequestSpawn(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 1;	// DEFAULT TO 1!

	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerRequestSpawn", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
int CGameMode::OnPlayerEnterVehicle(cell playerid, cell vehicleid, cell ispassenger)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerEnterVehicle", &idx))
	{
		amx_Push(&m_amx, ispassenger);
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerExitVehicle(playerid, vehicleid);
int CGameMode::OnPlayerExitVehicle(cell playerid, cell vehicleid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	
	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerExitVehicle", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerStateChange(playerid, newstate, oldstate);
int CGameMode::OnPlayerStateChange(cell playerid, cell newstate, cell oldstate)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	
	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerStateChange", &idx))
	{
		amx_Push(&m_amx, oldstate);
		amx_Push(&m_amx, newstate);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
// forward OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);

int CGameMode::OnPlayerInteriorChange(cell playerid, cell newid, cell oldid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	
	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerInteriorChange", &idx))
	{
		amx_Push(&m_amx, oldid);
		amx_Push(&m_amx, newid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterCheckpoint(playerid);
int CGameMode::OnPlayerEnterCheckpoint(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	
	if(!pNetGame->GetPlayerPool()->GetSlotState((BYTE)playerid))
		return (int)ret;

	if (!amx_FindPublic(&m_amx, "OnPlayerEnterCheckpoint", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerLeaveCheckpoint(playerid);
int CGameMode::OnPlayerLeaveCheckpoint(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerLeaveCheckpoint", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerEnterRaceCheckpoint(playerid);
int CGameMode::OnPlayerEnterRaceCheckpoint(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerEnterRaceCheckpoint", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerLeaveRaceCheckpoint(playerid);
int CGameMode::OnPlayerLeaveRaceCheckpoint(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerLeaveRaceCheckpoint", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerKeyStateChange(playerid,newkeys,oldkeys);
int CGameMode::OnPlayerKeyStateChange(cell playerid, cell newkeys, cell oldkeys)
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerKeyStateChange", &idx))
	{
		amx_Push(&m_amx, oldkeys);
		amx_Push(&m_amx, newkeys);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnRconCommand(cmd[]);
int CGameMode::OnRconCommand(char* szCommand)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;
	int orig_strlen = strlen(szCommand);

	if (!amx_FindPublic(&m_amx, "OnRconCommand", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, szCommand, 0, 0);
		amx_Exec(&m_amx, &ret, idx);
		amx_Release(&m_amx, amx_addr);
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnObjectMoved(objectid);
int CGameMode::OnObjectMoved(cell objectid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnObjectMoved", &idx))
	{
		amx_Push(&m_amx, objectid);
		amx_Exec(&m_amx, &ret, idx);
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerObjectMoved(playerid, objectid);
int CGameMode::OnPlayerObjectMoved(cell playerid, cell objectid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerObjectMoved", &idx))
	{
		amx_Push(&m_amx, objectid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}

	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerPickedUpPickup(playerid, pickupid);
int CGameMode::OnPlayerPickedUpPickup(cell playerid, cell pickupid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerPickUpPickup", &idx))
	{
		amx_Push(&m_amx, pickupid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerExitedMenu(playerid);
int CGameMode::OnPlayerExitedMenu(cell playerid)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerExitedMenu", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnPlayerSelectedMenuRow(playerid, row);
int CGameMode::OnPlayerSelectedMenuRow(cell playerid, cell row)
{
	CHECK_INIT();

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerSelectedMenuRow", &idx))
	{
		amx_Push(&m_amx, row);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleRespray(playerid, vehicleid, color1, color2);
int CGameMode::OnVehicleRespray(cell playerid, cell vehicleid, cell color1, cell color2)
{
	CHECK_INIT();
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnVehicleRespray", &idx))
	{
		amx_Push(&m_amx, color2);
		amx_Push(&m_amx, color1);
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehicleMod(playerid, vehicleid, componentid);
int CGameMode::OnVehicleMod(cell playerid, cell vehicleid, cell componentid)
{
	CHECK_INIT();
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnVehicleMod", &idx))
	{
		amx_Push(&m_amx, componentid);
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------

// forward OnVehiclePaintjob(playerid, vehicleid, paintjobid);

int CGameMode::OnVehiclePaintjob(cell playerid, cell vehicleid, cell paintjobid)
{
	CHECK_INIT();
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnVehiclePaintjob", &idx))
	{
		amx_Push(&m_amx, paintjobid);
		amx_Push(&m_amx, vehicleid);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
// forward OnPlayerUpdate(playerid)

int CGameMode::OnPlayerUpdate(cell playerid)
{
	CHECK_INIT();
	int idx;
	cell ret = 1;

	if(!amx_FindPublic(&m_amx, "OnPlayerUpdate", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

//----------------------------------------------------------------------------------
