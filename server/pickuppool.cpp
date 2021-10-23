/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: pickuppool.cpp,v 1.5 2006/05/07 15:35:32 kyeman Exp $

*/

#include "main.h"

CPickupPool::CPickupPool()
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		m_bSlotState[i] = false;
		m_iVirtualWorld[i] = 0;
	}
	m_iPoolSize = -1; // 0;
	m_iPickupCount = 0;
}

int CPickupPool::New(int iModel, int iType, float fX, float fY, float fZ, int iVirtualWorld)
{
	int i;
	for (i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_bSlotState[i] == false) break;
	}

	if (i == MAX_PICKUPS) return -1;

	m_Pickups[i].iModel = iModel;
	m_Pickups[i].iType = iType;
	m_Pickups[i].fX = fX;
	m_Pickups[i].fY = fY;
	m_Pickups[i].fZ = fZ;
	m_bSlotState[i] = true;
	m_iVirtualWorld[i] = iVirtualWorld;
	m_iPickupCount++;

	UpdatePoolSize();

	return i;
}

int CPickupPool::Destroy(int iPickup)
{
	// Despite being signed, SA-MP only checked if the iPickup is lower than MAX_PICKUPS
	if (iPickup < 0 || iPickup >= MAX_PICKUPS || !m_bSlotState[iPickup])
	{
		return 0;
	}

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	for (int i = 0; i <= pPlayerPool->GetPoolSize(); i++)
	{
		if (pPlayerPool->GetSlotState(i))
		{
			CPlayer* pPlayer = pPlayerPool->GetAt(i);
			if (pPlayer && pPlayer->IsPickupStreamedIn(iPickup))
			{
				pPlayer->StreamPickupOut(iPickup);
			}
		}
	}

	m_bSlotState[iPickup] = false;
	m_iVirtualWorld[iPickup] = 0;
	m_iPickupCount--;

	UpdatePoolSize();
	
	return 1;
}

void CPickupPool::SpawnPickupForPlayer(int iPickup, WORD wPlayer)
{
	if (iPickup < 0 || iPickup >= MAX_PICKUPS || !m_bSlotState[iPickup])
	{
		return;
	}

	RakNet::BitStream bsPickup;
	bsPickup.Write(iPickup);
	bsPickup.Write((PCHAR)&m_Pickups[iPickup], sizeof (PICKUP));
	pNetGame->RPC(RPC_Pickup, &bsPickup, wPlayer, 2);
}

void CPickupPool::DeletePickupForPlayer(int iPickup, WORD wPlayer)
{
	if (iPickup < 0 || iPickup >= MAX_PICKUPS || !m_bSlotState[iPickup])
	{
		return;
	}

	RakNet::BitStream bsPickup;
	bsPickup.Write(iPickup);
	pNetGame->RPC(RPC_DestroyPickup, &bsPickup, wPlayer, 2);
}

void CPickupPool::UpdatePoolSize()
{
	int iNewSize = -1; // 0;
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_bSlotState[i] == true)
			iNewSize = i;
	}
	m_iPoolSize = iNewSize;
}
