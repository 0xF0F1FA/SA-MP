
#ifndef SAMPSRV_THREADEDHTTP_H
#define SAMPSRV_THREADEDHTTP_H

#define MAX_SCRIPT_HTTP_ENTRIES 200

struct HTTP_ENTRY
{
	CHttpClient* pHttpClient;
	int iState;
	int iIndex;
	int iType;
	std::string url;
	std::string data;
	bool bQuietMode;
	AMX* pAMX;
	char szCallback[40];
	//RakNet::Time StartTime; // Unused? It's sets the time in thread, but newer used afterwards
	//DWORD dwUnk;

	HTTP_ENTRY(char* szBindAddress)
	{
		pHttpClient = new CHttpClient(szBindAddress);
		iState = 0;
		iIndex = 0;
		iType = 0;
		bQuietMode = true;
		memset(szCallback, 0, sizeof(szCallback));
	};

	~HTTP_ENTRY()
	{
		SAFE_DELETE(pHttpClient);
	};

	CHttpClient* GetHttpClient()
	{
		return pHttpClient;
	};
};

class CThreadedHttp
{
private:
	HTTP_ENTRY* m_pEntries[MAX_SCRIPT_HTTP_ENTRIES];

public:
	CThreadedHttp();
	void Process();
	bool AddEntry(int iIndex, int iType, char* szUrl, char* szData,
		char* szBindAddress, bool bQuietMode, AMX* pAMX, char* szCallback);
};

#endif // SAMPSRV_THREADEDHTTP_H
