//
// Version: $Id: label.h,v 1.2 2006/03/20 17:44:19 kyeman Exp $
//

#pragma once
#ifndef LABEL_H
#define LABEL_H

class CLabel
{
private:
	IDirect3DDevice9* m_pDevice;
	ID3DXSprite* m_pSprite;

public:
	CLabel(IDirect3DDevice9* pDevice);
	~CLabel();

	void Begin();
	void End();

	int IsLineOfSightClear(float fX, float fY, float fZ);

	void Draw(D3DXVECTOR3* ppos, char* szText, DWORD dwColor, bool bShadowed, bool bDoLOS);
	void DeleteDeviceObjects();
	void RestoreDeviceObjects();
};

#endif