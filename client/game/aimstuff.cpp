//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: aimstuff.cpp,v 1.6 2006/05/07 15:38:35 kyeman Exp $
//
//----------------------------------------------------------

#include <stdio.h>
#include <windows.h>
#include "common.h"
#include "aimstuff.h"
#include "address.h"

CAMERA_AIM * pcaInternalAim = (CAMERA_AIM *)0xB6F32C;
static CAMERA_AIM caLocalPlayerAim;
static CAMERA_AIM caRemotePlayerAim[MAX_PLAYERS];
extern BYTE * pbyteCameraMode;

static float * pfCameraExtZoom = (float *)0xB6F250;
static float fCameraExtZoom[MAX_PLAYERS];		// stored as a normalized multiplier float
static float fLocalCameraExtZoom;

static BYTE byteCameraMode[MAX_PLAYERS];

static float * pfPlayerStats = (float *)0xB79380;
static float fLocalWeaponSkills[11];
static float fWeaponSkills[MAX_CLIENT_PLAYERS][11];

//----------------------------------------------------------

void __stdcall GameStoreLocalPlayerCameraExtZoom()
{
	fLocalCameraExtZoom = *pfCameraExtZoom;
}

//----------------------------------------------------------

void __stdcall GameSetLocalPlayerCameraExtZoom()
{
	*pfCameraExtZoom = fLocalCameraExtZoom;
}

//----------------------------------------------------------

void __stdcall GameSetPlayerCameraExtZoom(BYTE bytePlayerID, float fZoom)
{
	fCameraExtZoom[bytePlayerID] = fZoom;
}

//----------------------------------------------------------

float __stdcall GameGetPlayerCameraExtZoom(BYTE bytePlayerID)
{
	return fCameraExtZoom[bytePlayerID];
}

//----------------------------------------------------------

float __stdcall GameGetLocalPlayerCameraExtZoom()
{
	float value = ((*pfCameraExtZoom) - 35.0f) / 35.0f;	// normalize for 35.0 to 70.0
	return value;
}

//----------------------------------------------------------

void __stdcall GameSetRemotePlayerCameraExtZoom(BYTE bytePlayerID)
{
	float fZoom = fCameraExtZoom[bytePlayerID];
	float fValue = fZoom * 35.0f + 35.0f; // unnormalize for 35.0 to 70.0
	*pfCameraExtZoom = fValue;
}

//----------------------------------------------------------

void __stdcall GameSetPlayerCameraMode(BYTE byteMode, BYTE bytePlayerID)
{
	byteCameraMode[bytePlayerID] = byteMode;
}

//----------------------------------------------------------

BYTE __stdcall GameGetPlayerCameraMode(BYTE bytePlayerID)
{
	return byteCameraMode[bytePlayerID];
}

//----------------------------------------------------------

BYTE __stdcall GameGetLocalPlayerCameraMode()
{
	return *pbyteCameraMode;
}

//----------------------------------------------------------

void __stdcall GameAimSyncInit()
{
	memset(&caLocalPlayerAim,0,sizeof(CAMERA_AIM));
	memset(caRemotePlayerAim,0,sizeof(CAMERA_AIM) * MAX_PLAYERS);
	memset(byteCameraMode,4,MAX_PLAYERS);
	for(int i=0; i<MAX_PLAYERS; i++)
		fCameraExtZoom[i] = 1.0f;
}

//----------------------------------------------------------

void __stdcall GameStoreLocalPlayerAim()
{
	memcpy(&caLocalPlayerAim,pcaInternalAim,sizeof(CAMERA_AIM));
}

//----------------------------------------------------------

void __stdcall GameSetLocalPlayerAim()
{
	memcpy(pcaInternalAim,&caLocalPlayerAim,sizeof(CAMERA_AIM));
	//memcpy(pInternalCamera,&SavedCam,sizeof(MATRIX4X4));
}

//----------------------------------------------------------

CAMERA_AIM * __stdcall GameGetInternalAim()
{
	return pcaInternalAim;
}

//----------------------------------------------------------

void __stdcall GameStoreRemotePlayerAim(int iPlayer, CAMERA_AIM * caAim)
{
	memcpy(&caRemotePlayerAim[iPlayer],caAim,sizeof(CAMERA_AIM));
}

//----------------------------------------------------------

void __stdcall GameSetRemotePlayerAim(int iPlayer)
{
	memcpy(pcaInternalAim,&caRemotePlayerAim[iPlayer],sizeof(CAMERA_AIM));
}

//----------------------------------------------------------

CAMERA_AIM * __stdcall GameGetRemotePlayerAim(int iPlayer)
{
    return &caRemotePlayerAim[iPlayer];
}

//----------------------------------------------------------

void __stdcall GameStoreLocalPlayerWeaponSkills()
{
	for (int i = 0; i < ARRAY_SIZE(fLocalWeaponSkills); i++)
		fLocalWeaponSkills[i] = pfPlayerStats[69 + i];
}

//----------------------------------------------------------

void __stdcall GameSetLocalPlayerWeaponSkills()
{
	for (int i = 0; i < ARRAY_SIZE(fLocalWeaponSkills); i++)
		pfPlayerStats[69 + i] = fLocalWeaponSkills[i];
}

//----------------------------------------------------------

void __stdcall GameResetLocalPlayerWeaponSkills()
{
	for (int i = 0; i < ARRAY_SIZE(fLocalWeaponSkills); i++)
		pfPlayerStats[69 + i] = 999.0f;

	GameStoreLocalPlayerWeaponSkills();
}

//----------------------------------------------------------

void __stdcall GameSetRemotePlayerWeaponSkills(int iPlayer)
{
	for (int i = 0; i < ARRAY_SIZE(fLocalWeaponSkills); i++)
		pfPlayerStats[69+i] = fWeaponSkills[iPlayer][i];
}
