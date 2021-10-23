
#include "main.h"

#define REVERSE(x) ((((x) << 16) | (x) & 0xFF00) << 8) | (((x >> 16) | x & 0xFF0000) >> 8)

CTextDrawSelect::CTextDrawSelect()
{
	m_bEnabled = false;
	m_dwHoverColor = 0xFFFFFFFF;
	m_wHoveredText = 0xFFFF;
}

void CTextDrawSelect::ProcessTextSelection()
{
	CTextDrawPool* pTextDrawPool;
	CTextDraw* pTextDraw;
	POINT point;
	
	if (!pNetGame || !(pTextDrawPool = pNetGame->GetTextDrawPool()))
		return;

	GetCursorPos(&point);
	ScreenToClient(pGame->GetMainWindowHwnd(), &point);

	m_wHoveredText = 0xFFFF;

	for (WORD i = 0; i < MAX_CLIENT_TEXT_DRAWS; i++)
	{
		pTextDraw = pTextDrawPool->GetAt(i);
		if (pTextDraw)
		{
			pTextDraw->m_bMouseover = false;
			pTextDraw->m_dwHoverColor = 0;
			
			if (pTextDraw->m_TextDrawData.byteSelectable &&
				pTextDraw->m_TextDrawData.bHasBoundingBox &&
				PtInRect(&pTextDraw->m_rcBoundingBox, point))
			{
				m_wHoveredText = i;

				pTextDraw->m_bMouseover = true;
				pTextDraw->m_dwHoverColor = m_dwHoverColor;
			}
		}
	}
}

void CTextDrawSelect::Process()
{
	if (m_bEnabled)
	{
		pGame->ToggleKeyInputsDisabled(2);
		pGame->DisplayHud(false);

		ProcessTextSelection();
	}
}

void CTextDrawSelect::Enable(DWORD dwHoverColor)
{
	m_bEnabled = true;
	m_dwHoverColor = REVERSE(dwHoverColor);
	m_wHoveredText = 0xFFFF;
}

void CTextDrawSelect::SendNotification()
{
	RakNet::BitStream bsSend;

	bsSend.Write(m_wHoveredText);

	pNetGame->Send(RPC_ScrClickTextDraw, &bsSend);
}

void CTextDrawSelect::Disable()
{
	m_bEnabled = false;

	pGame->ToggleKeyInputsDisabled(0);
	
	if (pCmdWindow->isEnabled())
	{
		pCmdWindow->Disable();
		pCmdWindow->Enable();
	}

	m_wHoveredText = 0xFFFF;
	SendNotification();

	if (pNetGame)
	{
		CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
		if (pTextDrawPool)
		{
			for (WORD i = 0; i < MAX_CLIENT_TEXT_DRAWS; i++)
			{
				CTextDraw* pTextDraw = pTextDrawPool->GetAt(i);
				if (pTextDraw)
				{
					pTextDraw->m_bMouseover = false;
					pTextDraw->m_dwHoverColor = 0;
				}
			}
		}
	}
}

bool CTextDrawSelect::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && uMsg == WM_LBUTTONUP)
	{
		if (m_wHoveredText != 0xFFFF)
		{
			SendNotification();
		}
		return 1;
	}
	return 0;
}
