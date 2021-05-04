//
// Version: $Id: scoreboard.h,v 1.6 2006/04/15 20:40:13 spookie Exp $
//

#pragma once

class CScoreBoard
{
private:
	IDirect3DDevice9* m_pDevice;
	CDXUTDialog* m_pDialog;
	CDXUTListBox* m_pListBox;
	float m_fWidth;
	float m_fHeight;
	int m_iLastPlayerCount;
	bool m_bVisible;
	float m_fScreenOffsetX;
	float m_fScreenOffsetY;
	float m_fScalar;
	float m_fHeaderHeight;
	float m_fNameOffset;
	float m_fPingOffset;
	float m_fScoreOffset;
	DWORD m_dwSortType;

public:
	CScoreBoard(IDirect3DDevice9* pDevice);
	void CalcClientSize();
	void GetRect(RECT* rect);
	void ResetDialogControls();
	void ProcessClickEvent();
	void UpdateList();
	void Show();
	void Hide(bool bDisableControls);
	void Draw();
	void MsgProc(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);

	bool IsVisible() { return m_bVisible; };
};