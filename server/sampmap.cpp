
#include "main.h"

SAMPMAP::SAMPMAP()
{
	m_pdwPtrs = NULL;
	m_dwSize = 0;
}

SAMPMAP::~SAMPMAP()
{
	Reallocate(0);
}

bool SAMPMAP::Reallocate(DWORD dwNewSize)
{
	DWORD* pdwTemp;
	DWORD dwIndex;

	if (dwNewSize != 0)
	{
		pdwTemp = (DWORD*)realloc(m_pdwPtrs, sizeof(DWORD) * dwNewSize);
		if (pdwTemp != NULL)
		{
			m_pdwPtrs = pdwTemp;

			for (dwIndex = m_dwSize; dwIndex < dwNewSize; dwIndex++)
			{
				m_pdwPtrs[dwIndex] = 0;
			}
			m_dwSize = dwNewSize;
			return true;
		}
	}
	else
	{
		if (m_pdwPtrs)
		{
			free(m_pdwPtrs);
			m_pdwPtrs = NULL;
		}
		m_dwSize = 0;
		return true;
	}
	return false;
}

void SAMPMAP::Resize()
{
	int iIndex = 0;
	int iLastIndex = -1;

	if (m_dwSize != 0)
	{
		while (iIndex != m_dwSize)
		{
			if (m_pdwPtrs[iIndex] != 0)
			{
				iLastIndex = iIndex;
			}
			iIndex++;
		}

		if (iLastIndex != -1)
		{
			Reallocate(iLastIndex + 1);
			return;
		}
	}

	Reallocate(0);
}

int SAMPMAP::Add(DWORD dwPtr)
{
	int iIndex = 0;
	int iEnd = (int)m_dwSize;

	if (!iEnd)
		goto reallocate;

	while (m_pdwPtrs[iIndex] != 0)
	{
		++iIndex;
		if (iIndex == iEnd)
			goto reallocate;
	}

	if (iIndex >= 0)
	{
		m_pdwPtrs[iIndex] = dwPtr;
		return iIndex;
	}
	else
	{
	reallocate:
		if (Reallocate(iEnd + 1))
		{
			m_pdwPtrs[iEnd] = dwPtr;
			return iEnd;
		}
	}
	return -1;
}

bool SAMPMAP::Remove(DWORD dwIndex)
{
	if (dwIndex < m_dwSize)
	{
		m_pdwPtrs[dwIndex] = 0;
		Resize();
		return true;
	}
	return false;
}

DWORD SAMPMAP::GetAt(DWORD dwIndex)
{
	if (dwIndex < m_dwSize)
	{
		return m_pdwPtrs[dwIndex];
	}
	return 0;
}

