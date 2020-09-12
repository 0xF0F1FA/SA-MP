/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: pickuppool.cpp,v 1.5 2006/05/07 15:35:32 kyeman Exp $

*/

#include "main.h"

CPickupPool::CPickupPool()
{
	m_iPickupCount = 0;
	m_sLastPickupID = -1;
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		m_bActive[i] = false;
		m_iVirtualWorld[i] = 0;
	}
}

int CPickupPool::New(int iModel, int iType, float fX, float fY, float fZ, BYTE staticp, int iVirtualWorld)
{
	if (m_iPickupCount >= MAX_PICKUPS) return -1;

	for (int i = 0; i < MAX_PICKUPS; i++) {
		if (!m_bActive[i]) {
			// (-1) Static, can't be destroyed
			// ( 1) Dynamic, can be destroyed
			m_bActive[i] = (staticp) ? -1 : 1;

			m_Pickups[i].iModel = iModel;
			m_Pickups[i].iType = iType;
			m_Pickups[i].fX = fX;
			m_Pickups[i].fY = fY;	
			m_Pickups[i].fZ = fZ;
			m_iVirtualWorld[i] = iVirtualWorld;
			m_iPickupCount++;			
			ProcessLastID();
			return i;
		}
	}
	return -1;
}

int CPickupPool::Destroy(int iPickup)
{
	if (iPickup >= 0 && iPickup < MAX_PICKUPS && m_bActive[iPickup] == 1 && pNetGame->GetPlayerPool()) {
		// Destroying the given pickup for all player who has that pickup streamed in
		for (BYTE i = 0; i < MAX_PLAYERS; i++) {
			CPlayer* pPlayer = pNetGame->GetPlayerPool()->GetAt(i);
			if (pPlayer && pPlayer->IsPickupStreamedIn(iPickup)) {
				StreamOut(iPickup, i); // a.k.a. destroy it...
			}
		}
		m_bActive[iPickup] = 0;
		m_iPickupCount--;
		ProcessLastID();
		return 1;		
	}
	return 0;
}

void CPickupPool::ProcessLastID()
{
	m_sLastPickupID = -1;
	for (int i = 0; i < MAX_PICKUPS; i++) {
		if (m_bActive[i])
			m_sLastPickupID = i;
	}
}

bool CPickupPool::IsValid(int iPickupId)
{
	if (0 <= iPickupId && iPickupId <= MAX_PICKUPS && m_bActive[iPickupId] != 0)
		return true;

	return false;
}

bool CPickupPool::IsStatic(int iPickupId)
{
	if (0 <= iPickupId && iPickupId <= MAX_PICKUPS && m_bActive[iPickupId] == -1)
		return true;

	return false;
}

void CPickupPool::StreamIn(int iPickupID, BYTE bytePlayerID)
{
	if (IsValid(iPickupID)) {
		RakNet::BitStream bsPickup;
		bsPickup.Write(iPickupID);
		bsPickup.Write((PCHAR)&m_Pickups[iPickupID], sizeof(PICKUP));
		pNetGame->SendToPlayer(bytePlayerID, RPC_Pickup, &bsPickup);
	}
}

void CPickupPool::StreamOut(int iPickupID, BYTE bytePlayerID)
{
	if (IsValid(iPickupID)) {
		RakNet::BitStream bsPickup;	
		bsPickup.Write(iPickupID);
		pNetGame->SendToPlayer(bytePlayerID, RPC_DestroyPickup, &bsPickup);
	}
}

int CPickupPool::GetVirtualWorld(int iPickupID)
{
	return m_iVirtualWorld[iPickupID];
}
