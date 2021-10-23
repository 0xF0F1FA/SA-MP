
#ifndef SAMPSRV_ACTORPOOL_H
#define SAMPSRV_ACTORPOOL_H

class CActorPool
{
private:
	int m_iVirtualWorld[MAX_ACTORS];
	bool m_bSlotState[MAX_ACTORS]; // BOOL
	CActor* m_pActors[MAX_ACTORS];
	int m_iPoolSize;

public:
	CActorPool();
	~CActorPool();

	void UpdatePoolSize();
	WORD New(int iModelID, VECTOR* vecPos, float fAngle);
	bool Delete(WORD wActorID);

	CActor* GetAt(WORD wActorID) {
		if (wActorID >= MAX_ACTORS) { return NULL; }
		return m_pActors[wActorID];
	};

	bool GetSlotState(WORD wActorID) {
		if (wActorID >= MAX_ACTORS) { return false; }
		return m_bSlotState[wActorID];
	};

	int SetActorVirtualWorld(WORD wActorID) {
		if (wActorID >= MAX_ACTORS || !m_bSlotState[wActorID]) { return 0; }
		return m_iVirtualWorld[wActorID];
	};

	int GetActorVirtualWorld(WORD wActorID) {
		if (wActorID >= MAX_ACTORS) { return 0; }
		return m_iVirtualWorld[wActorID];
	};

	void StreamActorInForPlayer(unsigned short ActorID, unsigned short PlayerID);
	void StreamActorOutForPlayer(unsigned short ActorID, unsigned short PlayerID);

	int GetPoolSize() { return m_iPoolSize; };
};

#endif // SAMPSRV_ACTORPOOL_H
