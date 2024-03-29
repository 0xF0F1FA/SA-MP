/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		console.cpp
	desc:
		Console command and variable system.

    Version: $Id: console.cpp,v 1.21 2006/04/15 17:21:15 kyeman Exp $

*/

#include "main.h"

void con_exit()
{
	bQuitApp = true;
}

void con_echo()
{
	char* arg = strtok(NULL, "");
	if (arg)
	{
		logprintf("%s", arg);
	} else {
		logprintf("");
	}
}

void con_exec()
{
	char* arg = strtok(NULL, " ");
	if (arg)
	{
		char tmp[256];
		char tmpvarname[128];
		sprintf(tmp, "%s.cfg", arg);
		FILE* f = fopen(tmp, "r");
		if (!f)
		{
			logprintf("Unable to exec file '%s'.", tmp);
		} else {
			while (fgets(tmp, sizeof(tmp), f))
			{
				int len = strlen(tmp);
				if (len > 0 && tmp[len-1] == '\n')
					tmp[strlen(tmp)-1] = 0;
				len = strlen(tmp);
				if (len > 0 && tmp[len-1] == '\r')
					tmp[strlen(tmp)-1] = 0;
				if (tmp[0] == 0)
					continue;
				// If the line has a comment, finish it there.
				for (size_t i=0; i<strlen(tmp)-1; i++)
				{
					if (((tmp[i] == '/') && (tmp[i+1] == '/')) || tmp[i] == '#')
					{
						tmp[i] = 0;
						break;
					}
				}
				if (strlen(tmp) > 2)
				{
					if ((tmp[0] != '/') && (tmp[1] != '/'))
						for(int i = 0; tmp[i] != '\0'; i++)
						{
							if (tmp[i] == ' ')
							{
								tmpvarname[i] = '\0';
								break;
							}

							tmpvarname[i] = tmp[i];
						}
						if (pConsole->FindVariable(tmpvarname))
						{
							pConsole->Execute(tmp);
						}
				}
			}
			fclose(f);
		}
	} else {
		logprintf("Usage:");
		logprintf("  exec <filename>");
	}
}

void con_kick()
{
	char* arg = strtok(NULL, "");
	if(arg)
	{
		CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
		WORD wPlayerId = atoi(arg);

		if(pPlayerPool->GetSlotState(wPlayerId))
		{
			RakServerInterface* pRak = pNetGame->GetRakServer();
			PlayerID Player = pRak->GetPlayerIDFromIndex(wPlayerId);
			in_addr in;
			in.s_addr = Player.binaryAddress;
			logprintf("%s <#%d - %s> has been kicked.",pPlayerPool->GetAt(wPlayerId)->GetName(), wPlayerId, inet_ntoa(in));
			pNetGame->KickPlayer(wPlayerId);
		}
	}
}

void con_ban()
{
	char* arg = strtok(NULL, " ");
	if (arg)
	{
		CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
		WORD wPlayerId = atoi(arg);
		if (pPlayerPool->GetSlotState(wPlayerId))
		{
			RakServerInterface* pRak = pNetGame->GetRakServer();
			PlayerID Player = pRak->GetPlayerIDFromIndex(wPlayerId);
			in_addr in;
			in.s_addr = Player.binaryAddress;
			CPlayer* pPlayer = pPlayerPool->GetAt(wPlayerId);
			logprintf("%s <#%d - %s> has been banned.", pPlayer->GetName(), wPlayerId, inet_ntoa(in));
			pNetGame->AddBan((char*)pPlayer->GetName(), inet_ntoa(in), "CONSOLE BAN");
			pNetGame->KickPlayer(wPlayerId);
		}
	}
}

bool IsStrIp(char* szIn)
{
	char* part;
	char tmp[16];
	memcpy(&tmp, szIn, 16);
	int parts = 0;
	part = strtok(szIn, ".");
	while (part != NULL)
	{
		parts++;
		//if (parts > 4) return false;
		if (part != "*")
		{
			if (atoi(part) < 0 || atoi(part) > 255) return false;
		}
		part = strtok(NULL, ".");
	}
	if (parts != 4) return false;
	memcpy(szIn, &tmp, 16);
	return true;
}

void con_banip()
{
	char* arg = strtok(NULL, " ");
	if (arg && IsStrIp(arg))
	{
		logprintf("IP %s has been banned.", arg);
		pNetGame->AddBan("NONE", arg, "IP BAN");
	}
}

void con_unbanip()
{
	char* arg = strtok(NULL, " ");
	if (arg && IsStrIp(arg))
	{
		pNetGame->RemoveBan(arg);
	}
}

void con_gmx()
{
	if (pNetGame && pNetGame->GetGameState() == GAMESTATE_RUNNING)
	{
		// restart with no rotations.
		//int tmp = pNetGame->GetRepeats;
		// Gets the name of the next mode to avoid standard cycling
		char* szMode;
		szMode = pNetGame->GetNextScriptFile();
		if (szMode != NULL && pNetGame->SetNextScriptFile(szMode))
		{
			bGameModeFinished = true;
		}
	}
}

void con_changemode()
{
	if (pNetGame && pNetGame->GetGameState() == GAMESTATE_RUNNING)
	{
		char* arg = strtok(NULL, "");
		if (arg)
		{
			if (pNetGame->SetNextScriptFile(arg)) {
				bGameModeFinished = true;
			}
			// do nothing if we can't set the requested script.
		}
	}
}

void con_cmdlist();

void con_varlist()
{
	pConsole->PrintVariableList();
}

void con_say() {
	char* arg = strtok(NULL, "");
	char Message[256];
	if (arg) {
		sprintf(Message, "* Admin: %.230s", arg);
		pNetGame->SendClientMessageToAll(0x2587CEAA, Message);
	}
}

void con_reloadbans() {
	pNetGame->LoadBanList();
}

void con_reloadlog() {
	LoadLogFile();
}

void con_players() {
	RakServerInterface* pRak = pNetGame->GetRakServer();

	if ( pRak->GetConnectedPlayers() == 0 ) // If there is no players, why continue?
		return;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	logprintf("ID\tName\tPing\tIP");

	for( int i = 0; i < MAX_PLAYERS; i++)
	{
		if ( pPlayerPool->GetSlotState(i) )
		{
			PlayerID Player = pRak->GetPlayerIDFromIndex(i);
			in_addr in;
			in.s_addr = Player.binaryAddress;

			logprintf("%d\t%s\t%d\t%s", i, pPlayerPool->GetAt(i)->GetName(), pRak->GetLastPing( Player ), inet_ntoa(in));
		}
	}
}

/*void con_gravity()
{
	char* arg = strtok(NULL, " ");
	if (arg)
	{
		pNetGame->SetGravity((float)atof(arg));
	}
}

void con_weather()
{
	char* arg = strtok(NULL, " ");
	if (arg)
	{
		pNetGame->SetWeather(atoi(arg));
	}
}*/

void con_loadfs()
{
	WORD wTemp = wRconUser;
	// Stop sending all the filterscripts prints to the user if they used in-game RCON
	wRconUser = INVALID_ID;
	char* arg = strtok(NULL, "");
	if (arg)
	{
		if(!pNetGame->GetFilterScripts()->LoadFilterScript(arg))
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' load failed.", arg);
		}
		else
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' loaded.", arg);
		}
	}
}

void con_reloadfs()
{
	WORD wTemp = wRconUser;
	// Stop sending all the filterscripts prints to the user if they used in-game RCON
	wRconUser = INVALID_ID;
	char* arg = strtok(NULL, "");
	if (arg)
	{
		if(pNetGame->GetFilterScripts()->UnloadOneFilterScript(arg))
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' unloaded.", arg);
		}
		else
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' unload failed.", arg);
		}

		wRconUser = INVALID_ID;
		if(!pNetGame->GetFilterScripts()->LoadFilterScript(arg))
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' load failed'.", arg);
		}
		else
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' loaded.", arg);
		}
	}
}

void con_unloadfs()
{
	WORD wTemp = wRconUser;
	// Stop sending all the filterscripts prints to the user if they used in-game RCON
	wRconUser = INVALID_ID;
	char* arg = strtok(NULL, "");
	if (arg)
	{
		if(pNetGame->GetFilterScripts()->UnloadOneFilterScript(arg))
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' unloaded.", arg);
		}
		else
		{
			wRconUser = wTemp;
			logprintf("  Filterscript '%s.amx' unload failed.", arg);
		}
	}
}

static void con_clear()
{
#ifdef WIN32
	DWORD mode;
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hnd, &mode);
	SetConsoleMode(hnd, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
	printf("\033[2J\033[1;1H");
#ifdef WIN32
	SetConsoleMode(hnd, mode);
#endif
}

#define CON_CMDFLAG_DEBUG		1
#define CON_CMDFLAG_HIDDEN		2

struct ConsoleCommand_s
{
	char CmdName[255];
	DWORD CmdFlags;
	void (*CmdFunc)();
} ConsoleCommands[] = {
	{"echo",		0,	con_echo},
	{"exec",		0,	con_exec},
	{"cmdlist",		0,	con_cmdlist},
	{"varlist",		0,	con_varlist},
	{"exit",		0,	con_exit},
	{"kick",		0,	con_kick},
	{"ban",			0,	con_ban},
	{"gmx",			0,	con_gmx},
	{"changemode",	0,	con_changemode},
	{"say",			0,	con_say},
	{"reloadbans",	0,	con_reloadbans},
	{"reloadlog",	0,	con_reloadlog},
	{"players",		0,	con_players},
	{"banip",		0,	con_banip},
	{"unbanip",		0,	con_unbanip},
	//{"gravity",		0,	con_gravity},
	//{"weather",		0,	con_weather},
	{"loadfs",		0,	con_loadfs},
	{"unloadfs",	0,	con_unloadfs},
	{"reloadfs",	0,	con_reloadfs},
	{"cls", 0, con_clear},
};

void con_cmdlist()
{
	logprintf("Console Commands:");
	for (int i=0; i<ARRAY_SIZE(ConsoleCommands); i++)
	{
		logprintf("  %s%s", ConsoleCommands[i].CmdName, (ConsoleCommands[i].CmdFlags & CON_CMDFLAG_DEBUG)?" (DEBUG)":"");
	}
	logprintf("");
}

char* strrtrim(char* str)
{
	for (int i=strlen(str)-1; i>=0; i--)
	{
		if ((str[i] == ' ') || (str[i] == '\t'))
		{
			str[i] = 0;
		} else {
			break;
		}
	}
	return str;
}

CConsole::CConsole()
{

}

CConsole::~CConsole()
{
	ConsoleVariables.clear();
}

ConsoleVariable_s* CConsole::FindVariable(char* pVarName)
{
	if (pVarName == 0)
		return 0;

	char VarName[255];
	strncpy(VarName, pVarName, 255);
	strlwr(VarName);

	StringConvarMap::iterator itor;
	itor = ConsoleVariables.find(VarName);
	if (itor != ConsoleVariables.end())
	{
		return itor->second;
	}
	return NULL;
}

void CConsole::PrintVariableList()
{
	char val[1034]; // Max str size of 1024 + the (string) tag.
	logprintf("Console Variables:");
	StringConvarMap::iterator itor;
	for (itor = ConsoleVariables.begin(); itor != ConsoleVariables.end(); itor++)
	{
		val[0] = 0;
		switch (itor->second->VarType)
		{
			case CON_VARTYPE_FLOAT:
				sprintf(val, "%f  (float)", *(float*)itor->second->VarPtr);
				break;
			case CON_VARTYPE_INT:
				sprintf(val, "%d  (int)", *(int*)itor->second->VarPtr);
				break;
			case CON_VARTYPE_BOOL:
				sprintf(val, "%d  (bool)", *(bool*)itor->second->VarPtr);
				break;
			case CON_VARTYPE_STRING:
				sprintf(val, "\"%s\"  (string)", (char*)itor->second->VarPtr);
				break;
		}
		const char* VarName = itor->first.c_str();
		logprintf("  %s\t%s= %s%s%s%s", VarName, (strlen(VarName)<6)?"\t":"", val,
			(itor->second->VarFlags & CON_VARFLAG_READONLY)?" (read-only)":"",
			(itor->second->VarFlags & CON_VARFLAG_DEBUG)?" (debug)":"",
			(itor->second->VarFlags & CON_VARFLAG_RULE)?" (rule)":"");
	}
	logprintf("");
}

void CConsole::AddVariable(char* pVarName, CON_VARTYPE VarType, DWORD VarFlags, void* VarPtr,
						   VARCHANGEFUNC VarChangeFunc)
{
	char VarName[255];
	strncpy(VarName, pVarName, 255);
	strlwr(VarName);

	if (FindVariable(VarName) != NULL)
	{
		logprintf("Unable to add console variable '%s'. Already exists.", VarName);
		return;
	}

	ConsoleVariable_s* ConVar = new ConsoleVariable_s;
	ConVar->VarType = VarType;
	ConVar->VarFlags = VarFlags;
	ConVar->VarPtr = VarPtr;
	ConVar->VarChangeFunc = VarChangeFunc;

	if (VarChangeFunc)
		VarChangeFunc();

	ConsoleVariables.insert(StringConvarMap::value_type(VarName, ConVar));
}

char* CConsole::AddStringVariable(char* pVarName, DWORD VarFlags, char* pInitStr,
								 VARCHANGEFUNC VarChangeFunc)
{
	char* str;
	if (!pInitStr)
	{
		// Empty string
		str = (char*)malloc(1);
		str[0] = 0;
	} else {
		str = (char*)malloc(strlen(pInitStr)+1);
		strcpy(str, pInitStr);
	}
	AddVariable(pVarName, CON_VARTYPE_STRING, VarFlags, (void*)str, VarChangeFunc);
	return str;
}

void CConsole::RemoveVariable(char* pVarName)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_STRING)
		{
			if (ConVar->VarPtr != NULL)
				free(ConVar->VarPtr);
		}
		char VarName[255];
		strncpy(VarName, pVarName, 255);
		strlwr(VarName);
		SAFE_DELETE(ConVar);
		ConsoleVariables.erase(VarName);
	}
}

void CConsole::ModifyVariableFlags(char* pVarName, DWORD VarFlags)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		ConVar->VarFlags = VarFlags;
	}
}

char* CConsole::GetStringVariable(char* pVarName)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_STRING)
			return (char*)ConVar->VarPtr;
	}
	return NULL;
}

void CConsole::SetStringVariable(char* pVarName, char* pString)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_STRING)
		{
			if (ConVar->VarPtr != NULL)
				free(ConVar->VarPtr);

			char* str = (char*)malloc(strlen(pString)+1);
			strcpy(str, pString);
			ConVar->VarPtr = str;
		}
	}
}

float CConsole::GetFloatVariable(char* pVarName)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_FLOAT)
			return *(float*)ConVar->VarPtr;
	}
	return 0.0f;
}

void CConsole::SetFloatVariable(char* pVarName, float fFloat)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_FLOAT)
			*(float*)ConVar->VarPtr = fFloat;
	}
}


int CConsole::GetIntVariable(char* pVarName)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_INT)
			return *(int*)ConVar->VarPtr;
	}
	return 0;
}

void CConsole::SetIntVariable(char* pVarName, int iInt)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_INT)
			*(int*)ConVar->VarPtr = iInt;
	}
}

bool CConsole::GetBoolVariable(char* pVarName)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_BOOL)
			return *(bool*)ConVar->VarPtr;
	}
	return false;
}

void CConsole::SetBoolVariable(char* pVarName, bool bBool)
{
	ConsoleVariable_s* ConVar = FindVariable(pVarName);
	if (ConVar != NULL)
	{
		if (ConVar->VarType == CON_VARTYPE_BOOL)
			*(bool*)ConVar->VarPtr = bBool;
	}
}

void CConsole::SendRules(SOCKET s, char* data, const sockaddr_in* to, int tolen)
{
	const char* VarName;
	char VarValue[1024];

	StringConvarMap::iterator itor;

	WORD wRuleCount = 0;
	for (itor = ConsoleVariables.begin(); itor != ConsoleVariables.end(); itor++)
		if (itor->second->VarFlags & CON_VARFLAG_RULE)
			wRuleCount++;

	char* newdata = (char*)malloc(13 + (wRuleCount * 62)); // malloc without a free
	char* keep_ptr = newdata;
	// Previous Data
	memcpy(newdata, data, 11);
	newdata += 11;

	// Player Count
	memcpy(newdata, &wRuleCount, sizeof(WORD));
	newdata += sizeof(WORD);

	BYTE byteStrLen;

	for (itor = ConsoleVariables.begin(); itor != ConsoleVariables.end(); itor++)
	{
		if (itor->second->VarFlags & CON_VARFLAG_RULE)
		{
			VarName = itor->first.c_str();
			switch (itor->second->VarType)
			{
				case CON_VARTYPE_FLOAT:
					sprintf(VarValue, "%f", *(float*)itor->second->VarPtr);
					break;
				case CON_VARTYPE_INT:
					sprintf(VarValue, "%d", *(int*)itor->second->VarPtr);
					break;
				case CON_VARTYPE_BOOL:
					sprintf(VarValue, "%d", *(bool*)itor->second->VarPtr);
					break;
				case CON_VARTYPE_STRING:
					strcpy(VarValue, (char*)itor->second->VarPtr);
					break;
			}

			byteStrLen = (BYTE)strlen(VarName);
			memcpy(newdata, &byteStrLen, sizeof(BYTE));
			newdata += sizeof(BYTE);
			memcpy(newdata, VarName, byteStrLen);
			newdata += byteStrLen;

			byteStrLen = (BYTE)strlen(VarValue);
			memcpy(newdata, &byteStrLen, sizeof(BYTE));
			newdata += sizeof(BYTE);
			memcpy(newdata, VarValue, byteStrLen);
			newdata += byteStrLen;
		}
	}

	sendto(s, keep_ptr, (int)(newdata - keep_ptr), 0, (sockaddr*)to, tolen);

	free(keep_ptr);
}

void CConsole::Execute(char* pExecLine)
{
	if (!pExecLine) return;

	// Ya can't strtok on a read-only string!
	char cpy[256];
	memset(cpy, 0, sizeof cpy); 
	strncpy(cpy, pExecLine, 255);
	char* tmp = strtok(cpy, " ");
	if (!tmp)
		return;
	char* cmd = strlwr(tmp);

	for (int i=0; i<ARRAY_SIZE(ConsoleCommands); i++)
	{
		if (stricmp(cmd, ConsoleCommands[i].CmdName) == 0)
		{
			if (ConsoleCommands[i].CmdFlags & CON_CMDFLAG_DEBUG)
			{
				#ifndef _DEBUG
					logprintf("Unable to execute command '%s'. Debug mode required.", ConsoleCommands[i].CmdName);
					return;
				#endif
			}
			ConsoleCommands[i].CmdFunc();
			return;
		}
	}

	ConsoleVariable_s* ConVar = FindVariable(cmd);
	if (ConVar != NULL)
	{
		bool readonly = (ConVar->VarFlags & CON_VARFLAG_READONLY) == CON_VARFLAG_READONLY;
		if (ConVar->VarFlags & CON_VARFLAG_DEBUG)
		{
			#ifndef _DEBUG
				logprintf("Unable to access variable '%s'. Debug mode required.", cmd);
				return;
			#endif
		}
		bool bChangedVar = false;
		char* arg = strtok(NULL, " ");
		switch (ConVar->VarType)
		{
			case CON_VARTYPE_FLOAT:
				if ((arg) && (!readonly))
				{
					*(float*)ConVar->VarPtr = (float)atof(arg);
					bChangedVar = true;
				} else {
					logprintf("%s = %f  (float%s)", cmd, *(float*)ConVar->VarPtr, readonly?", read-only":"");
				}
				break;
			case CON_VARTYPE_INT:
				if ((arg) && (!readonly))
				{
					*(int*)ConVar->VarPtr = atoi(arg);
					bChangedVar = true;
				} else {
					logprintf("%s = %d  (int%s)", cmd, *(int*)ConVar->VarPtr, readonly?", read-only":"");
				}
				break;
			case CON_VARTYPE_BOOL:
				if ((arg) && (!readonly))
				{
					bool bValue = true;
					switch (arg[0])
					{
					case 'T': case 't':
					case 'Y': case 'y':
					case '1':
						bValue = true;
						break;
					case 'F': case 'f':
					case 'N': case 'n':
					case '0':
						bValue = false;
						break;
					case 'O': case 'o':
						if (arg[1] == 'N' || arg[1] == 'n')
							bValue = true;
						if (arg[1] == 'F' || arg[1] == 'f')
							bValue = false;
						break;
					default:
						bValue = (atoi(arg) > 0);
					}
					*(bool*)ConVar->VarPtr = bValue;
					bChangedVar = true;
				} else {
					logprintf("%s = %d  (bool%s)", cmd, *(bool*)ConVar->VarPtr, readonly?", read-only":"");
				}
				break;
			case CON_VARTYPE_STRING:
				if ((arg) && (!readonly))
				{
					if (ConVar->VarPtr != NULL)
						free(ConVar->VarPtr);

					char* str;
					char* ext = strtok(NULL, "");
					if (ext)
					{
						str = (char*)malloc(strlen(arg)+strlen(ext)+2);
						strcpy(str, arg);
						strcat(str, " ");
						strrtrim(ext);
						strcat(str, ext);
					} else {
						str = (char*)malloc(strlen(arg)+1);
						strcpy(str, arg);
					}
					if (strlen(str) > 1023) str[1022] = 0;
					ConVar->VarPtr = str;
					bChangedVar = true;
				} else {
					logprintf("%s = \"%s\"  (string%s)", cmd, (char*)ConVar->VarPtr, readonly?", read-only":"");
				}
				break;
		}
		if (bChangedVar)
		{
			if (ConVar->VarChangeFunc)
			{
				ConVar->VarChangeFunc();
			}
		}
		return;
	}

	if (pNetGame->GetGameMode() != NULL && pNetGame->GetFilterScripts() != NULL) {
		if (!pNetGame->GetGameMode()->OnRconCommand(pExecLine) ||
			!pNetGame->GetFilterScripts()->OnRconCommand(pExecLine))
		{
			logprintf("Unknown command or variable:\n  %s", cmd);
		}
	}
}