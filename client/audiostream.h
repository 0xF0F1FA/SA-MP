
#pragma once

class CAudioStream
{
public:
	CAudioStream();
	~CAudioStream();

	void Play(char* szURL, float fX, float fY, float fZ, float fDist);
	void Stop();

	void Process();

private:
	static void CALLBACK AudioStreamMetaSync(HSYNC handle, DWORD channel, DWORD data, void* user);
	void InitBass();

	HSTREAM m_hStream;
	ULONGLONG m_ullLastUpdate;
	float m_fX, m_fY, m_fZ;
	float m_fDistance;
	char* m_szTitle;
	char* m_szLabel;

	struct {
		unsigned char bInited : 1;
		unsigned char bPlaying : 1;
	} m_Flags;
};
