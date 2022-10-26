#ifndef __SDK_SUPPORT_
#define __SDK_SUPPORT_

#include <Windows.h>
#include <string>
#include <map>
#include "CEsdk.h"

namespace CELogging
{


	typedef CEResult_t (*CELOGGING_LogDecision_t)(CEHandle handle, 
		CEString cookie, 
		CEResponse_t userResponse, 
		CEAttributes * optAttributes);
	typedef struct
	{
		const WCHAR* libname;  // library name (e.g. cebrain.dll)
	} sdk_lib_t;
#ifdef _WIN64
#define CELOGGING    L"celogging.dll"
#define CEBRAIN		 L"cebrain.dll"
#define CECEM		 L"cecem.dll"
#define CEMARSHAL50  L"cemarshal50.dll"
#define CEPEPMAN	 L"cepepman.dll"
#define CETRANSPORT  L"cetransport.dll"
#define CECONN       L"ceconn.dll"
#else
#define CELOGGING    L"celogging32.dll"
#define CEBRAIN		 L"cebrain32.dll"
#define CECEM		 L"cecem32.dll"
#define CEMARSHAL50  L"cemarshal5032.dll"
#define CEPEPMAN	 L"cepepman32.dll"
#define CETRANSPORT  L"cetransport32.dll"
#define CECONN       L"ceconn32.dll"
#endif

	typedef struct
	{
		
		CELOGGING_LogDecision_t CELOGGING_LogDecision;
		std::map<std::wstring,HMODULE> libmap;

		/***********************************************************************
		* Connection oriented state
		**********************************************************************/
		CRITICAL_SECTION cs;  /* Protect connections state */
		bool connected;       /* Current connection state */
		CEHandle connHandle;  /* Connection handle */
	} Handle;	

	/** Load
	*
	*  Load the SDK into the given handle.  The SDK connection is not initiated from
	*  this method.
	*
	*  \return true on success, otherwise false.
	*/
	BOOL Load( std::wstring path, Handle* handle );

	/** Unload
	*
	*  \brief Unload the SDK at the given handle.
	*/
	void Unload( Handle* handle );



}/* namespace CESDK */





#endif