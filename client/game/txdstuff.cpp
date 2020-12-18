
#include "../main.h"

int FindTxdSlot(char* szName)
{
	int iResult;

	_asm
	{
		push szName
		mov edx, 0x731850
		call edx
		pop edx
		mov iResult, eax
	}

	return iResult;
}

int AddTxdSlot(char* szName)
{
	int iResult;

	_asm
	{
		push szName
		mov edx, 0x731C80
		call edx
		pop edx
		mov iResult, eax
	}

	return iResult;
}

bool LoadTxd(int iIndex, char* szFileName)
{
	bool bResult = false;

	_asm
	{
		push szFileName
		push iIndex
		mov edx, 0x7320B0
		call edx
		mov bResult, al
		pop edx
		pop edx
	}

	return bResult;
}

void AddRef(int iIndex)
{
	_asm
	{
		push iIndex
		mov edx, 0x731A00
		call edx
		pop edx
	}
}

void PushCurrentTxd()
{
	_asm
	{
		mov edx, 0x7316A0
		call edx
	}
}

void PopCurrentTxd()
{
	_asm
	{
		mov edx, 0x7316B0
		call edx
	}
}

void SetCurrentTxd(int iIndex)
{
	_asm
	{
		push iIndex
		mov edx, 0x7319C0
		call edx
		pop edx
	}
}

void RemoveTxd(int iIndex)
{
	if (iIndex != -1)
	{
		_asm
		{
			push iIndex
			mov edx, 0x731E90
			call edx
			pop edx
		}
	}
}

void RemoveTxdSlot(int iIndex)
{
	if (iIndex != -1)
	{
		_asm
		{
			push iIndex
			mov edx, 0x731CD0
			call edx
			pop edx
		}
	}
}

unsigned int GetNumRefs(int iIndex)
{
	int iResult = -1;

	if (iIndex != -1)
	{
		_asm
		{
			push iIndex
			mov edx, 0x731AA0
			call edx
			mov iResult, eax
			pop edx
		}
	}

	return iResult;
}
