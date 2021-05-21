
#ifndef SAMPSRV_ARTWORK_H
#define SAMPSRV_ARTWORK_H

#include <list>

typedef struct {
	BYTE byteType;
	int iVirtualWorld;
	int iBaseID;
	int iNewID;
	char szDffName[MAX_PATH+1];
	char szTxdName[MAX_PATH+1];
	DWORD dwDffCRC;
	DWORD dwTxdCRC;
	DWORD dwDffFileSize;
	DWORD dwTxdFileSize;
	BYTE byteTimeOff;
	BYTE byteTimeOn;
} ARTWORK_DATA;

class CArtwork
{
private:
	char m_szArtPath[MAX_PATH];
	SAMPMAP m_map;

public:
	CArtwork(char* szArtPath);

	int FilterInvalidCharacters(char* szOut, int iLen, char* szIn);
	char* GetParams(char* szFuncLine);
	void ProcessLine(char* szLine);
	void ReadConfig(char* szArtConfig);
	void AddCharModelEntry(char* szParams);
	void AddSimpleModelEntry(char* szParams);
	DWORD GetFileChecksum(char* szFileName);
	DWORD GetFileChecksum(char* szFileName, DWORD* pdwCRC);
	ARTWORK_DATA* FindDataFromModelCRC(DWORD dwDffCRC);
	ARTWORK_DATA* FindDataFromTextureCRC(DWORD dwTxdCRC);
	DWORD GetBaseID(int iNewID);

	int AddModelEntry(BYTE byteType, int iVW, int iBaseID, int iNewID,
		char* szDffName, char* szTxtName, bool bTimeOn, bool bTimeOff);
};

#endif
