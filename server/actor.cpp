
#include "main.h"

CActor::CActor(int iSkin, VECTOR* vecPos, float fRotation)
{
	m_SpawnInfo.byteTeam = 0;
	m_SpawnInfo.vecPos.X = vecPos->X;
	m_SpawnInfo.vecPos.Y = vecPos->Y;
	m_SpawnInfo.vecPos.Z = vecPos->Z;

	if (iSkin > 20000 && pArtwork) {
		int iNewSkin = pArtwork->GetBaseIDFromNewID(iSkin);
		if (iNewSkin < 0 || iNewSkin >= 20000) {
			m_SpawnInfo.iSkin = 0;
			m_SpawnInfo.iBaseSkin = 0;
		} else {
			m_SpawnInfo.iSkin = iNewSkin;
			m_SpawnInfo.iBaseSkin = iSkin;
		}
	} else {
		m_SpawnInfo.iSkin = iSkin;
		m_SpawnInfo.iBaseSkin = 0;
	}

	m_SpawnInfo.fRotation = fRotation;
	m_vecPos.X = vecPos->X;
	m_vecPos.Y = vecPos->Y;
	m_vecPos.Z = vecPos->Z;
	m_fRotation = fRotation;
	bHasAnimation = false;
	m_fHealth = 100.0f;
	m_byteInvulnerable = 1;
	memset(&m_Animation, 0, sizeof(ANIMATION_INFO));
}

CActor::~CActor()
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool) {
		pPlayerPool->DestroyActorForPlayers(m_wActorID);
	}
}

void CActor::UpdatePosition(float x, float y, float z)
{
	m_vecPos.X=x;
	m_vecPos.Y=y;
	m_vecPos.Z=z;
}

void CActor::UpdateRotation(float fRotation)
{
	m_fRotation = fRotation;
}

float CActor::GetSquaredDistanceFrom2DPoint(float fX, float fY)
{
	float fSX,fSY;

	fSX = m_vecPos.X - fX * m_vecPos.X - fX,
	fSY = m_vecPos.Y - fY * m_vecPos.Y - fY;

	return fSX + fSY;
}

void CActor::SetHealth(float fHealth)
{
	m_fHealth = fHealth;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool) return;

	RakNet::BitStream bsHealth;
	bsHealth.Write(m_wActorID);
	bsHealth.Write(m_fHealth);

	for (int i = 0; i <= pPlayerPool->GetPoolSize(); i++)
	{
		if (pPlayerPool->GetSlotState(i))
		{
			CPlayer* pPlayer = pPlayerPool->GetAt(i);
			if (pPlayer)
			{
				if (pPlayer->IsActorStreamedIn(m_wActorID))
				{
					pNetGame->RPC(RPC_ScrSetActorHealth, &bsHealth, i, 2);
				}
			}
		}
	}
}

void CActor::SendAnimation(WORD wPlayerID, ANIMATION_INFO* pAnim)
{
	RakNet::BitStream bsSend;
	BYTE byteAnimLibLen = (BYTE)strlen(pAnim->szAnimLib);
	BYTE byteAnimNameLen; (BYTE)strlen(pAnim->szAnimName);

	bsSend.Write(m_wActorID);
	bsSend.Write(byteAnimLibLen);
	bsSend.Write(pAnim->szAnimLib, byteAnimLibLen);
	bsSend.Write(byteAnimNameLen);
	bsSend.Write(pAnim->szAnimName, byteAnimNameLen);
	bsSend.Write(pAnim->fDelta);
	bsSend.Write(pAnim->bLoop);
	bsSend.Write(pAnim->bLockX);
	bsSend.Write(pAnim->bLockY);
	bsSend.Write(pAnim->bFreeze);
	bsSend.Write(pAnim->iTime);

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool && pPlayerPool->GetAt(wPlayerID)) {
		pNetGame->RPC(RPC_ScrApplyActorAnimation, &bsSend, wPlayerID, 2);
	}
}

void CActor::ClearAnimations()
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool) return;

	RakNet::BitStream bsSend;
	bsSend.Write(m_wActorID);
	for (int i = 0; i <= pPlayerPool->GetPoolSize(); i++)
	{
		if (pPlayerPool->GetSlotState(i))
		{
			CPlayer* pPlayer = pPlayerPool->GetAt(i);
			if (pPlayer)
			{
				if (pPlayer->IsActorStreamedIn(m_wActorID))
				{
					pNetGame->RPC(RPC_ScrClearActorAnimation, &bsSend, i, 2);
				}
			}
		}
	}

	bHasAnimation = false;
	memset(&m_Animation, 0, sizeof(ANIMATION_INFO));
}

void CActor::ApplyAnimation(char* szAnimLib, char* szAnimName, float fDelta, bool bLoop, bool bLockX, bool bLockY, bool bFreeze, int iTime)
{
	ANIMATION_INFO Animation;
	memset(&Animation, 0, sizeof(ANIMATION_INFO));

	strncpy(Animation.szAnimLib, szAnimLib, 64);
	strncpy(Animation.szAnimName, szAnimName, 64);
	Animation.fDelta = fDelta;
	Animation.bLoop = bLoop;
	Animation.bLockX = bLockX;
	Animation.bLockY = bLockY;
	Animation.bFreeze = bFreeze;
	Animation.iTime = iTime;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (pPlayerPool)
	{
		for (int i = 0; i <= pPlayerPool->GetPoolSize(); i++)
		{
			if (pPlayerPool->GetSlotState(i))
			{
				CPlayer* pPlayer = pPlayerPool->GetAt(i);
				if (pPlayer)
				{
					if (pPlayer->IsActorStreamedIn(m_wActorID))
					{
						SendAnimation(i, &Animation);
					}
				}
			}
		}
	}

	if (bLoop || bFreeze)
	{
		bHasAnimation = true;
		memcpy(&m_Animation, &Animation, sizeof(ANIMATION_INFO));
	}
	else
	{
		bHasAnimation = false;
		memset(&m_Animation, 0, sizeof(ANIMATION_INFO));
	}
}
