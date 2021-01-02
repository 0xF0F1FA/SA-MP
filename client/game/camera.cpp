//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: camera.cpp,v 1.4 2006/03/20 17:44:20 kyeman Exp $
//
//----------------------------------------------------------

#include "game.h"
#include "util.h"

//-----------------------------------------------------------

void CCamera::SetBehindPlayer()
{
	m_pCamera->bLockPosition = 0;
	m_pEntity = NULL;

	Reset();

	ScriptCommand(&set_camera_behind_player);
	ScriptCommand(&restore_camera_jumpcut);
	//ScriptCommand(&restore_camera);
}

//-----------------------------------------------------------

void CCamera::SetPosition(float fX, float fY, float fZ, float fRotationX, float fRotationY, float fRotationZ)
{
	Reset();

	m_pEntity = NULL;

	ScriptCommand(&set_camera_position,fX,fY,fZ,fRotationX,fRotationY,fRotationZ);
}

//-----------------------------------------------------------

void CCamera::LookAtPoint(float fX, float fY, float fZ, int iType)
{
	Reset();

	m_pEntity = NULL;

	ScriptCommand(&point_camera,fX,fY,fZ,iType);
}

//-----------------------------------------------------------

void CCamera::Restore()
{
	m_pEntity = NULL;

	ScriptCommand(&restore_camera_jumpcut);
}

//-----------------------------------------------------------

void CCamera::Reset()
{
	DWORD dwThis = (DWORD)m_pCamera;

	_asm mov ecx, dwThis
	_asm mov eax, 0x50D2D0
	_asm call eax
}

//-----------------------------------------------------------

void CCamera::AttachToEntity(CEntity* pEntity)
{
	MATRIX4X4 mat;

	m_pEntity = pEntity;

	if (pEntity)
	{
		pEntity->GetMatrix(&mat);

		if (mat.pos.X < 20000.0f && mat.pos.X > -20000.0f &&
			mat.pos.Y < 20000.0f && mat.pos.Y > -20000.0f &&
			mat.pos.Z < 100000.0f && mat.pos.Z > -10000.0f)
		{
			InterpolateCameraPos(&mat.pos, &mat.pos, 100.0f, 1);
		}
	}
}

//-----------------------------------------------------------

void CCamera::Update()
{
	MATRIX4X4 mat;

	if (m_pEntity && m_pEntity->m_pEntity)
	{
		m_pEntity->GetMatrix(&mat);

		InterpolateCameraPos(&mat.pos, &mat.pos, 100.0f, 1);
	}
}

//-----------------------------------------------------------

void CCamera::Fade(int iInOut)
{
	ScriptCommand(&fade,500,iInOut);
}

//-----------------------------------------------------------

void CCamera::GetMatrix(PMATRIX4X4 Matrix)
{
	Matrix->right = m_matPos->right;
	Matrix->up = m_matPos->up;
	Matrix->at = m_matPos->at;
	Matrix->pos = m_matPos->pos;
}

void CCamera::InterpolateCameraPos(VECTOR* from, VECTOR* to, FLOAT time, BYTE mode)
{
	Reset();

	m_pCamera->bLockPosition = 1;

	((void(__thiscall*)(CAMERA_TYPE*, VECTOR*, VECTOR*, FLOAT, BYTE))0x50D160)(m_pCamera, from, to, time, mode);
}

void CCamera::InterpolateCameraLookAt(VECTOR* from, VECTOR* to, FLOAT time, BYTE mode)
{
	m_pCamera->bLockTargetPoint = 1;

	((void(__thiscall*)(CAMERA_TYPE*, VECTOR*, VECTOR*, FLOAT, BYTE))0x50D1D0)(m_pCamera, from, to, time, mode);
}

// OPC: 02A3 (toggle_widescreen) - 0x0047F684
void CCamera::ToggleWidescreen(bool bOn)
{
	DWORD dwThis = (DWORD)m_pCamera;
	DWORD dwFunc = bOn ? 0x50C140 : 0x50C150;
	_asm {
		mov ecx, dwThis
		call dwFunc
	}
}
