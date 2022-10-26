
#pragma once
#ifndef __PA_LOAD_H__
#define __PA_LOAD_H__
#include <string>
#include <boost/algorithm/string.hpp>
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
#ifdef _WIN64
	const wchar_t PA_MODULE_NAME_ENC[]	= L"pa_encrypt.dll"    ;
	const wchar_t PA_MODULE_NAME_TAG[]	= L"pa_filetagging.dll" ;
	const wchar_t PA_MODULE_NAME_PE[]	= L"PA_PE.dll" ;	//Portable encrytion
	const wchar_t PAF_MODULE_NAME[]	= L"pafUI.dll" ;
	#ifdef MSO2K3
		const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA2003.dll";
	#else 
		const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA2007.dll";
	#endif
#else
	const wchar_t PA_MODULE_NAME_ENC[]	= L"pa_encrypt32.dll"    ;
	const wchar_t PA_MODULE_NAME_TAG[]	= L"pa_filetagging32.dll" ;
	const wchar_t PA_MODULE_NAME_PE[]	= L"PA_PE32.dll" ;	//Portable encrytion
	const wchar_t PAF_MODULE_NAME[]	= L"pafUI32.dll" ;
	#ifdef MSO2K3
		const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA200332.dll";
	#else
		const wchar_t PA_MODULE_NAME_NLVISUALABELING[]	= L"NLVisualLabelingPA200732.dll";
	#endif
#endif
	const wchar_t PA_KEY_VALUE_PATH[]	= L"SOFTWARE\\Nextlabs\\" ;
	const wchar_t PA_ENCRYPTION[]		= L"Encryption" ;
	const wchar_t PA_FILETAGGING[]	    = L"FileTagging" ;
	const wchar_t PA_PATH_NAME[]		= L"Path"  ;
	const wchar_t PA_MODULE_NAME[]		= L"Name"	;
	/*
	added by chellee for the windows 2000...
	There are lack of GDIPlus.dll
	*/
	const wchar_t GDIPLUS_MODULE_NAME[]	= L"GdiPlus.dll"  ;
		/*
	added by chellee on 10/22/2008... 17:49 
	for the GDIPlus.dll
	*/
	//----------------------------------------------
	BOOL Check_GDIPlus_Module( const wchar_t *pszInstallPath ) 
	{
		BOOL bRet = FALSE ;
		wchar_t szBuf[MAX_PATH+1] = {0} ;
		if( !GetModuleHandle( GDIPLUS_MODULE_NAME ) )
		{//Check if there has exist this module...
			if( !LoadLibraryW( GDIPLUS_MODULE_NAME ) )
			{
				if( pszInstallPath )
				{
					::wcsncpy_s( szBuf, MAX_PATH,pszInstallPath, _TRUNCATE );
					if(	 szBuf[wcslen(szBuf)-1] != '\\' )
					{
						wcsncat_s( szBuf, MAX_PATH, L"\\", _TRUNCATE ) ;
					}
					::wcsncat_s(	 szBuf, MAX_PATH, GDIPLUS_MODULE_NAME, _TRUNCATE  ) ;
					LoadLibraryW( szBuf )	;
				}
			}
		}
		return bRet ;
	};
    //----------------------------------------------
    // Following code is changed by Gavin to pass W4 and static analysis
    //
	BOOL LoadModuleInCurrentDir(IN LPCWSTR pcwzModuleName, IN OUT HMODULE& hMod, IN LPCWSTR pcwzCurrentPath)
	{
		std::wstring wstrModPath = L"";

		// Sanity check
		if(NULL==pcwzModuleName || NULL==pcwzCurrentPath)
			return FALSE;
		if(0==pcwzModuleName[0] || 0==pcwzCurrentPath[0])
			return FALSE;

		wstrModPath = pcwzCurrentPath;
		if(!boost::algorithm::ends_with(wstrModPath, L"\\") && !boost::algorithm::ends_with(wstrModPath, L"/"))
			wstrModPath.append(L"\\");

		wstrModPath.append(pcwzModuleName);
		hMod = ::LoadLibraryW(wstrModPath.c_str());
		return (NULL==hMod)?FALSE:TRUE;
	}

	bool LoadCommonLibraries(const wchar_t* pszCommonPath)
	{
		bool bRet = true;
#ifdef _WIN64
		static wchar_t* szCommonDLLs[] =
		{
			L"celog.dll",
			L"zlibwapi.dll",
			L"nl_sysenc_lib.dll",
			L"tag_office2k7.dll",
			L"Podofolib.dll",
			L"libtiff.dll",
			L"resattrlib.dll",
			L"resattrmgr.dll",
			L"NextLabsTaggingLib.dll"
		};
#else
		static wchar_t* szCommonDLLs[] =
		{
			L"celog32.dll",
			L"zlib1.dll",
			L"nl_sysenc_lib32.dll",
			L"tag_office2k732.dll",
			L"Podofolib.dll",
			L"libtiff.dll",
			L"resattrlib32.dll",
			L"resattrmgr32.dll",
			L"NextLabsTaggingLib32.dll"
		};
#endif
		std::wstring strCommonPath;
		if(pszCommonPath && wcslen(pszCommonPath) >= 2)
		{
			strCommonPath = pszCommonPath;
			if(strCommonPath[strCommonPath.length() - 1] != '\\')
			{
				strCommonPath.append(L"\\");
			}
		}

		wchar_t szDir[MAX_PATH + 1] = {0};
		GetCurrentDirectoryW(MAX_PATH, szDir);

		SetCurrentDirectoryW(strCommonPath.c_str());
		
		for( int i = 0; i < _countof(szCommonDLLs); i++ )
		{
			std::wstring strDLLPath = strCommonPath + szCommonDLLs[i];
			HMODULE hMod = (HMODULE)LoadLibraryW(strDLLPath.c_str());
			if(hMod == NULL)	bRet = false;
		}

		SetCurrentDirectoryW(szDir);
		return bRet;
	}
	/*
	Load the library for the policy assist...
	i_pszModuleName:
	[in]Module name, like: pa_encrypt.dll
	i_pszKeyName:
	[in]It identifies the registey key of the PA.
	o_hModule
	[out]Returns the handle of the module.
	i_strCurPath:
	[in]If load the module by the registry failure! here will try to load it in the current path.
	*/
	BOOL LoadModuleByName( const wchar_t* pszModuleName,const wchar_t* pszKeyName,HMODULE &o_hModule, const wchar_t* pszCurPath = NULL) 
	{
		//BOOL         bRet = TRUE ;
        HKEY         hKey = NULL;
        LONG         lRet = 0;
        std::wstring wstrKey = L"";
        std::wstring wstrMod = L"";
        wchar_t      pszBuf[MAX_PATH+1] = {0} ;
        DWORD        dwBufLen = MAX_PATH;

		
		//----------------------------------------------
		Check_GDIPlus_Module( pszCurPath ) ;
		//----------------------------------------------

		static bool bLoadCommon = false;
		if(!bLoadCommon)
		{
			if(LoadCommonLibraries(pszCurPath))			bLoadCommon = true;
		}
        // If the key name is empty, we can't get the module path in registry
        // So, we get current directory, and find module in this directory
		if(NULL == pszKeyName)
            goto _RETURN_DEFAULT_MOD;

        // Otherwise, we need to get module path from registry
        wstrKey = PA_KEY_VALUE_PATH;
        wstrKey += pszKeyName;
        lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, wstrKey.c_str(), 0, KEY_ALL_ACCESS, &hKey);
        if(ERROR_SUCCESS != lRet)
            goto _RETURN_DEFAULT_MOD;

        lRet = RegQueryValueEx( hKey ,PA_PATH_NAME, NULL, NULL,(LPBYTE)pszBuf, &dwBufLen ) ;
        if(ERROR_SUCCESS != lRet || L'\0'==pszBuf[0])
            goto _RETURN_DEFAULT_MOD;
        wstrMod = pszBuf;
        
        /*
        added by chellee on 10/22/2008... 17:49 
        for the GDIPlus.dll
        */
        Check_GDIPlus_Module( pszBuf ) ;
        /*add end*/

        if(!boost::algorithm::ends_with(wstrMod, L"\\"))
            wstrMod.append(L"\\");
        dwBufLen = MAX_PATH;
        memset(pszBuf, 0, sizeof(pszBuf));
        lRet = RegQueryValueExW( hKey ,PA_MODULE_NAME, NULL, NULL,(LPBYTE)pszBuf, &dwBufLen );
        if(ERROR_SUCCESS==lRet && L'\0'!=pszBuf[0])
        {
            wstrMod += pszBuf;
        }
        else
        {
            if(NULL!=pszModuleName && 0!=pszModuleName[0])
                wstrMod += pszModuleName;
            else
                goto _RETURN_DEFAULT_MOD;
        }

        o_hModule = ::LoadLibraryW(wstrMod.c_str());
        return (NULL==o_hModule)?FALSE:TRUE;


_RETURN_DEFAULT_MOD:
        return LoadModuleInCurrentDir(pszModuleName, o_hModule, pszCurPath);
	} ;

};

#endif 
