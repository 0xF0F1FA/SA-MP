/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

*/

#include "../main.h"
#include "../game/util.h"

CObjectPool::CObjectPool()
{
	for(WORD wObjectID = 0; wObjectID < MAX_OBJECTS; wObjectID++)
	{
		m_bObjectSlotState[wObjectID]	= false;
		m_pObjects[wObjectID]			= NULL;
	}
	m_iPoolSize = 0;
};

CObjectPool::~CObjectPool()
{
	for(int i = 0; i < MAX_OBJECTS; i++)
	{
		Delete(i);
	}
}

bool CObjectPool::Delete(WORD wObjectID)
{
	if(!GetSlotState(wObjectID) || !m_pObjects[wObjectID])
	{
		return false; // Vehicle already deleted or not used.
	}

	CCamera* pCamera = pGame->GetCamera();
	if (pCamera->m_pEntity == m_pObjects[wObjectID])
	{
		pCamera->AttachToEntity(NULL);
	}

	m_bObjectSlotState[wObjectID] = false;
	delete m_pObjects[wObjectID];
	m_pObjects[wObjectID] = NULL;

	return true;
}

bool CObjectPool::New(byte byteObjectID, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDist)
{
	if (m_pObjects[byteObjectID] != NULL)
	{
		Delete(byteObjectID);
	}

	m_pObjects[byteObjectID] = pGame->NewObject(iModel, vecPos.X, vecPos.Y, vecPos.Z, vecRot, fDrawDist);

	if (m_pObjects[byteObjectID])
	{
		m_bObjectSlotState[byteObjectID] = true;

		return true;
	}

	return false; // Will only be called if m_pObjects[byteObjectID] is null
}

//----------------------------------------------------

void CObjectPool::UpdatePoolSize()
{
	int iNewSize = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (m_bObjectSlotState[i])
		{
			iNewSize = 0;
		}
	}
	m_iPoolSize = iNewSize;
}

//----------------------------------------------------

int CObjectPool::FindIDFromGtaPtr(ENTITY_TYPE * pGtaObject)
{
	int x=1;

	while(x!=MAX_OBJECTS) {
		if(pGtaObject == m_pObjects[x]->m_pEntity) return x;
		x++;
	}

	return (-1);
}

void CObjectPool::Process()
{
	static unsigned long s_ulongLastCall = 0;
	if (!s_ulongLastCall) s_ulongLastCall = RakNet::GetTime();
	unsigned long ulongTick = GetTickCount();
	float fElapsedTime = ((float)(ulongTick - s_ulongLastCall)) / 1000.0f;
	// Get elapsed time in seconds
	for (int i = 0; i <= m_iPoolSize; i++)
	{
		if (m_bObjectSlotState[i]) m_pObjects[i]->Process(fElapsedTime);
	}
	s_ulongLastCall = ulongTick;
}

int CObjectPool::GetCount()
{
	int iCount = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (m_bObjectSlotState[i])
		{
			iCount++;
		}
	}
	return iCount;
}