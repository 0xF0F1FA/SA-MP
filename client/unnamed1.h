
#pragma once

class CUnnamed1 // size=40
{
private:
	IDirect3DDevice9* m_pDevice; // 0
	// 12 (280)
	// 16 (150)
	int m_i20; // 20 (210)
	int m_i24; // 24 (30)
	int m_i28; // 28 (38)
	CDXUTDialog* m_pDialog; // 32 (0)
	// 36 (0)

public:
	CUnnamed1(IDirect3DDevice9* pDevice);

	void InitUI(CDXUTDialog* pDialog);
	void Draw();
};
