
#include "../main.h"
#include "rwstuff.h"

void RtQuatConvertFromMatrix(MATRIX4X4* mat, QUATERNION* quat)
{
	RtQuat quatDest;

	DWORD dwFunc = (iGtaVersion != GTASA_VERSION_USA10) ? 0x7EB600 : 0x7EB5C0;
	DWORD dwQuat = (DWORD)&quatDest;

	_asm push mat
	_asm push dwQuat
	_asm mov eax, dwFunc
	_asm call eax
	_asm pop edx
	_asm pop edx

	quat->W = quatDest.real;
	quat->X = quatDest.imag.x;
	quat->Y = quatDest.imag.y;
	quat->Z = quatDest.imag.z;
}