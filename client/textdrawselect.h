
#pragma once

class CTextDrawSelect
{
public:
	bool m_bEnabled; // BOOL
	DWORD m_dwHoverColor;
	WORD m_wHoveredText;

	CTextDrawSelect();

	void ProcessTextSelection();
	void Process();
	void Enable(DWORD dwHoverColor);
	void SendNotification();
	void Disable();
	bool MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
