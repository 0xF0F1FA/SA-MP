
#ifndef _PLAYERTEXTDRAWPOOL_H
#define _PLAYERTEXTDRAWPOOL_H

#define RGBA_ABGR(n) (((n >> 24) & 0x000000FF) | ((n >> 8) & 0x0000FF00) | ((n << 8) & 0x00FF0000) | ((n << 24) & 0xFF000000))

class CPlayerTextDrawPool
{
private:
	TEXT_DRAW_TRANSMIT* m_pTextDraws[MAX_PLAYER_TEXT_DRAWS];
	char* m_szFontText[MAX_PLAYER_TEXT_DRAWS];
	bool m_bHasText[MAX_PLAYER_TEXT_DRAWS];

public:
	CPlayerTextDrawPool();
	~CPlayerTextDrawPool();

	bool IsValid(int iID);

	int New(float fX, float fY, char* szText);
	void SetLetterSize(int iID, float fWidth, float fHeight);
	void SetTextSize(int iID, float fWidth, float fHeight);
	void SetAlignment(int iID, int iAlignment);
	void SetColor(int iID, unsigned long ulColor);
	void SetBoxColor(int iID, unsigned long ulColor);
	void SetBackgroundColor(int iID, unsigned long ulColor);
	void SetUseBox(int iID, int iUse);
	void SetShadow(int iID, unsigned char ucShadow);
};

#endif // _PLAYERTEXTDRAWPOOL_H
