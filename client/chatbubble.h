
#pragma once

typedef struct
{
	bool bValid;
	char szText[MAX_CHAT_BUBBLE_TEXT+1];
	int iLineCount;
	DWORD dwExpireTick;
	float fDistance;
	DWORD dwColor;
} CHAT_BUBBLE_ENTRY;

class CChatBubble
{
private:
	CHAT_BUBBLE_ENTRY m_ChatBubbleEntries[MAX_PLAYERS];

public:
	CChatBubble();

	void AddText(WORD wPlayerID, char* szText, DWORD dwColor, float fDistance, int iDuration);
	void Draw();
};
