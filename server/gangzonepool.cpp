/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: gangzonepool.cpp,v 1.0 2007/05/25 19:26:45 Y_Less Exp $

Based on original hook by Peter

*/

#include "main.h"

#define RGBA_ABGR(n) (((n >> 24) & 0x000000FF) | ((n >> 8) & 0x0000FF00) | ((n << 8) & 0x00FF0000) | ((n << 24) & 0xFF000000))

//----------------------------------------------------

CGangZonePool::CGangZonePool()
{
	for (WORD wZone = 0; wZone < MAX_GANG_ZONES; wZone++)
	{
		m_bSlotState[wZone] = false;
	}
}

WORD CGangZonePool::New(float fMinX, float fMinY, float fMaxX, float fMaxY)
{
	WORD wZone = 0;
	while (wZone < MAX_GANG_ZONES)
	{
		if (!m_bSlotState[wZone]) break;
		wZone++;
	}
	if (wZone == MAX_GANG_ZONES) return 0xFFFF;
	m_fGangZone[wZone][0] = fMinX - (fMinX - floor(fMinX));
	m_fGangZone[wZone][1] = fMinY - (fMinY - floor(fMinY));
	m_fGangZone[wZone][2] = fMaxX - (fMaxX - floor(fMaxX));
	m_fGangZone[wZone][3] = fMaxY - (fMaxY - floor(fMaxY));
	m_bSlotState[wZone] = true;
	return wZone;
}

void CGangZonePool::Delete(WORD wZone)
{
	m_bSlotState[wZone] = false;
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->BroadcastData(RPC_ScrRemoveGangZone, &bsParams, INVALID_PLAYER_ID, 2);
}

void CGangZonePool::ShowForPlayer(WORD wPlayer, WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	bsParams.Write(m_fGangZone[wZone][0]);
	bsParams.Write(m_fGangZone[wZone][1]);
	bsParams.Write(m_fGangZone[wZone][2]);
	bsParams.Write(m_fGangZone[wZone][3]);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->RPC(RPC_ScrAddGangZone, &bsParams, wPlayer, 2);
}

void CGangZonePool::ShowForAll(WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	bsParams.Write(m_fGangZone[wZone][0]);
	bsParams.Write(m_fGangZone[wZone][1]);
	bsParams.Write(m_fGangZone[wZone][2]);
	bsParams.Write(m_fGangZone[wZone][3]);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->BroadcastData(RPC_ScrAddGangZone, &bsParams, INVALID_PLAYER_ID, 2);
}

void CGangZonePool::HideForPlayer(WORD wPlayer, WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->RPC(RPC_ScrRemoveGangZone, &bsParams, wPlayer, 2);
}

void CGangZonePool::HideForAll(WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->BroadcastData(RPC_ScrRemoveGangZone, &bsParams, INVALID_PLAYER_ID, 2);
}

void CGangZonePool::FlashForPlayer(WORD wPlayer, WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->RPC(RPC_ScrFlashGangZone, &bsParams, wPlayer, 2);
}

void CGangZonePool::FlashForAll(WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->BroadcastData(RPC_ScrFlashGangZone, &bsParams, INVALID_PLAYER_ID, 2);
}

void CGangZonePool::StopFlashForPlayer(WORD wPlayer, WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->RPC(RPC_ScrStopFlashGangZone, &bsParams, wPlayer, 2);
}

void CGangZonePool::StopFlashForAll(WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->BroadcastData(RPC_ScrStopFlashGangZone, &bsParams, INVALID_PLAYER_ID, 2);
}
