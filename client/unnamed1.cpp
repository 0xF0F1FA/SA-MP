
#include "main.h"

// global variable CUnnamed1: dword_1026E9CC
// global variable for CDXUTDialog: dword_1026EAAC

#define SLIDER_1_ID 30
#define SLIDER_2_ID 32
#define SLIDER_3_ID 34

CUnnamed1::CUnnamed1(IDirect3DDevice9* pDevice) // sub_10071690
{
	m_pDevice = pDevice; // 0
	m_pDialog = NULL; // 32
	// 36
	// 12 (280)
	// 16 (150)
	m_i20 = 210; // 20 (210)
	m_i24 = 30; // 24 (30)
	m_i28 = 38; // 28 (38)
}

/*
	In SetupGameUI():

	dword_1026EAAC = new CDXUTDialog();
	dword_1026EAAC->Init(pDialogResourceManager);
	dword_1026EAAC->SetCallback(nullsub_8);
	dword_1026EAAC->SetLocation(0,0);
	dword_1026EAAC->SetSize(500, 160);
	dword_1026EAAC->EnableMouseInput(true);
	dword_1026EAAC->EnableKeyboardInput(true);
	dword_1026EAAC->SetBackgroundColors(0xDC050505);
	dword_1026EAAC->SetVisible(false);
	if(dword_1026E9CC)
		dword_1026E9CC->sub_100716D0(dword_1026EAAC);
*/
void CUnnamed1::InitUI(CDXUTDialog* pDialog) // sub_100716D0
{
	CDXUTControl* pControl;
	int y;

	m_pDialog = pDialog;
	if (pDialog)
	{
		pDialog->AddSlider(SLIDER_1_ID, 50, 10, m_i20, m_i24, -100, 100, 0, false, NULL);
		pControl = m_pDialog->GetControl(SLIDER_1_ID, DXUT_CONTROL_SLIDER);

		y = m_i28 + 10;
		m_pDialog->AddSlider(SLIDER_2_ID, 50, y, m_i20, m_i24, -100, 100, 0, false, NULL);
		pControl = m_pDialog->GetControl(SLIDER_2_ID, DXUT_CONTROL_SLIDER);

		m_pDialog->AddSlider(SLIDER_3_ID, 50, y + m_i28, m_i20, m_i24, -100, 100, 0, false, NULL);
		pControl = m_pDialog->GetControl(SLIDER_3_ID, DXUT_CONTROL_SLIDER);
	}
}


void CUnnamed1::Draw()
{
	//if (m_p36)
	{
		//sub_10071960();
		m_pDialog->OnRender(10.0f);
		//sub_10071A10();
	}
}
