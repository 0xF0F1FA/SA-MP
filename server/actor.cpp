
#include "main.h"

CActor::CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle)
{
	m_fHealth = 100.0f;
	m_iModelID = iModelID;
	m_vecSpawnPosition = vecPos;
	m_fSpawnFacingAngle = fAngle;
	m_vecPosition = vecPos;
	m_fFacingAngle = fAngle;
	m_bInvulnerable = true;
	m_iVirtualWorld = 0;

VECTOR* CActor::GetPosition()
{
	return &m_vecPosition;
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
