
#include "main.h"

CActor::CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle)
{
	m_fHealth = 100.0f;
	m_bInvulnerable = true;
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

