
#ifndef SAMPSRV_ACTORPOOL_H
#define SAMPSRV_ACTORPOOL_H

class CActorPool
{
private:
	CActor* m_pActors[MAX_ACTORS];
	bool m_bSlotState[MAX_ACTORS];
	int m_iLastActorID;

public:
	CActorPool();
	~CActorPool();

	CActor* GetAt(int iActorID);
	bool GetSlotState(int iActorID);

	void UpdateLastActorID();
	unsigned short New(int iModelID, VECTOR vecPos, float fAngle);
	bool Destroy(int iActorID);
	inline int GetLastActorID() { return m_iLastActorID; };

};

#endif // SAMPSRV_ACTORPOOL_H
