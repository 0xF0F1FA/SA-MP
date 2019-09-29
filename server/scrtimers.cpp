/*

	SA:MP Multiplayer Modification
	Copyright 2004-2006 SA:MP Team

	file:
		scrtimers.cpp
	desc:
		Gamemode script timers.

    Version: $Id: scrtimers.cpp,v 1.7 2006/03/25 09:12:48 kyeman Exp $

*/

//----------------------------------------------------------------------------------

#include "main.h"

extern CNetGame* pNetGame;

//----------------------------------------------------------------------------------

CScriptTimers::CScriptTimers()
{
	m_dwTimerCount = 0;
}

//----------------------------------------------------------------------------------

CScriptTimers::~CScriptTimers()
{
	DwordTimerMap::iterator itor;
	for (itor = m_Timers.begin(); itor != m_Timers.end(); itor++)
	{
		FreeMem(itor->second);
		SAFE_DELETE(itor->second);
	}
	m_Timers.clear();
}

//----------------------------------------------------------------------------------

void CScriptTimers::FreeMem(ScriptTimer_s* Timer)
{
	if (Timer->cellParams != NULL)
	{
		free(Timer->cellParams);
		Timer->cellParams = NULL;
	}
}

//----------------------------------------------------------------------------------

// Kills only the timers found in one mode

void CScriptTimers::DeleteForMode(AMX* pEndedAMX)
{
	bool bRestart = false;
	DwordTimerMap::iterator itor;
	for (itor = m_Timers.begin(); itor != m_Timers.end(); bRestart?(itor=m_Timers.begin()):(itor++))
	{
		bRestart = false;
		if (itor->second->pAMX == pEndedAMX)
		{
			FreeMem(itor->second);
			SAFE_DELETE(itor->second);
			m_Timers.erase(itor);
			
			// Can't continue iteration if a node is deleted, start iteration from start again.
			bRestart = true;
		}
	}
}

//----------------------------------------------------------------------------------

DWORD CScriptTimers::New(char* szScriptFunc, int iInterval, BOOL bRepeating, AMX* pAMX)
{
	m_dwTimerCount++;

	ScriptTimer_s* NewTimer = new ScriptTimer_s;

	//if(iInterval < 500) iInterval = 500;

	strncpy(NewTimer->szScriptFunc, szScriptFunc, 255);
	NewTimer->iTotalTime = iInterval;
	NewTimer->iRemainingTime = iInterval;
	NewTimer->bRepeating = bRepeating;
	NewTimer->iParamCount = 0;
	NewTimer->bKilled = false;
	NewTimer->pAMX = pAMX;
	NewTimer->cellParams = NULL;
	// Checks if it's called from a filterscript, if not, mark it for destruction at gamemode end
	/*if (pAMX == pNetGame->GetGameMode()->GetGameModePointer())
	{
		NewTimer->bFilterscript = false;
		//print("GM");
	}
	else
	{
		NewTimer->bFilterscript = true;
		//print("FS");
	}*/
	m_Timers.insert(DwordTimerMap::value_type(m_dwTimerCount, NewTimer));
	return m_dwTimerCount;
}

//----------------------------------------------------------------------------------
// Same as new only with parameters to be passed to the called function
cell* get_amxaddr(AMX *amx, cell amx_addr);
DWORD CScriptTimers::NewEx(char* szScriptFunc, int iInterval, BOOL bRepeating, cell *params, AMX* pAMX)
{
	m_dwTimerCount++;

	ScriptTimer_s* NewTimer = new ScriptTimer_s;

	strncpy(NewTimer->szScriptFunc, szScriptFunc, 255);
	NewTimer->iTotalTime = iInterval;
	NewTimer->iRemainingTime = iInterval;
	NewTimer->bRepeating = bRepeating;
	NewTimer->bKilled = false;
	NewTimer->pAMX = pAMX;
	
	cell amx_addr[256];
	
	char* szParamList;
	amx_StrParam(pAMX, params[4], szParamList);
	int j, numstr, iOff = 5; // Count, func, interval, repeat, map
	if (szParamList == NULL) j = 0;
	else j = strlen(szParamList);
	numstr = 0;
	while (j)
	{
		j--;
		cell *paddr = NULL;
		if (*(szParamList + j) == 'a')
		{
			int numcells = *get_amxaddr(pAMX, params[j + iOff + 1]);
			if (amx_Allot(pAMX, numcells, &amx_addr[numstr], &paddr) == AMX_ERR_NONE)
			{
				memcpy(paddr, get_amxaddr(pAMX, params[j + iOff]), numcells * sizeof (cell));
				numstr++;
			}
		}
		else if (*(szParamList + j) == 's')
		{
			char* szParamText;
			amx_StrParam(pAMX, params[j + iOff], szParamText);
			if (szParamText != NULL && strlen(szParamText) > 0)
			{
				int numcells = strlen(szParamText) + 1;
				if (amx_Allot(pAMX, numcells, &amx_addr[numstr], &paddr) == AMX_ERR_NONE)
				{
					amx_SetString(paddr, szParamText, 0, 0, UNLIMITED);
					numstr++;
				}
			}
			else
			{
				*szParamText = 1;
				*(szParamText + 1) = 0;
				if (amx_Allot(pAMX, 1, &amx_addr[numstr], &paddr) == AMX_ERR_NONE)
				{
					amx_SetString(paddr, szParamText, 0, 0, UNLIMITED);
					numstr++;
				}
			}
		}
		else
		{
			amx_addr[numstr] = *get_amxaddr(pAMX, params[j + iOff]);
			numstr++;
		}
	}
	void* mem = NULL;
	if (numstr)
	{
		mem = malloc(numstr * sizeof (cell));
		memcpy(mem, &amx_addr, numstr * sizeof (cell));
		NewTimer->cellParams = mem;
	}
	else
	{
		NewTimer->cellParams = NULL;
	}
	NewTimer->iParamCount = numstr;
	
	m_Timers.insert(DwordTimerMap::value_type(m_dwTimerCount, NewTimer));
	return m_dwTimerCount;
}

//----------------------------------------------------------------------------------

void CScriptTimers::Delete(DWORD dwTimerId)
{
	DwordTimerMap::iterator itor;
	itor = m_Timers.find(dwTimerId);
	if (itor != m_Timers.end())
	{
		FreeMem(itor->second);
		SAFE_DELETE(itor->second);
		m_Timers.erase(itor);
		//Delete(itor->first);
	}
}

//----------------------------------------------------------------------------------

void CScriptTimers::Kill(DWORD dwTimerId)
{
	DwordTimerMap::iterator itor;
	itor = m_Timers.find(dwTimerId);
	if (itor != m_Timers.end())
	{
		//SAFE_DELETE(itor->second);
		//m_Timers.erase(itor);
		itor->second->iRemainingTime = 0;
		itor->second->bKilled = true;
		itor->second->bRepeating = false;
		//Delete(itor->first);
	}
}

//-----------------------------------------------------------

//int AMXAPI amx_Push(AMX *amx, cell value);
void CScriptTimers::Process(int iElapsedTime)
{
	DwordTimerMap::iterator itor;
	CGameMode *pGameMode;
	for (itor = m_Timers.begin(); itor != m_Timers.end(); itor++)
	{
		itor->second->iRemainingTime -= iElapsedTime;
		if (itor->second->iRemainingTime <= 0)
		{
			DwordTimerMap::iterator itor_tmp = ++itor; itor--;
			if (!itor->second->bKilled)
			{
				pGameMode = pNetGame->GetGameMode();
				if (pGameMode)
				{
					int idx;
					AMX* amx = itor->second->pAMX;
					if (amx && !amx_FindPublic(amx, itor->second->szScriptFunc, &idx))
					{
						cell ret;
						//cell releases[16];
						int count = itor->second->iParamCount;
						int i = 0;
						if (count > 0)
						{
							//strings = PushList(amx, (cell*)itor->second->cellParams, count);
							cell* pars = (cell*)itor->second->cellParams;
							while (i < count)
							{
								amx_Push(amx, pars[i]);
								i++; // Go forwards to maintain push order
							}
						}
						amx_Exec(amx, &ret, idx);
						/*if (itor->second->iParamCount > 0)
						{
							while (strings)
							{
								strings--;
								amx_Release(amx, releases[strings]);
							}
						}*/
					}
				}
			}
	
			if (itor->second->bRepeating)
			{
				itor->second->iRemainingTime = itor->second->iTotalTime;
			}
			else
			{
				// Release parameter memory
				FreeMem(itor->second);
				Delete(itor->first);
			}
			itor = itor_tmp;
		}
		if (itor == m_Timers.end()) break;
	}
}

//----------------------------------------------------------------------------------

/*int CScriptTimers::PushList(AMX* amxFile, cell* params, int count)
{
	//cell amx_addr[16]; // = NULL, *phys_addr[16];
	int j = 0;
	
	while (j < count)
	{
		j++; // Go forwards to maintain push order
		amx_push(amxFile, params[j]);
	}
}*/

//----------------------------------------------------------------------------------
// EOF
