
#pragma once

class CDialog
{
private:
	IDirect3DDevice9* m_pDevice;
	CDXUTDialog* m_pDialog;
	CDXUTListBox* m_pListBox;
	CDXUTIMEEditBox* m_pEditBox;
	int m_iWidth;
	int m_iHeight;
	int m_i20;
	int m_i24;
	int m_iScreenOffsetX;
	int m_iScreenOffsetY;
	bool m_bVisible;
	bool m_bSend;
	int m_iID;
	int m_iStyle;
	char m_szCaption[MAX_DIALOG_CAPTION];
	char* m_szInfo;
	SIZE m_InfoSize;

public:
	CDialog(IDirect3DDevice9* pDevice);

	void ResetDialogControls();
	bool MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LONG GetTextWidth(char* szText);
	LONG GetFontHeight();
	void UpdateLayout();
	bool IsCandicateActive();
	void GetRect(RECT* rect);
	void Draw();
	void Show(int iID, int iStyle, char* szCaption, char* szText,
		char* szButton1, char* szButton2, bool bSend);
	void Hide();
};
