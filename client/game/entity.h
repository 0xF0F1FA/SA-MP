//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: entity.h,v 1.8 2006/04/16 11:37:59 kyeman Exp $
//
//----------------------------------------------------------

#pragma once

#include "game.h"

//----------------------------------------------------------

class CEntity
{
public:
	CEntity() {};
	virtual ~CEntity() {};
	virtual void  Add();
	virtual void  Remove();

	bool  IsAdded();

	void  GetMatrix(PMATRIX4X4 Matrix);
	void  SetMatrix(MATRIX4X4 Matrix);
	void  GetMoveSpeedVector(PVECTOR Vector);
	void  SetMoveSpeedVector(VECTOR Vector);
	void  GetTurnSpeedVector(PVECTOR Vector);
	void  SetTurnSpeedVector(VECTOR Vector);
	void  GetBoundCentre(PVECTOR Vector);
	UINT  GetModelIndex();
	void  SetModelIndex(UINT uiModel);
	void  TeleportTo(float x, float y, float z);
	float GetDistanceFromLocalPlayerPed();
	float GetDistanceFromCamera();
	float GetDistanceFromPoint(float X, float Y, float Z);
	void  ApplyMoveSpeed();
	bool  IsStationary();
	
	bool  EnforceWorldBoundries(float fPX, float fZX, float fPY, float fNY);
	bool  HasExceededWorldBoundries(float fPX, float fZX, float fPY, float fNY);

	bool UsesCollision();
	void SetCollisionChecking(bool bCheck);
	void SetGravityProcessing(bool bState);

	ENTITY_TYPE *m_pEntity;
	DWORD		m_dwGTAId;
};

//----------------------------------------------------------