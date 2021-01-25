
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

class CActor
{
private:
	bool m_bInvulnerable;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	//~CActor() {}

	bool IsInvulnerable();
};

#endif // SAMPSRV_ACTOR_H
