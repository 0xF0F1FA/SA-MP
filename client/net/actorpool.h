
#pragma once

class CActorPool
{
private:
	int m_iLastActorID;
	CActor* m_pActor[MAX_ACTORS];
	bool m_bSlotState[MAX_ACTORS];
	//DWORD m_dwPed[MAX_ACTORS];

public:
	CActorPool();
	~CActorPool();

	CActor* GetAt(int iActorID)
	{
		if (iActorID >= 0 && iActorID < MAX_ACTORS)
		{
			return  m_pActor[iActorID];
		}
		return NULL;
	}

	void UpdateLastActorID();
	bool New(ACTOR_TRANSMIT* pTransmit);
	bool Delete(unsigned short usActorID);
	unsigned short FindIDFromGtaPtr(DWORD dwTargetPed);
};
