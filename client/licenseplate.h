
#pragma once

class CLicensePlate
{
public:
	CLicensePlate(IDirect3DDevice9* pDevice);
	~CLicensePlate();

	void DeleteDeviceObjects();
	void RestoreDeviceObjects();

	IDirect3DTexture9* Make(char* szPlateText);

private:
	IDirect3DDevice9* m_pDevice;
	D3DDISPLAYMODE m_DisplayMode;
	IDirect3DTexture9* m_pTexture;
	IDirect3DSurface9* m_pSurface;
	ID3DXRenderToSurface* m_pRenderToSurface;
	IDirect3DTexture9* m_pLastTexture; // Holds a copy of texture what CLicensePlate::Make() create
};
