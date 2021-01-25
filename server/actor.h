
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

class CActor
{
private:
	float m_fHealth;
	bool m_bInvulnerable;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	//~CActor() {}

	float GetHealth();
	void SetInvulnerable(bool bInvulnerable);
	bool IsInvulnerable();
};

#endif // SAMPSRV_ACTOR_H
