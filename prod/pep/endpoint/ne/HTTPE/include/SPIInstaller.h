
#ifndef __PROVIDER_INSTALL_H__
#define __PROVIDER_INSTALL_H__

class CProviderInstall
{
public:
	 CProviderInstall() ;
	 ~CProviderInstall() ;
public:
	 BOOL InstallProvider( ) ;
	 static BOOL GetFilter(LPWSAPROTOCOL_INFOW &protoinfo, DWORD& protoinfosize,DWORD& totalprotos) ;
	 static VOID FreeFilter(LPWSAPROTOCOL_INFOW &protoinfo) ;
	 BOOL InstallFilter() ;
	 BOOL UnInstallFilter() ;
};
#endif