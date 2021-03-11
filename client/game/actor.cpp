
#include "../main.h"
#include "util.h"

CActor::CActor(int iModelID, VECTOR vecPos, float fAngle)
{
	DWORD dwHandle;
	float fZ;

	m_pPed = NULL;
	m_dwGTAId = 0;
	m_bInvulnerable = false;

	if (!pGame->IsModelLoaded(iModelID))
	{
		pGame->RequestModel(iModelID);
		pGame->LoadRequestedModels();
		while (!pGame->IsModelLoaded(iModelID))
			Sleep(1);
	}

	fZ = vecPos.Z - 1.0f;
	ScriptCommand(&create_actor, 5, iModelID, vecPos.X, vecPos.Y, fZ, &dwHandle);
	ScriptCommand(&set_actor_z_angle, dwHandle, fAngle);

	m_dwGTAId = dwHandle;
	m_pPed = GamePool_Ped_GetAt(dwHandle);
	m_pEntity = (ENTITY_TYPE*)m_pPed;

	ScriptCommand(&set_actor_can_be_decapitated, m_dwGTAId, 0);
	ScriptCommand(&actor_task_sit, m_dwGTAId, 65542);
}

CActor::~CActor()
{
	Destroy();
}

void CActor::Destroy()
{
	DWORD dwPed = (DWORD)m_pPed;

	if (m_pPed && GamePool_Ped_GetAt(m_dwGTAId) && m_pPed->entity.vtable != ADDR_PLACEABLE_VTBL)
	{
		_asm
		{
			mov ecx, dwPed;
			mov ebx, [ecx];
			push 1
			call [ebx]
		}

		m_pPed = NULL;
		m_pEntity = NULL;
	}
	else
	{
		m_dwGTAId = 0;
		m_pPed = NULL;
		m_pEntity = NULL;
	}
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
