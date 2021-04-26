//----------------------------------------------------------
//
// SA:MP Multiplayer Modification For GTA:SA
// Copyright 2004-2005 SA:MP team
//
// Version: $Id: deathwindow.cpp,v 1.9 2006/05/08 17:35:55 kyeman Exp $
//
//----------------------------------------------------------

#include "main.h"

static CHAR SpriteIDForWeapon(BYTE byteWeaponID)
{
	switch (byteWeaponID)
	{
	case WEAPON_FIST:
		return '%';
	case WEAPON_BRASSKNUCKLE:
		return 'B';
	case WEAPON_GOLFCLUB:
		return '>';
	case WEAPON_NITESTICK:
		return '(';
	case WEAPON_KNIFE:
		return 'C';
	case WEAPON_BAT:
		return '?';
	case WEAPON_SHOVEL:
		return '&';
	case WEAPON_POOLSTICK:
		return '"'; //return '\\';
	case WEAPON_KATANA:
		return '!';
	case WEAPON_CHAINSAW:
		return '1';
	case WEAPON_DILDO:
		return 'E';
	case WEAPON_DILDO2:
		return 'E';
	case WEAPON_VIBRATOR:
		return 'E';
	case WEAPON_VIBRATOR2:
		return 'E';
	case WEAPON_FLOWER:
		return '$';
	case WEAPON_CANE:
		return '#';
	case WEAPON_GRENADE:
		return '@';
	case WEAPON_TEARGAS:
		return 'D';
	case WEAPON_COLT45:
		return '6';
	case WEAPON_SILENCED:
		return '2';
	case WEAPON_DEAGLE:
		return '3';
	case WEAPON_SHOTGUN:
		return '=';
	case WEAPON_SAWEDOFF:
		return '0';
	case WEAPON_SHOTGSPA:
		return '+';
	case WEAPON_UZI:
		return 'I';
	case WEAPON_MP5:
		return '8';
	case WEAPON_AK47:
		return 'H';
	case WEAPON_M4:
		return '5';
	case WEAPON_TEC9:
		return '7';
	case WEAPON_RIFLE:
		return '.';
	case WEAPON_SNIPER:
		return 'A';
	case WEAPON_ROCKETLAUNCHER:
		return '4';
	case WEAPON_HEATSEEKER:
		return ')';
	case WEAPON_FLAMETHROWER:
		return 'P';
	case WEAPON_MINIGUN:
		return 'F';
	case WEAPON_SATCHEL:
		return '<';
	case WEAPON_BOMB:
		return ';';
	case WEAPON_SPRAYCAN:
		return '/';
	case WEAPON_FIREEXTINGUISHER:
		return ',';
	case WEAPON_PARACHUTE:
		return ':';
	case WEAPON_VEHICLE:
		return 'L';
	case WEAPON_DROWN:
		return 'J';
	case WEAPON_HELIBLADES:
		return 'R';
	case WEAPON_EXPLOSION:
		return 'Q';
	case WEAPON_COLLISION:
		return 'K';
	case SPECIAL_ENTRY_CONNECT:
		return 'N';
	case SPECIAL_ENTRY_DISCONNECT:
		return 'N';
	}
	return 'J';
}

CDeathWindow::CDeathWindow(IDirect3DDevice9 *pD3DDevice)
{
	m_bEnabled = true;
	m_pD3DFont = NULL;
	m_pWeaponFont = NULL;
	m_pSprite = NULL;
	m_pRoundedBoxFont = NULL;
	m_bAuxFontInited = false;
	m_pD3DAuxFont = NULL;
	m_pD3DAuxBoxFont = NULL;

	m_pD3DDevice = pD3DDevice;

	CreateFonts();
	ClearWindow();
}

CDeathWindow::~CDeathWindow()
{
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pD3DFont);
	SAFE_RELEASE(m_pWeaponFont);
	SAFE_RELEASE(m_pRoundedBoxFont);
	
	// additional
	SAFE_RELEASE(m_pD3DAuxFont);
	SAFE_RELEASE(m_pD3DAuxBoxFont);
}

void CDeathWindow::CreateFonts()
{
	if(!m_pD3DDevice) return;
	
	HRESULT hResult = S_OK;
	RECT rectLongestNick = {0,0,0,0};
	int	iFontSize;
	char* szFontFace;
	int iFontWeight;

	SAFE_RELEASE(m_pD3DFont);
	SAFE_RELEASE(m_pWeaponFont);
	SAFE_RELEASE(m_pRoundedBoxFont);
	SAFE_RELEASE(m_pSprite);

	// Create a sprite to use when drawing text
	D3DXCreateSprite(m_pD3DDevice, &m_pSprite);

	iFontSize = GetDeathWindowFontSize();
	szFontFace = GetFontFace();
	iFontWeight = GetFontWeight();

	D3DXCreateFontA(m_pD3DDevice, iFontSize, 0, iFontWeight, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, szFontFace, &m_pD3DFont);

	// Store the rect for right aligned name (DT_RIGHT fucks the text)
	if (m_pD3DFont)
		m_pD3DFont->DrawTextA(0, "LONGESTNICKNICK_NICKNICK", -1, &rectLongestNick, DT_CALCRECT | DT_LEFT, 0xFF000000);

	m_iLongestNickLength = rectLongestNick.right - rectLongestNick.left;

	D3DXCreateFontA(m_pD3DDevice, iFontSize+8, 0, FW_NORMAL, 1, FALSE,
		SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "GTAWEAPON3", &m_pWeaponFont);
	D3DXCreateFontA(m_pD3DDevice, iFontSize+12, 0, FW_NORMAL, 1, FALSE,
		SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "GTAWEAPON3", &m_pRoundedBoxFont);

	SIZE size;
	GetRoundedBoxSize(&size);
	m_lRoundedBoxSizeX = size.cx;
	m_lRoundedBoxSizeY = size.cy;
}

SIZE* CDeathWindow::GetRoundedBoxSize(SIZE* pOutSize)
{
	SIZE size = {100,100};
	RECT rect = {0,0,100,100};

	if (m_pRoundedBoxFont)
	{
		m_pRoundedBoxFont->DrawTextA(NULL, "G", -1, &rect, DT_VCENTER | DT_NOCLIP | DT_CALCRECT, 0xFF000000);

		size.cx = rect.right;
		size.cy = rect.bottom;
	}

	pOutSize->cx = size.cx - rect.left;
	pOutSize->cy = size.cy - rect.top;

	return pOutSize;
}

void CDeathWindow::OnLostDevice()
{
	if (m_pSprite)
		m_pSprite->OnLostDevice();
	if (m_pD3DFont)
		m_pD3DFont->OnLostDevice();
	if (m_pWeaponFont)
		m_pWeaponFont->OnLostDevice();
	if (m_pRoundedBoxFont)
		m_pRoundedBoxFont->OnLostDevice();
	if (m_pD3DAuxFont)
		m_pD3DAuxFont->OnLostDevice();
	if (m_pD3DAuxBoxFont)
		m_pD3DAuxBoxFont->OnLostDevice();	
}

void CDeathWindow::OnResetDevice()
{
	if (m_pSprite)
		m_pSprite->OnResetDevice();
	if (m_pD3DFont)
		m_pD3DFont->OnResetDevice();
	if (m_pWeaponFont)
		m_pWeaponFont->OnResetDevice();
	if (m_pRoundedBoxFont)
		m_pRoundedBoxFont->OnResetDevice();
	if (m_pD3DAuxFont)
		m_pD3DAuxFont->OnResetDevice();
	if (m_pD3DAuxBoxFont)
		m_pD3DAuxBoxFont->OnResetDevice();
}

//----------------------------------------------------

void CDeathWindow::Draw()
{
	if (m_pD3DFont && m_pWeaponFont && m_bEnabled)
	{
		RECT
			rect,
			rectNickSize;
		int
			iScreenHeight = pGame->GetScreenHeight(),
			iScreenWidth = pGame->GetScreenWidth(),
			iVerticalBase = (int)(iScreenHeight * 0.3f),
			iHorizontalBase = (int)(iScreenWidth * 0.75f),
			iBaseX = m_lRoundedBoxSizeX + 2 * m_iLongestNickLength,
			x = 0;

		if ((iBaseX + iHorizontalBase) > iScreenWidth)
			iHorizontalBase = iScreenWidth - iBaseX;

		rect.top = iVerticalBase;
		rect.left = iHorizontalBase;
		rect.bottom = rect.top + 30;
		rect.right = rect.left + 60;

		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);

		while (x != MAX_DISP_DEATH_MESSAGES)
		{
			//if(strlen(m_DeathWindowEntries[x].szKiller) && strlen(m_DeathWindowEntries[x].szKillee))
			if (m_DeathWindowEntries[x].szKiller[0] != '\0' && m_DeathWindowEntries[x].szKillee[0] != '\0')
			{
				// Get the rect length of the killee's nick so we can right justify.
				m_pD3DFont->DrawText(0, m_DeathWindowEntries[x].szKiller, -1,
					&rectNickSize, DT_CALCRECT | DT_LEFT, 0xFFFFFFFF);

				// Move in so it's right justified. (DT_RIGHT fucks the text)
				rect.left += m_iLongestNickLength - (rectNickSize.right - rectNickSize.left);

				RenderText(m_DeathWindowEntries[x].szKiller, rect,
					m_DeathWindowEntries[x].dwKillerColor, DT_LEFT);

				rect.left = iHorizontalBase + m_iLongestNickLength + 3;
				rect.right = rect.left + 35;

				RenderWeaponSprite(SpriteIDForWeapon(m_DeathWindowEntries[x].byteWeaponType),
					rect, 0xFFFFFFFF);

				rect.left += m_lRoundedBoxSizeX;
				rect.right += m_lRoundedBoxSizeX + 35;

				RenderText(m_DeathWindowEntries[x].szKillee, rect, m_DeathWindowEntries[x].dwKilleeColor, DT_LEFT);
			}
			//else if(!strlen(m_DeathWindowEntries[x].szKiller) && strlen(m_DeathWindowEntries[x].szKillee))
			else if (m_DeathWindowEntries[x].szKiller[0] == '\0' && m_DeathWindowEntries[x].szKillee[0] != '\0')
			{
				DWORD dwColor = 0xFFFFFFFF;

				// Get the rect length of the killee's nick so we can right justify.
				m_pD3DFont->DrawText(0, m_DeathWindowEntries[x].szKillee, -1,
					&rectNickSize, DT_CALCRECT | DT_LEFT, 0xFF000000);

				// Move in so it's right justified. (DT_RIGHT fucks the text)
				rect.left += m_iLongestNickLength - (rectNickSize.right - rectNickSize.left);

				RenderText(m_DeathWindowEntries[x].szKillee, rect,
					m_DeathWindowEntries[x].dwKilleeColor, DT_LEFT);

				rect.left = iHorizontalBase + m_iLongestNickLength + 3;
				rect.right = iHorizontalBase + m_iLongestNickLength + 38;

				if (m_DeathWindowEntries[x].byteWeaponType == SPECIAL_ENTRY_CONNECT)
					dwColor = 0xFF1111AA;
				else if (m_DeathWindowEntries[x].byteWeaponType == SPECIAL_ENTRY_DISCONNECT)
					dwColor = 0xFFAA1111;

				RenderWeaponSprite(SpriteIDForWeapon(m_DeathWindowEntries[x].byteWeaponType), rect, dwColor);
			}

			rect.top += m_lRoundedBoxSizeY + 5;
			rect.bottom += m_lRoundedBoxSizeY + 5;
			rect.left = iHorizontalBase;
			rect.right = rect.left + 60;

			x++;
		}

		m_pSprite->End();
	}
}

void CDeathWindow::AddMessage(PCHAR szKiller, PCHAR szKillee,
	DWORD dwKillerColor, DWORD dwKilleeColor, BYTE byteWeaponID)
{
	int n = MAX_DISP_DEATH_MESSAGES-1;

	PushBack();

	m_DeathWindowEntries[n].byteWeaponType = byteWeaponID;
	m_DeathWindowEntries[n].dwKilleeColor = dwKilleeColor;
	m_DeathWindowEntries[n].dwKillerColor = dwKillerColor;

	if(szKiller) strcpy_s(m_DeathWindowEntries[n].szKiller, szKiller);
	else m_DeathWindowEntries[n].szKiller[0] = '\0';

	if(szKillee) strcpy_s(m_DeathWindowEntries[n].szKillee, szKillee);	
	else m_DeathWindowEntries[n].szKillee[0] = '\0';
}

// Used by CPlayerTags for drawing sandglass icon when player is inactive
void CDeathWindow::CreateAuxiliaryFont()
{
	D3DXCreateFontA(m_pD3DDevice, 20, 0, FW_NORMAL, 1, FALSE,
		SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "SAMPAUX3", &m_pD3DAuxFont);
	D3DXCreateFontA(m_pD3DDevice, 22, 0, FW_NORMAL, 1, FALSE,
		SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "SAMPAUX3", &m_pD3DAuxBoxFont);

	m_bAuxFontInited = true;
}

void CDeathWindow::PushBack()
{
	memcpy(&m_DeathWindowEntries[0], &m_DeathWindowEntries[1], sizeof(DEATH_WINDOW_ENTRY) * (MAX_DISP_DEATH_MESSAGES - 1));
}

void CDeathWindow::RenderText(PCHAR sz,RECT rect,DWORD dwColor,DWORD dwParams)
{
	rect.top -= 1;
	m_pD3DFont->DrawText(m_pSprite, sz, -1, &rect, DT_NOCLIP | dwParams, 0xFF000000);
	rect.top += 2;
	m_pD3DFont->DrawText(m_pSprite, sz, -1, &rect, DT_NOCLIP | dwParams, 0xFF000000);
	rect.left -= 1;
	rect.top -= 1;
	m_pD3DFont->DrawText(m_pSprite, sz, -1, &rect, DT_NOCLIP | dwParams, 0xFF000000);
	rect.left += 2;
	m_pD3DFont->DrawText(m_pSprite, sz, -1, &rect, DT_NOCLIP | dwParams, 0xFF000000);
	rect.left -= 1;

	m_pD3DFont->DrawText(m_pSprite,sz,-1,&rect,DT_NOCLIP|dwParams,dwColor);	
}

void CDeathWindow::RenderWeaponSprite(CHAR WeaponChar,RECT rect,DWORD dwColor)
{
	rect.top -= 5;
	m_pRoundedBoxFont->DrawTextA(m_pSprite,"G",-1,&rect,DT_NOCLIP|DT_LEFT,0xFF000000);

	m_pRoundedBoxFont->DrawTextA(m_pSprite,"G",-1,&rect,DT_NOCLIP|DT_LEFT|DT_CALCRECT,0xFF000000);

	m_pWeaponFont->DrawText(m_pSprite,&WeaponChar,-1,&rect,DT_CENTER|DT_VCENTER|DT_NOCLIP,dwColor);
}

void CDeathWindow::ClearWindow()
{
	SecureZeroMemory(m_DeathWindowEntries, sizeof(m_DeathWindowEntries));
}

void CDeathWindow::ChangeNick(PCHAR szOldNick, PCHAR szNewNick)
{
	int x = 0;
	while (x != MAX_DISP_DEATH_MESSAGES) {
		if (m_DeathWindowEntries[x].szKiller[0] != '\0' &&
			strcmp(m_DeathWindowEntries[x].szKiller, szOldNick) == 0)
			strcpy_s(m_DeathWindowEntries[x].szKiller, szNewNick);

		if (m_DeathWindowEntries[x].szKillee[0] != '\0' && 
			strcmp(m_DeathWindowEntries[x].szKillee, szOldNick) == 0)
			strcpy_s(m_DeathWindowEntries[x].szKillee, szNewNick);
		x++;
	}
}

void CDeathWindow::ToggleEnabled()
{
	m_bEnabled = (m_bEnabled == TRUE);
}
