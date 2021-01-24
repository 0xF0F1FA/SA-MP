
#include "main.h"

CActorPool::CActorPool()
{
	memset(m_bSlotState, 0, sizeof(bool) * MAX_ACTORS);
	m_iLastActorID = -1;
}
CActorPool::~CActorPool()
{
	m_iLastActorID = -1;
}
bool CActorPool::GetSlotState(int iActorID)
{
	if (iActorID >= 0 && iActorID < MAX_ACTORS)
	{
		return  m_bSlotState[iActorID];
	}
	return false;
}

void CActorPool::UpdateLastActorID()
{
	m_iLastActorID = -1;

	for (int i = 0; i < MAX_ACTORS; i++)
	{
		if (m_bSlotState[i])
		{
			m_iLastActorID = i;
		}
	}
}
