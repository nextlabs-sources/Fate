#ifndef __HOOK_RESOURCE_H__
#define __HOOK_RESOURCE_H__

namespace APIRES{
	/*
	define the hook structure...
	*/
	//******************************************************************
	//Store the support drop window's handle &						*
	//replace the drop address										*
	//******************************************************************
	typedef struct _tagDROPWNDINFO
	{
		DWORD		threadID;				//Drop thread ID.
		HWND		hwnd;					//The handle of window.which need to register
		wchar_t		cName[MAX_PATH];		//Windows Class Name.
		PVOID		pDropTarget;			//Pointer to DROPTARGET
		LONG_PTR	orig_DropProc;			//Origin Drop Address.
	}DROPWNDINFO,*pDROPWNDINFO;

	typedef struct _tagHOOKAPIINFO
	{
		DWORD		threadID;				//Drop thread ID.
		void*		pRealAPIAddress;			//Pointer to DROPTARGET
		LONG_PTR	orig_APIAddress;			//Origin Drop Address.
		PVOID		pObjectAddress ;
	}HOOKAPIINFO,*PHOOKAPIINFO;

	typedef struct _tagHOOKPROCINFO
	{
		char*	pszModule ;					//The module name include the API .
		char*	pszOrigName ;				//Origin API Name .
		PVOID	pOldProc ;					//Old API Address.
		PVOID	*pNextProc ;					//New API Address .
		PVOID   pNewProc ;					//Callback API Adress
		HMODULE hDllInst ;					//Handle to LoadLibrary(pszModule) .
	}HOOKPROCINFO,PHOOKPROCINFO ; 

	//Based on the offset of a special module.
	typedef	struct _tagHOOKADDRINFO
	{
		char* pszModuleName ;				//The module file name.
		DWORD dOffset ;						//The offset of the start address of the module which identify by the pszModuleName
		PVOID pOldProc ;					//Store the origin function address.
		PVOID *pNextProc ;					//The hooked address function.
		PVOID pNewProc;						//Callback API Adress
	}HOOKADDRINFO,*PHOOKADDRINFO ;
	//Based on the export ID of a special module.
	typedef struct _tagHOOKEXPIDINFO
	{
		wchar_t* pszModuleName ;				//In this module. including the export ID function.
		DWORD dID ;							//The export function ID;
		PVOID pOldProc ;					//Store the origin function address.
		PVOID *pNextProc ;					//The hooked address function.
		PVOID pNewProc;					//Callback API Adress
	}HOOKEXPIDINFO,*PHOOKEXPIDINFO ;

}
#endif