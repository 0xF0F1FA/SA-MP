
#include "../main.h"

char szNPCModeFileName[256];

CNPCMode::CNPCMode()
{
	m_bInitialised = false;
	m_bSleeping = false;
}

CNPCMode::~CNPCMode()
{
	Unload();
}
	
bool CNPCMode::Load(char* pFileName)
{
	if(m_bInitialised)
		Unload();
	
	FILE* f = fopen(pFileName, "rb");
	if (!f) return false;
	fclose(f);
	
	memset((void*)&m_amx, 0, sizeof(AMX));
	m_fSleepTime = 0.0f;
	
	strcpy(szNPCModeFileName, pFileName);
	int err = aux_LoadProgram(&m_amx, szNPCModeFileName);
	if (err != AMX_ERR_NONE)
	{
		AMXPrintError(this, &m_amx, err);
		return false;
	}

	amx_CoreInit(&m_amx);
	amx_FloatInit(&m_amx);
	amx_StringInit(&m_amx);
	amx_FileInit(&m_amx);
	amx_TimeInit(&m_amx);
	amx_CustomInit(&m_amx);
	
	m_bInitialised = true;
	
	int tmp;
	if (!amx_FindPublic(&m_amx, "OnNPCModeInit", &tmp))
		amx_Exec(&m_amx, (cell*)&tmp, tmp);

	// TODO: Finish the rest of it
}

void CNPCMode::Unload()
{
	int tmp;
	if (!amx_FindPublic(&m_amx, "OnNPCModeExit", &tmp))
		amx_Exec(&m_amx, (cell*)&tmp, tmp);
	
	if (m_bInitialised)
	{
		aux_FreeProgram(&m_amx);
		amx_TimeCleanup(&m_amx);
		amx_FileCleanup(&m_amx);
		amx_StringCleanup(&m_amx);
		amx_FloatCleanup(&m_amx);
		amx_CoreCleanup(&m_amx);
	}
	
	m_bInitialised = false;
	m_bSleeping = false;
}

void CNPCMode::Frame(float fElapsedTime)
{
	// TODO
}
	
int CNPCMode::OnNPCConnect(cell myplayerid)
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnNPCConnect", &idx))
	{
		amx_Push(&m_amx, myplayerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

// Listed forward in a_npc.inc, but this part actually doesn't exist
/*int CNPCMode::OnNPCDisconnect(unsigned char* reason)
{
	int idx;
	cell ret = 0;
	int orig_strlen = strlen((char*)reason);
	
	if (!amx_FindPublic(&m_amx, "OnNPCDisconnect", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)reason, 0, 0);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)reason, phys_addr, 0, orig_strlen + 1);
		amx_Release(&m_amx, amx_addr);
	}
	return (int)ret;
}*/

int CNPCMode::OnNPCSpawn()
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnNPCSpawn", &idx))
	{
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnNPCEnterVehicle(cell vehicleid, cell seatid)
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnNPCEnterVehicle", &idx))
	{
		amx_Push(&m_amx, seatid);
		amx_Push(&m_amx, vehicleid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnNPCExitVehicle()
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnNPCExitVehicle", &idx))
	{
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnClientMessage(cell color, unsigned char* text)
{
	int idx;
	cell ret = 1;
	int orig_strlen = strlen((char*)text);
	
	if (!amx_FindPublic(&m_amx, "OnClientMessage", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)text, 0, 0);
		amx_Push(&m_amx, color);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)text, phys_addr, 0, orig_strlen + 1);
		amx_Release(&m_amx, amx_addr);
	}
	return (int)ret;
}

int CNPCMode::OnPlayerDeath(cell playerid)
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerDeath", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnPlayerText(cell playerid, unsigned char* text)
{
	int idx;
	cell ret = 1;
	int orig_strlen = strlen((char*)text);
	
	if (!amx_FindPublic(&m_amx, "OnPlayerText", &idx))
	{
		cell amx_addr, *phys_addr;
		amx_PushString(&m_amx, &amx_addr, &phys_addr, (char*)text, 0, 0);
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
		amx_GetString((char*)text, phys_addr, 0, orig_strlen + 1);
		amx_Release(&m_amx, amx_addr);
	}
	return (int)ret;
}

int CNPCMode::OnPlayerStreamIn(cell playerid)
{
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerStreamIn", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnPlayerStreamOut(cell playerid)
{
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerStreamOut", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnVehicleStreamIn(cell vehicleid)
{
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnVehicleStreamIn", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnVehicleStreamOut(cell vehicleid)
{
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnVehicleStreamOut", &idx))
	{
		amx_Push(&m_amx, vehicleid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

int CNPCMode::OnRecordingPlaybackEnd()
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnRecordingPlaybackEnd", &idx))
	{
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}
