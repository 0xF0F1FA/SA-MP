
#include "main.h"

CPlayerTextDrawPool::CPlayerTextDrawPool()
{
	for (int i = 0; i < MAX_PLAYER_TEXT_DRAWS; i++) {
		m_pTextDraws[i] = NULL;
		m_szFontText[i] = NULL;
		m_bHasText[i] = false;
	}
}

CPlayerTextDrawPool::~CPlayerTextDrawPool()
{
	for (int i = 0; i < MAX_PLAYER_TEXT_DRAWS; i++) {
		if (m_pTextDraws[i] != NULL) {
			free(m_pTextDraws[i]);
			m_pTextDraws[i] = NULL;
		}
		if (m_szFontText[i] != NULL) {
			free(m_szFontText[i]);
			m_szFontText[i] = NULL;
		}
		m_bHasText[i] = false;
	}
}

bool CPlayerTextDrawPool::IsValid(int iID)
{
	return ((iID >= 0 && iID < MAX_PLAYER_TEXT_DRAWS) && m_pTextDraws[iID] != NULL);
}

int CPlayerTextDrawPool::New(float fX, float fY, char* szText)
{
	// Find unused slot for new textdraw
	int iSlot = 0;
	while (iSlot < MAX_PLAYER_TEXT_DRAWS) {
		if (m_pTextDraws[iSlot] == NULL) break;
		iSlot++;
	}
	if (iSlot < MAX_PLAYER_TEXT_DRAWS) {
		m_pTextDraws[iSlot] = (TEXT_DRAW_TRANSMIT*)malloc(sizeof(TEXT_DRAW_TRANSMIT));
		m_szFontText[iSlot] = (char*)malloc(MAX_TEXT_DRAW_LINE + 1);
		if (m_pTextDraws[iSlot] != NULL && m_szFontText[iSlot] != NULL) {
			strcpy_s(m_szFontText[iSlot], MAX_TEXT_DRAW_LINE, szText);
			m_szFontText[iSlot][MAX_TEXT_DRAW_LINE] = 0;

			m_pTextDraws[iSlot]->fLetterWidth = 0.48f;
			m_pTextDraws[iSlot]->fLetterHeight = 1.12f;
			m_pTextDraws[iSlot]->dwLetterColor = 0xFFE1E1E1;
			m_pTextDraws[iSlot]->byteCenter = 0;
			m_pTextDraws[iSlot]->byteBox = 0;
			m_pTextDraws[iSlot]->fLineWidth = 1280.0f;
			m_pTextDraws[iSlot]->fLineHeight = 1280.0f;
			m_pTextDraws[iSlot]->dwBoxColor = 0x80808080;
			m_pTextDraws[iSlot]->byteProportional = 1;
			m_pTextDraws[iSlot]->dwBackgroundColor = 0xFF000000;
			m_pTextDraws[iSlot]->byteShadow = 2;
			m_pTextDraws[iSlot]->byteOutline = 0;
			m_pTextDraws[iSlot]->byteLeft = 0;
			m_pTextDraws[iSlot]->byteRight = 0;
			m_pTextDraws[iSlot]->byteStyle = 1;
			m_pTextDraws[iSlot]->fX = fX;
			m_pTextDraws[iSlot]->fY = fY;

			return iSlot;
		}
	}
	return INVALID_PLAYER_TEXT_DRAW;
}

void CPlayerTextDrawPool::SetLetterSize(int iID, float fWidth, float fHeight)
{
	m_pTextDraws[iID]->fLetterWidth = fWidth;
	m_pTextDraws[iID]->fLetterHeight = fHeight;
}
