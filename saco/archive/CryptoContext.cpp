//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#include "CryptoContext.h"
#include "CryptoFns.h"

//------------------------------------

DWORD CCryptoContext::ms_dwRefCount = 0;
DWORD CCryptoContext::ms_dwProviderType = PROV_RSA_FULL;
LPTSTR CCryptoContext::ms_szProviderName = NULL;
LPTSTR CCryptoContext::ms_szContainerName = (LPTSTR)"SAMP";
HMODULE CCryptoContext::ms_hAdvApi32 = NULL;

//------------------------------------

CCryptoContext::CCryptoContext(void)
{
	/*
	if (!ms_hAdvApi32) 
	{
		ms_hAdvApi32 = LoadLibrary("advapi32.dll");
	}
	*/
	
	// Open existing context, if not found, create one!
	if ( !CRYPT(AcquireContext)(&m_hCryptProv, ms_szContainerName, ms_szProviderName, ms_dwProviderType, 0) ) {
		if( !CRYPT(AcquireContext)(&m_hCryptProv, ms_szContainerName, ms_szProviderName, ms_dwProviderType, CRYPT_NEWKEYSET) ) {
			throw(1);
		}
	}

	ms_dwRefCount++;

}

//------------------------------------

CCryptoContext::~CCryptoContext(void)
{
	
	// Release the context
	CRYPT(ReleaseContext)(m_hCryptProv, 0);
	ms_dwRefCount--;

	if (ms_dwRefCount == 0) {
		/*
		// Free the library
		if (ms_hAdvApi32)
			FreeLibrary(ms_hAdvApi32);
		*/

		// Delete the context
		CRYPT(AcquireContext)(&m_hCryptProv, ms_szContainerName, ms_szProviderName, ms_dwProviderType, CRYPT_DELETEKEYSET);
	}

}

//------------------------------------

HCRYPTPROV CCryptoContext::GetProvider()
{
	return m_hCryptProv;
}

//------------------------------------

#ifdef ARCTOOL
void CCryptoContext::GenerateRandom(DWORD dwLength, BYTE* pbBuffer) 
{
	CryptGenRandom(m_hCryptProv, dwLength, pbBuffer);
}
#endif
