//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: chatwindow.cpp,v 1.20 2006/05/08 14:31:50 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"

extern CGame *pGame;

//----------------------------------------------------

CChatWindow::CChatWindow(IDirect3DDevice9 *pD3DDevice, ID3DXFont *pFont)
{
	int x=0;

	m_pD3DDevice		= pD3DDevice;
	m_pD3DFont			= pFont;
	m_iEnabled			= CHAT_WINDOW_MODE_FULL;
	m_iCurrentPage		= 1;

	// Create a sprite to use when drawing text
	D3DXCreateSprite(pD3DDevice,&m_pChatTextSprite);

	// Init the chat window lines to 0
	while(x!=MAX_MESSAGES) {
		memset(&m_ChatWindowEntries[x],0,sizeof(CHAT_WINDOW_ENTRY));
		x++;
	}

	RECT rectSize;
	m_pD3DFont->DrawText(0,"Y",-1,&rectSize,DT_CALCRECT|DT_SINGLELINE|DT_LEFT,0xFF000000);
	m_lFontSizeY = rectSize.bottom - rectSize.top;

	m_dwChatTextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_dwChatInfoColor = D3DCOLOR_ARGB(255,0,200,200);
	m_dwChatDebugColor = D3DCOLOR_ARGB(255,244,164,25);
	m_dwChatBackgroundColor = D3DCOLOR_ARGB(255,0,0,0);
}

//----------------------------------------------------

CChatWindow::~CChatWindow() {}

//----------------------------------------------------

void CChatWindow::ResetDialogControls(CDXUTDialog *pGameUI)
{
	m_pGameUI = pGameUI;

	if(pGameUI) {
		//pGameUI->AddEditBox(IDC_CHATBACK,"",5,5,420,170,true,&m_pEditBackground);

		m_pScrollBar = new CDXUTScrollBar(pGameUI);        
		pGameUI->AddControl(m_pScrollBar);
		m_pScrollBar->SetVisible(true);
		m_pScrollBar->SetEnabled(true);
		m_pScrollBar->SetLocation(5,40);
        m_pScrollBar->SetSize(15,((m_lFontSizeY+1)*DISP_MESSAGES)-60);
		m_pScrollBar->SetTrackRange(0,(MAX_MESSAGES-DISP_MESSAGES)-1);
		m_pScrollBar->SetPageSize(5);
		m_pScrollBar->ShowItem(MAX_MESSAGES-1);

		/*
		m_pEditBackground->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB( 170, 20, 20, 20 ));
		m_pEditBackground->GetElement(1)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(2)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(3)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(4)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(5)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(6)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(7)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->GetElement(8)->TextureColor.Init(D3DCOLOR_ARGB( 200, 0, 0, 0 ));
		m_pEditBackground->SetEnabled(false);
		m_pEditBackground->SetVisible(true);*/

	}
}

//----------------------------------------------------

void CChatWindow::ScrollBarPosFromCurrentPage()
{
	if(m_pScrollBar) {
		if(m_iCurrentPage==1) m_pScrollBar->ShowItem(39);
		if(m_iCurrentPage==2) m_pScrollBar->ShowItem(29);
		if(m_iCurrentPage==3) m_pScrollBar->ShowItem(19);
		if(m_iCurrentPage==4) m_pScrollBar->ShowItem(9);
		if(m_iCurrentPage==5) m_pScrollBar->ShowItem(0);
	}
}

//----------------------------------------------------

void CChatWindow::PageUp()
{
	if(!m_iEnabled) return;

    m_iCurrentPage++;
	if(m_iCurrentPage > CHAT_WINDOW_PAGES) {
		m_iCurrentPage = CHAT_WINDOW_PAGES;
	} else {
		ScrollBarPosFromCurrentPage();
	}
	
}

//----------------------------------------------------

void CChatWindow::PageDown()
{
	if(!m_iEnabled) return;

    m_iCurrentPage--;
	if(m_iCurrentPage < 1) {
		m_iCurrentPage = 1;
	} else {
		ScrollBarPosFromCurrentPage();
	}
}

//----------------------------------------------------

void CChatWindow::Draw()
{
	DWORD dwColorChat=0;
	RECT rect;
	RECT rectSize;
	int x=0;
	int i=0;
	int iMessageAt;
		
	rect.top		= 10;
	rect.left		= 30;
	rect.bottom		= 110;
	rect.right		= 550;

	if(!m_iEnabled || m_iCurrentPage == 1) {
		m_pScrollBar->SetVisible(false);
	} else {
		m_pScrollBar->SetVisible(true);
	}

	iMessageAt = (m_iCurrentPage * DISP_MESSAGES) - 1;

	if(m_pD3DFont && m_iEnabled)
	{
		m_pChatTextSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );

		while(x!=DISP_MESSAGES) {

			switch(m_ChatWindowEntries[iMessageAt].eType) {

				case CHAT_TYPE_CHAT:

					i = strlen(m_ChatWindowEntries[iMessageAt].szNick);

					if(i) {
						m_pD3DFont->DrawText(0,m_ChatWindowEntries[iMessageAt].szNick,-1,&rectSize,DT_CALCRECT|DT_LEFT,0xFF000000);
						RenderText(m_ChatWindowEntries[iMessageAt].szNick,rect,m_ChatWindowEntries[iMessageAt].dwNickColor);
						rect.left = 35 + (rectSize.right - rectSize.left);
					}					

					RenderText(m_ChatWindowEntries[iMessageAt].szMessage,rect,m_ChatWindowEntries[iMessageAt].dwTextColor);
	
					break;

				case CHAT_TYPE_INFO:
				case CHAT_TYPE_DEBUG:

					RenderText(m_ChatWindowEntries[iMessageAt].szMessage,rect,m_ChatWindowEntries[iMessageAt].dwTextColor);
					break;
			}		

			rect.top+=m_lFontSizeY+1;
			rect.bottom= rect.top + m_lFontSizeY+1;
			rect.left = 30;
			
			iMessageAt--;
			x++;
		}
		m_lChatWindowBottom = rect.bottom;

		m_pChatTextSprite->End();
	}	
}

//----------------------------------------------------

void CChatWindow::AddChatMessage(CHAR *szNick, DWORD dwNickColor, CHAR *szMessage)
{
	FilterInvalidChars(szMessage);

	if(strlen(szMessage) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_CHAT,szMessage,szNick,m_dwChatTextColor,dwNickColor);
}

//----------------------------------------------------

void CChatWindow::AddInfoMessage(CHAR * szFormat, ...)
{
	char tmp_buf[512];
	memset(tmp_buf,0,512);

	va_list args;
	va_start(args, szFormat);
	vsprintf(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);

	if(strlen(tmp_buf) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_INFO,tmp_buf,NULL,m_dwChatInfoColor,0);
}

//----------------------------------------------------

void CChatWindow::AddDebugMessage(CHAR * szFormat, ...)
{
	char tmp_buf[512];
	memset(tmp_buf,0,512);

	va_list args;
	va_start(args, szFormat);
	vsprintf(tmp_buf, szFormat, args);
	va_end(args);

	FilterInvalidChars(tmp_buf);

	if(strlen(tmp_buf) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_DEBUG,tmp_buf,NULL,m_dwChatDebugColor,0);
	OutputDebugString(tmp_buf);
}

//----------------------------------------------------

void CChatWindow::AddClientMessage(DWORD dwColor, PCHAR szStr)
{
	dwColor = (dwColor >> 8) | 0xFF000000; // convert to ARGB

	FilterInvalidChars(szStr);

	if(strlen(szStr) > MAX_MESSAGE_LENGTH) return;

	AddToChatWindowBuffer(CHAT_TYPE_INFO,szStr,NULL,dwColor,0);
}

//----------------------------------------------------

void CChatWindow::FilterInvalidChars(PCHAR szString)
{
	while(*szString) {
		if(*szString > 0 && *szString < ' ') {
			*szString = ' ';
		}
		szString++;
	}
}

//----------------------------------------------------

void CChatWindow::AddToChatWindowBuffer(eChatMessageType eType, 
										PCHAR szString, 
										PCHAR szNick,
										DWORD dwTextColor,
										DWORD dwChatColor)
{
	int iBestLineLength=0;

	PushBack();

	m_ChatWindowEntries[0].eType = eType;
	m_ChatWindowEntries[0].dwTextColor = dwTextColor;
	m_ChatWindowEntries[0].dwNickColor = dwChatColor;
	
	if(szNick) {
		strcpy(m_ChatWindowEntries[0].szNick,szNick);
		strcat(m_ChatWindowEntries[0].szNick,":");
	} else {
		m_ChatWindowEntries[0].szNick[0] = '\0';
	}

	if(m_ChatWindowEntries[0].eType == CHAT_TYPE_CHAT && strlen(szString) > MAX_LINE_LENGTH)
	{
		iBestLineLength = MAX_LINE_LENGTH;
		// see if we can locate a space.
		while(szString[iBestLineLength] != ' ' && iBestLineLength)
			iBestLineLength--;

		if((MAX_LINE_LENGTH - iBestLineLength) > 12) {
			// we should just take the whole line
			strncpy(m_ChatWindowEntries[0].szMessage,szString,MAX_LINE_LENGTH);
			m_ChatWindowEntries[0].szMessage[MAX_LINE_LENGTH] = '\0';

			PushBack();
			
			m_ChatWindowEntries[0].eType = eType;
			m_ChatWindowEntries[0].dwTextColor = dwTextColor;
			m_ChatWindowEntries[0].dwNickColor = dwChatColor;
			m_ChatWindowEntries[0].szNick[0] = '\0';

			strcpy(m_ChatWindowEntries[0].szMessage,szString+MAX_LINE_LENGTH);
		}
		else {
			// grab upto the found space.
			strncpy(m_ChatWindowEntries[0].szMessage,szString,iBestLineLength);
			m_ChatWindowEntries[0].szMessage[iBestLineLength] = '\0';

			PushBack();

			m_ChatWindowEntries[0].eType = eType;
			m_ChatWindowEntries[0].dwTextColor = dwTextColor;
			m_ChatWindowEntries[0].dwNickColor = dwChatColor;
			m_ChatWindowEntries[0].szNick[0] = '\0';

			strcpy(m_ChatWindowEntries[0].szMessage,szString+(iBestLineLength+1));
		}
	}
	else {
		strncpy(m_ChatWindowEntries[0].szMessage,szString,MAX_MESSAGE_LENGTH);
		m_ChatWindowEntries[0].szMessage[MAX_MESSAGE_LENGTH] = '\0';
	}
	
}

//----------------------------------------------------

void CChatWindow::PushBack()
{
	int x=MAX_MESSAGES-1;
	while(x) {
		memcpy(&m_ChatWindowEntries[x],&m_ChatWindowEntries[x-1],sizeof(CHAT_WINDOW_ENTRY));
		x--;
	}
}

//----------------------------------------------------

void CChatWindow::RenderText(CHAR *sz,RECT rect,DWORD dwColor)
{
	if(m_iEnabled == CHAT_WINDOW_MODE_FULL) {
		// above
		rect.top -= 1;
		m_pD3DFont->DrawText(m_pChatTextSprite,sz,-1,&rect,DT_NOCLIP|DT_SINGLELINE|DT_LEFT,0xFF000000);
		rect.top += 1;

		// below
		rect.top += 1;
		m_pD3DFont->DrawText(m_pChatTextSprite,sz,-1,&rect,DT_NOCLIP|DT_SINGLELINE|DT_LEFT,0xFF000000);
		rect.top -= 1;

		// left
		rect.left -= 1;
		m_pD3DFont->DrawText(m_pChatTextSprite,sz,-1,&rect,DT_NOCLIP|DT_SINGLELINE|DT_LEFT,0xFF000000);
		rect.left += 1;

		// right
		rect.left += 1;
		m_pD3DFont->DrawText(m_pChatTextSprite,sz,-1,&rect,DT_NOCLIP|DT_SINGLELINE|DT_LEFT,0xFF000000);
		rect.left -= 1;
	}

	m_pD3DFont->DrawText(m_pChatTextSprite,sz,-1,&rect,DT_NOCLIP|DT_SINGLELINE|DT_LEFT,dwColor);
}

//----------------------------------------------------
// EOF