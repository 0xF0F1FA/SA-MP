#pragma once

/*

SA:MP Multiplayer Modification
Copyright 2004-2005 SA:MP Team

Version: $Id: textdraw.h,v 1.3 2008-02-11 22:30:15 kyecvs Exp $

*/

#define MAX_TEXT_DRAW_LINE 256

//-----------------------------------------------------------

#pragma pack(1)
typedef struct _TEXT_DRAW_DATA
{
	float fLetterWidth;		// -4
	float fLetterHeight;	// 0
	DWORD dwLetterColor;	// 4
	BYTE byteUnk12;			// 8
	BYTE byteCentered;		// 9
	BYTE byteBox;			// 10
	PADDING(_pad15, 1);		// 11
	float fLineWidth;		// 12
	float fLineHeight;		// 16
	DWORD dwBoxColor;		// 20
	BYTE byteProportional;	// 24
	DWORD dwBackgroundColor;// 25-29
	BYTE byteShadow;		// 29
	BYTE byteOutline;		// 30
	BYTE byteAlignLeft;		// 31
	BYTE byteAlignRight;	// 32
	PADDING(_pad37, 3);		// 33,34,35
	DWORD dwStyle;			// 36
	float fX;				// 40
	float fY;				// 44
	char cGXT[8];			// 48
	DWORD dwParam1;			// 60-64
	DWORD dwParam2;			// 64-68
} TEXT_DRAW_DATA;

//-----------------------------------------------------------

#pragma pack(1)
typedef struct _TEXT_DRAW_TRANSMIT
{
	float fLetterWidth;
	float fLetterHeight;
	DWORD dwLetterColor;
	float fLineWidth;
	float fLineHeight;
	DWORD dwBoxColor;
	union
	{
		BYTE byteFlags;
		struct
		{
			BYTE byteBox : 1;
			BYTE byteLeft : 1;
			BYTE byteRight : 1;
			BYTE byteCenter : 1;
			BYTE byteProportional : 1;
			BYTE bytePadding : 3;
		};
	};
	BYTE byteShadow;
	BYTE byteOutline;
	DWORD dwBackgroundColor;
	BYTE byteStyle;
	float fX;
	float fY;
} TEXT_DRAW_TRANSMIT;

//----------------------------------------------------

#pragma pack(1)

class CTextDraw
{
private:

	CHAR			m_szText[MAX_TEXT_DRAW_LINE];
	CHAR			m_szString[MAX_TEXT_DRAW_LINE*4];

	TEXT_DRAW_DATA  m_TextDrawData;

public:
	CTextDraw(TEXT_DRAW_TRANSMIT *TextDrawTransmit, PCHAR szText);
	~CTextDraw(){};
	
	PCHAR GetText() { return m_szText; };
	void SetText(char* szText);

	void Draw();
};


//----------------------------------------------------
