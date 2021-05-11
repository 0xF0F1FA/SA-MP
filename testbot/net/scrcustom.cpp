
/*
	TODO:
		- Add ability to set NPC data (eg.: health, armour, state)
		  without handling an NPC with a *.rec file
	
	Missing functions(?):
		- native GetPlayerFacingAngle(playerid, &Float:ang);
		  (listed in a_npc.inc, but not actually registered native)

*/

#include "../main.h"

#define CHECK_PARAMS(n) { if (params[0] != (n * sizeof(cell))) { logprintf("SCRIPT: Bad parameter count (Count is %d, Should be %d): ", params[0] / sizeof(cell), n); return 0; } }
#define DEFINE_NATIVE(func) {#func, n_##func}

// native print(const string[]);
static cell n_print(AMX* amx, cell* params)
{
	char* msg;
	amx_StrParam(amx,params[1],msg);
	return 0;
}

// native printf(const format[], {Float,_}:...);
static cell n_printf(AMX* amx, cell* params)
{
	// Does nothing
	return 0;
}

// native format(output[], len, const format[], {Float,_}:...);
static cell n_format(AMX* amx, cell* params)
{
	return 0;
}

// native SetTimer(funcname[], interval, repeating);
static cell n_SetTimer(AMX* amx, cell* params)
{
	CHECK_PARAMS(3);
	return 0;
}

// native KillTimer(timerid);
static cell n_KillTimer(AMX* amx, cell* params)
{
	//pNetGame->GetTimers()->Kill(params[1]);
	return 1;
}

// native GetTickCount();
static cell n_GetTickCount(AMX* amx, cell* params)
{
	CHECK_PARAMS(0);

	return (cell)GetTickCount();
}

// native Float:asin(Float:value);
static cell n_asin(AMX* amx, cell* params)
{
	CHECK_PARAMS(1);
	
	float fResult = (float)asin(amx_ctof(params[1])) * 180 / PI; 
	
	return amx_ftoc(fResult);
}

// native Float:acos(Float:value);
static cell n_acos(AMX* amx, cell* params)
{
	CHECK_PARAMS(1);
	
	float fResult = (float)acos(amx_ctof(params[1])) * 180 / PI;
	
	return amx_ftoc(fResult);
}

// native Float:atan2(Float:x, Float:y);
static cell n_atan2(AMX* amx, cell* params)
{
	CHECK_PARAMS(2);

	float fResult = (float)atan2(amx_ctof(params[2]), amx_ctof(params[1])) * 180 / PI; 
	
	return amx_ftoc(fResult);
}

// native Float:atan(Float:value);
static cell n_atan(AMX* amx, cell* params)
{
	CHECK_PARAMS(1);
	
	float fResult = (float)atan2(1.0f, amx_ctof(params[1])) * 180 / PI; 
	
	return amx_ftoc(fResult);
}

// native StartRecordingPlayback(playbacktype, recordname[]);
static cell n_StartRecordingPlayback(AMX* amx, cell* params)
{
	char* szRecordName;
	amx_StrParam(amx, params[2], szRecordName);
	
	int iPlaybackType = params[1];
	if(iPlaybackType == 1 || iPlaybackType == 2)
	{
		//pNetGame->StartRecordingPlayback(iPlaybackType, szRecordName);
	}
	return 1;
}

// native StopRecordingPlayback();
static cell n_StopRecordingPlayback(AMX* amx, cell* params)
{
	//pNetGame->StopRecordingPlayback();
	return 1;
}

// native PauseRecordingPlayback();
static cell n_PauseRecordingPlayback(AMX* amx, cell* params)
{
	//pNetGame->PauseRecordingPlayback();
	return 1;
}

// native ResumeRecordingPlayback();
static cell n_ResumeRecordingPlayback(AMX* amx, cell* params)
{
	//pNetGame->ResumeRecordingPlayback();
	return 1;
}

// native SendChat(msg[]);
static cell n_SendChat(AMX* amx, cell* params)
{
	char* msg;
	amx_StrParam(amx, params[1], msg);
	//if(pNetGame)
		//pNetGame->SendChat(msg);
	return 1;
}

// native SendCommand(commandtext[]);
static cell n_SendCommand(AMX* amx, cell* params)
{
	char* command;
	amx_StrParam(amx, params[1], command);
	//if(pNetGame)
		//pNetGame->SendCommand(command);
	return 0;
}

// native GetPlayerState(playerid);
static cell n_GetPlayerState(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerPos(playerid, &Float:x, &Float:y, &Float:z);
static cell n_GetPlayerPos(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerVehicleID(playerid);
static cell n_GetPlayerVehicleID(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerArmedWeapon(playerid);
static cell n_GetPlayerArmedWeapon(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerHealth(playerid);
static cell n_GetPlayerHealth(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerArmour(playerid);
static cell n_GetPlayerArmour(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerSpecialAction(playerid);
static cell n_GetPlayerSpecialAction(AMX* amx, cell* params)
{
	return 0;
}

// native IsPlayerStreamedIn(playerid);
static cell n_IsPlayerStreamedIn(AMX* amx, cell* params)
{
	return 0;
}

// native IsVehicleStreamedIn(vehicleid);
static cell n_IsVehicleStreamedIn(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerKeys(playerid, &keys, &updown, &leftright);
static cell n_GetPlayerKeys(AMX* amx, cell* params)
{
	return 0;
}

// native GetMyPos(&Float:x, &Float:y, &Float:z);
static cell n_GetMyPos(AMX* amx, cell* params)
{
	return 0;
}

// native SetMyPos(Float:x, Float:y, Float:z);
static cell n_SetMyPos(AMX* amx, cell* params)
{
	return 0;
}

// native GetMyFacingAngle(&Float:ang);
static cell n_GetMyFacingAngle(AMX* amx, cell* params)
{
	return 0;
}

// native SetMyFacingAngle(Float:ang);
static cell n_SetMyFacingAngle(AMX* amx, cell* params)
{
	return 0;
}

// native GetDistanceFromMeToPoint(Float:X, Float:Y, Float:Z, &Float:Distance);
static cell n_GetDistanceFromMeToPoint(AMX* amx, cell* params)
{
	return 0;
}

// native IsPlayerInRangeOfPoint(playerid, Float:range, Float:X, Float:Y, Float:Z);
static cell n_IsPlayerInRangeOfPoint(AMX* amx, cell* params)
{
	return 0;
}

// native GetPlayerName(playerid, const name[], len);
static cell n_GetPlayerName(AMX* amx, cell* params)
{
	return 0;
}

// native IsPlayerConnected(playerid);
static cell n_IsPlayerConnected(AMX* amx, cell* params)
{
	return pNetGame->GetPlayerPool()->GetSlotState((WORD)params[1]);
}

AMX_NATIVE_INFO custom_Natives[] =
{
	DEFINE_NATIVE(print),
	DEFINE_NATIVE(printf),
	DEFINE_NATIVE(format),
	DEFINE_NATIVE(SetTimer),
	DEFINE_NATIVE(KillTimer),
	DEFINE_NATIVE(GetTickCount),
	DEFINE_NATIVE(asin),
	DEFINE_NATIVE(acos),
	DEFINE_NATIVE(atan2),
	DEFINE_NATIVE(atan),
	DEFINE_NATIVE(StartRecordingPlayback),
	DEFINE_NATIVE(StopRecordingPlayback),
	DEFINE_NATIVE(PauseRecordingPlayback),
	DEFINE_NATIVE(ResumeRecordingPlayback),
	DEFINE_NATIVE(SendChat),
	DEFINE_NATIVE(SendCommand),
	DEFINE_NATIVE(GetPlayerState),
	DEFINE_NATIVE(GetPlayerPos),
	DEFINE_NATIVE(GetPlayerVehicleID),
	DEFINE_NATIVE(GetPlayerArmedWeapon),
	DEFINE_NATIVE(GetPlayerHealth),
	DEFINE_NATIVE(GetPlayerArmour),
	DEFINE_NATIVE(GetPlayerSpecialAction),
	DEFINE_NATIVE(IsPlayerStreamedIn),
	DEFINE_NATIVE(IsVehicleStreamedIn),
	DEFINE_NATIVE(GetPlayerKeys),
	DEFINE_NATIVE(GetMyPos),
	DEFINE_NATIVE(SetMyPos),
	DEFINE_NATIVE(GetMyFacingAngle),
	DEFINE_NATIVE(SetMyFacingAngle),
	DEFINE_NATIVE(GetDistanceFromMeToPoint),
	DEFINE_NATIVE(IsPlayerInRangeOfPoint),
	DEFINE_NATIVE(GetPlayerName),
	DEFINE_NATIVE(IsPlayerConnected),
	{ NULL, NULL }
};

int amx_CustomInit(AMX *amx)
{
  return amx_Register(amx, custom_Natives, -1);
}
