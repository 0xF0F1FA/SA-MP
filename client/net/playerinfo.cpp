
#include "../main.h"

#include "playerinfo.h"

//----------------------------------------------------

CPlayerInfo::CPlayerInfo(char* szName, bool bIsNPC)
{
	m_bIsNPC = bIsNPC;
	m_dwPing = 0;
	m_iScore = 0;
	m_name = szName;
	m_pRemotePlayer = new CRemotePlayer();
}

CPlayerInfo::~CPlayerInfo()
{
	SAFE_DELETE(m_pRemotePlayer);
}
