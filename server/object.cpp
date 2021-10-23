/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: vehicle.cpp,v 1.5 2006/05/07 15:35:32 kyeman Exp $

*/

#include "main.h"

//----------------------------------------------------------
// Global

CObject::CObject(int iModel, VECTOR * vecPos, VECTOR * vecRot, float fDrawDist)
{
	// Set the initial pos
	memset(&m_matWorld,0,sizeof(MATRIX4X4));

	m_vecRot.X = vecRot->X;
	m_vecRot.Y = vecRot->Y;
	m_vecRot.Z = vecRot->Z;

	m_matWorld.pos.X = vecPos->X;
	m_matWorld.pos.Y = vecPos->Y;
	m_matWorld.pos.Z = vecPos->Z;

	m_fDrawDistance = fDrawDist;
	m_byteMoving = 0;
	m_iModel = iModel;
	m_bIsActive = true;
	m_byteDefaultCameraCol = (pNetGame && pNetGame->m_bDefaultObjectCameraCol);

	m_matWorld.up.X = vecRot->X;
	m_matWorld.up.Y = vecRot->Y;
	m_matWorld.up.Z = vecRot->Z;

	

	
	
	
}

void CObject::SetRotation(VECTOR* vecRot)
{
	m_vecRot.X = vecRot->X;
	m_vecRot.Y = vecRot->Y;
	m_vecRot.Z = vecRot->Z;
}

//----------------------------------------------------
// This is the method used for spawning players that
// already exist in the world when the client connects.

void CObject::SpawnForPlayer(BYTE byteForPlayerID)
{
	RakNet::BitStream bsObjectSpawn;

	bsObjectSpawn.Write(m_wObjectID);
	bsObjectSpawn.Write(m_iModel);
	bsObjectSpawn.Write(m_matWorld.pos.X);
	bsObjectSpawn.Write(m_matWorld.pos.Y);
	bsObjectSpawn.Write(m_matWorld.pos.Z);
	bsObjectSpawn.Write(m_matWorld.up.X);
	bsObjectSpawn.Write(m_matWorld.up.Y);
	bsObjectSpawn.Write(m_matWorld.up.Z);
	bsObjectSpawn.Write(m_fDrawDistance);
	
	//printf("player: %d, id: %d, model: %d, others: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", byteForPlayerID, m_byteObjectID, m_iModel, m_matWorld.pos.X, m_matWorld.pos.Y, m_matWorld.pos.Z, m_matWorld.up.X, m_matWorld.up.Y, m_matWorld.up.Z);

	pNetGame->GetRakServer()->RPC(RPC_ScrCreateObject,&bsObjectSpawn,HIGH_PRIORITY,RELIABLE_ORDERED,
		0,pNetGame->GetRakServer()->GetPlayerIDFromIndex(byteForPlayerID),false,false);
}

int CObject::Process(float fElapsedTime)
{
	int ret = 0;
	if (m_byteMoving & 1)
	{
		// Calculate new position based on elapsed time (interpolate)
		// distance = speed * time
		// time = fElapsedTime
		// speed = m_fMoveSpeed
		float distance = fElapsedTime * m_fMoveSpeed;
		float remaining = DistanceRemaining();
		if (distance >= remaining)
		{
			ret |= 1; // The object will reach it's destination
			m_byteMoving &= ~1; // Stop it moving
			m_matWorld.pos.X = m_matTarget.pos.X;
			m_matWorld.pos.Y = m_matTarget.pos.Y;
			m_matWorld.pos.Z = m_matTarget.pos.Z;
			// Force the final location so we don't overshoot slightly
		}
		else
		{
			// Else interpolate the new position between the current and final positions
			remaining /= distance; // Calculate ratio
			m_matWorld.pos.X += (m_matTarget.pos.X - m_matWorld.pos.X) / remaining;
			m_matWorld.pos.Y += (m_matTarget.pos.Y - m_matWorld.pos.Y) / remaining;
			m_matWorld.pos.Z += (m_matTarget.pos.Z - m_matWorld.pos.Z) / remaining;
		}
	}
	return ret;
}

float CObject::DistanceRemaining()
{
	float	fSX,fSY,fSZ;
	fSX = (m_matWorld.pos.X - m_matTarget.pos.X) * (m_matWorld.pos.X - m_matTarget.pos.X);
	fSY = (m_matWorld.pos.Y - m_matTarget.pos.Y) * (m_matWorld.pos.Y - m_matTarget.pos.Y);
	fSZ = (m_matWorld.pos.Z - m_matTarget.pos.Z) * (m_matWorld.pos.Z - m_matTarget.pos.Z);
	return (float)sqrt(fSX + fSY + fSZ);
}

float CObject::MoveTo(float X, float Y, float Z, float speed, float RotX, float RotY, float RotZ)
{
	m_matTarget.pos.X = X;
	m_matTarget.pos.Y = Y;
	m_matTarget.pos.Z = Z;
	m_fMoveSpeed = speed;
	if (RotX > -999.0 && RotY > -999.0 && RotZ > -999.0)
	{
		m_vecRot.X = RotX;
		m_vecRot.Y = RotY;
		m_vecRot.Z = RotZ;
	}

	m_byteMoving |= 1;
	
	float	fSX,fSY,fSZ;
	fSX = (X - m_matWorld.pos.X) * (X - m_matWorld.pos.X);
	fSY = (Y - m_matWorld.pos.Y) * (Y - m_matWorld.pos.Y);
	fSZ = (Z - m_matWorld.pos.Z) * (Z - m_matWorld.pos.Z);

	return (float)(sqrt(fSX + fSY + fSZ) / speed);
}