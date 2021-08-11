
#include "main.h"

#define IDC_DLGEDITBOX 19
#define IDC_DLGBUTTON1 20
#define IDC_DLGBUTTON2 21

VOID CALLBACK OnDialogEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

char* TokensStringUntilNewLine(char* szSrc, char* szDest, int iMaxLen)
{
	int x = 0;
	for (; x < iMaxLen && szSrc[x] != '\0'; x++)
	{
		if (szSrc[x] == '\n')
			break;

		szDest[x] = szSrc[x];
	}
	szDest[x] = '\0';

	return szSrc[x] != '\0' ? &szSrc[x] : NULL;
}

CDialog::CDialog(IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice;
	m_pDialog = NULL;
	m_pListBox = NULL;
	m_pEditBox = NULL;
	m_iWidth = 600;
	m_iHeight = 300;
	m_i20 = 100;
	m_i24 = 30;
	m_bVisible = false;
}

void CDialog::ResetDialogControls()
{
	SAFE_DELETE(m_pDialog);

	m_pDialog = new CDXUTDialog();
	if (!m_pDialog) return;

	m_pDialog->Init(pDialogResourceManager);
	m_pDialog->SetCallback(OnDialogEvent);
	m_pDialog->SetLocation(0, 0);
	m_pDialog->SetSize(600, 300);
	m_pDialog->EnableMouseInput(true);
	m_pDialog->EnableKeyboardInput(true);
	m_pDialog->SetBackgroundColors(D3DCOLOR_ARGB(220, 5, 5, 5));
	m_pDialog->SetVisible(false);

	m_pListBox = new CDXUTListBox(m_pDialog);
	m_pDialog->AddControl(m_pListBox);
	m_pListBox->SetLocation(10, 10);
	m_pListBox->SetSize(m_iWidth, m_iHeight - 100);
	m_pListBox->OnInit();
	m_pListBox->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB(200, 255, 255, 255));
	m_pListBox->m_nColumns = 0;
	m_pListBox->SetEnabled(false);
	m_pListBox->SetVisible(false);

	m_pDialog->AddButton(IDC_DLGBUTTON1, "BUTTON1", 10, 5, m_i20, m_i24);
	m_pDialog->AddButton(IDC_DLGBUTTON2, "BUTTON2", 110, 5, m_i20, m_i24);

	m_pDialog->AddIMEEditBox(IDC_DLGEDITBOX, "", 10, 175, 570, 40, true, &m_pEditBox);
	if (pConfigFile->GetInt("ime"))
	{
		CDXUTIMEEditBox::EnableImeSystem(true);
		CDXUTIMEEditBox::StaticOnCreateDevice();
	}

	m_pEditBox->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB(240, 5, 5, 5));
	m_pEditBox->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
	m_pEditBox->SetCaretColor(D3DCOLOR_ARGB(255, 150, 150, 150));
	m_pEditBox->SetSelectedBackColor(D3DCOLOR_ARGB(255, 185, 34, 40));
	m_pEditBox->SetSelectedTextColor(D3DCOLOR_ARGB(255, 10, 10, 15));
	m_pEditBox->SetEnabled(false);
	m_pEditBox->SetVisible(false);
}

bool CDialog::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CDXUTIMEEditBox::StaticMsgProc(uMsg, wParam, lParam) != 0;
}

LONG CDialog::GetTextWidth(char* szText)
{
	ID3DXFont* pFont = m_pDialog->GetFont(1)->pFont;

	//if(szText && strlen(szText) != 0 && pFont)
	if (szText && szText[0] != '\0' && pFont)
	{
		char szBuf[132];
		strcpy_s(szBuf, szText);
		RemoveColorEmbedsFromString(szBuf);

		RECT rect;
		pFont->DrawTextA(NULL, szBuf, -1, &rect, 1344, D3DCOLOR_ARGB(255,255,255,255));

		return rect.right - rect.left;
	}
	return -1;
}

LONG CDialog::GetFontHeight()
{
	return m_pDialog->GetFont(1)->nHeight;
}

void CDialog::UpdateLayout()
{
	int iSize = 2 * m_i20 + 30;
	if (m_iWidth < iSize)
		m_iWidth = iSize;

	m_pDialog->SetSize(m_iWidth, m_iHeight);

	RECT rect;
	GetClientRect(pGame->GetMainWindowHwnd(), &rect);
	m_iScreenOffsetX = (rect.right / 2) - (m_iWidth / 2);
	m_iScreenOffsetY = (rect.bottom / 2) - (m_iHeight / 2);
	m_pDialog->SetLocation(m_iScreenOffsetX, m_iScreenOffsetY);

	CDXUTControl* pButton1, *pButton2;
	pButton2 = m_pDialog->GetControl(IDC_DLGBUTTON2);
	int iFormHeight = m_iHeight - m_pDialog->GetCaptionHeight();
	if (pButton2->GetVisible())
	{
		pButton1 = m_pDialog->GetControl(IDC_DLGBUTTON1);
		pButton1->SetLocation(
			(m_iWidth / 2) - (m_i20 - 10),
			iFormHeight - m_i24 - 5);

		pButton2->SetLocation(
			(m_iWidth / 2) + 10,
			iFormHeight - m_i24 - 5);
	}
	else
	{
		pButton1 = m_pDialog->GetControl(IDC_DLGBUTTON1);

		pButton1->SetLocation(
			(m_iWidth / 2) - (m_i20 / 2),
			iFormHeight - m_i24 - 5);
	}

	LONG lHeight = LONG(m_pDialog->GetFont(0)->nHeight * -1.5f);
	m_pEditBox->SetSize(m_iWidth - 10, 14 - lHeight);
	lHeight = LONG(m_pDialog->GetFont(0)->nHeight * 1.5f);
	m_pEditBox->SetLocation(5, iFormHeight - lHeight - m_i24 - 24);
}

bool CDialog::IsCandicateActive()
{
	return (m_pEditBox && CDXUTIMEEditBox::IsCandidateActive());
}

void CDialog::GetRect(RECT* rect)
{
	rect->left = m_iScreenOffsetX;
	rect->right = m_iScreenOffsetX + m_iWidth;
	rect->top = m_iScreenOffsetY;
	rect->bottom = m_iScreenOffsetY + m_iHeight;
}

void CDialog::Draw()
{

	pGame->ToggleKeyInputsDisabled(2);

	if (!m_pDialog) return;

	m_pDialog->OnRender(10.0f);

	/*if ()
	{

	}
	else
	{

	}*/
}

void CDialog::Show(int iID, int iStyle, char* szCaption, char* szText,
	char* szButton1, char* szButton2, bool bSend)
{
	if (iID >= 0)
	{
		if (pCmdWindow && pCmdWindow->isEnabled())
			pCmdWindow->Disable();

		m_iID = iID;
		m_iStyle = iStyle;
		m_bSend = bSend;

		SecureZeroMemory(m_szCaption, MAX_DIALOG_CAPTION);
		strncpy_s(m_szCaption, szCaption, MAX_DIALOG_CAPTION);

		if (m_szInfo)
			free(m_szInfo);
		size_t len = strlen(szText) + 64;
		m_szInfo = (char*)calloc(1, len);
		strcpy_s(m_szInfo, len, szText);

		SIZE size;
		pDefaultFont->MeasureSmallerText(&m_InfoSize, m_szInfo, 64);
		//m_InfoSize.cx = size.cx;
		//m_InfoSize.cy = size.cy;

		//if(strlen(szCaption))
		if (szCaption && szCaption[0] != '\0')
		{
			pDefaultFont->MeasureSmallerText(&size, "Y", 0);
			strcpy_s(m_pDialog->GetCaptionText(), 256, szCaption);
			m_pDialog->EnableCaption(true);
			m_pDialog->SetCaptionHeight(size.cy + 4);
		}

		switch (m_iStyle)
		{
		case DIALOG_STYLE_MSGBOX:
			m_iWidth = m_InfoSize.cx + 40;
			m_iHeight = m_InfoSize.cy + m_pDialog->GetCaptionHeight() + m_i24 + 25;

			m_pEditBox->SetVisible(false);
			m_pEditBox->SetEnabled(false);
			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		case DIALOG_STYLE_INPUT:
			m_iWidth = m_InfoSize.cx + 40;
			m_iHeight = 0;

			m_pEditBox->SetVisible(true);
			m_pEditBox->SetEnabled(true);
			m_pEditBox->SetText("");
			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		case DIALOG_STYLE_PASSWORD:
			m_pEditBox->SetText("");
			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		case DIALOG_STYLE_LIST:
			break;
		case DIALOG_STYLE_TABLIST:
			break;
		case DIALOG_STYLE_TABLIST_HEADERS:
			break;
		}
	}
	else
		Hide();

} 

void CDialog::Hide()
{
	pGame->ToggleKeyInputsDisabled(0);
	m_pDialog->SetVisible(false);
	m_bVisible = false;
}

VOID CALLBACK OnDialogEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{

}
