
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

class CActor
{
private:
	float m_fSpawnFacingAngle;
	float m_fHealth;
	float m_fFacingAngle;
	bool m_bInvulnerable;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	//~CActor() {}

	float GetFacingAngle();
	float GetHealth();
	void SetInvulnerable(bool bInvulnerable);
	bool IsInvulnerable();
};

#endif // SAMPSRV_ACTOR_H
