
#include "../main.h"
#include "util.h"

//-----------------------------------------------------------
// This is the constructor for creating new player.

CActor::CActor(int iSkin, float fX, float fY, float fZ, float fRotation)
{
	m_pPed = 0;
	m_dwGTAId = 0;
	DWORD dwActorID = 0;

	if (!pGame->IsModelLoaded(iSkin))
	{
		pGame->RequestModel(iSkin);
		pGame->LoadRequestedModels();
		while (!pGame->IsModelLoaded(iSkin))
			Sleep(1);
	}

	ScriptCommand(&create_actor,5,iSkin,fX,fY,fZ-1.0f,&dwActorID);
	ScriptCommand(&set_actor_z_angle,dwActorID,fRotation);

	m_dwGTAId = dwActorID;
	m_pPed = GamePool_Ped_GetAt(m_dwGTAId);
	m_pEntity = (ENTITY_TYPE*)m_pPed;
	ScriptCommand(&set_actor_can_be_decapitated,m_dwGTAId,0);
	ScriptCommand(&set_actor_dicision,m_dwGTAId,65542);
}

CActor::~CActor()
{
	Destroy();
}

void CActor::Destroy()
{
	DWORD dwPedPtr = (DWORD)m_pPed;

	// If it points to the CPlaceable vtable it's not valid
	if (!m_pPed || !GamePool_Ped_GetAt(m_dwGTAId) || m_pPed->entity.vtable == 0x863C40)
	{
		m_pPed = NULL;
		m_pEntity = NULL;
		m_dwGTAId = 0;
		return;
	}

	// DESTROY METHOD
	_asm mov ecx, dwPedPtr
	_asm mov ebx, [ecx] ; vtable
	_asm push 1
	_asm call [ebx] ; destroy

	m_pPed = NULL;
	m_pEntity = NULL;
}

void CActor::ApplyAnimation(char* szAnimName, char* szAnimFile, float fT,
	int opt1, int opt2, int opt3, int opt4, int iUnk)
{
	int iCount = 0;

	if (!m_pPed) return;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return;
	//if (!stricmp(szAnimFile, "SEX")) return;

	if (pGame->IsAnimationLoaded(szAnimFile) != -1)
	{
		pGame->RequestAnimation(szAnimFile);

		while (!pGame->IsAnimationLoaded(szAnimFile))
		{
			Sleep(1);
			if (++iCount == 15)
				return;
		}

		ScriptCommand(&apply_animation, m_dwGTAId, szAnimName,
			szAnimFile, fT, opt1, opt2, opt3, opt4, iUnk);

	}
}

bool CActor::ClearAnimations()
{
	if (!m_pPed) return false;

	_asm
	{
		mov edx, m_pPed
		mov ecx, [edx+1148]
		push 1
		mov eax, 0x601640
		call eax
	}
	return true;
}

void CActor::SetAngle(float fAngle)
{
	if (!m_pPed) return;
	if (!GamePool_Ped_GetAt(m_dwGTAId)) return;

	m_pPed->fRotation2 = DegToRad(fAngle);
}

/*float CActor::GetHealth() // unused
{
	if (!m_pPed) return 0.0f;

	return m_pPed->fHealth;
}*/

void CActor::SetHealth(float fHealth)
{
	if (!m_pPed) return;

	m_pPed->fHealth = fHealth;

	if (m_pPed->fHealth <= 0.0f)
		ScriptCommand(&kill_actor, m_dwGTAId);
}

/*float CActor::GetArmour() // unused
{
	if (!m_pPed) return 0.0f;

	return m_pPed->fArmour;
}

void CActor::SetArmour(float fArmour) // unused
{
	if (!m_pPed) return;

	m_pPed->fArmour = fArmour;
}

unsigned int CActor::GetStateFlags() // unused
{
	if (!m_pPed) return 0;

	return m_pPed->dwStateFlags;
}

void CActor::SetStateFlags(unsigned int uiState) // unused
{
	if (!m_pPed) return;

	m_pPed->dwStateFlags = uiState;
}

bool CActor::IsDead() // unused
{
	if (!m_pPed) return true;
	if (m_pPed->fHealth > 0.0f) return false;

	return true;
}

unsigned int CActor::GetAction() // unused
{
	return m_pPed->dwAction;
}

void CActor::SetAction(unsigned int uiAction) // unused
{
	if (!m_pPed) return;

	m_pPed->dwAction = uiAction;
}

bool CActor::IsInVehicle() // unused
{
	if (!m_pPed) return false;

	return IN_VEHICLE(m_pPed);
}*/

void CActor::SetImmunities(bool bImmuneToAll)
{
	int b;

	if (!m_pPed) return;

	b = bImmuneToAll ? 1 : 0;
	m_bInvulnerable = bImmuneToAll;

	ScriptCommand(&set_actor_immunities, m_dwGTAId, b, b, b, 1, b);
}
