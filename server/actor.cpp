
#include "main.h"

CActor::CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle)
{
	m_usActorID = usActorID;
	m_fHealth = 100.0f;
	m_iModelID = iModelID;
	m_vecSpawnPosition = vecPos;
	m_fSpawnFacingAngle = fAngle;
	m_vecPosition = vecPos;
	m_fFacingAngle = fAngle;
	m_bInvulnerable = true;
	m_iVirtualWorld = 0;
	m_bAnimLoopedOrFreezed = false;

	memset(&m_Animation, 0, sizeof(ACTOR_ANIMATION));
}

CActor::~CActor()
{
	if (pNetGame->GetPlayerPool())
	{
		pNetGame->GetPlayerPool()->DestroyActorForPlayers(m_usActorID);
	}
}

void CActor::SetPosition(float fX, float fY, float fZ)
{
	CPlayerPool* pPlayerPool;
	CPlayer* pPlayer;

	m_vecPosition.X = fX;
	m_vecPosition.Y = fY;
	m_vecPosition.Z = fZ;

	pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
	{
		RakNet::BitStream bsSend;

		bsSend.Write(m_usActorID);
		/*bsSend.Write(fX);
		bsSend.Write(fY);
		bsSend.Write(fZ);*/
		bsSend.Write(m_vecPosition);

		for (int i = 0; i <= pPlayerPool->GetLastPlayerId(); i++)
		{
			pPlayer = pPlayerPool->GetAt(i);

			if (pPlayer && pPlayer->IsActorStreamedIn(m_usActorID))
			{
				pNetGame->SendToPlayer(i, RPC_ScrSetActorPos, &bsSend);
			}
		}
	}
}

VECTOR* CActor::GetPosition()
{
	return &m_vecPosition;
}

void CActor::SetFacingAngle(float fAngle)
{
	CPlayerPool* pPlayerPool;
	CPlayer* pPlayer;

	m_fFacingAngle = fAngle;

	pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
	{
		RakNet::BitStream bsSend;

		bsSend.Write(m_usActorID);
		bsSend.Write(fAngle);

		for (int i = 0; i <= pPlayerPool->GetLastPlayerId(); i++)
		{
			pPlayer = pPlayerPool->GetAt(i);

			if (pPlayer && pPlayer->IsActorStreamedIn(i))
			{
				pNetGame->SendToPlayer(i, RPC_ScrSetActorFacingAngle, &bsSend);
			}
		}
	}
}

float CActor::GetFacingAngle()
{
	return m_fFacingAngle;
}

float CActor::GetSquaredDistanceFrom2DPoint(float fX, float fY)
{
	float
		X = m_vecPosition.X - fX,
		Y = m_vecPosition.Y - fY;

	return Y * Y + X * X;
}

void CActor::SetHealth(float fHealth)
{
	CPlayerPool* pPlayerPool;
	CPlayer* pPlayer;

	m_fHealth = fHealth;

	pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
	{
		RakNet::BitStream bsSend;

		bsSend.Write(m_usActorID);
		bsSend.Write(m_fHealth);

		for (int i = 0; i <= pPlayerPool->GetLastPlayerId(); i++)
		{
			pPlayer = pPlayerPool->GetAt(i);

			if (pPlayer && pPlayer->IsActorStreamedIn(m_usActorID))
			{
				pNetGame->SendToPlayer(i, RPC_ScrSetActorHealth, &bsSend);
			}
		}
	}
}

float CActor::GetHealth()
{
	return m_fHealth;
}

void CActor::ApplyAnimation(char* szAnimLib, char* szAnimName, float fDelta,
	bool bLoop, bool bLockX, bool bLockY, bool bFreeze, int iTime)
{
	ACTOR_ANIMATION Animation;
	CPlayerPool* pPlayerPool;
	CPlayer* pPlayer;

	memset(&Animation, 0, sizeof(ACTOR_ANIMATION));

	strncpy_s(Animation.szAnimLib, szAnimLib, sizeof(Animation.szAnimLib));
	strncpy_s(Animation.szAnimName, szAnimName, sizeof(Animation.szAnimName));
	
	Animation.fDelta = fDelta;
	Animation.bLoop = bLoop;
	Animation.bLockX = bLockX;
	Animation.bLockY = bLockY;
	Animation.bFreeze = bFreeze;
	Animation.iTime = iTime;

	pPlayerPool = pNetGame->GetPlayerPool();

	if (pPlayerPool)
	{
		for (int i = 0; i <= pPlayerPool->GetLastPlayerId(); i++)
		{
			pPlayer = pPlayerPool->GetAt(i);

			if (pPlayer && pPlayer->IsActorStreamedIn(m_usActorID))
			{
				SendAnimation((unsigned short)i, &Animation);
			}
		}
	}

	if (bLoop || bFreeze)
	{
		memcpy(&m_Animation, &Animation, sizeof(ACTOR_ANIMATION));
		m_bAnimLoopedOrFreezed = true;
	}
	else
	{
		memset(&m_Animation, 0, sizeof(ACTOR_ANIMATION));
		m_bAnimLoopedOrFreezed = false;
	}
}

void CActor::SendAnimation(unsigned short usPlayerID, ACTOR_ANIMATION* pAnim)
{
	unsigned char ucLibLen, ucNameLen;

	ucLibLen = (unsigned char)strlen(pAnim->szAnimLib);
	ucNameLen = (unsigned char)strlen(pAnim->szAnimName);

	RakNet::BitStream bsSend;

	bsSend.Write(m_usActorID);
	bsSend.Write(ucLibLen);
	bsSend.Write(pAnim->szAnimLib, ucLibLen);
	bsSend.Write(ucNameLen);
	bsSend.Write(pAnim->szAnimName, ucNameLen);
	bsSend.Write(pAnim->fDelta);
	bsSend.Write(pAnim->bLoop);
	bsSend.Write(pAnim->bLockX);
	bsSend.Write(pAnim->bLockY);
	bsSend.Write(pAnim->bFreeze);
	bsSend.Write(pAnim->iTime);

	if (pNetGame->GetPlayerPool() &&
		pNetGame->GetPlayerPool()->GetSlotState(usPlayerID))
	{
		pNetGame->SendToPlayer(usPlayerID, RPC_ScrApplyActorAnimation, &bsSend);
	}
}

void CActor::ClearAnimations()
{
	CPlayerPool* pPlayerPool;
	CPlayer* pPlayer;

	pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
	{
		RakNet::BitStream bsSend;

		bsSend.Write(m_usActorID);

		for (int i = 0; i <= pPlayerPool->GetLastPlayerId(); i++)
		{
			pPlayer = pPlayerPool->GetAt(i);
			
			if (pPlayer && pPlayer->IsActorStreamedIn(m_usActorID))
			{
				pNetGame->SendToPlayer(i, RPC_ScrClearActorAnimation, &bsSend);
			}
		}
	}

	m_bAnimLoopedOrFreezed = false;
	memset(&m_Animation, 0, sizeof(ACTOR_ANIMATION));
}

void CActor::SetInvulnerable(bool bInvulnerable)
{
	m_bInvulnerable = bInvulnerable;
}

bool CActor::IsInvulnerable()
{
	return m_bInvulnerable;
}

void CActor::SetVirtualWorld(int iVirtualWorld)
{
	m_iVirtualWorld = iVirtualWorld;
}

int CActor::GetVirtualWorld()
{
	return m_iVirtualWorld;
}
