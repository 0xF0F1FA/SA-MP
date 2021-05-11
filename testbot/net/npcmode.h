
#pragma once

class CNPCMode
{
private:
	AMX m_amx;
	bool m_bInitialised;
	bool m_bSleeping;
	float m_fSleepTime;

public:
	CNPCMode();
	~CNPCMode();
	
	bool Load(char* pFileName);
	void Unload();
	void Frame(float fElapsedTime);
	
	int OnNPCConnect(cell myplayerid);
	//int OnNPCDisconnect(unsigned char* reason);
	int OnNPCSpawn();
	int OnNPCEnterVehicle(cell vehicleid, cell seatid);
	int OnNPCExitVehicle();
	int OnClientMessage(cell color, unsigned char* text);
	int OnPlayerDeath(cell playerid);
	int OnPlayerText(cell playerid, unsigned char* text);
	int OnPlayerStreamIn(cell playerid);
	int OnPlayerStreamOut(cell playerid);
	int OnVehicleStreamIn(cell vehicleid);
	int OnVehicleStreamOut(cell vehicleid);
	int OnRecordingPlaybackEnd();
};
