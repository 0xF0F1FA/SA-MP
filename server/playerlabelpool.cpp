
#include "main.h"

CPlayerLabelPool::CPlayerLabelPool()
{
	m_wPlayerID = INVALID_PLAYER_ID;

	for (WORD wLabelID = 0; wLabelID < MAX_LABEL_PLAYER; wLabelID++)
	{
		memset(&m_Labels[wLabelID], 0, sizeof(LABEL));
		m_bSlotState[wLabelID] = false;
		//m_byte37888[wLabelID] = 0;
	}
}

CPlayerLabelPool::~CPlayerLabelPool()
{
	for (WORD wLabelID = 0; wLabelID < MAX_LABEL_PLAYER; wLabelID++)
	{
		if (m_bSlotState[wLabelID])
		{
			Delete(wLabelID);
		}
	}
}

WORD CPlayerLabelPool::New(char* szText, DWORD dwColor, float fX, float fY, float fZ,
	float fDistance, WORD wAttachedPlayerID, WORD wAttachedVehicleID, bool bTestLOS)
{
	WORD wLabelID = 0;
	while (wLabelID < MAX_LABEL_PLAYER)
	{
		if (!m_bSlotState[wLabelID]) break;
		wLabelID++;
	}
	if (wLabelID == MAX_LABEL_PLAYER) return INVALID_LABEL_ID;

	LABEL* pLabel = &m_Labels[wLabelID];

	DWORD dwLen = strlen(szText);
	pLabel->wLength = (dwLen > MAX_LABEL_TEXT) ? MAX_LABEL_TEXT : (WORD)dwLen;
	pLabel->szText = (char*)calloc(1, dwLen + 1);

	if (pLabel->szText != NULL)
	{
		strncpy(pLabel->szText, szText, pLabel->wLength);

		pLabel->dwColor = dwColor;
		pLabel->fX = fX;
		pLabel->fY = fY;
		pLabel->fZ = fZ;
		pLabel->fDrawDistance = fDistance;
		pLabel->bTestLOS = bTestLOS;
		pLabel->iVirtualWorld = -1;
		pLabel->wAttachedPlayerID = wAttachedPlayerID;
		pLabel->wAttachedVehicleID = wAttachedVehicleID;
		m_bSlotState[wLabelID] = true;

		InitForPlayer(wLabelID);

		return wLabelID;
	}
	return INVALID_LABEL_ID;
}

bool CPlayerLabelPool::Delete(WORD wLabelID)
{
	if (wLabelID < MAX_LABEL_PLAYER && m_bSlotState[wLabelID])
	{
		LABEL* pLabel = &m_Labels[wLabelID];

		if (pLabel->szText)
			free(pLabel->szText);

		DeleteForPlayer(wLabelID);

		memset(pLabel, 0, sizeof(LABEL));
		m_bSlotState[wLabelID] = false;

		return true;
	}
	return false;
}

bool CPlayerLabelPool::UpdateText(WORD wLabelID, DWORD dwColor, char* szText)
{
	if (wLabelID < MAX_LABEL_PLAYER && m_bSlotState[wLabelID])
	{
		DWORD dwTempLen = strlen(szText);
		dwTempLen = (dwTempLen > MAX_LABEL_TEXT) ? MAX_LABEL_TEXT : dwTempLen;
		PCHAR szTempText = (PCHAR)calloc(1, dwTempLen + 1);

		if (szTempText)
		{
			LABEL* pLabel = &m_Labels[wLabelID];

			if (pLabel->szText)
				free(pLabel->szText);

			//strcpy(szTempText, szText);
			strncpy(szTempText, szText, dwTempLen);

			pLabel->szText = szTempText;
			pLabel->wLength = (WORD)dwTempLen;
			pLabel->dwColor = dwColor;

			DeleteForPlayer(wLabelID);
			InitForPlayer(wLabelID);

			return true;
		}
	}
	return false;
}

void CPlayerLabelPool::InitForPlayer(WORD wLabelID)
{
	if (m_bSlotState[wLabelID])
	{
		RakNet::BitStream bsSend;
		LABEL* pLabel = &m_Labels[wLabelID];

		bsSend.Write((WORD)(wLabelID + MAX_LABEL_GLOBAL));
		bsSend.Write(pLabel->dwColor);
		bsSend.Write(pLabel->fX);
		bsSend.Write(pLabel->fY);
		bsSend.Write(pLabel->fZ);
		bsSend.Write(pLabel->fDrawDistance);
		bsSend.Write((BYTE)pLabel->bTestLOS);
		bsSend.Write(pLabel->wAttachedPlayerID);
		bsSend.Write(pLabel->wAttachedVehicleID);

		stringCompressor->EncodeString(pLabel->szText, pLabel->wLength + 1, &bsSend);

		pNetGame->SendToPlayer(m_wPlayerID, RPC_CreateLabel, &bsSend);
	}
}

void CPlayerLabelPool::DeleteForPlayer(WORD wLabelID)
{
	if (m_bSlotState[wLabelID])
	{
		RakNet::BitStream bsSend;

		bsSend.Write((WORD)(wLabelID + MAX_LABEL_GLOBAL));

		pNetGame->SendToPlayer(m_wPlayerID, RPC_DestroyLabel, &bsSend);
	}
}
