/*

	SA:MP Multiplayer Modification
	Copyright 2004-2006 SA:MP Team

	file:
		filterscripts.h
	desc:
		FilterScript functions and management header file. 
*/

#ifndef SAMPSRV_FILTERSCRIPTS_H
#define SAMPSRV_FILTERSCRIPTS_H

//----------------------------------------------------------------------------------

class CFilterScripts
{
private:
	AMX* m_pFilterScripts[MAX_FILTER_SCRIPTS];
	char m_szFilterScriptName[MAX_FILTER_SCRIPTS][255];
	int m_iFilterScriptCount;
	//CScriptTimers* m_pScriptTimers;
public:
	CFilterScripts();
	~CFilterScripts();

	//CScriptTimers* GetTimers() { return m_pScriptTimers; };

	bool LoadFilterScript(char* pFileName);
	bool LoadFilterScriptFromMemory(char* pFileName, char* pFileData);
	bool UnloadOneFilterScript(char* pFilterScript);
	void RemoveFilterScript(int iIndex);
	void UnloadFilterScripts();
	void Frame(float fElapsedTime);
	
	AMX* GetFilterScript(DWORD dwIndex) { return m_pFilterScripts[dwIndex]; };

	int CallPublic(char* szFuncName);

	int OnGameModeInit();
	int OnGameModeExit();
	int OnPlayerConnect(cell playerid);
	int OnPlayerDisconnect(cell playerid, cell reason);
	int OnPlayerSpawn(cell playerid);
	int OnPlayerDeath(cell playerid, cell killerid, cell reason);
	int OnVehicleSpawn(cell vehicleid);
	int OnVehicleDeath(cell vehicleid, cell killerid);
	int OnPlayerText(cell playerid, unsigned char * szText);
	int OnPlayerPrivmsg(cell playerid, cell toplayerid, unsigned char * szText);
	int OnPlayerTeamPrivmsg(cell playerid, unsigned char * szText);
	int OnPlayerCommandText(cell playerid, unsigned char * szCommandText);
	int OnPlayerInfoChange(cell playerid);
	int OnPlayerRequestClass(cell playerid, cell classid);
	int OnPlayerRequestSpawn(cell playerid);
	int OnPlayerEnterVehicle(cell playerid, cell vehicleid, cell ispassenger);
	int OnPlayerExitVehicle(cell playerid, cell vehicleid);
	int OnPlayerStateChange(cell playerid, cell newstate, cell oldstate);
	int OnPlayerEnterCheckpoint(cell playerid);
	int OnPlayerLeaveCheckpoint(cell playerid);
	int OnPlayerEnterRaceCheckpoint(cell playerid);
	int OnPlayerLeaveRaceCheckpoint(cell playerid);
	int OnRconCommand(char* szCommand);
	int OnObjectMoved(cell objectid);
	int OnPlayerObjectMoved(cell playerid, cell objectid);
	int OnPlayerPickedUpPickup(cell playerid, cell pickupid);
	int OnPlayerExitedMenu(cell playerid);
	int OnPlayerSelectedMenuRow(cell playerid, cell row);
	int OnVehicleRespray(cell playerid, cell vehicleid, cell color1, cell color2);
	int OnVehicleMod(cell playerid, cell vehicleid, cell componentid);
	int OnVehiclePaintjob(cell playerid, cell vehicleid, cell paintjobid);
	int OnPlayerInteriorChange(cell playerid, cell newid, cell oldid);
	int OnPlayerKeyStateChange(cell playerid, cell newkeys, cell oldkeys);
	int OnPlayerUpdate(cell playerid);
};

//----------------------------------------------------------------------------------

#endif

