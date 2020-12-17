
#include "main.h"

CLicensePlate::CLicensePlate(IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice;
	m_pTexture = NULL;
	m_pRenderToSurface = NULL;
	m_pSurface = NULL;

	RestoreDeviceObjects();
}

CLicensePlate::~CLicensePlate()
{
	DeleteDeviceObjects();
}

void CLicensePlate::DeleteDeviceObjects()
{
	SAFE_RELEASE(m_pSurface);
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pRenderToSurface);
	SAFE_RELEASE(m_pLastTexture);
}

void CLicensePlate::RestoreDeviceObjects()
{
	HRESULT hResult;
	D3DSURFACE_DESC Desc;

	m_pDevice->GetDisplayMode(0, &m_DisplayMode);

	hResult = D3DXCreateTexture(m_pDevice, 128, 32, 1, 1, m_DisplayMode.Format, D3DPOOL_DEFAULT, &m_pTexture);
	if (SUCCEEDED(hResult))
	{
		m_pTexture->GetSurfaceLevel(0, &m_pSurface);
		m_pSurface->GetDesc(&Desc);
		D3DXCreateRenderToSurface(m_pDevice, Desc.Width, Desc.Height, Desc.Format, 1, D3DFMT_D16, &m_pRenderToSurface);
	}
}

IDirect3DTexture9* CLicensePlate::Make(char* szPlateText)
{
	IDirect3DTexture9* pTexture;
	IDirect3DSurface9* pSurface;

	if (m_pDevice &&
		pDefaultFont &&
		m_pRenderToSurface &&
		m_pTexture &&
		m_pSurface &&
		SUCCEEDED(D3DXCreateTexture(m_pDevice, 64, 32, 1, 0, m_DisplayMode.Format, D3DPOOL_DEFAULT, &pTexture)))
	{
		m_pRenderToSurface->BeginScene(m_pSurface, NULL);
		m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFBEB6A8, 1.0f, 0);
		pDefaultFont->DrawTextForPlate(szPlateText, { 0, 3, 128, 32 }, 0xEE444470);
		m_pRenderToSurface->EndScene(0);

		pTexture->SetAutoGenFilterType(D3DTEXF_LINEAR);

		pTexture->GetSurfaceLevel(0, &pSurface);
		D3DXLoadSurfaceFromSurface(pSurface, NULL, NULL, m_pSurface, NULL, NULL, D3DX_FILTER_LINEAR, 0);
		pSurface->Release();

		return pTexture;
	}
	return NULL;
}
