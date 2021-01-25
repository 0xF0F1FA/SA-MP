
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

class CActor
{
private:
	VECTOR m_vecSpawnPosition;
	float m_fSpawnFacingAngle;
	float m_fHealth;
	VECTOR m_vecPosition;
	float m_fFacingAngle;
	bool m_bInvulnerable;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	//~CActor() {}

	VECTOR* GetPosition();
	float GetFacingAngle();
	float GetHealth();
	void SetInvulnerable(bool bInvulnerable);
	bool IsInvulnerable();
};

#endif // SAMPSRV_ACTOR_H
