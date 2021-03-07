
#pragma once

#define CONFIG_VALUE_TYPE_NONE		0
#define CONFIG_VALUE_TYPE_INT		1
#define CONFIG_VALUE_TYPE_STRING	2
#define CONFIG_VALUE_TYPE_FLOAT		3

#define MAX_CONFIG_ENTRIES 30 // 512 little bit overkill
#define MAX_CONFIG_KEY 40

typedef struct
{
	char szKey[MAX_CONFIG_KEY];
	char cType;
	int iValue;
	bool bReadOnly;
	float fValue;
	char* szValue;
} CONFIG_ENTRY_INFO;

class CConfigFile
{
private:
	CONFIG_ENTRY_INFO m_Entries[MAX_CONFIG_ENTRIES];
	bool m_bSlotState[MAX_CONFIG_ENTRIES];
	char m_szConfigFile[MAX_PATH];
	int m_iLastEntry;

	void UpdateEntrySize();
	void UpdateFile();
	int FindValidEntry(char* szKey);
	int AddKey(char* szKey);
	int GetTypeFromValue(char* szValue);
	void AddEntry(char* szKey, char* szValue);
	bool ReadFile();

public:
	CConfigFile(char* szPath);
	~CConfigFile();

	bool IsValidKey(char* szKey);
	
	int GetInt(char* szKey);
	bool SetInt(char* szKey, int iValue, bool bReadOnly = false);
	char* GetString(char* szKey);
	bool SetString(char* szKey, char* szValue, bool bReadOnly = false);
	float GetFloat(char* szKey);
	bool SetFloat(char* szKey, float fValue, bool bReadOnly = false);
};

