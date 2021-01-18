
#pragma once

class CUnnamed2 // size=279
{
private:
	RECT m_Rect[7]; // 8
	int m_iEditObjectType; // 120
	int m_i124; // 124
	// 128
	// 132
	unsigned short m_usObjectID; // 136
	DWORD m_dwTick; // 158
	// 162
	// 163
	// 164
	// 165
	// 166
	int m_iScreenCenterX; // 167
	int m_iScreenCenterY; // 171
	IDirect3DDevice9* m_pDevice; // 259
	ID3DXLine* m_pLine; // 263
	ID3DXFont* m_pSmallFont; // 267
	ID3DXFont* m_pBigFont; // 271
	int m_i275; // 275

public:
	CUnnamed2(IDirect3DDevice9* pDevice);

	void RestoreDeviceObjects();
	void DetroyDeviceObjects();

	void Draw();

	const char* GetRoundedBox();
	const char* GetIcon(int iType);
	BOOL SetCursorToScreenCenter();
	bool sub_10071D50();
	float sub_10071C50(D3DXVECTOR3* pVector, D3DXVECTOR3* pOut);
	void sub_100720D0();

	void SendEditObjectNotification(int iResponse);
	void SendEditAttachedObjectNotification();
};