
#ifndef __PROVIDER_INSTALL_H__
#define __PROVIDER_INSTALL_H__
/*
12/1/2009. Code package for the SPI( Service Provider Interface ) ...
*/
class CProviderInstall
{
public:
	 CProviderInstall() ;
	 ~CProviderInstall() ;
public:
	 BOOL InstallProvider( wchar_t *i_MoudleName, INT iType ) ;
	 INT GetCurrentFilePath( wchar_t *o_pathName ) ;
	 static BOOL GetFilter(LPWSAPROTOCOL_INFOW &protoinfo, DWORD& protoinfosize,DWORD& totalprotos) ;
	 static VOID FreeFilter(LPWSAPROTOCOL_INFOW &protoinfo) ;
	 BOOL InstallFilter(/*wchar_t* i_fileFullPath*/) ;

	 BOOL UnInstallFilter() ;
	// BOOL InitProtocolInfo( 
};
#endif