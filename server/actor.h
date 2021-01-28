
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

typedef struct // size = 140
{
	char szAnimLib[64];
	char szAnimName[64];
	float fDelta;
	bool bLoop;
	bool bLockX;
	bool bLockY;
	bool bFreeze;
	int iTime;
} ACTOR_ANIMATION;

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
	bool m_bAnimLoopedOrFreezed;
	ACTOR_ANIMATION m_Animation;

public:
	CActor(unsigned short usActorID, int iModelID, VECTOR vecPos, float fAngle);
	~CActor();

	void SetPosition(float fX, float fY, float fZ);
	VECTOR* GetPosition();
	void SetFacingAngle(float fAngle);
	float GetFacingAngle();
	float GetSquaredDistanceFrom2DPoint(float fX, float fY);
	void SetHealth(float fHealth);
	float GetHealth();
	void ApplyAnimation(char* szAnimLib, char* szAnimName, float fDelta, bool bLoop, bool bLockX, bool bLockY, bool bFreeze, int iTime);
	void SendAnimation(unsigned short usPlayerID, ACTOR_ANIMATION* pAnim);
	void ClearAnimations();
	void SetInvulnerable(bool bInvulnerable);
	bool IsInvulnerable();
	void SetVirtualWorld(int iVirtualWorld);
	int GetVirtualWorld();

	inline int GetModel() { return m_iModelID; };
	inline bool IsAnimationOnLoop() { return m_bAnimLoopedOrFreezed; };
	inline ACTOR_ANIMATION* GetLoopedAnimationData() { return &m_Animation; };
};

#endif // SAMPSRV_ACTOR_H
