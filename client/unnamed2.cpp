
#include "main.h"

extern D3DXMATRIX matView, matProj;

CUnnamed2::CUnnamed2(IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice; // 259
	m_i124 = 0; // 124 (0)
	m_i275 = -1; // 275 (-1)
	// 128 (0)
	// 132 (0)
	m_iEditObjectType = 0; // 120 (0)
	// 163 (0)
	// 164 (0)
	// 165 (0)
	// 166 (0)
	// 162 (0)
	m_usObjectID = (unsigned short)-1; // 136 (-1)
	m_dwTick = GetTickCount(); // 158

	D3DXCreateLine(m_pDevice, &m_pLine);

	D3DXCreateFontA(m_pDevice, 22, 0, FW_NORMAL, 1, FALSE, SYMBOL_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "SAMPAUX3", &m_pSmallFont);
	D3DXCreateFontA(m_pDevice, 28, 0, FW_NORMAL, 1, FALSE, SYMBOL_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "SAMPAUX3", &m_pBigFont);
}

void CUnnamed2::RestoreDeviceObjects()
{
	if (m_pSmallFont)
		m_pSmallFont->OnResetDevice();

	if (m_pBigFont)
		m_pBigFont->OnResetDevice();

	if (m_pLine)
		m_pLine->OnResetDevice();
}

void CUnnamed2::DetroyDeviceObjects()
{
	if (m_pSmallFont)
		m_pSmallFont->OnLostDevice();

	if (m_pBigFont)
		m_pBigFont->OnLostDevice();

	if (m_pLine)
		m_pLine->OnLostDevice();
}

void CUnnamed2::Draw()
{

}

const char* CUnnamed2::GetRoundedBox()
{
	return "C";
}

const char* CUnnamed2::GetIcon(int iType)
{
	switch (iType)
	{
	case 0:
		switch (m_i124)
		{
		case 0: return "6"; // X movement
		case 1: return "3"; // X rotation
		case 2: return "9"; // X scaling
		}
		break;
	case 1:
		switch (m_i124)
		{
		case 0: return "7";
		case 1: return "4";
		case 2: return "A";
		}
		break;
	case 2:
		switch (m_i124)
		{
		case 0: return "8";
		case 1: return "5";
		case 2: return "B";
		}
		break;
	}
	return "0";
}

void ResetRect(RECT* rect)
{
	rect->left = 0;
	rect->top = 0;
	rect->right = 0;
	rect->bottom = 0;
}

BOOL CUnnamed2::SetCursorToScreenCenter()
{
	RECT Rect;
	LONG X, Y;

	GetClientRect(pGame->GetMainWindowHwnd(), &Rect);

	X = (Rect.right - Rect.left) / 2;
	Y = (Rect.bottom - Rect.top) / 2;

	m_iScreenCenterX = X;
	m_iScreenCenterY = Y;

	return SetCursorPos(X, Y);
}

void CUnnamed2::SendEditObjectNotification(int iResponse) // sub_100728E0
{
	MATRIX4X4 mat;
	DWORD dwTickNow;

	dwTickNow = GetTickCount();
	if (dwTickNow - m_dwTick >= 250)
	{
		m_dwTick = dwTickNow;

		RakNet::BitStream bsSend;

		//bsSend.Write()
	}
}

void CUnnamed2::SendEditAttachedObjectNotification() // sub_10072AF0
{
}

void MultiplateVectorValue(D3DXVECTOR3* pOut, MATRIX4X4* pMatrix, D3DXVECTOR3* pVector)
{
	pOut->x = pMatrix->at.X * pVector->z + pMatrix->up.X * pVector->y + pMatrix->right.X * pVector->x + pMatrix->pos.X;
	pOut->y = pMatrix->at.Y * pVector->z + pMatrix->up.Y * pVector->y +	pMatrix->right.Y * pVector->x + pMatrix->pos.Y;
	pOut->z = pMatrix->at.Z * pVector->z + pMatrix->up.Z * pVector->y + pMatrix->right.Z * pVector->y + pMatrix->pos.Z;
}

bool CUnnamed2::sub_10071D50()
{
	D3DXVECTOR2 VertexList;

	if (m_pLine)
	{
		m_pLine->SetAntialias(TRUE);

		m_pLine->Begin();
		m_pLine->Draw(&VertexList, 2, 0xFFCC9999);
		m_pLine->End();
	}
	return false;
}

float CUnnamed2::sub_10071C50(D3DXVECTOR3* pVector, D3DXVECTOR3* pOut)
{
	D3DVIEWPORT9 ViewPort;
	D3DXVECTOR3 Out;
	D3DXMATRIX matIdent;

	m_pDevice->GetViewport(&ViewPort);

	matIdent = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	D3DXVec3Project(&Out, pVector, &ViewPort, &matProj, &matView, &matIdent);

	pOut->x = Out.x;
	pOut->y = Out.y;

	return pOut->z;
}

// Not exacty how it looks like, but it would be a nightmare to insert it down here
void CUnnamed2::sub_100720D0() // sub_100720D0
{
	POINT Point;

	GetCursorPos(&Point);
	ScreenToClient(pGame->GetMainWindowHwnd(), &Point);

	//if(this+33 && pGame && pGame+97)
	{
		m_i275 = -1;

		for (int i = 0; i < ARRAY_SIZE(m_Rect); i++)
		{
			if (PtInRect(&m_Rect[i], Point))
			{
				// On the 6th check, its sets this+275 to 10, instead of 6, currently not sure why
				m_i275 = i;
				return;
			}
		}
	}
	//else
		//m_i275 = -1;
}

/*
*						 Exterminate! Exterminate!
*						/
		  _n____n__
		 /         \---||--<
		/___________\
		_|____|____|_
		_|____|____|_
		 |    |    |
		--------------
		| || || || ||\
		| || || || || \++++++++------<
		===============
		|   |  |  |   |
	   (| O | O| O| O |)
	   |   |   |   |   |
	  (| O | O | O | O |)
	   |   |   |   |    |
	 (| O |  O | O  | O |)
	  |   |    |    |    |
	 (| O |  O |  O |  O |)
	 ======================
*/
