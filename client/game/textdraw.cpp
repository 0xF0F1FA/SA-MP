/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdraw.cpp,v 1.4 2008-04-16 08:54:17 kyecvs Exp $

*/

#include "../main.h"
#include "font.h"

#define MAX_SPRITES 200
#define INVALID_SPRITE_ID -1

//char ProvideTmp[1024];

bool bSpriteSlotState[MAX_SPRITES]; // BOOL

int FindFreeSpriteSlot()
{
	int iSpriteID = 0;
	while (iSpriteID < MAX_SPRITES)
	{
		if (bSpriteSlotState[iSpriteID] == 0)
			break;
		iSpriteID++;
	}
	if (iSpriteID == MAX_SPRITES) return -1;

	bSpriteSlotState[iSpriteID] = true;

	return iSpriteID;
}

CTextDraw::CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, PCHAR szText)
{
	memset(&m_TextDrawData,0,sizeof(TEXT_DRAW_DATA));

	// Set standard parameters
	m_TextDrawData.fLetterWidth = TextDrawTransmit->fLetterWidth;
	m_TextDrawData.fLetterHeight = TextDrawTransmit->fLetterHeight;
	m_TextDrawData.dwLetterColor = TextDrawTransmit->dwLetterColor;
	m_TextDrawData.byteUnk12 = 0;
	m_TextDrawData.byteCentered = TextDrawTransmit->byteCenter;
	m_TextDrawData.byteBox = TextDrawTransmit->byteBox;
	m_TextDrawData.fLineWidth = TextDrawTransmit->fLineWidth;
	m_TextDrawData.fLineHeight = TextDrawTransmit->fLineHeight;
	m_TextDrawData.dwBoxColor = TextDrawTransmit->dwBoxColor;
	m_TextDrawData.byteProportional = TextDrawTransmit->byteProportional;
	m_TextDrawData.dwBackgroundColor = TextDrawTransmit->dwBackgroundColor;
	m_TextDrawData.byteShadow = TextDrawTransmit->byteShadow;
	m_TextDrawData.byteOutline = TextDrawTransmit->byteOutline;
	m_TextDrawData.byteAlignLeft = TextDrawTransmit->byteLeft;
	m_TextDrawData.byteAlignRight = TextDrawTransmit->byteRight;
	m_TextDrawData.dwStyle = TextDrawTransmit->byteStyle;
	m_TextDrawData.fX = TextDrawTransmit->fX;
	m_TextDrawData.fY = TextDrawTransmit->fY;
	m_TextDrawData.dwParam1 = 0xFFFFFFFF;
	m_TextDrawData.dwParam2 = 0xFFFFFFFF;
	m_TextDrawData.byteSelectable = TextDrawTransmit->byteSelectable;

	m_TextDrawData.iSpriteID = -1;

	//m_bHasKeyCode = false;

	SetText(szText);

	if (m_TextDrawData.dwStyle == TEXT_DRAW_FONT_SPRITE)
	{
		m_TextDrawData.iSpriteID = FindFreeSpriteSlot();
		SetupSprite();
	}

	m_TextDrawData.bHasBoundingBox = false;
	m_rcBoundingBox.left = 0;
	m_rcBoundingBox.right = 0;
	m_rcBoundingBox.top = 0;
	m_rcBoundingBox.bottom = 0;
	m_bMouseover = false;
	m_dwHoverColor = 0;
}

void CTextDraw::SetupSprite()
{
	char szTxdName[64];
	char szTextureName[64];

	// CHANGED: No free slot, return here instead after TXD
	// and texture names being processed
	if (m_TextDrawData.iSpriteID == -1)
		return;

	char* szText = m_szText;
	char* szSplitPos = strchr(m_szText,':');

	if (!szSplitPos || strlen(m_szText) >= 64 ||
		strchr(m_szText, '\\') || strchr(m_szText, '/'))
		return;

	strncpy_s(szTxdName, m_szText, szSplitPos - szText);
	strcpy_s(szTextureName, szSplitPos + 1);

	if (!memcmp(szTxdName, "hud", 4))
	{

	}
	if (!memcmp(szTxdName, "samaps", 7))
	{

	}
	if (!memcmp(szTxdName, "vehicleprev", 12))
	{

	}
}

void CTextDraw::SetText(char* szText)
{
	/*memset(m_szText,0,MAX_TEXT_DRAW_LINE);

	if (strlen(szText) >= MAX_TEXT_DRAW_LINE)
		return;

	strncpy_s(m_szText, szText, MAX_TEXT_DRAW_LINE);
	m_szText[MAX_TEXT_DRAW_LINE-1] = 0;

	//m_bHasKeyCode = false;

	char* szText = m_szText;
	while (*szText)
	{
		if (*szText == '~')
		{

		}
		szText++;
	}*/
}


void CTextDraw::Draw()
{
	//if(!m_szText || !strlen(m_szText)) return;
	if (!m_szText || m_szText[0] == '\0') return;

	strcpy(m_szString,m_szText);

	int iScreenWidth, iScreenHeight;
	float fVertHudScale, fHorizHudScale;

    iScreenWidth = pGame->GetScreenWidth();
	iScreenHeight = pGame->GetScreenHeight();
	fVertHudScale = pGame->GetHudVertScale();
	fHorizHudScale = pGame->GetHudHorizScale();

    float fScaleY = (float)iScreenHeight * fVertHudScale * m_TextDrawData.fLetterHeight * 0.5f;
	float fScaleX = (float)iScreenWidth * fHorizHudScale * m_TextDrawData.fLetterWidth;

	Font_SetScale(fScaleX,fScaleY);

	if (m_bMouseover)
		Font_SetColor(m_dwHoverColor);
	else
		Font_SetColor(m_TextDrawData.dwLetterColor);

    Font_Unk12(0);

	if(m_TextDrawData.byteAlignRight) Font_SetJustify(2);
	else if(m_TextDrawData.byteCentered) Font_SetJustify(0);
    else Font_SetJustify(1);

	//Font_SetRightJustifyWrap(0.0f);

	float fLineWidth = iScreenWidth * fHorizHudScale * m_TextDrawData.fLineWidth;
	Font_SetLineWidth(fLineWidth);

	float fLineHeight = iScreenWidth * fHorizHudScale * m_TextDrawData.fLineHeight;
	Font_SetLineHeight(fLineHeight);

	Font_UseBox(m_TextDrawData.byteBox,0);
	Font_UseBoxColor(m_TextDrawData.dwBoxColor);
	Font_SetProportional(m_TextDrawData.byteProportional);
	Font_SetDropColor(m_TextDrawData.dwBackgroundColor);

	if(m_TextDrawData.byteOutline) {
		Font_SetOutline(m_TextDrawData.byteOutline);
	} else {
		Font_SetShadow(m_TextDrawData.byteShadow);
	}

	Font_SetFontStyle(m_TextDrawData.dwStyle);

	Font_UnkConv(m_szString,-1,-1,-1,-1,-1,-1,m_szString);
	Font_UnkConv2(m_szString);

	float fPS2Height = 448.0f;
	float fPS2Width = 640.0f;
	float fScriptY =  m_TextDrawData.fY;
	float fScriptX =  m_TextDrawData.fX;
	float fUseX,fUseY;

	fUseY = iScreenHeight - ((fPS2Height - fScriptY) * (iScreenHeight * fVertHudScale));
    fUseX = iScreenWidth - ((fPS2Width - fScriptX) * (iScreenWidth * fHorizHudScale));
    
    Font_PrintString(fUseX,fUseY,m_szString);
	Font_SetOutline(0);

	if (m_TextDrawData.byteAlignRight)
	{
		m_rcBoundingBox.left = (LONG)(fUseX - (fLineWidth - fUseX));
		m_rcBoundingBox.right = (LONG)fUseX;
		m_rcBoundingBox.top = (LONG)fUseY;
		m_rcBoundingBox.bottom = (LONG)(fUseY + fLineHeight);
		m_TextDrawData.bHasBoundingBox = true;
	}
	else if (m_TextDrawData.byteCentered)
	{
		float fX = fUseX - fLineHeight * 0.5f;
		m_rcBoundingBox.left = (LONG)fX;
		m_rcBoundingBox.right = (LONG)(fX + fLineHeight);
		m_rcBoundingBox.top = (LONG)fUseY;
		m_rcBoundingBox.bottom = (LONG)(fUseY + fLineWidth);
		m_TextDrawData.bHasBoundingBox = true;
	}
	else
	{
		m_rcBoundingBox.left = (LONG)fUseX;
		m_rcBoundingBox.right = (LONG)(fLineWidth - fUseX + fUseX);
		m_rcBoundingBox.top = (LONG)fUseY;
		m_rcBoundingBox.bottom = (LONG)(fUseY + fLineHeight);
		m_TextDrawData.bHasBoundingBox = true;;
	}
}
