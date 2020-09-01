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
	ScriptCommand(&set_camera_behind_player);
	ScriptCommand(&restore_camera_jumpcut);
	//ScriptCommand(&restore_camera);
}

//-----------------------------------------------------------

void CCamera::SetPosition(float fX, float fY, float fZ, float fRotationX, float fRotationY, float fRotationZ)
{
	ScriptCommand(&set_camera_position,fX,fY,fZ,fRotationX,fRotationY,fRotationZ);
}

//-----------------------------------------------------------

void CCamera::LookAtPoint(float fX, float fY, float fZ, int iType)
{
	ScriptCommand(&point_camera,fX,fY,fZ,iType);
}

//-----------------------------------------------------------

void CCamera::Restore()
{
	ScriptCommand(&restore_camera_jumpcut);
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
	((void(__thiscall*)(CAMERA_TYPE*, VECTOR*, VECTOR*, FLOAT, BYTE))0x50D160)(m_pCamera, from, to, time, mode);
}

void CCamera::InterpolateCameraLookAt(VECTOR* from, VECTOR* to, FLOAT time, BYTE mode)
{
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
