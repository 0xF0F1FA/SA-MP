
#pragma once

class CCursor
{
private:
	IDirect3DDevice9* m_pD3DDevice;
	IDirect3DSurface9* m_pSurface;

public:
	CCursor(IDirect3DDevice9* pD3DDevice);

	void DeleteDeviceObjects();
	void RestoreDeviceObjects();
};
