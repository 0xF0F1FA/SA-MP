
#ifndef SAMPSRV_ACTOR_H
#define SAMPSRV_ACTOR_H

typedef struct _ANIMATION_INFO // size = 142
{
	char szAnimLib[65];
	char szAnimName[65];
	float fDelta;
	bool bLoop;
	bool bLockX;
	bool bLockY;
	bool bFreeze;
	int iTime;
} ANIMATION_INFO;

typedef struct _ACTOR_SPAWN_INFO
{
	BYTE byteTeam;
	int iSkin;
	int iBaseSkin;
	VECTOR vecPos;
	float fRotation;
} ACTOR_SPAWN_INFO;

class CActor
{
private:
	ACTOR_SPAWN_INFO		m_SpawnInfo;

public:
	VECTOR					m_vecPos;
	float					m_fRotation;
	bool					bHasAnimation;
	ANIMATION_INFO			m_Animation;
	float					m_fHealth;
	BYTE					m_byteInvulnerable;
	WORD					m_wActorID;

	CActor(int iModelID, VECTOR* vecPos, float fAngle);
	~CActor();

	void UpdatePosition(float x, float y, float z);
	void UpdateRotation(float fRotation);
	float GetSquaredDistanceFrom2DPoint(float fX, float fY);
	void SetHealth(float fHealth);
	void SendAnimation(WORD wPlayerID, ANIMATION_INFO* pAnim);
	void ApplyAnimation(char* szAnimLib, char* szAnimName, float fDelta, bool bLoop, bool bLockX, bool bLockY, bool bFreeze, int iTime);
	void ClearAnimations();

	ACTOR_SPAWN_INFO* GetSpawnInfo() { return &m_SpawnInfo; };

	void SetID(WORD ActorID) { m_wActorID = ActorID; }
};

#endif // SAMPSRV_ACTOR_H
