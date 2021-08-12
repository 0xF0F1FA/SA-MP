
#include "main.h"

CPlayerVars::CPlayerVars()
{
	for (int VarIndex=0; VarIndex != MAX_PLAYER_VARS; VarIndex++)
	{
		memset(&PlayerVariable[VarIndex], 0, sizeof(PlayerVariable_s));
		m_bVarSlotState[VarIndex] = false;
	}
	m_iPoolSize = 0;
}

CPlayerVars::~CPlayerVars()
{
	for (int VarIndex=0; VarIndex != MAX_PLAYER_VARS; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex])
		{
			if (PlayerVariable[VarIndex].VarStr)
				free(PlayerVariable[VarIndex].VarStr);
		}
		m_bVarSlotState[VarIndex] = false;
	}
}

void CPlayerVars::UpdatePoolSize()
{
	int NewSize = -1;
	for (int VarIndex = 0; VarIndex != MAX_PLAYER_VARS; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex] == true)
			NewSize = VarIndex;
	}
	m_iPoolSize = NewSize + 1;
}

int CPlayerVars::FindVariable(char* pVarName)
{
	Util_strupr(pVarName);

	if (m_iPoolSize <= 0)
		return -1;

	for (int VarIndex = 0; VarIndex < m_iPoolSize; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex] == true &&
			PlayerVariable[VarIndex].VarName[0] == pVarName[0] &&
			strcmp(pVarName, PlayerVariable[VarIndex].VarName) == 0)
		{
			return VarIndex;
		}
	}
	return -1;
}

int CPlayerVars::AddVariable(char* pVarName)
{
	int VarIndex;
	for (VarIndex=0; VarIndex != MAX_PLAYER_VARS; VarIndex++)
	{
		if (!m_bVarSlotState[VarIndex]) break;
	}

	if (VarIndex == MAX_PLAYER_VARS) return -1;

	if (strlen(pVarName) <= MAX_PLAYER_VARS_NAME)
	{
		Util_strupr(pVarName);

		PlayerVariable_s* PVar = &PlayerVariable[VarIndex];
		strcpy(PVar->VarName, pVarName);
		PVar->VarReadOnly = false;
		PVar->VarType = PLAYER_VARTYPE_NONE;

		m_bVarSlotState[VarIndex] = true;

		UpdatePoolSize();

		return VarIndex;
	}
	return -1;
}

bool CPlayerVars::SetIntVariable(char* pVarName, int iInt, bool bReadOnly)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex < 0)
	{
		VarIndex = AddVariable(pVarName);
		if (VarIndex < 0)
		{
			return false;
		}
	}

	PlayerVariable_s* PVar = &PlayerVariable[VarIndex];

	if (bReadOnly)
	{
		PVar->VarReadOnly = true;
	}
	else if (PVar->VarReadOnly)
	{
		return false;
	}

	PVar->VarType = PLAYER_VARTYPE_INT;
	PVar->VarInt = iInt;

	return true;
}

bool CPlayerVars::SetStringVariable(char* pVarName, char* pString, bool bReadOnly)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex < 0)
	{
		VarIndex = AddVariable(pVarName);
		if (VarIndex < 0)
		{
			return false;
		}
	}

	PlayerVariable_s* PVar = &PlayerVariable[VarIndex];

	if (bReadOnly)
	{
		PVar->VarReadOnly = true;
	}
	else if (PVar->VarReadOnly)
	{
		return false;
	}

	PVar->VarType = PLAYER_VARTYPE_STRING;

	if (PVar->VarStr)
		free(PVar->VarStr);
	PVar->VarStr = (char*)calloc(1,strlen(pString)+1);
	strcpy(PVar->VarStr, pString);

	return true;
}

bool CPlayerVars::SetFloatVariable(char* pVarName, float fFloat, bool bReadOnly)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex < 0)
	{
		VarIndex = AddVariable(pVarName);
		if (VarIndex < 0)
		{
			return false;
		}
	}

	PlayerVariable_s* PVar = &PlayerVariable[VarIndex];

	if (bReadOnly)
	{
		PVar->VarReadOnly = true;
	}
	else if (PVar->VarReadOnly)
	{
		return false;
	}

	PVar->VarType = PLAYER_VARTYPE_FLOAT;
	PVar->VarFlt = fFloat;

	return true;
}

int CPlayerVars::GetIntVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		PlayerVariable_s* PVar = &PlayerVariable[VarIndex];
		if (PVar->VarType == PLAYER_VARTYPE_INT)
		{
			return PVar->VarInt;
		}
	}
	return 0;
}

char* CPlayerVars::GetStringVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		PlayerVariable_s* PVar = &PlayerVariable[VarIndex];
		if (PVar->VarType == PLAYER_VARTYPE_STRING)
		{
			return PVar->VarStr;
		}
	}
	return NULL;
}

float CPlayerVars::GetFloatVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		PlayerVariable_s* PVar = &PlayerVariable[VarIndex];
		if (PVar->VarType == PLAYER_VARTYPE_FLOAT)
		{
			return PVar->VarFlt;
		}
	}
	return 0.0f;
}

bool CPlayerVars::RemoveVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		PlayerVariable_s* PVar = &PlayerVariable[VarIndex];
		if (PVar->VarStr)
			free(PVar->VarStr);
		memset(PVar, 0, sizeof(PlayerVariable_s));
		m_bVarSlotState[VarIndex] = false;

		UpdatePoolSize();

		return true;
	}
	return false;
}

int CPlayerVars::GetVariableType(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if(VarIndex >= 0)
	{
		return PlayerVariable[VarIndex].VarType;
	}
	return PLAYER_VARTYPE_NONE;
}

char* CPlayerVars::GetVariableNameAtIndex(int iIndex)
{
	if (iIndex >= 0 && iIndex < MAX_PLAYER_VARS && m_bVarSlotState[iIndex])
	{
		return PlayerVariable[iIndex].VarName;
	}
	return NULL;
}
