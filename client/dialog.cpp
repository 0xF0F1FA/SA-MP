
#include "main.h"

#define IDC_DLGEDITBOX 19
#define IDC_DLGBUTTON1 20
#define IDC_DLGBUTTON2 21

CDialog::CDialog(IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice;
	m_pDialog = NULL;
	m_bVisible = false;
	m_szContent = NULL;
	m_iID = 0;
	m_iStyle = 0;
	m_iWidth = 600;
	m_iHeight = 300;
	m_iBtnWidth = 100;
	m_iBtnHeight = 30;
	m_pListBox = NULL;
	m_pEditBox = NULL;
	m_bNotify = false;

	//memset(m_137, 0, 516);
}

CDialog::~CDialog()
{
}

int FormatDialogStr1(char* szSrc, char* szDest, int iLen)
{
	return 0;
}

bool CDialog::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CDXUTIMEEditBox::StaticMsgProc(uMsg, wParam, lParam) != false;
}

void CDialog::ResetDialogControls(CDXUTDialog* pDialogUI)
{
	m_pDialog = new CDXUTDialog();
	if (!m_pDialog) return;

	m_pDialog->Init(pDialogResourceManager);
	m_pDialog->SetCallback(CDialog::OnEvent);
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

	m_pDialog->AddButton(IDC_DLGBUTTON1, "BUTTON1", 10, 5, m_iBtnWidth, m_iBtnHeight);
	m_pDialog->AddButton(IDC_DLGBUTTON2, "BUTTON2", 110, 5, m_iBtnWidth, m_iBtnHeight);
	
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

void CDialog::sub_100700D0()
{

}

void CDialog::Show(int iID, int iStyle, char* szCaption, char* szInfo,
	char* szButton1, char* szButton2, bool bNotify)
{
	if(iID < 0) {
		if(m_bVisible) {
			Hide();
		}
		return;
	}

	if(pCmdWindow && pCmdWindow->isEnabled()) {
		pCmdWindow->Disable();
	}

	m_iID = iID;
	m_iStyle = iStyle;
	m_bNotify = bNotify;

	memset(m_szCaption, 0, sizeof(m_szCaption));
	strncpy(m_szCaption, szCaption, MAX_CAPTION);

	if (m_szContent) {
		free(m_szContent);
	}
	m_szContent = (char*)calloc(1, strlen(szInfo) + 64);
	strcpy(m_szContent, szInfo);


	if(szCaption && szCaption[0] != 0) {
		//pDefaultFont->MeasureSmallerText();
	}












	if (iID >= 0)
	{
		SIZE ContentSize;
		pDefaultFont->MeasureSmallerText(&ContentSize, m_szContent, DT_EXPANDTABS);
		m_iContentSizeX = ContentSize.cx;
		m_iContentSizeY = ContentSize.cy;

		if (szCaption[0] != '\0')
		{
			SIZE size;
			pDefaultFont->MeasureText(&size, "Y", DT_LEFT);
			strcpy(m_pDialog->GetCaptionText(), szCaption);
			m_pDialog->EnableCaption(true);
			m_pDialog->SetCaptionHeight(size.cy + 4);
		}

		switch (m_iStyle)
		{
		case DIALOG_STYLE_MSGBOX:
		{
			m_iWidth = m_iContentSizeX + 40;
			m_iHeight = m_iContentSizeY + m_pDialog->GetCaptionHeight() + m_iBtnHeight + 25;

			m_pEditBox->SetVisible(false);
			m_pEditBox->SetEnabled(false);

			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		}
		case DIALOG_STYLE_INPUT:
		{
			m_iWidth = m_iContentSizeX + 40;
			m_iHeight = m_iBtnHeight
				+ m_iContentSizeY
				+ (int)(m_pDialog->GetFont(1)->nHeight * 1.6f + 14.0f)
				+ m_pDialog->GetCaptionHeight()
				+ 25;

			m_pEditBox->SetVisible(true);
			m_pEditBox->SetEnabled(true);
			m_pEditBox->SetCharactersHidden(false);
			m_pEditBox->SetText("");

			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		}
		case DIALOG_STYLE_PASSWORD:
		{
			m_iWidth = m_iContentSizeX + 40;
			m_iHeight = m_iBtnHeight
				+ m_iContentSizeY
				+ (int)(m_pDialog->GetFont(1)->nHeight * 1.6f + 14.0f)
				+ m_pDialog->GetCaptionHeight()
				+ 25;

			m_pEditBox->SetVisible(true);
			m_pEditBox->SetEnabled(true);
			m_pEditBox->SetCharactersHidden(true);
			m_pEditBox->SetText("");

			m_pListBox->SetVisible(false);
			m_pListBox->SetEnabled(false);
			break;
		}
		case DIALOG_STYLE_LIST:
		{
			break;
		}
		case DIALOG_STYLE_TABLIST:
		{
			break;
		}
		case DIALOG_STYLE_TABLIST_HEADERS:
		{
			break;
		}
		default:
			break;
		}

		if (szButton2[0] != 0)
		{
			m_pDialog->GetButton(IDC_DLGBUTTON1)->SetText(szButton1);
			m_pDialog->GetButton(IDC_DLGBUTTON2)->SetText(szButton2);

			m_pDialog->GetButton(IDC_DLGBUTTON2)->SetVisible(true);
		}
		else
		{
			m_pDialog->GetButton(IDC_DLGBUTTON1)->SetText(szButton1);
			m_pDialog->GetButton(IDC_DLGBUTTON2)->SetText("");

			m_pDialog->GetButton(IDC_DLGBUTTON2)->SetVisible(false);
		}

		m_pDialog->SetVisible(true);

		if (m_iStyle == DIALOG_STYLE_INPUT || m_iStyle == DIALOG_STYLE_PASSWORD)
		{
			m_pDialog->RequestFocus(m_pEditBox);
		}
		if (m_iStyle == DIALOG_STYLE_LIST || m_iStyle == DIALOG_STYLE_TABLIST || m_iStyle == DIALOG_STYLE_TABLIST_HEADERS)
		{
			m_pDialog->RequestFocus(m_pListBox);
			m_pListBox->SelectItem(0);
		}

		sub_1006EF40();
		m_bVisible = true;
	}
	else if (m_bVisible)
	{
		Hide();
	}
}

void CDialog::Hide()
{
	pGame->ToggleKeyInputsDisabled(0);
	m_pDialog->SetVisible(false);
	m_bVisible = false;
}

bool CDialog::IsCandidateActive()
{
	if (m_pEditBox)
		return CDXUTIMEEditBox::IsCandidateActive();

	return false;
}

int CDialog::GetTextWidth(char* szText)
{
	ID3DXFont* pFont = m_pDialog->GetFont(1)->pFont;
	RECT rect = {0,0,0,0};

	if (szText && szText[0] != '\0' && pFont)
	{
		char szBuffer[128];
		strcpy_s(szBuffer, szText);
		RemoveColorEmbedsFromString(szBuffer);
		
		pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_EXPANDTABS | DT_NOCLIP | DT_CALCRECT, -1);
		
		return rect.right - rect.left;
	}
	return -1;
}

int CDialog::GetFontHeight()
{
	return m_pDialog->GetFont(1)->nHeight;
}

void CDialog::Draw()
{
	if (!m_bVisible) return;

	if (!pCmdWindow->isEnabled())
	{
		pGame->ToggleKeyInputsDisabled(2);
		if (m_pDialog)
		{
			m_pDialog->OnRender(10.0f);
			
			RECT rect;
			GetRect(&rect);

			if (!m_szContent || m_iStyle != DIALOG_STYLE_INPUT)
			{
				if (m_iStyle == DIALOG_STYLE_TABLIST_HEADERS)
				{

				}
			}
			else
			{
				rect.left += 20;
				rect.top += m_pDialog->GetCaptionHeight() + 5;
				pDefaultFont->RenderSmallerText(0,m_szContent,rect, 
					DT_LEFT|DT_EXPANDTABS|DT_NOCLIP,D3DCOLOR_ARGB(255,169,196,230),true);
			}
		}
	}
}

void CDialog::SendResponseNotification(BYTE byteResponse)
{
	if (!m_bVisible) return;

	char szInputText[MAX_INPUTTEXT];
	memset(szInputText,0,sizeof(szInputText));

	BYTE byteInputTextLen = 0;
	int iListItem = -1;
	

	if (m_iStyle == DIALOG_STYLE_LIST ||
		m_iStyle == DIALOG_STYLE_TABLIST ||
		m_iStyle == DIALOG_STYLE_TABLIST_HEADERS)
	{

	}
	else if (m_iStyle== DIALOG_STYLE_INPUT ||
		m_iStyle == DIALOG_STYLE_PASSWORD)
	{

	}

	if (m_bNotify)
	{
		RakNet::BitStream bsSend;
		bsSend.Write((WORD)m_iID);
		bsSend.Write(byteResponse);
		bsSend.Write(byteInputTextLen);
		if (byteInputTextLen)
			bsSend.Write(szInputText, byteInputTextLen);

		//pNetGame->Send(RPC_DialogResponse, &bsSend);
	}

	Hide();
}

void CDialog::sub_1006EF40()
{
	int iNewWidth = 2 * m_iBtnWidth + 30;
	if (m_iWidth < iNewWidth)
		m_iWidth = iNewWidth;

	m_pDialog->SetSize(m_iWidth, m_iHeight);

	RECT rect;
	GetClientRect(pGame->GetMainWindowHwnd(), &rect);

	m_iScreenOffsetX = rect.right / 2 - m_iWidth / 2;
	m_iScreenOffsetY = rect.bottom / 2 - m_iHeight / 2;

	m_pDialog->SetLocation(m_iScreenOffsetX, m_iScreenOffsetY);

	int iHeight = m_iHeight - m_pDialog->GetCaptionHeight();

	if (m_pDialog->GetButton(IDC_DLGBUTTON2)->GetVisible())
	{
		CDXUTButton* pBtn1 = m_pDialog->GetButton(IDC_DLGBUTTON1);
		pBtn1->SetLocation(m_iWidth / 2 - m_iBtnWidth - 10, iHeight - m_iBtnHeight - 5 );

		CDXUTButton* pBtn2 = m_pDialog->GetButton(IDC_DLGBUTTON2);
		pBtn2->SetLocation(m_iWidth / 2 + 10, iHeight - m_iBtnHeight - 5);
	}
	else
	{
		CDXUTButton* pBtn1 = m_pDialog->GetButton(IDC_DLGBUTTON1);
		pBtn1->SetLocation(m_iWidth / 2 - m_iBtnWidth / 2, iHeight - m_iBtnHeight - 5);
	}

	//m_pDialog->GetFont(0)->nHeight * -1.5f;
	//m_pEditBox->SetSize();

	//m_pDialog->GetFont(0)->nHeight * 1.5f;
	//m_pEditBox->SetLocation(5, - 24);
}

void CDialog::GetRect(RECT* rect)
{
	rect->left = m_iScreenOffsetX;
	rect->right = m_iScreenOffsetX + m_iWidth;
	rect->top = m_iScreenOffsetY;
	rect->bottom = m_iScreenOffsetY + m_iHeight;
}

void CDialog::sub_1006F630(char* szContent, SIZE* size)
{
	m_pListBox->RemoveAllItems();
	m_pListBox->m_nColumns = 0;

	char szListText[256];
	while (true)
	{
		memset(szListText, 0, sizeof(szListText));
	}

	if (size)
	{
		size->cx;
		size->cy;
	}
}

VOID CALLBACK CDialog::OnEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	if (nControlID == IDC_DLGEDITBOX)
	{
		if (nEvent == EVENT_EDITBOX_STRING)
		{
			if (pDialog && !pCmdWindow->IsCandidateActive())
				pDialog->SendResponseNotification(1);
		}
	}
	else if (nControlID)
	{

	}
}
