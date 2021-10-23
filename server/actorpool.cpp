
#include "main.h"

CActorPool::CActorPool()
{
	for (WORD wActorID = 0; wActorID != MAX_ACTORS; wActorID++)
	{
		m_bSlotState[wActorID] = false;
		m_pActors[wActorID] = NULL;
		m_iVirtualWorld[wActorID] = 0;
	}
	m_iPoolSize = -1; // 0;
}

CActorPool::~CActorPool()
{
	for (WORD wActorID = 0; wActorID < MAX_ACTORS; wActorID++) {
		Delete(wActorID);
	}
}

void CActorPool::UpdatePoolSize()
{
	int iNewSize = -1;
	for (int i = 0; i < MAX_ACTORS; i++)
	{
		if (m_bSlotState[i])
			iNewSize = i;
	}
	m_iPoolSize = iNewSize;
}

WORD CActorPool::New(int iModelID, VECTOR* vecPos, float fAngle)
{
	WORD ActorID;
	for (ActorID = 0; ActorID != MAX_ACTORS; ActorID++)
	{
		if (!m_bSlotState[ActorID]) break;
	}

	if (ActorID == MAX_ACTORS) return INVALID_ACTOR_ID;

	m_pActors[ActorID] = new CActor(iModelID, vecPos, fAngle);
	if (m_pActors[ActorID])
	{
		m_pActors[ActorID]->SetID(ActorID);
		m_bSlotState[ActorID] = true;
		m_iVirtualWorld[ActorID] = 0;

		UpdatePoolSize();

		return ActorID;
	}
	return INVALID_ACTOR_ID;
}

bool CActorPool::Delete(WORD wActorID)
{
	if (!GetSlotState(wActorID) || !m_pActors[wActorID])
	{
		return false;
	}

	delete m_pActors[wActorID];
	m_pActors[wActorID] = NULL;
	m_bSlotState[wActorID] = false;

	UpdatePoolSize();

	return true;
}

void CActorPool::StreamActorInForPlayer(WORD wActorID, WORD wPlayerID)
{
	CActor* pActor;
	ACTOR_TRANSMIT Transmit;

	if (wActorID < MAX_ACTORS && m_bSlotState[wActorID])
	{
		RakNet::BitStream bsSend;

		pActor = m_pActors[wActorID];
		Transmit.wActorID = wActorID;
		Transmit.iSkin = pActor->GetSpawnInfo()->iSkin;
		Transmit.iBaseSkin = pActor->GetSpawnInfo()->iBaseSkin;
		Transmit.vecPos.X = pActor->m_vecPos.X;
		Transmit.vecPos.Y = pActor->m_vecPos.Y;
		Transmit.vecPos.Z = pActor->m_vecPos.Z;
		Transmit.fRotation = pActor->m_fRotation;
		Transmit.fHealth = pActor->m_fHealth;
		Transmit.byteInvurnable = pActor->m_byteInvulnerable;

		bsSend.Write((char*)&Transmit, sizeof(ACTOR_TRANSMIT));

		pNetGame->RPC(RPC_WorldAddActor, &bsSend, wPlayerID, 2);

		if (pActor->bHasAnimation)
			pActor->SendAnimation(wPlayerID, &pActor->m_Animation);
	}
}

void CActorPool::StreamActorOutForPlayer(unsigned short ActorID, unsigned short PlayerID)
{
	if (ActorID < MAX_ACTORS && m_bSlotState[ActorID])
	{
		RakNet::BitStream bsSend;

		bsSend.Write(ActorID);

		pNetGame->SendToPlayer(PlayerID, RPC_WorldRemoveActor, &bsSend);
	}
}
