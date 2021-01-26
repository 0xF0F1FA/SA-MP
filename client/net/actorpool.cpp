
#include "../main.h"

CActorPool::CActorPool()
{
	m_iLastActorID = -1;

	SecureZeroMemory(m_pActor, sizeof(CActor*) * MAX_ACTORS);
	SecureZeroMemory(m_bSlotState, sizeof(bool) * MAX_ACTORS);
}

CActorPool::~CActorPool()
{
	for (unsigned short i = 0; i < MAX_ACTORS; i++)
	{
		Delete(i);
	}
}

void CActorPool::UpdateLastActorID()
{
	m_iLastActorID = -1;

	for (unsigned short i = 0; i < MAX_ACTORS; i++)
	{
		if (m_bSlotState[i])
		{
			m_iLastActorID = i;
		}
	}
}

bool CActorPool::New(ACTOR_TRANSMIT* pTransmit)
{
	CActor* pActor;
	unsigned short usID;

	usID = pTransmit->usActorID;

	if (usID < MAX_ACTORS && m_bSlotState[usID])
	{
		pChatWindow->AddDebugMessage("Warning: actor %u was not deleted", usID);
		Delete(usID);
	}

	pActor = new CActor(pTransmit->iModelID,
		pTransmit->vecPosition, pTransmit->fFacingAngle);
	if (pActor && usID < MAX_ACTORS)
	{
		m_pActor[usID] = pActor;
		m_bSlotState[usID] = true;

		pActor->SetHealth(pTransmit->fHealth);
		pActor->SetImmunities(pTransmit->bInvurnable);

		UpdateLastActorID();

		return true;
	}
	return false;
}

bool CActorPool::Delete(unsigned short usActorID)
{
	if (usActorID < MAX_ACTORS && m_bSlotState[usActorID] && m_pActor[usActorID])
	{
		m_bSlotState[usActorID] = false;

		delete m_pActor[usActorID];
		m_pActor[usActorID] = 0;

		UpdateLastActorID();

		return true;
	}
	return false;
}

