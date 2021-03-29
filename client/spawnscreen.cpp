//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#include "main.h"

//----------------------------------------------------

CSpawnScreen::CSpawnScreen()
{
	m_pDialog = NULL;
	SetupUI();
}

//----------------------------------------------------

CSpawnScreen::~CSpawnScreen()
{
	ToggleVisibility(false);
	SAFE_DELETE(m_pDialog);
}

//----------------------------------------------------

void CSpawnScreen::Draw()
{
	if (m_pDialog) {
		if (m_pDialog->GetVisible()) {
			RECT rect;
			GetClientRect(pGame->GetMainWindowHwnd(), &rect);

			m_pDialog->SetLocation(
				rect.right / 2 - m_pDialog->GetWidth() / 2,
				rect.bottom - m_pDialog->GetHeight() - 50);
		}

		m_pDialog->OnRender(10.f);
	}
}

//----------------------------------------------------

void CSpawnScreen::SetupUI()
{
	// Destroy any initialized dialog 
	// Assigned controls to the dialog are destroyes also, with the dialog
	SAFE_DELETE(m_pDialog);

	m_pDialog = new CDXUTDialog;
	if (m_pDialog && pDialogResourceManager) {
		m_pDialog->Init(pDialogResourceManager);
		m_pDialog->SetCallback(CSpawnScreen::OnEvent);
		m_pDialog->SetLocation(0, 0);
		m_pDialog->SetSize(310, 40);
		m_pDialog->SetBackgroundColors(0x960A0A0A);
		m_pDialog->EnableMouseInput(true);
		m_pDialog->EnableKeyboardInput(false);
		m_pDialog->SetVisible(false);

		m_pDialog->AddButton(ID_CONTROL_LEFT, "<<", 10, 5, 90, 30, 0, false, NULL);
		m_pDialog->AddButton(ID_CONTROL_RIGHT, ">>", 110, 5, 90, 30, 0, false, NULL);
		m_pDialog->AddButton(ID_CONTROL_SPAWN, "Spawn", 210, 5, 90, 30, 0, false, NULL);
	}
}

void CSpawnScreen::MsgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pDialog && m_pDialog->GetVisible())
		m_pDialog->MsgProc(hwnd, uiMsg, wParam, lParam);
}

VOID CALLBACK CSpawnScreen::OnEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	if (pNetGame && nEvent == EVENT_BUTTON_CLICKED) {
		CLocalPlayer* pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		switch (nControlID) {
		case ID_CONTROL_LEFT:
		case ID_CONTROL_RIGHT:
		case ID_CONTROL_SPAWN:
			pLocalPlayer->ProcessClassSelection(nControlID);
		}
	}
}

void CSpawnScreen::ToggleVisibility(bool bVisible)
{
	if (m_pDialog) {
		m_pDialog->SetVisible(bVisible);
	}
}
