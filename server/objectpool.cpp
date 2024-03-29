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
	for(BYTE byteObjectID = 0; byteObjectID != MAX_OBJECTS; byteObjectID++) {
		m_bObjectSlotState[byteObjectID] = false;
		m_pObjects[byteObjectID] = NULL;
		m_bPlayersObject[byteObjectID] = false;
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			m_bPlayerObjectSlotState[i][byteObjectID] = false;
			m_pPlayerObjects[i][byteObjectID] = NULL;
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

BYTE CObjectPool::New(int iModel, VECTOR * vecPos, VECTOR * vecRot, float fDrawDist)
{
	BYTE byteObjectID;

	for(byteObjectID=1; byteObjectID != MAX_OBJECTS; byteObjectID++)
	{
		if(m_bObjectSlotState[byteObjectID] == false && m_bPlayersObject[byteObjectID] == false) break;
	}

	if(byteObjectID == MAX_OBJECTS) return 0xFF;		

	m_pObjects[byteObjectID] = new CObject(iModel,vecPos,vecRot,fDrawDist);

	if(m_pObjects[byteObjectID])
	{
		m_pObjects[byteObjectID]->SetID(byteObjectID);
		m_bObjectSlotState[byteObjectID] = true;
		m_bPlayersObject[byteObjectID] = false;

		//pNetGame->GetGameMode()->OnObjectSpawn(byteObjectID);

		return byteObjectID;
	}
	return 0xFF;
}

//----------------------------------------------------

BYTE CObjectPool::New(int iPlayer, int iModel, VECTOR* vecPos, VECTOR* vecRot, float fDrawDist)
{
	BYTE byteObjectID;

	for(byteObjectID=1; byteObjectID != MAX_OBJECTS; byteObjectID++)
	{
		if(m_bObjectSlotState[byteObjectID] == false && m_bPlayerObjectSlotState[iPlayer][byteObjectID] == false) break;
	}

	if(byteObjectID == MAX_OBJECTS) return 0xFF;		

	CObject *pObject = new CObject(iModel, vecPos, vecRot, fDrawDist);
	
	if (pObject)
	{
		pObject->SetID(byteObjectID);
		m_bPlayerObjectSlotState[iPlayer][byteObjectID] = true;
		m_pPlayerObjects[iPlayer][byteObjectID] = pObject;
		m_bPlayersObject[byteObjectID] = true;
		return byteObjectID;
	}
	return 0xFF;
}

//----------------------------------------------------

void CObjectPool::Process(float fElapsedTime)
{
	// Two loops is more efficient than one big one in this case
	for (BYTE i = 0; i < MAX_PLAYERS; i++)
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

bool CObjectPool::Delete(BYTE byteObjectID)
{
	if(!GetSlotState(byteObjectID) || !m_pObjects[byteObjectID])
	{
		return false; // Object already deleted or not used.
	}

	//pNetGame->GetGameMode()->OnObjectDeath(byteObjectID, 0);

	m_bObjectSlotState[byteObjectID] = false;
	delete m_pObjects[byteObjectID];
	m_pObjects[byteObjectID] = NULL;

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