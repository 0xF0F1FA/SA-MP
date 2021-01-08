//----------------------------------------------------
//
//	Utility runtime procedures
//	(c) 2005 Kye Bitossi
//
// Version: $Id: runutil.cpp,v 1.3 2006/03/20 17:44:19 kyeman Exp $
//
//----------------------------------------------------

#include "main.h"

#include <stdio.h>
#include <string.h>

//----------------------------------------------------

void Util_UrlUnencode(char *enc)
{
	char *write_pos = enc;

	while(*enc)
	{
		if(*enc=='%')
		{
			*write_pos = (*++enc>'/'&&*enc<':')?((*enc-('0'))<<4):((*enc-('7'))<<4);
			*write_pos |= (*++enc>'/'&&*enc<':')?(*enc-'0'):(*enc-'7');
		}
		else if (*enc=='+')
			*write_pos= ' ';
		else
			*write_pos= *enc;

		write_pos++; enc++;
	}
	*write_pos='\0';
}

//----------------------------------------------------

char Util_toupper(char c) {return ((c>(char)0x60) && (c<(char)0x7b))? c-0x20:c;}

//----------------------------------------------------

char *Util_stristr(const char *String, const char *Pattern)
{
	char *pptri, *sptri, *starti;

      for (starti=(char *)String; *starti != '\0'; starti++)
      {

            /* find start of pattern in string */
            for (;((*starti!='\0') && (Util_toupper(*starti) != Util_toupper(*Pattern))); starti++);

            pptri = (char *)Pattern;
            sptri = (char *)starti;

            while (Util_toupper(*sptri) == Util_toupper(*pptri))
            {
                  sptri++;
                  pptri++;

                  /* if end of pattern then pattern was found */
                  if ('\0' == *pptri)
                        return (starti);
            }

      }

      return(0);
}

//----------------------------------------------------

void Util_strupr(char *string)
{
	char *p = string;

	while(*p) {
		*p = Util_toupper(*p);
		p++;
	}
}

//----------------------------------------------------

int Util_wildcmp(char *wild, char *string)
{
	char *cp, *mp;
	
	while((*string) && (*wild != '*'))
	{
		if((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}
		
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
		
	while (*wild == '*')
	{
		wild++;
	}

	return !*wild;
}

//----------------------------------------------------

int Util_strnicmp(const char *s1, const char *s2, size_t n)
{

  if (n == 0) return 0;

  do
  {
    if (Util_toupper((unsigned char)*s1) != Util_toupper((unsigned char)*s2++))
      return (int)Util_toupper((unsigned char)*s1) - (int)Util_toupper((unsigned char)*--s2);
    if (*s1++ == 0)
      break;

  } while (--n != 0);

  return 0;
}

//----------------------------------------------------

char *Util_strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

//----------------------------------------------------

char * Util_itoa(int v, char *s, int r)
{
	int i,neg = 0;
	char *p = s;
	char *q = s;

	if (r < 0 || r > 35) {
		*s = 0;
		return (s);
		}
	if (r == 0) r = 10;
	if (v == 0) {
		*p++ = '0';
		*p = 0;
		return (s);
		}
	if (v < 0) {
		neg = 1;
		v = -v;
		}
	while (v > 0) {
		i = v % r;
		if (i > 9) i += 7;
		*p++ = '0' + i;
		v /= r;
		}
	if (neg) *p++ = '-';
	*p-- = 0;
	q = s;
	while (p > q) {
		i = *q;
		*q++ = *p;
		*p-- = i;
		}
	return (s);
}

//----------------------------------------------------

char * Base64Encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void Util_Base64Encode( char *cpInput, char *cpOutput )
{
int nIdx[ 4 ];  
while ( '\0' != *cpInput )
{
  nIdx[0] = ((*cpInput) & 0xFC)>>2;
  nIdx[1] = ((*cpInput) & 0x03)<<4;
  cpInput++;
  if ( '\0' != *cpInput )
  {
    nIdx[1] |= ((*cpInput) & 0xF0)>>4;
    nIdx[2]  = ((*cpInput) & 0x0F)<<2;
    cpInput++;
    if ( '\0' != (*cpInput) )
    {
      nIdx[2] |= ((*cpInput) & 0xC0) >> 6;
      nIdx[3]  = (*cpInput) & 0x3F;
      cpInput++;
    }
    else
      nIdx[3] = 64;
  }
  else
  {
    nIdx[2] = 64;
    nIdx[3] = 64;
  }

  *(cpOutput+0) = *(Base64Encoding + nIdx[0]);
  *(cpOutput+1) = *(Base64Encoding + nIdx[1]);
  *(cpOutput+2) = *(Base64Encoding + nIdx[2]);
  *(cpOutput+3) = *(Base64Encoding + nIdx[3]);
  cpOutput += 4;
}

*cpOutput = '\0';

return;
}

//----------------------------------------------------
// Simple rotate right 3 character encoding for
// hiding strings in the exe.

void K_EncodeString(char *szInput, char *szOutput)
{
	char b;

	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm ror bl, 3
		_asm mov b, bl
		*szOutput = b;
		szInput++;
		szOutput++;
	}
	*szOutput = 0;
}

//----------------------------------------------------

char * K_DecodeString(unsigned char *szInput)
{
	char b;
	char *st = (char *)szInput;
    
	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm rol bl, 3
		_asm mov b, bl
		*szInput = b;
		szInput++;
	}

	return st;
}

unsigned long Util_GetTime()
{
	static bool bInited = false;
	static LARGE_INTEGER fli;
	LARGE_INTEGER cli;
	if (!bInited) {
		QueryPerformanceFrequency(&fli);
		bInited = true;
	}
	QueryPerformanceCounter(&cli);
	return (unsigned long)(cli.QuadPart * 1000 / fli.QuadPart);
}

const char* GetWeaponName(int iWeaponID)
{
	switch (iWeaponID) {
	case WEAPON_FIST:
		return "Fist";
	case WEAPON_BRASSKNUCKLE:
		return "Brass Knuckles";
	case WEAPON_GOLFCLUB:
		return "Golf Club";
	case WEAPON_NITESTICK:
		return "Nite Stick";
	case WEAPON_KNIFE:
		return "Knife";
	case WEAPON_BAT:
		return "Baseball Bat";
	case WEAPON_SHOVEL:
		return "Shovel";
	case WEAPON_POOLSTICK:
		return "Pool Cue";
	case WEAPON_KATANA:
		return "Katana";
	case WEAPON_CHAINSAW:
		return "Chainsaw";
	case WEAPON_DILDO:
		return "Dildo";
	case WEAPON_DILDO2:
		return "Dildo";
	case WEAPON_VIBRATOR:
		return "Vibrator";
	case WEAPON_VIBRATOR2:
		return "Vibrator";
	case WEAPON_FLOWER:
		return "Flowers";
	case WEAPON_CANE:
		return "Cane";
	case WEAPON_GRENADE:
		return "Grenade";
	case WEAPON_TEARGAS:
		return "Teargas";
	case WEAPON_MOLTOV:
		return "Molotov Cocktail";
	case WEAPON_COLT45:
		return "Colt 45";
	case WEAPON_SILENCED:
		return "Silenced Pistol";
	case WEAPON_DEAGLE:
		return "Desert Eagle";
	case WEAPON_SHOTGUN:
		return "Shotgun";
	case WEAPON_SAWEDOFF:
		return "Sawn-off Shotgun";
	case WEAPON_SHOTGSPA:
		return "Combat Shotgun";
	case WEAPON_UZI:
		return "UZI";
	case WEAPON_MP5:
		return "MP5";
	case WEAPON_AK47:
		return "AK47";
	case WEAPON_M4:
		return "M4";
	case WEAPON_TEC9:
		return "TEC9";
	case WEAPON_RIFLE:
		return "Rifle";
	case WEAPON_SNIPER:
		return "Sniper Rifle";
	case WEAPON_ROCKETLAUNCHER:
		return "Rocket Launcher";
	case WEAPON_HEATSEEKER:
		return "Heat Seaker";
	case WEAPON_FLAMETHROWER:
		return "Flamethrower";
	case WEAPON_MINIGUN:
		return "Minigun";
	case WEAPON_SATCHEL:
		return "Satchel Explosives";
	case WEAPON_BOMB:
		return "Bomb";
	case WEAPON_SPRAYCAN:
		return "Spray Can";
	case WEAPON_FIREEXTINGUISHER:
		return "Fire Extinguisher";
	case WEAPON_CAMERA:
		return "Camera";
	case WEAPON_NIGHT_VIS_GOGGLES:
		return "Night Vision Goggles";
	case WEAPON_THERMAL_GOGGLES:
		return "Thermal Goggles";
	case WEAPON_PARACHUTE:
		return "Parachute";
	case WEAPON_VEHICLE:
		return "Vehicle";
	case WEAPON_DROWN:
		return "Drowned";
	case WEAPON_COLLISION:
		return "Splat";
	}
	return "";
}

//----------------------------------------------------

bool DirExists(LPCSTR szPath)
{
	DWORD dwAttr;

	//struct _stat64i32 status;
	//return _stat(szPath, &status) == 0;

	dwAttr = GetFileAttributes(szPath);

	return dwAttr != INVALID_FILE_ATTRIBUTES &&
		dwAttr & FILE_ATTRIBUTE_DIRECTORY;
}

//----------------------------------------------------

bool IsHexChar(char c)
{
	return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

//----------------------------------------------------

bool IsHexCharW(wchar_t c)
{
	return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

//----------------------------------------------------

unsigned long GetColorFromStringEmbed(char* szString)
{
	char szHex[7] = { 0 }; // [17];

	if (*szString &&
		*szString == '{' &&
		*(szString + 1) &&
		IsHexChar(*(szString + 1)) &&
		*(szString + 2) &&
		IsHexChar(*(szString + 2)) &&
		*(szString + 3) &&
		IsHexChar(*(szString + 3)) &&
		*(szString + 4) &&
		IsHexChar(*(szString + 4)) &&
		*(szString + 5) &&
		IsHexChar(*(szString + 5)) &&
		*(szString + 6) &&
		IsHexChar(*(szString + 6)) &&
		*(szString + 7) &&
		*(szString + 7) == '}')
	{
		//memset(szHex, 0, sizeof(szHex));
		strncpy_s(szHex, szString + 1, 6);
		return strtoul(szHex, 0, 16);
	}
	return -1;
}

//----------------------------------------------------

unsigned long GetColorFromStringEmbedW(wchar_t* wszString)
{
	wchar_t wszHex[7] = { 0 }; // [17];

	if (*wszString &&
		*wszString == '{' &&
		*(wszString + 1) &&
		IsHexCharW(*(wszString + 1)) &&
		*(wszString + 2) &&
		IsHexCharW(*(wszString + 2)) &&
		*(wszString + 3) &&
		IsHexCharW(*(wszString + 3)) &&
		*(wszString + 4) &&
		IsHexCharW(*(wszString + 4)) &&
		*(wszString + 5) &&
		IsHexCharW(*(wszString + 5)) &&
		*(wszString + 6) &&
		IsHexCharW(*(wszString + 6)) &&
		*(wszString + 7) &&
		*(wszString + 7) == '}')
	{
		//memset(wszHex, 0, sizeof(wszHex));
		wcsncpy_s(wszHex, wszString + 1, 6);
		return wcstoul(wszHex, 0, 16); // wcstoxl ?!
	}
	return -1;
}

//----------------------------------------------------

void RemoveColorEmbedsFromString(char* szString)
{
	char* szCurrent, * szNext;

	szCurrent = szString;
	szNext = szString + 8;
	while (*szCurrent)
	{
		if (GetColorFromStringEmbed(szCurrent) == (unsigned long)-1)
		{
			szCurrent++;
			szNext++;
		}
		else
			strcpy_s(szCurrent, RSIZE_MAX, szNext);
	}
	*szCurrent = '\0';
}

//----------------------------------------------------

int ConvertMultiToWideString(LPCSTR szSource, LPWSTR szDest, int iLen)
{
	size_t nSourceLen;
	int iResult;

	SecureZeroMemory(szDest, sizeof(WCHAR) * iLen);

	nSourceLen = strlen(szSource);

	//iResult = MultiByteToWideChar(CP_ACP, 0, szSource, strlen(nSrcLen), NULL, 0);
	iResult = MultiByteToWideChar(CP_ACP, 0, szSource, nSourceLen, NULL, 0);

	if (iResult < iLen)
		//iResult = MultiByteToWideChar(CP_ACP, 0, szSource, strlen(szSource), szDest, iResult);
		iResult = MultiByteToWideChar(CP_ACP, 0, szSource, nSourceLen, szDest, iResult);

	return iResult;
}
