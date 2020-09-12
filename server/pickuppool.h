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
	int		m_iPickupCount;
	BYTE	m_bActive[MAX_PICKUPS];
	short	m_sLastPickupID;
	int		m_iVirtualWorld[MAX_PICKUPS];

public:
	CPickupPool();

	int New(int iModel, int iType, float fX, float fY, float fZ, BYTE staticp, int iVirtualWorld);
	int Destroy(int iPickup);
	bool IsValid(int iPickupId);
	bool IsStatic(int iPickupId);
	void ProcessLastID();
	void StreamIn(int iPickupID, BYTE bytePlayerID);
	void StreamOut(int iPickupID, BYTE bytePlayerID);
	int GetVirtualWorld(int iPickupID);

	inline bool IsActive(int iPickupID) { return m_bActive[iPickupID] != 0; }
	inline PICKUP Get(int iPickupId) { return m_Pickups[iPickupId]; }
	inline int GetCount() { return m_iPickupCount; }
	inline int GetLastID() { return m_sLastPickupID; }
};

#endif

