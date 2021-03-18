
#pragma once

#define MAX_LABELS (MAX_LABEL_GLOBAL + MAX_LABEL_PLAYER) // fu-sion-HA!

typedef struct
{
	char* szText;
	DWORD dwColor;
	float fX;
	float fY;
	float fZ;
	float fDrawDistance;
	bool bTestLOS;
	WORD wAttachedPlayerID;
	WORD wAttachedVehicleID;
} LABEL;

class CLabelPool
{
private:
	LABEL m_Labels[MAX_LABELS];
	bool m_bSlotState[MAX_LABELS];

public:
	CLabelPool();
	~CLabelPool();

	bool GetSlotState(WORD wLabelID)
	{
		if (wLabelID < MAX_LABELS)
		{
			return m_bSlotState[wLabelID];
		}
		return false;
	}

	void New(WORD wLabelID, char* szText, DWORD dwColor, VECTOR vecPos, float fDrawDistance,
		bool bTestLOS, WORD wAttachedPlayerID, WORD wAttachedVehicleID);
	bool Delete(WORD wLabelID);
	void Draw();
};
