//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2007 SA:MP Team
//
//----------------------------------------------------------

// Basic Task Control Class

#pragma once

class CPlayerPed;

class CTask
{
protected:
	bool m_bSelfCreated;

public:
	CPlayerPed *m_pPlayerPed;
	BYTE *m_pTaskType;

	CTask();
	CTask(DWORD dwSize);
	CTask(BYTE *pTaskType);
	virtual ~CTask();

	void Create(DWORD dwSize);
	void Create(BYTE *pTaskType);
	virtual CTask* CreateCopy();	
	virtual void Destroy();

	virtual void ApplyToPed(CPlayerPed *pPed);

	virtual DWORD GetID();

	virtual bool IsDestroyed();
	virtual BOOL IsSimple();

};

class CTaskJetpack :
	public CTask
{
public:
	CTaskJetpack();
	CTaskJetpack(BYTE *pTaskType);
	~CTaskJetpack();
};

class CTaskTakeDamageFall :
	public CTask
{
public:
	CTaskTakeDamageFall(DWORD dwFallType, DWORD dwNum);
};