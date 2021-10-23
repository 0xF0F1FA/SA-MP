/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

file:
vehicle.h
desc:
Vehicle handling header file.

Version: $Id: vehicle.h,v 1.3 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_OBJECT_H
#define SAMPSRV_OBJECT_H

//----------------------------------------------------

class CObject
{
public:

	WORD					m_wObjectID;
	int						m_iModel;
	bool					m_bIsActive;
	MATRIX4X4				m_matWorld;
	VECTOR					m_vecRot;
	BYTE m_byteDefaultCameraCol;
	MATRIX4X4				m_matTarget;
	BYTE					m_byteMoving;
	float					m_fMoveSpeed;
	float					m_fRotation;
	float					m_fDrawDistance;

	CObject(int iModel, VECTOR * vecPos, VECTOR * vecRot, float fDrawDist);
	~CObject(){};

	bool IsActive() { return m_bIsActive; }

	void SetID(WORD wObjectID) { m_wObjectID = wObjectID; };

	void SetRotation(VECTOR* vecRot);
	VECTOR* GetRotation() { return &m_vecRot; };

	void SpawnForPlayer(BYTE byteForPlayerID);
	int Process(float fElapsedTime);
	void Stop() { m_byteMoving &= ~1; };
	float DistanceRemaining();
	
	float MoveTo(float X, float Y, float Z, float speed, float RotX, float RotY, float RotZ);
};

#endif