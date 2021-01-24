
#ifndef SAMPSRV_ACTORPOOL_H
#define SAMPSRV_ACTORPOOL_H

class CActorPool
{
private:
	bool m_bSlotState[MAX_ACTORS];
	int m_iLastActorID;

public:
	CActorPool();
	~CActorPool();
	bool GetSlotState(int iActorID);
	inline int GetLastActorID() { return m_iLastActorID; };

};

#endif // SAMPSRV_ACTORPOOL_H
