
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

}

float CActor::GetHealth()
{
	return m_fHealth;
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
