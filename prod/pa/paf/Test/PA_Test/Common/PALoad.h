#ifndef __PA_LOAD_H__
#define __PA_LOAD_H__
/*
Added by chellee. for the PEP loading the PA by the register...
*/
#include <sstream>
namespace PA_LOAD
{
#define GET_PA_INTERFACE_ADDRESS( _moduleName, _APIName,_APIAddr ) \
	HMODULE hmod ; \
	hmod = LoadLibrary(_moduleName); \
	if( hmod ) \
	{\
	GET_API_ADDRESS( hmod, _APIName,_APIAddr ) ;\
}
	const wchar_t PA_MODULE_NAME_ENC[]	= L"pa_encrypt.dll"    ;
	const wchar_t PA_MODULE_NAME_TAG[]	= L"pa_filetagging.dll" ;
	const wchar_t PA_KEY_VALUE_PATH[]	= L"SOFTWARE\\Nextlabs\\" ;
	const wchar_t PA_ENCRYPTION[]		= L"Encryption" ;
	const wchar_t PA_FILETAGGING[]	    = L"FileTagging" ;
	const wchar_t PA_PATH_NAME[]		= L"Path"  ;
	const wchar_t PA_MODULE_NAME[]		= L"Name"	;
	/*
	Load the library for the policy assist...
	i_pszModuleName:
	[in]It is the name of the module which is loaded...
	i_pszKeyName:
	[in]It identifies the registey key of the PA.
	o_hModule
	[out]Returns the handle of the module.
	i_strCurPath:
	[in]If load the module by the registry failure! here will try to load it in the current path.
	*/
	BOOL LoadModuleByName( const wchar_t* i_pszModuleName,const wchar_t* i_pszKeyName,HMODULE &o_hModule, const wchar_t* i_strCurPath = NULL) 
	{
		BOOL bRet = TRUE ;
		wchar_t pszBuf[MAX_PATH] = {0} ;
		if(( i_pszModuleName == NULL )||( i_pszKeyName == NULL ) )
		{
			if( i_strCurPath != NULL )
			{
				::wcsncpy_s( pszBuf, MAX_PATH,i_strCurPath, _TRUNCATE );
				::wcsncat_s(	 pszBuf, MAX_PATH, i_pszModuleName, _TRUNCATE  ) ;
			        o_hModule = LoadLibrary(pszBuf);
				if( o_hModule == NULL )
				{
					bRet = FALSE ;
				}
			}
			else
			{
				bRet = FALSE ;
			}
			return bRet ;
		}
		wchar_t strKeyPath[MAX_PATH] = {0} ;
		::wcsncpy_s( strKeyPath, MAX_PATH,PA_KEY_VALUE_PATH, _TRUNCATE );
		::wcsncat_s( strKeyPath, MAX_PATH,i_pszKeyName, _TRUNCATE );
		//strKeyPath = strKeyPath + i_pszKeyName ;
		HKEY _hKey ;
		LONG lRet =	 RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyPath, 0, KEY_ALL_ACCESS, &_hKey )  ;
		if( lRet != ERROR_SUCCESS  )
		{
			if( i_strCurPath != NULL )
			{
				::wcsncpy_s( pszBuf, MAX_PATH,i_strCurPath, _TRUNCATE );
				::wcsncat_s(	 pszBuf, MAX_PATH, i_pszModuleName, _TRUNCATE  ) ;
			        o_hModule = LoadLibrary(pszBuf);
				if( o_hModule == NULL )
				{
					bRet = FALSE ;
				}
			}
			else
			{
				bRet = FALSE ;
			}
			return bRet ;
		}
		DWORD dwBufLen = MAX_PATH ;
		lRet = RegQueryValueEx( _hKey ,PA_PATH_NAME, NULL, NULL,(LPBYTE)pszBuf, &dwBufLen ) ;
		if( lRet != ERROR_SUCCESS )
		{
			if( i_strCurPath != NULL )
			{
				::wcsncpy_s( pszBuf, MAX_PATH,i_strCurPath, _TRUNCATE );
				::wcsncat_s(	 pszBuf, MAX_PATH, i_pszModuleName, _TRUNCATE  ) ;
			         o_hModule = LoadLibrary(pszBuf);
				if( o_hModule == NULL )
				{
					bRet = FALSE ;
				}
			}
			else
			{
				bRet = FALSE ;
			}
			return bRet ;
		}
		if(	 pszBuf[dwBufLen-1] != '\\' )
		{
			wcsncat_s( pszBuf, MAX_PATH, L"\\", _TRUNCATE ) ;
		}
		/*
		modified by on  09-16-2008,2:40, for the module name
		*/
		//------------------------------------
		wchar_t pszName[MAX_PATH] = {0} ;
		lRet = RegQueryValueEx( _hKey ,PA_MODULE_NAME, NULL, NULL,(LPBYTE)pszName, &dwBufLen ) ;
		if( lRet == ERROR_SUCCESS )
		{
			wcsncat_s(	 pszBuf, MAX_PATH,   pszName, _TRUNCATE ) ;
		}
		else
		{
			wcsncat_s(	 pszBuf, MAX_PATH,   i_pszModuleName, _TRUNCATE ) ;
		}
		o_hModule = LoadLibrary(pszBuf);
		DWORD le = GetLastError();
		if( o_hModule == NULL )
		{
			if( i_strCurPath != NULL )
			{
				::wcsncpy_s( pszBuf, MAX_PATH,i_strCurPath, _TRUNCATE );
				::wcsncat_s(	 pszBuf, MAX_PATH, i_pszModuleName, _TRUNCATE  ) ;
			        o_hModule = LoadLibrary(pszBuf);
				if( o_hModule == NULL )
				{
					bRet = FALSE ;
				}
			}
			else
			{
				bRet = FALSE ;
			}
		}
		return bRet ;
	} ;
};

#endif 
