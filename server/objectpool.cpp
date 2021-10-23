/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: Objectpool.cpp,v 1.10 2006/04/12 19:26:45 mike Exp $

*/

#include "main.h"

//----------------------------------------------------

CObjectPool::CObjectPool()
{
	// loop through and initialize all net players to null and slot states to false
	for(WORD wObjectID = 0; wObjectID != MAX_OBJECTS; wObjectID++) {
		m_bObjectSlotState[wObjectID] = false;
		m_pObjects[wObjectID] = NULL;
		m_bPlayersObject[wObjectID] = false;
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			m_bPlayerObjectSlotState[i][wObjectID] = false;
			m_pPlayerObjects[i][wObjectID] = NULL;
		}
	}
}

//----------------------------------------------------

CObjectPool::~CObjectPool()
{	
	for(BYTE byteObjectID = 0; byteObjectID != MAX_OBJECTS; byteObjectID++) {
		if (!Delete(byteObjectID) && m_bPlayersObject[byteObjectID])
		{
			// Try delete it for individuals
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				DeleteForPlayer(i, byteObjectID);
			}
		}
	}
}

//----------------------------------------------------

WORD CObjectPool::New(int iModel, VECTOR * vecPos, VECTOR * vecRot, float fDrawDist)
{
	WORD wObjectID;

	for(wObjectID=1; wObjectID != MAX_OBJECTS; wObjectID++)
	{
		if(m_bObjectSlotState[wObjectID] == false && m_bPlayersObject[wObjectID] == false) break;
	}

	if(wObjectID == MAX_OBJECTS) return INVALID_OBJECT_ID;

	m_pObjects[wObjectID] = new CObject(iModel,vecPos,vecRot,fDrawDist);

	if(m_pObjects[wObjectID])
	{
		m_pObjects[wObjectID]->SetID(wObjectID);
		m_bObjectSlotState[wObjectID] = true;
		m_bPlayersObject[wObjectID] = false;

		//pNetGame->GetGameMode()->OnObjectSpawn(byteObjectID);

		return wObjectID;
	}
	return INVALID_OBJECT_ID;
}

//----------------------------------------------------

WORD CObjectPool::New(int iPlayer, int iModel, VECTOR* vecPos, VECTOR* vecRot, float fDrawDist)
{
	WORD wObjectID;

	for(wObjectID=1; wObjectID != MAX_OBJECTS; wObjectID++)
	{
		if(m_bObjectSlotState[wObjectID] == false && m_bPlayerObjectSlotState[iPlayer][wObjectID] == false) break;
	}

	if(wObjectID == MAX_OBJECTS) return INVALID_OBJECT_ID;

	CObject *pObject = new CObject(iModel, vecPos, vecRot, fDrawDist);
	
	if (pObject)
	{
		pObject->SetID(wObjectID);
		m_bPlayerObjectSlotState[iPlayer][wObjectID] = true;
		m_pPlayerObjects[iPlayer][wObjectID] = pObject;
		m_bPlayersObject[wObjectID] = true;
		return wObjectID;
	}
	return INVALID_OBJECT_ID;
}

//----------------------------------------------------

void CObjectPool::Process(float fElapsedTime)
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	// Two loops is more efficient than one big one in this case
	for (int i = 0; i <= pPlayerPool->GetPoolSize(); i++)
	{
		if (pNetGame->GetPlayerPool()->GetSlotState(i))
		{
			for (BYTE j = 0; j < MAX_OBJECTS; j++)
			{
				if (m_bPlayersObject[j] && m_bPlayerObjectSlotState[i][j])
				{
					int ret = m_pPlayerObjects[i][j]->Process(fElapsedTime);
					if (ret & 1)
					{
						// Used for scripting paths, tell the script exactly when the object arrives
						pNetGame->GetFilterScripts()->OnPlayerObjectMoved(i, j);
						CGameMode *pGameMode = pNetGame->GetGameMode();
						if(pGameMode) {
							pGameMode->OnPlayerObjectMoved(i, j);
						}
					}
				}
			}
		}
	}
	for (BYTE i = 0; i < MAX_OBJECTS; i++)
	{
		if (!m_bPlayersObject[i] && m_bObjectSlotState[i])
		{
			int ret = m_pObjects[i]->Process(fElapsedTime);
			if (ret & 1) // Use & 1 for future expansion for rotation to use & 2
			{
				pNetGame->GetFilterScripts()->OnObjectMoved(i);
				CGameMode *pGameMode = pNetGame->GetGameMode();
				if(pGameMode) {
					pGameMode->OnObjectMoved(i);
				}
			}
		}
	}
}

//----------------------------------------------------

bool CObjectPool::Delete(WORD wObjectID)
{
	if(!GetSlotState(wObjectID) || !m_pObjects[wObjectID])
	{
		return false; // Object already deleted or not used.
	}

	//pNetGame->GetGameMode()->OnObjectDeath(byteObjectID, 0);

	m_bObjectSlotState[wObjectID] = false;
	delete m_pObjects[wObjectID];
	m_pObjects[wObjectID] = NULL;

	return true;
}

//----------------------------------------------------

bool CObjectPool::DeleteForPlayer(BYTE bytePlayerID, BYTE byteObjectID)
{
	if(!m_bPlayersObject[byteObjectID] || m_pPlayerObjects[bytePlayerID][byteObjectID] == NULL)
	{
		return false; // Object already deleted or not used or global.
	}

	//pNetGame->GetGameMode()->OnObjectDeath(byteObjectID, 0);

	m_bPlayerObjectSlotState[bytePlayerID][byteObjectID] = false;
	delete m_pPlayerObjects[bytePlayerID][byteObjectID];
	m_pPlayerObjects[bytePlayerID][byteObjectID] = NULL;
	for (int i = 0; i < MAX_PLAYERS; i++) // Check if anyone has it anymore
	{	
		if (m_bPlayerObjectSlotState[i][byteObjectID]) return true;
	}
	m_bPlayersObject[byteObjectID] = false; // Mark as an empty slot

	return true;
}

//----------------------------------------------------

void CObjectPool::InitForPlayer(BYTE bytePlayerID)
{	
	// Spawn all existing GLOBAL Objects for player.
	CObject *pObject;
	BYTE x=0;

	while(x!=MAX_OBJECTS) {
		if(GetSlotState(x) == true) {
			pObject = GetAt(x);
			if(pObject->IsActive()) pObject->SpawnForPlayer(bytePlayerID);
		}
		x++;
	}
}

//----------------------------------------------------