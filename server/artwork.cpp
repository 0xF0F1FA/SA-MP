
#include "main.h"

CArtwork::CArtwork(char* szArtPath)
{
	memset(m_szArtPath, 0, sizeof(m_szArtPath));
	strncpy(m_szArtPath, szArtPath, sizeof(m_szArtPath));
}

int CArtwork::FilterInvalidCharacters(char* szOut, int iLen, char* szIn)
{
	int iInLen = strlen(szIn);
	int iLenCopied, iIndex;

	iLenCopied = iIndex = 0;

	if (iInLen == 0)
	{
		szOut[0] = '\0';
	}
	else
	{
		while (iIndex != iInLen)
		{
			char c = szIn[iIndex];
			if (c >= '-' && c <= 'z')
			{
				szOut[iLenCopied] = c;
				iLenCopied++;
			}
			iIndex++;
		}
		szOut[iLenCopied] = '\0';
	}
	return iLenCopied;
}

void CArtwork::AddCharModelEntry(char* szParams)
{
	char* szParam;
	int iBaseID, iNewID;
	char szDffName[260], szTxdName[260];

	if (!szParams) return;

	szParam = strtok(szParams, ",");
	iBaseID = (int)atol(szParam);
	szParam = strtok(0, ",");
	iNewID = (int)atol(szParam);
	szParam = strtok(0, ",");
	FilterInvalidCharacters(szDffName, sizeof(szDffName), szParam);
	szParam = strtok(0, ",");
	FilterInvalidCharacters(szTxdName, sizeof(szTxdName), szParam);

	AddModelEntry(1, -1, iBaseID, iNewID, szDffName, szTxdName, false, false);
}

void CArtwork::AddSimpleModelEntry(char* szParams)
{
	char* szParam;
	int iVirtualWorld, iBaseID, iNewID;
	char szDffName[260], szTxdName[260];

	if (!szParams) return;

	szParam = strtok(szParams, ",");
	iVirtualWorld = (int)atol(szParam);
	szParam = strtok(0, ",");
	iBaseID = (int)atol(szParam);
	szParam = strtok(0, ",");
	iNewID = (int)atol(szParam);
	szParam = strtok(0, ",");
	FilterInvalidCharacters(szDffName, sizeof(szDffName), szParam);
	szParam = strtok(0, ",");
	FilterInvalidCharacters(szTxdName, sizeof(szTxdName), szParam);

	AddModelEntry(2, iVirtualWorld, iBaseID, iNewID, szDffName, szTxdName, false, false);
}

char* CArtwork::GetParams(char* szFuncLine)
{
	char* begin = strchr(szFuncLine, '(');
	char* end = strchr(szFuncLine, ')');

	if (begin && end)
	{
		int len = end - begin - 1;

		strncpy(szFuncLine, begin + 1, len);
		szFuncLine[len] = 0;

		return szFuncLine;
	}
	return NULL;
}

void CArtwork::ProcessLine(char* szLine)
{
	if (!strncmp(szLine, "AddCharModel", 12))
	{
		AddCharModelEntry(GetParams(szLine));
	}
	else if(!strncmp(szLine, "AddSimpleModel", 14))
	{
		AddSimpleModelEntry(GetParams(szLine));
	}
}

void CArtwork::ReadConfig(char* szArtConfig)
{
	char szArtFile[MAX_PATH];
	char szLine[512];
	FILE* pFile;

	sprintf(szArtFile, "%s/%s", m_szArtPath, szArtConfig);
	pFile = fopen(szArtFile, "r");
	if (pFile)
	{
		while (!feof(pFile))
		{
			memset(szLine, 0, sizeof(szLine));
			fgets(szLine, sizeof(szLine), pFile);
			ProcessLine(szLine);
		}
		fclose(pFile);
	}
}

DWORD CArtwork::GetFileChecksum(char* szFileName)
{
	return CRC32(0, szFileName, strlen(szFileName));
}

DWORD CArtwork::GetFileChecksum(char* szFileName, DWORD* pdwCRC)
{
	char szFile[MAX_PATH];
	sprintf(szFile, "%s/%s", m_szArtPath, szFileName);
	return GetFileCRC32Checksum(szFile, pdwCRC);
}

int CArtwork::AddModelEntry(BYTE byteType, int iVW, int iBaseID, int iNewID,
	char* szDffName, char* szTxtName, bool bTimeOn, bool bTimeOff)
{
	ARTWORK_DATA* pData;

	pData = (ARTWORK_DATA*)calloc(1, sizeof(ARTWORK_DATA));

	// Additional
	if (pData == NULL)
	{
		logprintf("[artwork:error] Allocation failed for %s", szDffName);
		return 0;
	}

	pData->iVirtualWorld = iVW; // volkswagen?
	pData->byteType = byteType;
	pData->iBaseID = iBaseID;
	pData->iNewID = iNewID;
	strncpy(pData->szDffName, szDffName, MAX_PATH);
	strncpy(pData->szTxdName, szTxtName, MAX_PATH);
	pData->byteTimeOff = bTimeOff;
	pData->byteTimeOn = bTimeOn;

	if (byteType == 4)
	{
		pData->dwDffCRC = GetFileChecksum(pData->szDffName);
	}
	else
	{
		pData->dwDffFileSize = GetFileChecksum(pData->szDffName, &pData->dwDffCRC);
		logprintf("[artwork:crc] %s CRC = 0x%X", pData->szDffName, pData->dwDffCRC);
		if (pData->dwDffFileSize == 0)
		{
			logprintf("[artwork:error] Bad file: %s", pData->szDffName);
			free(pData); // FIXFIX
			return 0;
		}
	}

	pData->dwTxdFileSize = GetFileChecksum(pData->szTxdName, &pData->dwTxdCRC);
	logprintf("[artwork:crc] %s CRC = 0x%X", pData->szTxdName, pData->dwTxdCRC);

	if (pData->dwTxdFileSize == 0)
	{
		logprintf("[artwork:error] Bad file: %s", pData->szTxdName);
		free(pData); // FIXFIX
		return 0;
	}

	int iIndex = m_map.Add((DWORD)pData);

	if (pNetGame)
	{
		RakNet::BitStream bsSend;
		bsSend.Write(iIndex);
		bsSend.Write(m_map.m_dwSize);

		bsSend.Write(pData->byteType);
		bsSend.Write(pData->iVirtualWorld);
		bsSend.Write(pData->iBaseID);
		bsSend.Write(pData->iNewID);
		bsSend.Write(pData->dwDffCRC);
		bsSend.Write(pData->dwTxdCRC);
		bsSend.Write(pData->dwDffFileSize);
		bsSend.Write(pData->dwTxdFileSize);
		bsSend.Write(pData->byteTimeOn);
		bsSend.Write(pData->byteTimeOff);

		pNetGame->SendToAll(RPC_ModelRequest, &bsSend);
	}

	return iIndex;
}

ARTWORK_DATA* CArtwork::FindDataFromModelCRC(DWORD dwDffCRC)
{
	ARTWORK_DATA* pData;

	if (m_map.m_dwSize)
	{
		for (DWORD dwIndex = 0; dwIndex < m_map.m_dwSize; dwIndex++)
		{
			pData = (ARTWORK_DATA*)m_map.GetAt(dwIndex);
			if (pData != 0 && pData->dwDffCRC == dwDffCRC)
			{
				return pData;
			}
		}
	}
	return NULL;
}

ARTWORK_DATA* CArtwork::FindDataFromTextureCRC(DWORD dwTxdCRC)
{
	ARTWORK_DATA* pData;

	if (m_map.m_dwSize)
	{
		for (DWORD dwIndex = 0; dwIndex < m_map.m_dwSize; dwIndex++)
		{
			pData = (ARTWORK_DATA*)m_map.GetAt(dwIndex);
			if (pData != 0 && pData->dwTxdCRC == dwTxdCRC)
			{
				return pData;
			}
		}
	}
	return NULL;
}

DWORD CArtwork::GetBaseID(int iNewID)
{
	ARTWORK_DATA* pData;

	if (m_map.m_dwSize)
	{
		for (DWORD dwIndex = 0; dwIndex < m_map.m_dwSize; dwIndex++)
		{
			pData = (ARTWORK_DATA*)m_map.GetAt(dwIndex);
			
			if (pData != 0 && pData->iNewID == iNewID)
			{
				return pData->iBaseID;
			}
		}
	}
	return -1;
}
