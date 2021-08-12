
#ifndef SAMPSRV_PLAYERVARS_H
#define SAMPSRV_PLAYERVARS_H

#define MAX_PLAYER_VARS			800
#define MAX_PLAYER_VARS_NAME	40

enum PLAYER_VARTYPE
{
	PLAYER_VARTYPE_NONE,
	PLAYER_VARTYPE_INT,
	PLAYER_VARTYPE_STRING,
	PLAYER_VARTYPE_FLOAT
};

typedef struct PlayerVariable_s
{
	char VarName[MAX_PLAYER_VARS_NAME+1];
	bool VarReadOnly; // BOOL
	PLAYER_VARTYPE VarType;
	int VarInt;
	float VarFlt;
	char* VarStr;
} PlayerVar_s;

class CPlayerVars
{
private:
	PlayerVariable_s PlayerVariable[MAX_PLAYER_VARS];
	bool m_bVarSlotState[MAX_PLAYER_VARS]; // BOOL
	int m_iPoolSize;

public:
	CPlayerVars();
	~CPlayerVars();

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
