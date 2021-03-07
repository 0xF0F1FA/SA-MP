
#include "main.h"

#define SKIP_WHITESPACES(ptr) \
	while(*ptr == ' ' || *ptr == '\t') \
		ptr++;

CConfigFile::CConfigFile(char* szPath)
{
	SecureZeroMemory(m_Entries, sizeof(m_Entries));
	SecureZeroMemory(m_bSlotState, sizeof(m_bSlotState));
	SecureZeroMemory(m_szConfigFile, sizeof(m_szConfigFile));

	m_iLastEntry = -1;

	if (szPath && szPath[0] != '\0')
	{
		sprintf_s(m_szConfigFile, "%s\\" CONFIG_FILE, szPath);

		ReadFile();
	}
}

CConfigFile::~CConfigFile()
{
	for (int i = 0; i < MAX_CONFIG_ENTRIES; i++)
	{
		if (m_Entries[i].szValue)
			free(m_Entries[i].szValue);
	}
}

void CConfigFile::UpdateEntrySize()
{
	m_iLastEntry = -1;

	for (int i = 0; i < MAX_CONFIG_ENTRIES; i++)
	{
		if (m_bSlotState[i])
		{
			m_iLastEntry = i;
		}
	}
}

int CConfigFile::FindValidEntry(char* szKey)
{
	int i;

	/*_strlwr(szKey);

	if (m_iLastEntry <= 0)
	{
		return -1;
	}*/

	i = 0;

	while (i <= m_iLastEntry)
	{
		//if(m_bSlotState[i] && !strcmp(szKey, m_Entries[i].szKey))
		if (m_bSlotState[i] && _stricmp(szKey, m_Entries[i].szKey) == 0)
		{
			return i;
		}
		i++;
	}
	return -1;
}

bool CConfigFile::IsValidKey(char* szKey)
{
	return FindValidEntry(szKey) >= 0;
}

int CConfigFile::AddKey(char* szKey)
{
	CONFIG_ENTRY_INFO* pEntry;
	int i;

	if (MAX_CONFIG_KEY < strlen(szKey))
	{
		return -1;
	}

	for (i = 0; i < MAX_CONFIG_ENTRIES; i++)
	{
		if (!m_bSlotState[i])
		{
			break;
		}
	}

	if (i == MAX_CONFIG_ENTRIES) return -1;

	_strlwr(szKey);
	pEntry = &m_Entries[i];
	strcpy_s(pEntry->szKey, szKey);
	pEntry->bReadOnly = false;
	pEntry->cType = CONFIG_VALUE_TYPE_NONE;
	m_bSlotState[i] = true;

	UpdateEntrySize();

	return i;
}

int CConfigFile::GetInt(char* szKey)
{
	int i = FindValidEntry(szKey);
	if (i >= 0 && m_Entries[i].cType == CONFIG_VALUE_TYPE_INT)
	{
		return m_Entries[i].iValue;
	}
	return 0;
}

char* CConfigFile::GetString(char* szKey)
{
	int i = FindValidEntry(szKey);
	if (i >= 0 && m_Entries[i].cType == CONFIG_VALUE_TYPE_STRING)
	{
		return m_Entries[i].szValue;
	}
	return NULL;
}

float CConfigFile::GetFloat(char* szKey)
{
	int i = FindValidEntry(szKey);
	if (i >= 0 && m_Entries[i].cType == CONFIG_VALUE_TYPE_FLOAT)
	{
		return m_Entries[i].fValue;
	}
	return 0.0f;
}

bool CConfigFile::SetInt(char* szKey, int iValue, bool bReadOnly)
{
	CONFIG_ENTRY_INFO* pEntry;
	int i = FindValidEntry(szKey);
	if (i < 0)
	{
		i = AddKey(szKey);
		if (i < 0)
		{
			return false;
		}
	}

	pEntry = &m_Entries[i];

	if (bReadOnly)
		pEntry->bReadOnly = true;
	else if (pEntry->bReadOnly)
		return false;

	pEntry->iValue = iValue;
	pEntry->cType = CONFIG_VALUE_TYPE_INT;
	UpdateFile();
	return true;
}

bool CConfigFile::SetString(char* szKey, char* szValue, bool bReadOnly)
{
	CONFIG_ENTRY_INFO* pEntry;
	int i;
	size_t length;

	i = FindValidEntry(szKey);
	if (i < 0)
	{
		i = AddKey(szKey);
		if (i < 0)
		{
			return false;
		}
	}

	pEntry = &m_Entries[i];

	if (bReadOnly)
		pEntry->bReadOnly = true;
	else if (pEntry->bReadOnly)
		return false;

	pEntry->cType = CONFIG_VALUE_TYPE_STRING;

	if (pEntry->szValue)
		free(pEntry->szValue);

	length = strlen(szValue);
	//pEntry->szValue = (char*)calloc(1, length + 1);
	pEntry->szValue = (char*)malloc(length + 1);

	if (pEntry->szValue)
		strcpy_s(pEntry->szValue, length + 1, szValue);

	UpdateFile();

	return true;
}

bool CConfigFile::SetFloat(char* szKey, float fValue, bool bReadOnly)
{
	CONFIG_ENTRY_INFO* pEntry;
	int i;

	i = FindValidEntry(szKey);
	if (i < 0)
	{
		i = AddKey(szKey);
		if (i < 0)
		{
			return false;
		}
	}

	pEntry = &m_Entries[i];

	if (bReadOnly)
		pEntry->bReadOnly = true;
	else if (pEntry->bReadOnly)
		return false;

	pEntry->fValue = fValue;
	pEntry->cType = CONFIG_VALUE_TYPE_FLOAT;
	UpdateFile();
	return true;
}

int CConfigFile::GetTypeFromValue(char* szValue)
{
	if (szValue && szValue[0] != '\0') // && strlen(szValue))
	{
		if (*szValue != '"' || szValue[strlen(szValue) - 1] != '"')
		{
			return strchr(szValue, '.') != NULL ? CONFIG_VALUE_TYPE_FLOAT : CONFIG_VALUE_TYPE_INT;
		}
		return CONFIG_VALUE_TYPE_STRING;
	}
	return CONFIG_VALUE_TYPE_NONE;
}

void CConfigFile::AddEntry(char* szKey, char* szValue)
{
	int iType;
	size_t length;

	// Not sure if decrementing the type by 1 was intentional, or compiler's fault
	iType = GetTypeFromValue(szValue);

	if (iType == CONFIG_VALUE_TYPE_INT)
	{
		SetInt(szKey, strtol(szValue, NULL, 0), false);
	}
	else if (iType == CONFIG_VALUE_TYPE_STRING)
	{
		length = strlen(szValue);
		strncpy_s(szValue, 256, szValue + 1, length - 1);
		szValue[length - 2] = '\0';

		SetString(szKey, szValue, false);
	}
	else if (iType == CONFIG_VALUE_TYPE_FLOAT)
	{
		SetFloat(szKey, strtof(szValue, NULL), false);
	}
}

bool CConfigFile::ReadFile()
{
	FILE* pFile;
	char szLine[256], szKey[256], szValue[256];
	char* szLinePtr;
	size_t i;

	fopen_s(&pFile, m_szConfigFile, "r");

	if (!pFile)
	{
		OutputDebugString("Failed to load the config file.");
		return false;
	}

	while (!feof(pFile))
	{
		fgets(szLine, sizeof(szLine), pFile);

		szLinePtr = szLine;

		i = 0;
		while (szLine[i] != '\0')
		{
			if (szLine[i] == ';' || szLine[i] == '#')
				szLine[i] = '\0';
			i++;
		}

		SKIP_WHITESPACES(szLinePtr);

		if (*szLinePtr == '\0' ||
			*szLinePtr == ';' ||
			*szLinePtr == '\n' ||
			*szLinePtr == '[')
		{
			continue;
		}

		i = 0;

		while (*szLinePtr != '\0' &&
			*szLinePtr != ' ' &&
			*szLinePtr != '=' &&
			*szLinePtr != '\n' &&
			*szLinePtr != ';')
		{
			szKey[i] = toupper(*szLinePtr);
			szLinePtr++;
			i++;
		}

		if (i == 0)
			continue;

		szKey[i] = '\0';

		SKIP_WHITESPACES(szLinePtr);

		if (*szLinePtr != '=')
		{
			continue;
		}

		szLinePtr++;

		SKIP_WHITESPACES(szLinePtr);

		if (*szLinePtr == '\0')
		{
			continue;
		}

		i = 0;

		while (*szLinePtr != '\n' &&
			*szLinePtr != '\0')
		{
			szValue[i] = *szLinePtr;
			szLinePtr++;
			i++;
		}

		if (i == 0)
			continue;

		szValue[i] = '\0';

		i--;
		while (
			szValue[i] == ' ' ||
			szValue[i] == '\t' ||
			szValue[i] == '\r')
		{
			szValue[i] = '\0';
			i--;
		}
		AddEntry(szKey, szValue);
	}

	fclose(pFile);

	return true;
}

void CConfigFile::UpdateFile()
{
	FILE* pFile;
	int i;

	fopen_s(&pFile, m_szConfigFile, "w");
	if (!pFile)
	{
		OutputDebugString("Could not update the config file");
		return;
	}

	for (i = 0; i < MAX_CONFIG_ENTRIES; i++)
	{
		if (m_bSlotState[i])
		{
			switch (m_Entries[i].cType)
			{
			case CONFIG_VALUE_TYPE_INT:
				fprintf_s(pFile, "%s=%d\n", m_Entries[i].szKey, m_Entries[i].iValue);
				break;
			case CONFIG_VALUE_TYPE_STRING:
				if (m_Entries[i].szValue)
					fprintf_s(pFile, "%s=\"%s\"\n", m_Entries[i].szKey, m_Entries[i].szValue);
				break;
			case CONFIG_VALUE_TYPE_FLOAT:
				fprintf_s(pFile, "%s=%f\n", m_Entries[i].szKey, m_Entries[i].fValue);
				break;
			}
		}
	}
	fclose(pFile);
}