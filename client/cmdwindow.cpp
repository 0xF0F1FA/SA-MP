//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: cmdwindow.cpp,v 1.18 2006/05/08 13:28:46 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"
#include "runutil.h"

//----------------------------------------------------

CCmdWindow::CCmdWindow(IDirect3DDevice9 *pD3DDevice)
{
	m_bEnabled				= FALSE;
	m_pD3DDevice			= pD3DDevice;
	m_iCmdCount				= 0;
	m_iCurrentRecallAt		= -1;
	m_iTotalRecalls			= 0; // I'll be bahk.
	m_pEditControl			= NULL;

	memset(&m_szRecallBuffer[0],0,(MAX_CMD_INPUT+1)*MAX_RECALLS);
    memset(&m_szInputBuffer[0],0,(MAX_CMD_INPUT+1));
	memset(&m_szCurBuffer[0],0,(MAX_CMD_INPUT+1));
}

//----------------------------------------------------

CCmdWindow::~CCmdWindow()
{
}

//----------------------------------------------------

void CCmdWindow::ResetDialogControls(CDXUTDialog *pGameUI)
{
	m_pGameUI = pGameUI;

	if(pGameUI) {
		pGameUI->AddIMEEditBox(IDC_CMDEDIT,"",10,175,570,40,true,&m_pEditControl);

		if (pConfigFile->GetInt("ime"))
		{
			m_pEditControl->EnableImeSystem(true);
			m_pEditControl->StaticOnCreateDevice();
		}

		m_pEditControl->GetElement(0)->TextureColor.Init(D3DCOLOR_ARGB(240, 5, 5, 5));
		m_pEditControl->SetTextColor(D3DCOLOR_ARGB(255, 255, 255, 255));
		m_pEditControl->SetCaretColor(D3DCOLOR_ARGB(255, 150, 150, 150));
		m_pEditControl->SetSelectedBackColor(D3DCOLOR_ARGB(255, 185, 34, 40));
		m_pEditControl->SetSelectedTextColor(D3DCOLOR_ARGB(255, 10, 10, 15));
		m_pEditControl->SetEnabled(false);
		m_pEditControl->SetVisible(false);
	}
}

//----------------------------------------------------

void CCmdWindow::Enable()
{
	if (!m_bEnabled)
	{
		if (m_pEditControl) {
			RECT rect;
			LONG lWidth, lHeight;
			GetClientRect(pGame->GetMainWindowHwnd(), &rect);

			lWidth = (LONG)(rect.right * 0.6f);
			lHeight = (LONG)(m_pGameUI->GetFont(0)->nHeight * -1.5f);

			if (lWidth > 800)
				lWidth = 800;

			m_pEditControl->SetEnabled(true);
			m_pEditControl->SetVisible(true);

			m_pEditControl->SetLocation(40, pChatWindow->GetChatWindowBottom());
			m_pEditControl->SetSize(lWidth, 14 - lHeight);
			m_pGameUI->RequestFocus(m_pEditControl);
			m_pEditControl->OnFocusIn();

			m_pGameUI->SetSize(lWidth + 100, pChatWindow->GetChatWindowBottom() + 50);
		}

		if (pNetGame) {
			RakNet::BitStream out;
			out.Write1();
			pNetGame->Send(RPC_TypingEvent, &out);
		}

		m_bEnabled = TRUE;
	}
}

//----------------------------------------------------

void CCmdWindow::Disable()
{
	if (m_bEnabled)
	{
		if (m_pEditControl) {
			m_pEditControl->SetEnabled(false);
			m_pEditControl->SetVisible(false);
		}

		pGame->ToggleKeyInputsDisabled(0, true);

		if (pNetGame) {
			RakNet::BitStream out;
			out.Write0();
			pNetGame->Send(RPC_TypingEvent, &out);
		}

		m_bEnabled = FALSE;
	}	
}

//----------------------------------------------------

void CCmdWindow::AddToRecallBuffer(char *szCmdInput)
{
	// Move all the existing recalls up 1
    int x=MAX_RECALLS-1;
	while(x) {
		strcpy_s(m_szRecallBuffer[x],m_szRecallBuffer[x-1]);
		x--;
	}
	// Copy this into the first recall slot
    strcpy_s(m_szRecallBuffer[0],szCmdInput);
	if(m_iTotalRecalls < MAX_RECALLS) {
		m_iTotalRecalls++;
	}
}

//----------------------------------------------------

void CCmdWindow::RecallUp()
{
	if(m_iCurrentRecallAt >= (m_iTotalRecalls - 1)) return;

	if(m_iCurrentRecallAt == -1) {
		// Save the current buffer incase we want to return to it.
		strncpy_s(m_szCurBuffer,m_pEditControl->GetText(),MAX_CMD_INPUT);
		m_szCurBuffer[MAX_CMD_INPUT] = '\0';
	}

	m_iCurrentRecallAt++;
	m_pEditControl->SetText(m_szRecallBuffer[m_iCurrentRecallAt]);
	//pChatWindow->AddDebugMessage("RecallAt: %d",m_iCurrentRecallAt);	
}

//----------------------------------------------------

void CCmdWindow::RecallDown()
{
	m_iCurrentRecallAt--;
	if(m_iCurrentRecallAt >= 0) {
		m_pEditControl->SetText(m_szRecallBuffer[m_iCurrentRecallAt]);
		//pChatWindow->AddDebugMessage("RecallAt: %d",m_iCurrentRecallAt);
	} else {
		if(m_iCurrentRecallAt == -1) {
			m_pEditControl->SetText(m_szCurBuffer);
			//pChatWindow->AddDebugMessage("RecallAt: -cur-");
		}
		m_iCurrentRecallAt = -1;		
	}	
}

//----------------------------------------------------

void CCmdWindow::Draw()
{
	if (m_bEnabled)
		pGame->ToggleKeyInputsDisabled(2);
}

//----------------------------------------------------

void CCmdWindow::ProcessInput()
{
	PCHAR szCmdEndPos;
	CMDPROC cmdHandler;

	if(!m_pEditControl) return;

	strncpy_s(m_szInputBuffer,m_pEditControl->GetText(),MAX_CMD_INPUT);
	m_szInputBuffer[MAX_CMD_INPUT] = '\0';
    
	// don't process 0 length input
	if(!strlen(m_szInputBuffer)) {
		if(m_bEnabled) Disable();
		return;
	}

    // remember this command for later use in the recalls.	
    AddToRecallBuffer(m_szInputBuffer);
	m_iCurrentRecallAt = -1;

	if(*m_szInputBuffer != CMD_CHARACTER) { 
		// chat type message	
		if(m_pDefaultCmd) {
			m_pDefaultCmd(m_szInputBuffer);
		}
	}
	else 
	{// possible valid command
		// find the end of the name
		szCmdEndPos = m_szInputBuffer + 1;
		while(*szCmdEndPos && *szCmdEndPos != ' ') szCmdEndPos++;
		if(*szCmdEndPos == '\0') {
			// Possible command with no params.
			cmdHandler = GetCmdHandler(m_szInputBuffer + 1);
			// If valid then call it.
			if(cmdHandler) {
				cmdHandler("");
			}
			else {
				if (pNetGame) {
					SendToServer(m_szInputBuffer);
				}
				else {
					pChatWindow->AddDebugMessage("I don't know that command.");
				}
			}
		}
		else {
			char szCopiedBuffer[MAX_CMD_INPUT+1];
			strcpy_s(szCopiedBuffer, m_szInputBuffer);

			*szCmdEndPos='\0'; // null terminate it
			szCmdEndPos++; // rest is the parameters.
			cmdHandler = GetCmdHandler(m_szInputBuffer + 1);
			// If valid then call it with the param string.
			if(cmdHandler) {
				cmdHandler(szCmdEndPos);
			}
			else {
				if (pNetGame) {
					SendToServer(szCopiedBuffer);
				}
				else {
					pChatWindow->AddDebugMessage("I don't know that command.");
				}
			}
		}
	}

	*m_szInputBuffer ='\0';	
	m_pEditControl->SetText("",false);

	if (m_bEnabled) {
		// Rolls back to the beggining
		if (pChatWindow)
			pChatWindow->ResetPage();
		Disable();
	}
}

//----------------------------------------------------

CMDPROC CCmdWindow::GetCmdHandler(PCHAR szCmdName)
{
	int x=0;
	while(x!=m_iCmdCount) {
		if(!_stricmp(szCmdName,m_szCmdNames[x])) {
			return m_pCmds[x];
		}
		x++;
	}
	return NULL;
}

//----------------------------------------------------

void CCmdWindow::AddDefaultCmdProc(CMDPROC cmdDefault)
{
	m_pDefaultCmd = cmdDefault;	
}

//----------------------------------------------------

void CCmdWindow::AddCmdProc(PCHAR szCmdName, CMDPROC cmdHandler)
{
	if(m_iCmdCount < MAX_CMDS && (strlen(szCmdName) < MAX_CMD_STRLEN)) {
		m_pCmds[m_iCmdCount] = cmdHandler;
		strcpy_s(m_szCmdNames[m_iCmdCount],szCmdName);
		m_iCmdCount++;
	}
}

//----------------------------------------------------

int CCmdWindow::MsgProc(UINT uMsg, DWORD wParam, DWORD lParam)
{
	return (m_bEnabled && m_pEditControl && m_pEditControl->StaticMsgProc(uMsg, wParam, lParam));
}

//----------------------------------------------------

void CCmdWindow::SendToServer(char* szServerCommand)
{
	if(!pNetGame) return;

	RakNet::BitStream bsParams;
	int iStrlen = strlen(szServerCommand);

	//pChatWindow->AddDebugMessage("SendToServer(%s,%u)",szServerCommand,iStrlen);

	bsParams.Write(iStrlen);
	bsParams.Write(szServerCommand, iStrlen);
	pNetGame->GetRakClient()->RPC(RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false);
}

//----------------------------------------------------

bool CCmdWindow::IsCandidateActive()
{
	if (m_pEditControl)
	{
		return CDXUTIMEEditBox::IsCandidateActive();
	}
	return false;
}

//----------------------------------------------------
// EOF
