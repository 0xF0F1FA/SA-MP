//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
//----------------------------------------------------------

#pragma once

#include "game.h"
#include "entity.h"
#include "scripting.h"

//----------------------------------------------------

class CObject : public CEntity
{
public:

	MATRIX4X4				m_matTarget;
	BYTE					m_byteMoving;
	float					m_fMoveSpeed;
	VECTOR					m_vecRot;
	BYTE m_byteRotate;
	QUATERNION m_quatTarget;
	DWORD m_dwStartTime;
	VECTOR m_vecAngleYXZ;
	VECTOR m_vecEulerAngle;
	QUATERNION m_quatCurr;
	float m_fMoveDistance;
	DWORD m_dwLastMoveTime;
	QUATERNION m_quatRot;

	CObject(int iModel, float fPosX, float fPosY, float fPosZ, VECTOR vecRot, float fDrawDistance=0.0f);
	~CObject();
	
	void Process(float fElapsedTime);
	float DistanceRemaining(MATRIX4X4* matPos);
	//void ToggleCollision(BYTE byteToggle) { ScriptCommand(&toggle_object_collision, m_dwGTAId, byteToggle); };
	
	void MoveTo(float X, float Y, float Z, float speed, float RotX, float RotY, float RotZ);
	void Stop();

	void Teleport(MATRIX4X4 mat);

	void InstantRotate(VECTOR* vecRot);

	float GetDistance(float e, float r);
	void SetTargetRot(float X, float Y, float Z);

	void SetScale(float fScale);
};