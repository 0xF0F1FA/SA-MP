//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
//----------------------------------------------------------


#include "../main.h"
#include "rwstuff.h"
#include "util.h"

bool IsObjectModel(MODEL_INFO_TYPE* t)
{
	if (t->vtable == 0x85BBF0 || t->vtable == 0x85BC30 || t->vtable == 0x85BC70 ||
		t->vtable == 0x85BCB0 || t->vtable == 0x85BCF0)
	{
		return true;
	}
	return false;
}

CObject::CObject(int iModel, float fPosX, float fPosY, float fPosZ, VECTOR vecRot, float fDrawDistance)
{
	DWORD dwRetID	= 0;
	m_pEntity		= 0;
	m_dwGTAId		= 0;

	MODEL_INFO_TYPE* pModelInfo = GetModelInfo(iModel);

	if (fDrawDistance != 0.0f && pModelInfo && IsObjectModel(pModelInfo))
		pModelInfo->fDrawDistance = fDrawDistance;

	ScriptCommand(&create_object, iModel, fPosX, fPosY, fPosZ, &dwRetID);
	ScriptCommand(&put_object_at, dwRetID, fPosX, fPosY, fPosZ);
	
	m_pEntity	=	GamePool_Object_GetAt(dwRetID);
	m_dwGTAId	=	dwRetID;
	m_byteMoving = 0;
	m_fMoveSpeed = 0.0;

	InstantRotate(&vecRot);
}

CObject::~CObject()
{
	m_pEntity	=	GamePool_Object_GetAt(m_dwGTAId);
	if (m_pEntity)
	{
		ScriptCommand(&destroy_object, m_dwGTAId);
	}
}



void CObject::Process(float fElapsedTime)
{
	if (m_byteMoving & 1)
	{
		// Calculate new position based on elapsed time (interpolate)
		// distance = speed * time
		// time = fElapsedTime
		// speed = m_fMoveSpeed
		VECTOR vecSpeed;
		vecSpeed.X = 0.0f;
		vecSpeed.Y = 0.0f;
		vecSpeed.Z = 0.0f;
		MATRIX4X4 matPos;
		GetMatrix(&matPos);
		float distance = fElapsedTime * m_fMoveSpeed;
		float remaining = DistanceRemaining(&matPos);
		DWORD time = RakNet::GetTime();

		VECTOR vecPos;
		vecPos.X = matPos.pos.X;
		vecPos.Y = matPos.pos.Y;
		vecPos.Z = matPos.pos.Z;

		MATRIX4X4 matTemp;
		if (distance >= remaining)
		{
			SetMoveSpeedVector(vecSpeed);
			SetTurnSpeedVector(vecSpeed);

			// Force the final location so we don't overshoot slightly
			matPos.pos.X = m_matTarget.pos.X;
			matPos.pos.Y = m_matTarget.pos.Y;
			matPos.pos.Z = m_matTarget.pos.Z;

			if (m_byteRotate)
				QuatRotate(&m_quatTarget, &matPos);

			memcpy(&matTemp, &matPos, sizeof(MATRIX4X4));
			Teleport(matTemp);

			Stop(); // Stop it moving
		}
		else if (fElapsedTime > 0.0)
		{
			remaining / distance;

			//float r = 1.0f


			vecSpeed.X = (matPos.pos.X - vecPos.X);
			vecSpeed.Y = (matPos.pos.Y - vecPos.Y);
			vecSpeed.Z = (matPos.pos.Z - vecPos.Z);
			
			vecSpeed.X *= 0.02f;
			vecSpeed.Y *= 0.02f;
			vecSpeed.Z *= 0.02f;

			SetMoveSpeedVector(vecSpeed);
			MoveStep();

			MATRIX4X4 mat;
			if (m_byteRotate)
			{
				MatrixToEulerAngles();

				vecSpeed.X = 0.0f;
				vecSpeed.Y = 0.0f;
				vecSpeed.Z = GetDistance(remaining, distance) * 0.01f;

				if (vecSpeed.Z > 0.001f)
					vecSpeed.Z = 0.001f;
				else if (vecSpeed.Z < -0.001f)
					vecSpeed.Z = -0.001f;
				
				SetTurnSpeedVector(vecSpeed);
				GetMatrix(&mat);
				QuatSlerp(&m_quatRot, &m_quatCurr, &m_quatTarget, );
				QuatNormalize(&m_quatRot);
				QuatRotate(&m_quatRot, &mat);
			}
			else
			{
				GetMatrix(&mat);
			}

			memcpy(&matTemp, &mat, sizeof(MATRIX4X4));
			Teleport(mat);
		}




		
		
		
		
		DWORD dt = (time - m_dwStartTime);
		float v26 = (dt / 1000.0f) * m_fMoveSpeed;
		m_dwLastMoveTime = time;
		

		
	}
}

float CObject::DistanceRemaining(MATRIX4X4* matPos)
{
	float	fSX,fSY,fSZ;
	fSX = (matPos->pos.X - m_matTarget.pos.X) * (matPos->pos.X - m_matTarget.pos.X);
	fSY = (matPos->pos.Y - m_matTarget.pos.Y) * (matPos->pos.Y - m_matTarget.pos.Y);
	fSZ = (matPos->pos.Z - m_matTarget.pos.Z) * (matPos->pos.Z - m_matTarget.pos.Z);
	return sqrtf(fSX + fSY + fSZ);
}

void CObject::MoveTo(float X, float Y, float Z, float speed, float RotX, float RotY, float RotZ)
{
	MATRIX4X4 matObject;
	GetMatrix(&matObject);

	if (m_byteMoving & 1)
	{
		Stop();
		
		matObject.pos.X = m_matTarget.pos.X;
		matObject.pos.Y = m_matTarget.pos.Y;
		matObject.pos.Z = m_matTarget.pos.Z;
		
		if (m_byteRotate)
			QuatRotate(&m_quatTarget, &matObject);
		
		MATRIX4X4 mat;
		memcpy(&mat, &matObject, sizeof(MATRIX4X4));
		Teleport(mat);
	}

	m_dwStartTime = RakNet::GetTime();
	m_matTarget.pos.X = X;
	m_matTarget.pos.Y = Y;
	m_matTarget.pos.Z = Z;
	m_fMoveSpeed = speed;
	m_byteMoving |= 1;

	if (RotX <= -999.0 || RotY <= -999.0 || RotZ <= -999.0)
	{
		m_byteRotate = 0;
	}
	else
	{
		m_byteRotate = 1;

		float fX, fY, fZ;
	
		MatrixToEulerAngles(&fX, &fY, &fZ);
		m_vecAngleYXZ.X = NormalizeAngle(RotY);
		m_vecAngleYXZ.Y = NormalizeAngle(RotX);
		m_vecAngleYXZ.Z = NormalizeAngle(RotZ);
		m_vecEulerAngle.X = GetDistance(fX, m_vecAngleYXZ.X);
		m_vecEulerAngle.Y = GetDistance(fY, m_vecAngleYXZ.Y);
		m_vecEulerAngle.Z = GetDistance(fZ, m_vecAngleYXZ.Z);

		SetTargetRot(RotX, RotY, RotZ);
		
		MATRIX4X4 mat;
		GetMatrix(&mat);
		RtQuatConvertFromMatrix(&mat, &m_quatCurr);
		RtQuatConvertFromMatrix(&m_matTarget, &m_quatTarget);
		QuatNormalize(&m_quatCurr);
		QuatNormalize(&m_quatTarget);
	}

	m_fMoveDistance = GetDistanceFromPoint(m_matTarget.pos.X, m_matTarget.pos.Y, m_matTarget.pos.Z);

	if (pNetGame)
	{
		CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
		if (pPlayerPool)
		{
			pPlayerPool->GetLocalPlayer()->UpdateSurfing();
		}
	}

	SetNotCollidable();
}

void CObject::Stop()
{
	VECTOR vecMoveSpeed;
	vecMoveSpeed.X = 0.0f;
	vecMoveSpeed.Y = 0.0f;
	vecMoveSpeed.Z = 0.0f;
	SetMoveSpeedVector(vecMoveSpeed);

	VECTOR vecTurnSpeed;
	vecTurnSpeed.X = 0.0f;
	vecTurnSpeed.Y = 0.0f;
	vecTurnSpeed.Z = 0.0f;
	SetTurnSpeedVector(vecTurnSpeed);

	m_byteMoving &= ~1;
}

void CObject::InstantRotate(VECTOR* vecRot)
{
	if(!m_pEntity) return;
	if(!GamePool_Object_GetAt(m_dwGTAId)) return;

	ScriptCommand(&set_object_rotation, m_dwGTAId, vecRot->X, vecRot->Y, vecRot->Z);

	m_vecRot.X = vecRot->X;
	m_vecRot.Y = vecRot->Y;
	m_vecRot.Z = vecRot->Z;
}

float CObject::GetDistance(float e, float r)
{
	return NormalizeAngle(NormalizeAngle(r) - e);
}

void CObject::SetTargetRot(float X, float Y, float Z)
{
	float rx, ry, rz, cx, sx, cy, sy, cz, sz;
	float t1, t2, t3;

	m_vecRot.X = X;
	m_vecRot.Y = Y;
	m_vecRot.Z = Z;

	rx = X * (PI / 180.0f);
	ry = Y * (PI / 180.0f);
	rz = Z * (PI / 180.0f);
	cx = (float)cos(rx);
	sx = (float)sin(rx);
	cy = (float)cos(ry);
	sy = (float)sin(ry);
	cz = (float)cos(rz);
	sz = (float)sin(rz);
	t1 = sx * sz;
	t2 = sx * cz;
	t3 = cx * cy;

	m_matTarget.right.X = cz * cy - t1 * sy;
	m_matTarget.right.Y = t2 * sy + sz * cy;
	m_matTarget.right.Z = -(sy * cx);
	m_matTarget.up.X = -(sz * cx);
	m_matTarget.up.Y = cz * cx;
	m_matTarget.up.Z = sx;
	m_matTarget.at.X = t1 * cy + cz * sy;
	m_matTarget.at.Y = sz * sy - t2 * cy;
	m_matTarget.at.Z = t3;
}


void CObject::SetScale(float fScale)
{
	ScriptCommand(&scale_object, m_dwGTAId, fScale);
}

