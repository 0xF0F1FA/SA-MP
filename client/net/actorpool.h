
#pragma once

class CActorPool
{
private:
	int m_iLastActorID;
	CActor* m_pActor[MAX_ACTORS];
	bool m_bSlotState[MAX_ACTORS];

public:
	CActorPool();
	~CActorPool();
	void UpdateLastActorID();
	bool New(ACTOR_TRANSMIT* pTransmit);
	bool Delete(unsigned short usActorID);
};
