/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

*/

#pragma once

class CObjectPool
{
private:
	int			m_iPoolSize;
	bool		m_bObjectSlotState[MAX_OBJECTS];
	CObject		*m_pObjects[MAX_OBJECTS];


public:
	CObjectPool();
	~CObjectPool();

	bool New(byte byteObjectID, int iModel, VECTOR vecPos, VECTOR vecRot, float fDrawDist=0.0f);
	bool Delete(WORD wObjectID);

	void UpdatePoolSize();

	// Find out if the slot is inuse.
	bool GetSlotState(WORD wObjectID) {
		if(wObjectID >= MAX_OBJECTS) { return false; }
		return m_bObjectSlotState[wObjectID];
	};

	// Retrieve a vehicle
	CObject* GetAt(WORD wObjectID) {
		if(wObjectID>= MAX_OBJECTS || !m_bObjectSlotState[wObjectID]) { return NULL; }
		return m_pObjects[wObjectID];
	};

	int FindIDFromGtaPtr(ENTITY_TYPE * pGtaObject);
	
	void Process();

	int GetCount();
};