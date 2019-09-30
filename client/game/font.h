//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2008 SA:MP team
//
// Version: $Id: font.h,v 1.2 2008-02-11 22:30:15 kyecvs Exp $
//
//----------------------------------------------------------

void Font_PrintString(float X, float Y, char *sz);
void Font_SetColor(DWORD dwColor);
void Font_SetDropColor(DWORD dwColor);
void Font_SetOutline(int pos);
void Font_Unk12(int unk);
void Font_SetScale(float X, float Y);
void Font_SetJustify(int just);
void Font_SetFontStyle(int style);
void Font_SetProportional(int prop);
void Font_SetRightJustifyWrap(float wrap);
void Font_UseBox(int use, int unk);
void Font_UseBoxColor(DWORD color);
void Font_SetLineWidth(float width);
void Font_SetLineHeight(float height);
void Font_SetShadow(int shadow);
void Font_UnkConv(char *sz, int param1, int param2, int param3, int param4, int param5, int param6, char * buf);
void Font_UnkConv2(char *sz);