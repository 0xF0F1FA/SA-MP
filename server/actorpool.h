
#ifndef SAMPSRV_ACTORPOOL_H
#define SAMPSRV_ACTORPOOL_H

class CActorPool
{
private:
	CActor* m_pActors[MAX_ACTORS];
	bool m_bSlotState[MAX_ACTORS];
	//int m_iVirtualWorld[MAX_ACTORS]; // moved to CActor
	int m_iLastActorID;

public:
	CActorPool();
	~CActorPool();

	CActor* GetAt(int iActorID);
	bool GetSlotState(int iActorID);

	void UpdateLastActorID();
	unsigned short New(int iModelID, VECTOR vecPos, float fAngle);
	bool Destroy(int iActorID);
	//void SetActorVirtualWorld(unsigned short ActorID, int iVirtualWorld); // moved to CActor
	//int GetActorVirtualWorld(unsigned short ActorID); // moved to CActor
	void StreamActorInForPlayer(unsigned short ActorID, unsigned short PlayerID);
	void StreamActorOutForPlayer(unsigned short ActorID, unsigned short PlayerID);

	inline int GetLastActorID() { return m_iLastActorID; };

};

#endif // SAMPSRV_ACTORPOOL_H
