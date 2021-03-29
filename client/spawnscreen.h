//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#pragma once

//----------------------------------------------------

#define ID_CONTROL_LEFT		1
#define ID_CONTROL_RIGHT	2
#define ID_CONTROL_SPAWN	3

class CSpawnScreen
{
private:
	CDXUTDialog* m_pDialog;

public:
	void SetupUI();
	void ToggleVisibility(bool bVisible);
	void MsgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static VOID CALLBACK OnEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
	void Draw();

	bool IsVisible() { return m_pDialog->GetVisible(); };

	CSpawnScreen();
	~CSpawnScreen();

};

//----------------------------------------------------
// EOF