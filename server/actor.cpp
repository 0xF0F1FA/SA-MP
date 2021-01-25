
#include "main.h"

CActor::CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle)
{
	m_bInvulnerable = true;
}


bool CActor::IsInvulnerable()
{
	return m_bInvulnerable;
}

