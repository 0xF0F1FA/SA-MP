
#ifndef SAMPSRV_LABELPOOL_H
#define SAMPSRV_LABELPOOL_H

typedef struct
{
	char* szText;
	WORD wLength; // Additional
	DWORD dwColor;
	float fX;
	float fY;
	float fZ;
	float fDrawDistance;
	bool bTestLOS;
	int iVirtualWorld;
	WORD wAttachedPlayerID;
	WORD wAttachedVehicleID;
} LABEL; // size=33

class CLabelPool
{
private:
	bool m_bSlotState[MAX_LABEL_GLOBAL];
	LABEL m_Labels[MAX_LABEL_GLOBAL];

public:
	CLabelPool();
	~CLabelPool();

	bool GetSlotState(int iLabel)
	{
		if (iLabel >= 0 && iLabel < MAX_LABEL_GLOBAL)
		{
			return m_bSlotState[iLabel];
		}
		return false;
	};

	LABEL* GetAt(int iLabel)
	{
		if (iLabel >= 0 && iLabel < MAX_LABEL_GLOBAL)
		{
			return &m_Labels[iLabel];
		}
		return NULL;
	}

	WORD New(char* szText, DWORD dwColor, float fX, float fY, float fZ, float fDistance, int iVirtualWorld, bool bTestLOS);
	bool Delete(WORD wLabelID);
	bool UpdateText(WORD wLabelID, DWORD dwColor, char* szText);
	bool AttachToPlayer(WORD wLabelID, WORD wPlayerID, float fOffsetX, float fOffsetY, float fOffsetZ);
	bool AttachToVehicle(WORD wLabelID, WORD wVehicleID, float fOffsetX, float fOffsetY, float fOffsetZ);
	void UpdateForAll(WORD wLabelID);
	void DeleteForAll(WORD wLabelID);
	void InitForPlayer(WORD wLabelID, WORD wPlayerID);
	void DeleteForPlayer(WORD wLabelID, WORD wPlayerID);
};

#endif
