
#include "main.h"

CActor::CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle)
{
	m_fHealth = 100.0f;
	m_fSpawnFacingAngle = fAngle;
	m_fFacingAngle = fAngle;
	m_bInvulnerable = true;

float CActor::GetFacingAngle()
{
	return m_fFacingAngle;
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

