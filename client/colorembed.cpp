
#include "main.h"

static ID3DXSprite* pSavedSprite;
static D3DCOLOR SavedColor;
static size_t nLastStringIndex;
static char* szModifiedString;
static char* szOriginalString;
static wchar_t wszStringCopy[MAX_STRING_LENGTH]; // [20000];

// There's must be a better solution...
static ID3DXSpriteCE x;
static ID3DXSprite* pFakeSprite = &x;

void AllocateBufferForColorEmbed()
{
	szModifiedString = (char*)calloc(1, MAX_STRING_LENGTH + 1);
	szOriginalString = (char*)calloc(1, MAX_STRING_LENGTH + 1);
}

static int ProcessStringForColorEmbed()
{
	unsigned long i;
	size_t n1, n2;
	wchar_t c;

	n1 = nLastStringIndex;
	c = *(wszStringCopy + nLastStringIndex);
	if (c)
	{
		do {
			if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
				break;
			c = *(wszStringCopy + n1++ + 1);
		} while (c);
		nLastStringIndex = n1;
	}

	for (i = GetColorFromStringEmbedW(wszStringCopy + n1);
		i != (unsigned long)-1;
		i = GetColorFromStringEmbedW(wszStringCopy + n2))
	{
		n2 = nLastStringIndex + 8;
		SavedColor = i | 0xFF000000;
		c = *(wszStringCopy + nLastStringIndex + 8);
		nLastStringIndex += 8;
		if (c)
		{
			do {
				if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
					break;
				c = *(wszStringCopy + n2++ + 1);
			} while (c);
			nLastStringIndex = n2;
		}
	}
	n1 = nLastStringIndex;
	if (szOriginalString[nLastStringIndex])
		n1 = nLastStringIndex++ + 1;
	return n1;
}

//----------------------------------------------------
// ID3DXFontCE
//----------------------------------------------------

#pragma region

HRESULT __stdcall ID3DXFontCE::QueryInterface(THIS_ REFIID iid, LPVOID* ppv)
{
	return m_pFont->QueryInterface(iid, ppv);
}

ULONG __stdcall ID3DXFontCE::AddRef(THIS)
{
	return m_pFont->AddRef();
}

ULONG __stdcall ID3DXFontCE::Release(THIS)
{
	return m_pFont->Release();
}

HRESULT __stdcall ID3DXFontCE::GetDevice(THIS_ LPDIRECT3DDEVICE9* ppDevice)
{
	return m_pFont->GetDevice(ppDevice);
}

HRESULT __stdcall ID3DXFontCE::GetDescA(THIS_ D3DXFONT_DESCA* pDesc)
{
	return m_pFont->GetDescA(pDesc);
}

HRESULT __stdcall ID3DXFontCE::GetDescW(THIS_ D3DXFONT_DESCW* pDesc)
{
	return m_pFont->GetDescW(pDesc);
}

BOOL __stdcall ID3DXFontCE::GetTextMetricsA(THIS_ TEXTMETRICA* pTextMetrics)
{
	return m_pFont->GetTextMetricsA(pTextMetrics);
}

BOOL __stdcall ID3DXFontCE::GetTextMetricsW(THIS_ TEXTMETRICW* pTextMetrics)
{
	return m_pFont->GetTextMetricsW(pTextMetrics);
}

HDC __stdcall ID3DXFontCE::GetDC(THIS)
{
	return m_pFont->GetDC();
}

HRESULT __stdcall ID3DXFontCE::GetGlyphData(THIS_ UINT Glyph, LPDIRECT3DTEXTURE9* ppTexture, RECT* pBlackBox, POINT* pCellInc)
{
	return m_pFont->GetGlyphData(Glyph, ppTexture, pBlackBox, pCellInc);
}

HRESULT __stdcall ID3DXFontCE::PreloadCharacters(THIS_ UINT First, UINT Last)
{
	return m_pFont->PreloadCharacters(First, Last);
}

HRESULT __stdcall ID3DXFontCE::PreloadGlyphs(THIS_ UINT First, UINT Last)
{
	return m_pFont->PreloadGlyphs(First, Last);
}

HRESULT __stdcall ID3DXFontCE::PreloadTextA(THIS_ LPCSTR pString, INT Count)
{
	return m_pFont->PreloadTextA(pString, Count);
}

HRESULT __stdcall ID3DXFontCE::PreloadTextW(THIS_ LPCWSTR pString, INT Count)
{
	return m_pFont->PreloadTextW(pString, Count);
}

INT __stdcall ID3DXFontCE::DrawTextA(THIS_ LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color)
{
	if (pSprite)
	{
		pSavedSprite = pSprite;
		SavedColor = Color;
		nLastStringIndex = 0;

		//if (strlen(pString) <= MAX_STRING_LENGTH)
		{
			strcpy_s(szOriginalString, MAX_STRING_LENGTH, pString);
			strcpy_s(szModifiedString, MAX_STRING_LENGTH, pString);
			ConvertMultiToWideString(szOriginalString, wszStringCopy, MAX_STRING_LENGTH);
			RemoveColorEmbedsFromString(szModifiedString);
			return m_pFont->DrawTextA(pFakeSprite, szModifiedString, strlen(szModifiedString), pRect, Format, Color);
		}
	}
	return m_pFont->DrawTextA(pSprite, pString, Count, pRect, Format, Color);

	return 0;
}

INT __stdcall ID3DXFontCE::DrawTextW(THIS_ LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color)
{
	return m_pFont->DrawTextW(pSprite, pString, Count, pRect, Format, Color);
}

HRESULT __stdcall ID3DXFontCE::OnLostDevice(THIS)
{
	return m_pFont->OnLostDevice();
}

HRESULT __stdcall ID3DXFontCE::OnResetDevice(THIS)
{
	return m_pFont->OnResetDevice();
}

#pragma endregion ID3DXFontCE

//----------------------------------------------------
// ID3DXSpriteCE
//----------------------------------------------------

#pragma region 

HRESULT __stdcall ID3DXSpriteCE::QueryInterface(THIS_ REFIID iid, LPVOID* ppv)
{
	return pSavedSprite->QueryInterface(iid, ppv);
}

ULONG __stdcall ID3DXSpriteCE::AddRef(THIS)
{
	return pSavedSprite->AddRef();
}

ULONG __stdcall ID3DXSpriteCE::Release(THIS)
{
	return pSavedSprite->Release();
}

HRESULT __stdcall ID3DXSpriteCE::GetDevice(THIS_ LPDIRECT3DDEVICE9* ppDevice)
{
	return pSavedSprite->GetDevice(ppDevice);
}

HRESULT __stdcall ID3DXSpriteCE::GetTransform(THIS_ D3DXMATRIX* pTransform)
{
	return pSavedSprite->GetTransform(pTransform);
}

HRESULT __stdcall ID3DXSpriteCE::SetTransform(THIS_ CONST D3DXMATRIX* pTransform)
{
	return pSavedSprite->SetTransform(pTransform);
}

HRESULT __stdcall ID3DXSpriteCE::SetWorldViewRH(THIS_ CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView)
{
	return pSavedSprite->SetWorldViewRH(pWorld, pView);
}

HRESULT __stdcall ID3DXSpriteCE::SetWorldViewLH(THIS_ CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView)
{
	return pSavedSprite->SetWorldViewLH(pWorld, pView);
}

HRESULT __stdcall ID3DXSpriteCE::Begin(THIS_ DWORD Flags)
{
	return pSavedSprite->Begin(Flags);
}

HRESULT __stdcall ID3DXSpriteCE::Draw(THIS_ LPDIRECT3DTEXTURE9 pTexture, CONST RECT* pSrcRect, CONST D3DXVECTOR3* pCenter, CONST D3DXVECTOR3* pPosition, D3DCOLOR Color)
{
	ProcessStringForColorEmbed();
	return  pSavedSprite->Draw(pTexture, pSrcRect, pCenter, pPosition, SavedColor);
}

HRESULT __stdcall ID3DXSpriteCE::Flush(THIS)
{
	return pSavedSprite->Flush();
}

HRESULT __stdcall ID3DXSpriteCE::End(THIS)
{
	return pSavedSprite->End();
}

HRESULT __stdcall ID3DXSpriteCE::OnLostDevice(THIS)
{
	return pSavedSprite->OnLostDevice();
}

HRESULT __stdcall ID3DXSpriteCE::OnResetDevice(THIS)
{
	return pSavedSprite->OnResetDevice();
}

#pragma endregion ID3DXSpriteCE
