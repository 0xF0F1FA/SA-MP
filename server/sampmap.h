
#ifndef SAMPSRV_SAMPMAP_H
#define SAMPSRV_SAMPMAP_H

class SAMPMAP
{
public:
	DWORD* m_pdwPtrs;
	DWORD m_dwSize;

	SAMPMAP();
	~SAMPMAP();

	bool Reallocate(DWORD dwNewSize);
	void Resize();
	int Add(DWORD dwPtr);
	bool Remove(DWORD dwIndex);
	DWORD GetAt(DWORD dwIndex);
};

#endif // SAMPSRV_SAMPMAP_H
