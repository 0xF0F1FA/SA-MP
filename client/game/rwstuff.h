
#pragma once

typedef float RwReal;

typedef struct RwV3d RwV3d;
struct RwV3d
{
	RwReal x;
	RwReal y;
	RwReal z;
};

typedef struct RtQuat RtQuat;
struct RtQuat
{
	RwV3d imag;
	RwReal real;
};

void RtQuatConvertFromMatrix(MATRIX4X4* mat, QUATERNION* quat);
