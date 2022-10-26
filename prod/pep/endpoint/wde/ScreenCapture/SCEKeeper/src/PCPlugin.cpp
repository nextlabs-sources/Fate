#include "stdafx.h"
#include "SCEKeeper.h"

static HANDLE hThreadSCEKeeper = NULL;

extern "C" int PluginEntry( void** /*in_context*/ ) 
{
	hThreadSCEKeeper = (HANDLE)_beginthreadex(NULL, 0, SvcInit, NULL, 0, NULL);
	return 1;
}

extern "C" int PluginUnload( void* /*in_context*/ )
{
	if(ghSvcStopEvent != NULL)
		SetEvent(ghSvcStopEvent);
	if(hThreadSCEKeeper != NULL)
	{
		::WaitForSingleObject(hThreadSCEKeeper, 10000);
		::CloseHandle(hThreadSCEKeeper);
	}
	return 1;
}