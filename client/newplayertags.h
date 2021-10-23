//
// Version: $Id: newplayertags.h,v 1.6 2006/05/07 15:38:35 kyeman Exp $
//

#pragma once

#define HealthBar1FVF D3DFVF_XYZRHW|D3DFVF_DIFFUSE

struct HealthBarVertices1_s
{
	float x, y, z, rhw;
	D3DCOLOR c;
};

class CNewPlayerTags
{
private:
	IDirect3DDevice9* m_pDevice;
	IDirect3DStateBlock9* m_pStateBlock;
	ID3DXSprite* m_pSprite;

public:
	CNewPlayerTags(IDirect3DDevice9* pDevice);
	~CNewPlayerTags();

	void BeginBars();
	void RestoreDeviceObjects();
	void DeleteDeviceObjects();
	void BeginLabels();
	void EndLabels();
	void EndBars();
	void Draw(D3DXVECTOR3* pPlayerPos, char* pNameText, DWORD dwColor, float fDistanceFromLocalPlayer);
	void Draw(D3DXVECTOR3* pPlayerPos, float fHealth, float fArmor, float fDistanceFromLocalPlayer);
};
