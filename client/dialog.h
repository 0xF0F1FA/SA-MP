
#pragma once

#define MAX_CAPTION 64
#define MAX_INPUTTEXT 128

#define DIALOG_STYLE_MSGBOX 0
#define DIALOG_STYLE_INPUT 1
#define DIALOG_STYLE_LIST 2
#define DIALOG_STYLE_PASSWORD 3
#define DIALOG_STYLE_TABLIST 4
#define DIALOG_STYLE_TABLIST_HEADERS 5

class CDialog
{
public:
	CDialog(IDirect3DDevice9* pDevice);
	~CDialog();

	bool MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool IsCandidateActive();
	void ResetDialogControls(CDXUTDialog* pDialogUI);
	void Show(int iID, int iStyle, char* szCaption, char* szInfo, char* szButton1, char* szButton2, bool bNotify);
	void Hide();
	void Draw();
	int GetTextWidth(char* szText);
	int GetFontHeight();
	void SendResponseNotification(BYTE byteResponse);
	void GetRect(RECT* rect);

	void sub_100700D0();
	void sub_1006EF40();
	void sub_1006F630(char* szContent, SIZE* size);

	static VOID CALLBACK OnEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
	
	IDirect3DDevice9* m_pDevice;
	int m_iScreenOffsetX;
	int m_iScreenOffsetY;
	int m_iWidth;
	int m_iHeight;
	int m_iBtnWidth;
	int m_iBtnHeight;
	CDXUTDialog* m_pDialog;
	CDXUTListBox* m_pListBox;
	CDXUTIMEEditBox* m_pEditBox;
	bool m_bVisible;
	int m_iStyle;
	int m_iID;
	char* m_szContent;
	int m_iContentSizeX;
	int m_iContentSizeY;
	char m_szCaption[MAX_CAPTION+1];
	bool m_bNotify;
};
