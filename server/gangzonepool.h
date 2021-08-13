#pragma once

/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: gangzonepool.h,v 1.0 2007/05/25 19:26:45 Y_Less Exp $

*/

//----------------------------------------------------

class CGangZonePool
{
private:
	float			m_fGangZone[MAX_GANG_ZONES][4];
	bool			m_bSlotState[MAX_GANG_ZONES];
public:
	CGangZonePool();
	~CGangZonePool() {};
	WORD New(float fMinX, float fMinY, float fMaxX, float fMaxY);
	void Delete(WORD wZone);
	void ShowForPlayer(WORD wPlayer, WORD wZone, DWORD dwColor);
	void ShowForAll(WORD wZone, DWORD dwColor);
	void HideForPlayer(WORD wPlayer, WORD wZone);
	void HideForAll(WORD wZone);
	void FlashForPlayer(WORD wPlayer, WORD wZone, DWORD dwColor);
	void FlashForAll(WORD wZone, DWORD dwColor);
	void StopFlashForPlayer(WORD wPlayer, WORD wZone);
	void StopFlashForAll(WORD wZone);
	bool GetSlotState(WORD wZone)
	{
		if (wZone >= MAX_GANG_ZONES) return FALSE;
		return m_bSlotState[wZone];
	}
};

//----------------------------------------------------
