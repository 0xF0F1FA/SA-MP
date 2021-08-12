
#include "main.h"

CServerVars::CServerVars()
{
	for (int VarIndex=0; VarIndex != MAX_SERVER_VARS; VarIndex++)
	{
		memset(&ServerVariables[VarIndex], 0, sizeof(ServerVariable_s));
		m_bVarSlotState[VarIndex] = false;
	}
	m_iPoolSize = 0;
}

CServerVars::~CServerVars()
{
	for (int VarIndex = 0; VarIndex != MAX_SERVER_VARS; VarIndex++)
	{
		if(m_bVarSlotState[VarIndex] == true)
		{
			if (ServerVariables[VarIndex].VarStr)
				free(ServerVariables[VarIndex].VarStr);
		}
		m_bVarSlotState[VarIndex] = false;
	}
}

void CServerVars::UpdatePoolSize()
{
	int NewSize = -1;
	for (int VarIndex = 0; VarIndex != MAX_SERVER_VARS; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex] == true)
			NewSize = VarIndex;
	}
	m_iPoolSize = NewSize + 1;
}

int CServerVars::FindVariable(char* pVarName)
{
	Util_strupr(pVarName);

	if (m_iPoolSize <= 0)
		return -1;

	for (int VarIndex = 0; VarIndex < m_iPoolSize; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex] == true &&
			ServerVariables[VarIndex].VarName[0] == pVarName[0] &&
			strcmp(pVarName, ServerVariables[VarIndex].VarName) == 0)
		{
			return VarIndex;
		}
	}
	return -1;
}

int CServerVars::AddVariable(char* pVarName)
{
	int VarIndex;
	for (VarIndex = 0; VarIndex != MAX_SERVER_VARS; VarIndex++)
	{
		if (m_bVarSlotState[VarIndex] == false) break;
	}

	if (VarIndex == MAX_SERVER_VARS) return -1;

	if (strlen(pVarName) <= MAX_SERVER_VARS_NAME)
	{
		Util_strupr(pVarName);

		ServerVariable_s* SVar = &ServerVariables[VarIndex];
		strcpy(SVar->VarName, pVarName);
		SVar->VarReadOnly = false;
		SVar->VarType = SERVER_VARTYPE_NONE;
		
		m_bVarSlotState[VarIndex] = true;

		UpdatePoolSize();

		return VarIndex;
	}
	return -1;
}

bool CServerVars::SetIntVariable(char* pVarName, int iInt, bool bReadOnly)
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

	ServerVariable_s* SVar = &ServerVariables[VarIndex];

	if(bReadOnly)
	{
		SVar->VarReadOnly = true;
	}
	else if (SVar->VarReadOnly)
	{
		return false;
	}

	SVar->VarType = SERVER_VARTYPE_INT;
	SVar->VarInt = iInt;

	return true;
}

bool CServerVars::SetStringVariable(char* pVarName, char* pString, bool bReadOnly)
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

	ServerVariable_s* SVar = &ServerVariables[VarIndex];

	if (bReadOnly)
	{
		SVar->VarReadOnly = true;
	}
	else if (SVar->VarReadOnly)
	{
		return false;
	}

	SVar->VarType = SERVER_VARTYPE_STRING;

	if (SVar->VarStr)
		free(SVar->VarStr);

	SVar->VarStr = (char*)calloc(1,strlen(pString)+1);
	strcpy(SVar->VarStr, pString);

	return true;
}

bool CServerVars::SetFloatVariable(char* pVarName, float fFloat, bool bReadOnly)
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

	ServerVariable_s* SVar = &ServerVariables[VarIndex];

	if (bReadOnly)
	{
		SVar->VarReadOnly = true;
	}
	else if (SVar->VarReadOnly)
	{
		return false;
	}

	SVar->VarType = SERVER_VARTYPE_FLOAT;
	SVar->VarFlt = fFloat;

	return true;
}

int CServerVars::GetIntVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		ServerVariable_s* SVar = &ServerVariables[VarIndex];
		if (SVar->VarType == SERVER_VARTYPE_INT)
			return SVar->VarInt;
	}
	return 0;
}

char* CServerVars::GetStringVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		ServerVariable_s* SVar = &ServerVariables[VarIndex];
		if (SVar->VarType == SERVER_VARTYPE_STRING)
			return SVar->VarStr;
	}
	return NULL;
}

float CServerVars::GetFloatVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		ServerVariable_s* SVar = &ServerVariables[VarIndex];
		if (SVar->VarType == SERVER_VARTYPE_FLOAT)
			return SVar->VarFlt;
	}
	return 0.0f;
}

bool CServerVars::RemoveVariable(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		ServerVariable_s* SVar = &ServerVariables[VarIndex];
		if (SVar->VarStr)
			free(SVar->VarStr);
		memset(SVar, 0, sizeof(ServerVariable_s));
		m_bVarSlotState[VarIndex] = false;
		UpdatePoolSize();
		return true;
	}
	return false;
}

int CServerVars::GetVariableType(char* pVarName)
{
	int VarIndex = FindVariable(pVarName);
	if (VarIndex >= 0)
	{
		return ServerVariables[VarIndex].VarType;
	}
	return SERVER_VARTYPE_NONE;
}

char* CServerVars::GetVariableNameAtIndex(int iIndex)
{
	if (iIndex >= 0 && iIndex < MAX_SERVER_VARS && m_bVarSlotState[iIndex])
	{
		return ServerVariables[iIndex].VarName;
	}
	return NULL;
}
