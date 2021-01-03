
#pragma once

class CActor : public CEntity
{
private:
	PED_TYPE* m_pPed;
	bool m_bInvulnerable;

public:
	CActor(int iModelID, VECTOR vecPos, float fAngle);
	virtual ~CActor();

	void Destroy();
	void ApplyAnimation(char* szAnimName, char* szAnimFile, float fT,
		int opt1, int opt2, int opt3, int opt4, int iUnk);
	bool ClearAnimations();
	//float GetHealth(); // unused
	void SetAngle(float fAngle);
	void SetHealth(float fHealth);
	//float GetArmour(); // unused
	//void SetArmour(float fArmour); // unused
	//unsigned int GetStateFlags(); // unused
	//void SetStateFlags(unsigned int uiState); // unused
	//bool IsDead(); // unused
	//unsigned int GetAction(); // unused
	//void SetAction(unsigned int uiAction); // unused
	//bool IsInVehicle(); // unused
	void SetImmunities(bool bImmuneToAll);

	inline DWORD GetPed() { return (DWORD)m_pPed; };
};
