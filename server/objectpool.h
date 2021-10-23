/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: vehiclepool.h,v 1.8 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_OBJECTPOOL_H
#define SAMPSRV_OBJECTPOOL_H

//----------------------------------------------------

class CObjectPool
{
private:

	bool m_bObjectSlotState[MAX_OBJECTS];
	CObject *m_pObjects[MAX_OBJECTS];
	
	bool m_bPlayerObjectSlotState[MAX_PLAYERS][MAX_OBJECTS];
	bool m_bPlayersObject[MAX_OBJECTS];
	CObject *m_pPlayerObjects[MAX_PLAYERS][MAX_OBJECTS];
public:
	CObjectPool();
	~CObjectPool();

	WORD New(int iModel, VECTOR * vecPos, VECTOR * vecRot, float fDrawDist);
	WORD New(int iPlayer, int iModel, VECTOR* vecPos, VECTOR* vecRot, float fDrawDist);
	
	bool Delete(WORD wObjectID);	
	bool DeleteForPlayer(BYTE bytePlayerID, BYTE byteObjectID);
	
	void Process(float fElapsedTime);

	// Retrieve an object by id
	CObject* GetAt(WORD wObjectID)
	{
		if(wObjectID >= MAX_OBJECTS) { return NULL; }
		return m_pObjects[wObjectID];
	};
	
	CObject* GetAtIndividual(int iPlayerID, int iObjectID)
	{
		return ((iPlayerID >= 0 && iPlayerID < MAX_PLAYERS) &&
			(iObjectID >= 0 && iObjectID < MAX_OBJECTS)) ? m_pPlayerObjects[iPlayerID][iObjectID] : nullptr;
	};

	// Find out if the slot is inuse.
	bool GetSlotState(WORD wObjectID)
	{
		if(wObjectID >= MAX_OBJECTS) { return false; }
		return m_bObjectSlotState[wObjectID];
	};
	
	// Find out if the slot is inuse by an individual (and not global).
	bool GetPlayerSlotState(int iPlayerID, int iObjectID)
	{
		return
			((iPlayerID >= 0 && iPlayerID < MAX_PLAYERS) &&
			(iObjectID >= 0 && iObjectID < MAX_OBJECTS) &&
			!m_bPlayersObject[iObjectID]) ? m_bPlayerObjectSlotState[iPlayerID][iObjectID] : false;
	};

	void InitForPlayer(BYTE bytePlayerID);
};

//----------------------------------------------------

#endif

