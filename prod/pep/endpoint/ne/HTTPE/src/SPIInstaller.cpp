#include "stdafx.h"
#include "SPIInstaller.h"
#include "Utilities.h"
#pragma comment(lib, "Ws2_32.lib")

extern CELog g_log;

// {E286F091-5551-4df3-851F-31F494E8B765}
GUID HTTPE_LAYER_GUID = 
{ 0xe286f091, 0x5551, 0x4df3, { 0x85, 0x1f, 0x31, 0xf4, 0x94, 0xe8, 0xb7, 0x65 } };

// {E6ADD72D-19AA-44e3-90F5-5B79D9EBCA0F}
GUID FTPE_LAYER_GUID = 
{ 0xe6add72d, 0x19aa, 0x44e3, { 0x90, 0xf5, 0x5b, 0x79, 0xd9, 0xeb, 0xca, 0xf } };

// {A2FDD0DF-AF38-4eb6-AF5E-B8BC9BCB8B03}
GUID HPE_LAYER_GUID = 
{ 0xa2fdd0df, 0xaf38, 0x4eb6, { 0xaf, 0x5e, 0xb8, 0xbc, 0x9b, 0xcb, 0x8b, 0x3 } };

GUID MSAFD_PROVIDER_GUID = 
{ 0xe70f1aa0, 0xab8b, 0x11cf, { 0x8c, 0xa3, 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x92 } };

CProviderInstall::CProviderInstall()
{
}

CProviderInstall::~CProviderInstall()
{
}

BOOL CProviderInstall::InstallProvider()
{
	BOOL bRet = FALSE ;

	bRet = InstallFilter() ;
	
	return bRet ;
}

BOOL CProviderInstall::GetFilter( LPWSAPROTOCOL_INFOW &protoinfo, DWORD& protoinfosize,DWORD& totalprotos )
{
     int   errorcode = ERROR_INVALID_SPI_VALUE;

     protoinfo=NULL;
     totalprotos=0;
     protoinfosize=0;

     if(WSCEnumProtocols(NULL,protoinfo,&protoinfosize,&errorcode)==SOCKET_ERROR)
     {
         if(errorcode!=WSAENOBUFS)
         {
             return FALSE;
         }
     }

     if((protoinfo=(LPWSAPROTOCOL_INFOW)GlobalAlloc(GPTR,protoinfosize))==NULL)
     {
         return FALSE;
     }

     if((totalprotos=WSCEnumProtocols(NULL,protoinfo,&protoinfosize,&errorcode))==SOCKET_ERROR)
     {
		 FreeFilter(protoinfo);
         return FALSE;
	 }
     return TRUE;
}

void CProviderInstall::FreeFilter(LPWSAPROTOCOL_INFOW &protoinfo)
{
     GlobalFree(protoinfo);
}

BOOL CProviderInstall::InstallFilter()
{
	WSAPROTOCOL_INFOW   HTTPE_LayeredProtocolInfo;
	LPWSAPROTOCOL_INFOW lpAllProtocolInfo = NULL;
	DWORD               cbAllProtocolInfo = 0;
	DWORD               dwTotalProtocols = 0;
	DWORD               dwIndex = 0;
	INT                 nErrno = 0;

	if(GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols) == FALSE)
	{
		return FALSE;
	}

	for(dwIndex = 0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(lpAllProtocolInfo[dwIndex].iAddressFamily == AF_INET &&
		   lpAllProtocolInfo[dwIndex].iProtocol == IPPROTO_TCP  &&
		   memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &MSAFD_PROVIDER_GUID, sizeof(GUID)) == 0 )
		{
			memcpy(&HTTPE_LayeredProtocolInfo, &lpAllProtocolInfo[dwIndex], sizeof(WSAPROTOCOL_INFOW));
			HTTPE_LayeredProtocolInfo.ProtocolChain.ChainLen = BASE_PROTOCOL;
			wcsncpy_s(HTTPE_LayeredProtocolInfo.szProtocol, WSAPROTOCOL_LEN, L"Nextlabs HTTPE Provider", _TRUNCATE);
			break;
		}
	}

	if(dwIndex >= dwTotalProtocols)
	{
		FreeFilter(lpAllProtocolInfo);
		return FALSE;
	}

	wchar_t pathProvider[1024] = {0};
#if defined(_WIN64)
	GetModuleName(pathProvider, 1023, ::GetModuleHandle(L"HTTPE.DLL"));
#else
	GetModuleName(pathProvider, 1023, ::GetModuleHandle(L"HTTPE32.DLL"));
#endif
	if(WSCInstallProvider(&HTTPE_LAYER_GUID, pathProvider, &HTTPE_LayeredProtocolInfo, 1, &nErrno) == SOCKET_ERROR)
	{
		FreeFilter(lpAllProtocolInfo);
		return FALSE;
	}

	FreeFilter(lpAllProtocolInfo);

	if(GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols) == FALSE)
	{
		return FALSE;
	}

	//Try to re-sort all the providers
	LPDWORD lpdwCatalogEntryIds = NULL;
	if((lpdwCatalogEntryIds = (LPDWORD)GlobalAlloc(GPTR, dwTotalProtocols * sizeof(DWORD))) == NULL)
	{
		FreeFilter(lpAllProtocolInfo);
		return FALSE;
	}


	DWORD dwCatalogIndex = 0;
	DWORD dwFTPE = 0xFFFFFFFF;
	DWORD dwHPE = 0xFFFFFFFF;
	DWORD dwHTTPE = 0xFFFFFFFF;
	DWORD dwMSAFD = 0xFFFFFFFF;

	//Get the index for NE components
	for(dwIndex=0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &FTPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			dwFTPE = dwIndex;
		}

		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			dwHPE = dwIndex;
		}

		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HTTPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			dwHTTPE = dwIndex;
		}
	}

	//Sort all non-NE providers before MSAFD_PROVIDER_GUID
    for(dwIndex=0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &MSAFD_PROVIDER_GUID,  sizeof(GUID)) == 0)
		{
			dwMSAFD = dwIndex;
			break;
		}

		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &FTPE_LAYER_GUID, sizeof(GUID)) != 0 &&
			memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HPE_LAYER_GUID,  sizeof(GUID)) != 0 &&
			memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HTTPE_LAYER_GUID,  sizeof(GUID)) != 0 )
		{
			lpdwCatalogEntryIds[dwCatalogIndex++] = lpAllProtocolInfo[dwIndex].dwCatalogEntryId;
		}
	}

	//Sort NE components
	if(dwHTTPE != 0xFFFFFFFF)
		lpdwCatalogEntryIds[dwCatalogIndex++] = lpAllProtocolInfo[dwHTTPE].dwCatalogEntryId;

	if(dwFTPE != 0xFFFFFFFF)
		lpdwCatalogEntryIds[dwCatalogIndex++] = lpAllProtocolInfo[dwFTPE].dwCatalogEntryId;

	if(dwHPE != 0xFFFFFFFF)
		lpdwCatalogEntryIds[dwCatalogIndex++] = lpAllProtocolInfo[dwHPE].dwCatalogEntryId;

	//add all providers after MSAFD(including MSAFD)
	for(dwIndex=dwMSAFD; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &FTPE_LAYER_GUID, sizeof(GUID)) != 0 &&
			memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HPE_LAYER_GUID,  sizeof(GUID)) != 0 &&
			memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HTTPE_LAYER_GUID,  sizeof(GUID)) != 0 )
		{
			lpdwCatalogEntryIds[dwCatalogIndex++] = lpAllProtocolInfo[dwIndex].dwCatalogEntryId;
		}
	}

	assert(dwCatalogIndex == dwTotalProtocols);

	//Re-sort all providers base on lpdwCatalogEntryIds
    if(WSCWriteProviderOrder(lpdwCatalogEntryIds, dwTotalProtocols) != ERROR_SUCCESS)
    {
		GlobalFree(lpdwCatalogEntryIds);
		FreeFilter(lpAllProtocolInfo);
        return FALSE;
    }

	GlobalFree(lpdwCatalogEntryIds);
	//re-sort finished.
    FreeFilter(lpAllProtocolInfo) ;

    return TRUE;
}


BOOL  CProviderInstall::UnInstallFilter() 
{
	int   errorcode;
	if(WSCDeinstallProvider(&HTTPE_LAYER_GUID, &errorcode) == SOCKET_ERROR)
	{
	  WCHAR msg[1024];
	  _snwprintf_s(msg, 1024, _TRUNCATE, L"WSCDeinstall error %d\n", errorcode);
	  g_log.Log(CELOG_DEBUG, msg);
	  return FALSE;
	}
	g_log.Log(CELOG_DEBUG, L"\nUnInstall IP Filter Successfully\n");
	return TRUE;
}
