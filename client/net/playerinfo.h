
#pragma once

#include <string>

//----------------------------------------------------
#pragma pack(1)
class CPlayerInfo
{
private:
	int m_iScore;
	BOOL m_bIsNPC; // BOOL
	CRemotePlayer* m_pRemotePlayer;
	DWORD m_dwPing;
	std::string m_name;

public:
	CPlayerInfo(char* szName, bool bIsNPC);
	~CPlayerInfo();

	void SetName(PCHAR szName) { m_name = szName; };
	PCHAR GetName() { return (PCHAR)m_name.c_str(); };
	void SetScore(int iScore) { m_iScore = iScore; };
	int GetScore() { return m_iScore; };
	void SetPing(DWORD dwPing) { m_dwPing = dwPing; };
	DWORD GetPing() { return m_dwPing; };
	CRemotePlayer* GetRemotePlayer() { return m_pRemotePlayer; };
};

//----------------------------------------------------