
#ifndef SAMPSRV_SERVERVARS_H
#define SAMPSRV_SERVERVARS_H

#define MAX_SERVER_VARS			2000
#define MAX_SERVER_VARS_NAME	40

enum SERVER_VARTYPE
{
	SERVER_VARTYPE_NONE,
	SERVER_VARTYPE_INT,
	SERVER_VARTYPE_STRING,
	SERVER_VARTYPE_FLOAT,
};

typedef struct ServerVariable_s
{
	char VarName[MAX_SERVER_VARS_NAME+1];
	bool VarReadOnly; // BOOL
	SERVER_VARTYPE VarType;
	int VarInt;
	float VarFlt;
	char* VarStr;
} ServerVariable_s;

class CServerVars
{
private:
	ServerVariable_s ServerVariables[MAX_SERVER_VARS];
	bool m_bVarSlotState[MAX_SERVER_VARS]; // BOOL
	int m_iPoolSize;

public:
	CServerVars();
	~CServerVars();

	void UpdatePoolSize();
	int FindVariable(char* pVarName);
	int AddVariable(char* pVarName);

	bool SetIntVariable(char* pVarName, int iInt, bool bReadOnly);
	bool SetStringVariable(char* pVarName, char* pString, bool bReadOnly);
	bool SetFloatVariable(char* pVarName, float fFloat, bool bReadOnly);
	int GetIntVariable(char* pVarName);
	char* GetStringVariable(char* pVarName);
	float GetFloatVariable(char* pVarName);

	bool RemoveVariable(char* pVarName);
	int GetVariableType(char* pVarName);
	char* GetVariableNameAtIndex(int iIndex);

	int GetPoolSize() { return m_iPoolSize; };
};

#endif
