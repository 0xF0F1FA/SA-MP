/*

	SA:MP Multiplayer Modification
	Copyright 2004-2005 SA:MP Team

	file:
		playerpool.h
	desc:
		Player pool handling header file.

    Version: $Id: playerpool.h,v 1.12 2006/04/09 09:54:46 kyeman Exp $

*/

#ifndef SAMPSRV_PLAYERPOOL_H
#define SAMPSRV_PLAYERPOOL_H

//----------------------------------------------------

class CPlayerPool
{
private:
	
	bool	m_bPlayerSlotState[MAX_PLAYERS];
	CPlayer *m_pPlayers[MAX_PLAYERS];
	CHAR	m_szPlayerName[MAX_PLAYERS][MAX_PLAYER_NAME+1];
	CHAR	m_szPlayerSerial[MAX_PLAYERS][MAX_PLAYER_SERIAL+1];
	CHAR	m_szPlayerVersion[MAX_PLAYERS][MAX_PLAYER_VERSION+1];
	int 	m_iPlayerScore[MAX_PLAYERS];
	int		m_iPlayerMoney[MAX_PLAYERS];
	bool	m_bIsAnAdmin[MAX_PLAYERS];
	int		m_iVirtualWorld[MAX_PLAYERS];
	bool	m_bIsAnNPC[MAX_PLAYERS];
	int		m_iPlayerCount;
	int		m_iPoolSize;
	float	m_fLastTimerTime;

public:
	
	CPlayerPool();
	~CPlayerPool();

	bool Process(float fElapsedTime);
	bool New(WORD wPlayerID, PCHAR szPlayerName, char* szSerial, char* szVersion, bool bIsNPC = false);
	bool Delete(WORD wPlayerID, BYTE byteReason);
	void UpdatePoolSize();

	// Retrieve a player
	CPlayer* GetAt(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return NULL; }
		return m_pPlayers[wPlayerID];
	};

	// Find out if the slot is inuse.
	bool GetSlotState(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return FALSE; }
		return m_bPlayerSlotState[wPlayerID];
	};
	};

	PCHAR GetPlayerName(WORD wPlayerID) {
		if(wPlayerID >= MAX_PLAYERS) { return NULL; }
		return m_szPlayerName[wPlayerID];
	};

	PCHAR GetPlayerVersion(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return NULL; }
		return m_szPlayerVersion[wPlayerID];
	};

	PCHAR GetPlayerSerial(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return NULL; }
		return m_szPlayerSerial[wPlayerID];
	};

	int GetPlayerScore(BYTE bytePlayerID) {
		if(bytePlayerID >= MAX_PLAYERS) { return 0; }
		return m_iPlayerScore[bytePlayerID];
	};

	void SetPlayerScore(BYTE bytePlayerID, int iScore) {
		if(bytePlayerID >= MAX_PLAYERS) return;
		m_iPlayerScore[bytePlayerID] = iScore;
	};

	void SetPlayerName(WORD wPlayerID, PCHAR szName) {
		strcpy(m_szPlayerName[wPlayerID], szName);
	}

	int GetPlayerMoney(WORD wPlayerID) {
		if(wPlayerID >= MAX_PLAYERS) { return 0; }
		return m_iPlayerMoney[wPlayerID];
	};

	void SetPlayerMoney(WORD wPlayerID, int iMoney) {
		if(wPlayerID >= MAX_PLAYERS) return;
		m_iPlayerMoney[wPlayerID] = iMoney;
	};

	void ResetPlayerScoresAndMoney() {
		memset(&m_iPlayerScore[0],0,sizeof(int) * MAX_PLAYERS);
		memset(&m_iPlayerMoney[0],0,sizeof(int) * MAX_PLAYERS);
		memset(&m_iVirtualWorld[0],0,sizeof(int) * MAX_PLAYERS);
	};
	
	void SetPlayerVirtualWorld(WORD wPlayerID, int iVirtualWorld);
	
	int GetPlayerVirtualWorld(WORD wPlayerID) {
		if (wPlayerID >= MAX_PLAYERS) { return 0; }
		return m_iVirtualWorld[wPlayerID];		
	};

	void SetAdmin(WORD wPlayerID, bool bAdmin = true) { m_bIsAnAdmin[wPlayerID] = bAdmin; };
	bool IsAdmin(WORD wPlayerID) { return m_bIsAnAdmin[wPlayerID]; };

	bool IsPlayerNPC(WORD wPlayerID) {
		return m_bIsAnNPC[wPlayerID];
	}

	bool IsValidID(WORD wPlayerID) { return wPlayerID < MAX_PLAYERS; };

	void InitPlayersForPlayer(BYTE bytePlayerID);
	void InitSpawnsForPlayer(BYTE bytePlayerID);

	BYTE GetKillType(BYTE byteWhoKilled, BYTE byteWhoDied);

	float GetDistanceFromPlayerToPlayer(WORD wPlayer1, WORD wPlayer2);
	float GetDistanceSquaredFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2);
	bool  IsNickInUse(PCHAR szNick);

	int GetPlayerCount();
	int GetNPCCount();

	int GetCount() { return m_iPlayerCount; };
	int GetPoolSize() { return m_iPoolSize; };

	void DestroyActorForPlayers(unsigned short usActorID);

	void UpdateTimersForAll();

	void DeactivateAll();
<<<<<<< Updated upstream
	int GetLastPlayerId() const { return m_iLastPlayerId; }
=======
>>>>>>> Stashed changes
};

//----------------------------------------------------

#endif