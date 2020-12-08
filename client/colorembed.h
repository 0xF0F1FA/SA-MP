
#pragma once

void AllocateBufferForColorEmbed();

struct ID3DXFontCE : public ID3DXFont
{
public:
	HRESULT __stdcall QueryInterface(THIS_ REFIID iid, LPVOID* ppv);
	ULONG __stdcall AddRef(THIS);
	ULONG __stdcall Release(THIS);
	HRESULT __stdcall GetDevice(THIS_ LPDIRECT3DDEVICE9* ppDevice);
	HRESULT __stdcall GetDescA(THIS_ D3DXFONT_DESCA* pDesc);
	HRESULT __stdcall GetDescW(THIS_ D3DXFONT_DESCW* pDesc);
	BOOL __stdcall GetTextMetricsA(THIS_ TEXTMETRICA* pTextMetrics);
	BOOL __stdcall GetTextMetricsW(THIS_ TEXTMETRICW* pTextMetrics);
	HDC __stdcall GetDC(THIS);
	HRESULT __stdcall GetGlyphData(THIS_ UINT Glyph, LPDIRECT3DTEXTURE9* ppTexture, RECT* pBlackBox, POINT* pCellInc);
	HRESULT __stdcall PreloadCharacters(THIS_ UINT First, UINT Last);
	HRESULT __stdcall PreloadGlyphs(THIS_ UINT First, UINT Last);
	HRESULT __stdcall PreloadTextA(THIS_ LPCSTR pString, INT Count);
	HRESULT __stdcall PreloadTextW(THIS_ LPCWSTR pString, INT Count);
	INT __stdcall DrawTextA(THIS_ LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color);
	INT __stdcall DrawTextW(THIS_ LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color);
	HRESULT __stdcall OnLostDevice(THIS);
	HRESULT __stdcall OnResetDevice(THIS);

	inline void SetDXFont(ID3DXFont* pFont) { m_pFont = pFont; };

private:
	ID3DXFont* m_pFont;
};

struct ID3DXSpriteCE : public ID3DXSprite
{
	virtual HRESULT __stdcall QueryInterface(THIS_ REFIID iid, LPVOID* ppv);
	virtual ULONG __stdcall AddRef(THIS);
	virtual ULONG __stdcall Release(THIS);
	virtual HRESULT __stdcall GetDevice(THIS_ LPDIRECT3DDEVICE9* ppDevice);
	virtual HRESULT __stdcall GetTransform(THIS_ D3DXMATRIX* pTransform);
	virtual HRESULT __stdcall SetTransform(THIS_ CONST D3DXMATRIX* pTransform);
	virtual HRESULT __stdcall SetWorldViewRH(THIS_ CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView);
	virtual HRESULT __stdcall SetWorldViewLH(THIS_ CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView);
	virtual HRESULT __stdcall Begin(THIS_ DWORD Flags);
	virtual HRESULT __stdcall Draw(THIS_ LPDIRECT3DTEXTURE9 pTexture, CONST RECT* pSrcRect, CONST D3DXVECTOR3* pCenter, CONST D3DXVECTOR3* pPosition, D3DCOLOR Color);
	virtual HRESULT __stdcall Flush(THIS);
	virtual HRESULT __stdcall End(THIS);
	virtual HRESULT __stdcall OnLostDevice(THIS);
	virtual HRESULT __stdcall OnResetDevice(THIS);
};
