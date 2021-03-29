
#include "main.h"

CCursor::CCursor(IDirect3DDevice9* pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;
	m_pSurface = NULL;

	//RestoreDeviceObjects();
}

void CCursor::DeleteDeviceObjects()
{
	SAFE_RELEASE(m_pSurface);
}

void CCursor::RestoreDeviceObjects()
{
	if (m_pSurface) return;

	m_pD3DDevice->CreateOffscreenPlainSurface(32, 32,
		D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pSurface, NULL);

	D3DXLoadSurfaceFromFileA(m_pSurface, NULL,
		NULL, "mouse.png", NULL, D3DX_FILTER_NONE, 0, NULL);

	if (m_pSurface)
		m_pD3DDevice->SetCursorProperties(0, 0, m_pSurface);
}
