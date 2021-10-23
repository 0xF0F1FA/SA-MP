/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

    Version: $Id: vehiclepool.h,v 1.8 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_VEHICLEPOOL_H
#define SAMPSRV_VEHICLEPOOL_H

#define INVALID_VEHICLE (0xFFFF)

//----------------------------------------------------

class CVehiclePool
{
private:

	bool m_bVehicleSlotState[MAX_VEHICLES];
	CVehicle* m_pVehicles[MAX_VEHICLES];
	//BYTE m_byteVirtualWorld[MAX_VEHICLES];
	int m_iPoolSize;
	unsigned short m_usVehicleModelsUsed[212];
public:
	CVehiclePool();
	~CVehiclePool();

	VEHICLEID New(int iVehicleType, VECTOR * vecPos, float fRotation, int iColor1, int iColor2, int iRespawnDelay, bool bAddSiren);

	bool Delete(VEHICLEID VehicleID);	
		
	void UpdatePoolSize();

	// Retrieve a vehicle by id
	CVehicle* GetAt(int iVehicleID)
	{
		return (iVehicleID >= 0 && iVehicleID < MAX_VEHICLES) ? m_pVehicles[iVehicleID] : nullptr;
	};

	// Find out if the slot is inuse.
	bool GetSlotState(int iVehicleID)
	{
		return (iVehicleID >= 0 && iVehicleID < MAX_VEHICLES) ? m_bVehicleSlotState[iVehicleID] : false;
	};

	void InitForPlayer(BYTE bytePlayerID);
	void InitVehicleForPlayer(VEHICLEID VehicleID, WORD wPlayerID);
	void DeleteVehicleForPlayer(VEHICLEID VehicleID, WORD wPlayerID);
	void Process(float fElapsedTime);

	/*void SetVehicleVirtualWorld(VEHICLEID VehicleID, BYTE byteVirtualWorld);

	BYTE GetVehicleVirtualWorld(VEHICLEID VehicleID) {
		if (VehicleID >= MAX_VEHICLES) { return 0; }
		return m_byteVirtualWorld[VehicleID];
	};*/

	int GetPoolSize() { return m_iPoolSize; }

	unsigned short GetVehicleModelsUsed(int iId) const
	{
		return m_usVehicleModelsUsed[iId];
	}

	unsigned int GetNumberOfModels();
};

//----------------------------------------------------


#endif

