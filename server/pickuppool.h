/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		pickuppool.h
	desc:
		Umm, Pickups?

*/
/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: pickuppool.h,v 1.2 2006/03/20 17:59:34 kyeman Exp $

*/

#ifndef SAMPSRV_PICKUPPOOL_H
#define SAMPSRV_PICKUPPOOL_H

class CPickupPool
{
private:
	PICKUP  m_Pickups[MAX_PICKUPS];
	bool	m_bSlotState[MAX_PICKUPS];
	int		m_iVirtualWorld[MAX_PICKUPS];
	int		m_iPoolSize;
	int		m_iPickupCount;

public:
	CPickupPool();

	int New(int iModel, int iType, float fX, float fY, float fZ, int iVirtualWorld);
	int Destroy(int iPickup);
	void UpdatePoolSize();
	void SpawnPickupForPlayer(int iPickup, WORD wPlayer);
	void DeletePickupForPlayer(int iPickup, WORD wPlayer);
	
	bool GetSlotState(int iPickup)
	{
		if (iPickup < 0 || iPickup >= MAX_PICKUPS) return false;
		return m_bSlotState[iPickup];
	}

	PICKUP* GetAt(int iPickup) { return &m_Pickups[iPickup]; }

	int GetVirtualWorld(int iPickup) { return m_iVirtualWorld[iPickup]; };
	void SetVirtualWorld(int iPickup, int iVirtualWorld) { m_iVirtualWorld[iPickup] = iVirtualWorld; };

	int GetPoolSize() { return m_iPoolSize; };
	int GetCount() { return m_iPickupCount; }
};

#endif

