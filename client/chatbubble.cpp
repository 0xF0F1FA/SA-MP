
#include "main.h"

CChatBubble::CChatBubble()
{
	SecureZeroMemory(m_ChatBubbleEntries, sizeof(m_ChatBubbleEntries));
}

void CChatBubble::AddText(WORD wPlayerID, char* szText,
	DWORD dwColor, float fDistance, int iDuration)
{
	CHAT_BUBBLE_ENTRY* pEntry;

	if (wPlayerID < MAX_PLAYERS)
	{
		pEntry = &m_ChatBubbleEntries[wPlayerID];

		SecureZeroMemory(pEntry, sizeof(CHAT_BUBBLE_ENTRY));

		if (strlen(szText) <= MAX_CHAT_BUBBLE_TEXT)
		{
			strcpy_s(pEntry->szText, szText);
			pEntry->iLineCount = FormatChatBubbleText(pEntry->szText, 36, 12) - 1;
			pEntry->dwColor = (dwColor >> 8) | 0xFF000000;
			pEntry->fDistance = fDistance;
			pEntry->dwExpireTick = GetTickCount() + (DWORD)iDuration;
			//m_ChatBubbleEntries[i].dwDuration = (DWORD)iDuration;
			//m_ChatBubbleEntries[i].dwCreationTick = GetTickCount();
			pEntry->bValid = true;
		}
	}
}

void CChatBubble::Draw()
{
	if (!pNetGame && !pGame->FindPlayerPed() && !pLabel)
		return;

	CHAT_BUBBLE_ENTRY* pEntry;
	DWORD dwTick = GetTickCount();
	CRemotePlayer* pRemotePlayer;
	CPlayerPed* pPlayerPed;
	VECTOR vecHeadBonePos;
	D3DXVECTOR3 vecPos;
	float fTextHeight;
	int i = 0;

	pLabel->Begin();

	while (i!=MAX_PLAYERS)
	{
		pEntry = &m_ChatBubbleEntries[i];

		if (pEntry->bValid)
		{
			//if(dwTick <= pEntry->dwCreationTick + pEntry->dwDuration)
			if (dwTick <= pEntry->dwExpireTick)
			{
				pRemotePlayer = pNetGame->GetPlayerPool()->GetAt(i);

				if (pRemotePlayer && pRemotePlayer->GetPlayerPed() && pRemotePlayer->GetState())
				{
					pPlayerPed = pRemotePlayer->GetPlayerPed();

					if (pPlayerPed->GetDistanceFromLocalPlayerPed() <= pEntry->fDistance)
					{
						pPlayerPed->GetBonePosition(8, &vecHeadBonePos);

						vecPos.x = vecHeadBonePos.X;
						vecPos.y = vecHeadBonePos.Y;
						vecPos.z = vecHeadBonePos.Z;
						
						fTextHeight = ((pEntry->iLineCount * 0.0125f) + 0.065f) + (pEntry->iLineCount * 0.0125f);
						vecPos.z += pPlayerPed->GetDistanceFromCamera() * fTextHeight + 0.2f;

						pLabel->Draw(&vecPos, pEntry->szText, pEntry->dwColor, true, false);
					}
				}
			}
			else
			{
				pEntry->bValid = false;
			}
		}
		i++;
	}

	pLabel->End();
}
