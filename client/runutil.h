//----------------------------------------------------
//
//	Utility and runtime procedures header
//	(c) 2005 Kye Bitossi
//
//  Version: $Id: runutil.h,v 1.3 2006/03/20 17:44:19 kyeman Exp $
//
//----------------------------------------------------

int FormatChatBubbleText(char* szText, int iWidth, int iMaxWord);
void Util_UrlUnencode(char *enc);
char Util_toupper(char c);
char *Util_stristr(const char *String, const char *Pattern);
void Util_strupr(char *string);
int Util_wildcmp(char *wild, char *string);
int Util_strnicmp(const char *s1, const char *s2, size_t n);
char *Util_strrev(char *str);
char * Util_itoa(int v, char *s, int r);
void Util_Base64Encode( char *cpInput, char *cpOutput );

char * K_DecodeString(unsigned char *szInput);
void K_EncodeString(char *szInput, char *szOutput);

const char* GetWeaponName(int iWeaponID);

bool DirExists(LPCSTR szPath);

bool IsHexChar(char c);
bool IsHexCharW(wchar_t c);
unsigned long GetColorFromStringEmbed(char* szString);
unsigned long GetColorFromStringEmbedW(wchar_t* wszString);
void RemoveColorEmbedsFromString(char* szString);
void FormatGameKeysInString(PCHAR buf);
DWORD FormatGameTextKey(PCHAR szBuf, DWORD dwMaxLen);
int ConvertMultiToWideString(LPCSTR szSource, LPWSTR szDest, int iLen);
void Transform(VECTOR* vecOut, MATRIX4X4* matIn, VECTOR* vecIn);
void CrossProduct(VECTOR* pIn, VECTOR* pOut);

//----------------------------------------------------