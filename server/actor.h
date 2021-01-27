
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

class CActor
{
private:
	VECTOR m_vecSpawnPosition;
	float m_fSpawnFacingAngle;
	int m_iModelID;
	unsigned short m_usActorID;
	float m_fHealth;
	VECTOR m_vecPosition;
	float m_fFacingAngle;
	bool m_bInvulnerable;
	int m_iVirtualWorld;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	//~CActor() {}

	void SetPosition(float fX, float fY, float fZ);
	VECTOR* GetPosition();
	void SetFacingAngle(float fAngle);
	float GetFacingAngle();
	float GetSquaredDistanceFrom2DPoint(float fX, float fY);
	void SetHealth(float fHealth);
	float GetHealth();
	void SetInvulnerable(bool bInvulnerable);
	bool IsInvulnerable();
	void SetVirtualWorld(int iVirtualWorld);
	int GetVirtualWorld();

	inline int GetModel() { return m_iModelID; };
};

#endif // SAMPSRV_ACTOR_H
