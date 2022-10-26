#include "stdafx.h"
#include "SDKSupport.h"
#include <cassert>


BOOL CELogging::Load( std::wstring path, Handle* handle)
{
	

	/****************************************************************
	* Load SDK libraries.  Order is critical due to link depends.
	***************************************************************/
	sdk_lib_t sdk_libs[] =
	{
		CEBRAIN, CETRANSPORT, CECEM, CEMARSHAL50, CEPEPMAN,CELOGGING,
	};

	BOOL status = FALSE;
	handle->libmap.clear();
	WCHAR wzPath[MAX_PATH]=L"";
	wcsncpy_s(wzPath,MAX_PATH,path.c_str(), _TRUNCATE);
	for( int i = 0 ; i < sizeof(sdk_libs) / sizeof(sdk_lib_t) ; i++ )
	{
		HMODULE hlib = NULL;
		WCHAR libpath[MAX_PATH] = {0};
		_snwprintf_s(libpath,sizeof(libpath)/sizeof(WCHAR), sizeof(libpath), L"%s\\%s",wzPath,sdk_libs[i].libname);
		hlib = LoadLibraryW(libpath);
		if( hlib  == NULL )
		{
			goto Load_done;
		}
		handle->libmap[sdk_libs[i].libname] = hlib;
	}


	handle->CELOGGING_LogDecision      = (CELOGGING_LogDecision_t)GetProcAddress(handle->libmap[CELOGGING],"CELOGGING_LogDecision");

	if( 
		handle->CELOGGING_LogDecision != NULL 
		)
	{
		status = TRUE;
	}

	InitializeCriticalSection(&handle->cs);

Load_done:
	/* If any of the functions cannot be found the SDK cannot be used. */
	if( status == FALSE )
	{
		Unload(handle);
	}/* if failed */

	return status;
}/* Load */

void CELogging::Unload( Handle* handle )
{
	std::map<std::wstring,HMODULE>::iterator it;
	for( it = handle->libmap.begin() ; it != handle->libmap.end() ; ++it )
	{
		FreeLibrary(it->second);
	}
}/* Unload */
