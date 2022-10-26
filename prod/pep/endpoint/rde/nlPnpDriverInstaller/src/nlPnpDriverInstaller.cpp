///////////////////////////////////////////////////////////////////////////////
//
//    (C) Copyright NextLabs, Inc.
//    All Rights Reserved
//
//
//    MODULE:
//
//      nlPnpDriverInstaller.cpp
//
//    ABSTRACT:
//
//      This code is to install a plug-and-play device driver. 
//
//    AUTHOR(S):
//
//      Nextlabs, Inc.
//
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>
#include <hidsdi.h>
#include <stdio.h>
#include <tchar.h>
#include <wdfinstaller.h>
#include <newdev.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <strsafe.h>
#include <infstr.h>
#include <regstr.h>
#include "celog.h"


#define CELOG_CUR_MODULE L"RDE"
#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_RDE_NLPNPDRIVERINSTALLER_SRC_NLPNPDRIVERINSTALLER_CPP


namespace {
struct GenericContext {
    DWORD count;
    DWORD control;
    BOOL  reboot;
    LPCTSTR strSuccess;
    LPCTSTR strReboot;
    LPCTSTR strFail;
};

struct IdEntry {
    LPCTSTR String;     // string looking for
    LPCTSTR Wild;       // first wild character if any
    BOOL    InstanceId;
};

///////////////////////////////////
// Function Pointers             //
//Runtime loaded function pointer//
///////////////////////////////////
typedef int (*CallbackFunc)(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Index, _In_ LPVOID Context);
typedef BOOL (WINAPI *SetupUninstallOEMInfProto)(_In_ LPCTSTR InfFileName,
                                                 _In_ DWORD Flags,
                                                 /* _Reserved_ */ PVOID Reserved
                                                 );
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(_In_ HWND hwndParent,
                                                              _In_ LPCTSTR HardwareId,
                                                              _In_ LPCTSTR FullInfPath,
                                                              _In_ DWORD InstallFlags,
                                                              _Out_opt_ PBOOL bRebootRequired
															  );
typedef BOOL (WINAPI *SetupCopyOEMInfProto)(_In_       PCTSTR SourceInfFileName,
											_In_       PCTSTR OEMSourceMediaLocation,
											_In_       DWORD OEMSourceMediaType,
											_In_       DWORD CopyStyle,
											_Out_opt_  PTSTR DestinationInfFileName,
											_In_       DWORD DestinationInfFileNameSize,
											_Out_opt_  PDWORD RequiredSize,
											_Out_opt_  PTSTR DestinationInfFileNameComponent);
/////////////////////
// Private GLOBALS //
/////////////////////
WCHAR     debugStr[1024];
LPCTSTR   _infFile;
LPCTSTR   _hardwareID;
BOOLEAN   _bInstall;

/////////////////////
// resources ID    //
/////////////////////
#define IDS_REMOVED         3009
#define IDS_REMOVED_REBOOT  3010
#define IDS_REMOVE_FAILED   3011

#define INSTANCEID_PREFIX_CHAR TEXT('@') // character used to prefix instance ID's
#define CLASS_PREFIX_CHAR      TEXT('=') // character used to prefix class name
#define WILD_CHAR              TEXT('*') // wild character
#define QUOTE_PREFIX_CHAR      TEXT('\'') // prefix character to ignore wild characters
#define SPLIT_COMMAND_SEP      TEXT(":=") // whole word, indicates end of id's

//Runtime loaded function names   
#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfA"
#endif
#define SETUPCOPYOEMINF "SetupCopyOEMInfW"

// exit codes
enum {
	EXIT_OK=0,
	EXIT_REBOOT,
	EXIT_FAIL
};


BOOL WildCardMatch(_In_ LPCTSTR Item, _In_ const IdEntry & MatchEntry)
/*++

Routine Description:

    Compare a single item against wildcard
    I'm sure there's better ways of implementing this
    Other than a command-line management tools
    it's a bad idea to use wildcards as it implies
    assumptions about the hardware/instance ID
    eg, it might be tempting to enumerate root\* to
    find all root devices, however there is a CfgMgr
    API to query status and determine if a device is
    root enumerated, which doesn't rely on implementation
    details.

Arguments:

    Item - item to find match for eg a\abcd\c
    MatchEntry - eg *\*bc*\*

Return Value:

    TRUE if any match, otherwise FALSE

--*/
{
    LPCTSTR scanItem;
    LPCTSTR wildMark;
    LPCTSTR nextWild;
    size_t matchlen;

    //
    // before attempting anything else
    // try and compare everything up to first wild
    //
    if(!MatchEntry.Wild) {
        return _tcsicmp(Item,MatchEntry.String) ? FALSE : TRUE;
    }
    if(_tcsnicmp(Item,MatchEntry.String,MatchEntry.Wild-MatchEntry.String) != 0) {
        return FALSE;
    }
    wildMark = MatchEntry.Wild;
    scanItem = Item + (MatchEntry.Wild-MatchEntry.String);

    for(;wildMark[0];) {
        //
        // if we get here, we're either at or past a wildcard
        //
        if(wildMark[0] == WILD_CHAR) {
            //
            // so skip wild chars
            //
            wildMark = CharNext(wildMark);
            continue;
        }
        //
        // find next wild-card
        //
        nextWild = _tcschr(wildMark,WILD_CHAR);
        if(nextWild) {
            //
            // substring
            //
            matchlen = nextWild-wildMark;
        } else {
            //
            // last portion of match
            //
            size_t scanlen = lstrlen(scanItem);
            matchlen = lstrlen(wildMark);
            if(scanlen < matchlen) {
                return FALSE;
            }
            return _tcsicmp(scanItem+scanlen-matchlen,wildMark) ? FALSE : TRUE;
        }
        if(_istalpha(wildMark[0])) {
            //
            // scan for either lower or uppercase version of first character
            //
            TCHAR u = _totupper(wildMark[0]);
            TCHAR l = _totlower(wildMark[0]);
            while(scanItem[0] && scanItem[0]!=u && scanItem[0]!=l) {
                scanItem = CharNext(scanItem);
            }
            if(!scanItem[0]) {
                //
                // ran out of string
                //
                return FALSE;
            }
        } else {
            //
            // scan for first character (no case)
            //
            scanItem = _tcschr(scanItem,wildMark[0]);
            if(!scanItem) {
                //
                // ran out of string
                //
                return FALSE;
            }
        }
        //
        // try and match the sub-string at wildMark against scanItem
        //
        if(_tcsnicmp(scanItem,wildMark,matchlen)!=0) {
            //
            // nope, try again
            //
            scanItem = CharNext(scanItem);
            continue;
        }
        //
        // substring matched
        //
        scanItem += matchlen;
        wildMark += matchlen;
    }
    return (wildMark[0] ? FALSE : TRUE);
}


BOOL WildCompareHwIds(_In_ PZPWSTR Array, _In_ const IdEntry & MatchEntry)
/*++

Routine Description:

    Compares all strings in Array against Id
    Use WildCardMatch to do real compare

Arguments:

    Array - pointer returned by GetDevMultiSz
    MatchEntry - string to compare against

Return Value:

    TRUE if any match, otherwise FALSE

--*/
{
    if(Array) {
        while(Array[0]) {
            if(WildCardMatch(Array[0],MatchEntry)) {
                return TRUE;
            }
            Array++;
        }
    }
    return FALSE;
}

LPTSTR * GetMultiSzIndexArray(_In_ LPTSTR MultiSz)
/*++

Routine Description:

    Get an index array pointing to the MultiSz passed in

Arguments:

    MultiSz - well formed multi-sz string

Return Value:

    array of strings. last entry+1 of array contains NULL
    returns NULL on failure

--*/
{
    LPTSTR scan;
    LPTSTR * array;
    int elements;

    for(scan = MultiSz, elements = 0; scan[0] ;elements++) {
        scan += lstrlen(scan)+1;
    }
    array = new LPTSTR[elements+2];
    if(!array) {
        return NULL;
    }
    array[0] = MultiSz;
    array++;
    if(elements) {
        for(scan = MultiSz, elements = 0; scan[0]; elements++) {
            array[elements] = scan;
            scan += lstrlen(scan)+1;
        }
    }
    array[elements] = NULL;
    return array;
}

LPTSTR * GetDevMultiSz(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop)
/*++

Routine Description:

    Get a multi-sz device property
    and return as an array of strings

Arguments:

    Devs    - HDEVINFO containing DevInfo
    DevInfo - Specific device
    Prop    - SPDRP_HARDWAREID or SPDRP_COMPATIBLEIDS

Return Value:

    array of strings. last entry+1 of array contains NULL
    returns NULL on failure

--*/
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    LPTSTR * array;
    DWORD szChars;

    size = 8192; // initial guess, nothing magic about this
    buffer = new TCHAR[(size/sizeof(TCHAR))+2];
    if(!buffer) {
        return NULL;
    }
    while(!SetupDiGetDeviceRegistryProperty(Devs,DevInfo,Prop,&dataType,(LPBYTE)buffer,size,&reqSize)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+2];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    buffer[szChars+1] = TEXT('\0');
    array = GetMultiSzIndexArray(buffer);
    if(array) {
        return array;
    }

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

void DelMultiSz(_In_opt_ PZPWSTR Array)
/*++

Routine Description:

    Deletes the string array allocated by GetDevMultiSz/GetRegMultiSz/GetMultiSzIndexArray

Arguments:

    Array - pointer returned by GetMultiSzIndexArray

Return Value:

    None

--*/
{
    if(Array) {
        Array--;
        if(Array[0]) {
            delete [] Array[0];
        }
        delete [] Array;
    }
}


LPTSTR * GetRegMultiSz(_In_ HKEY hKey, _In_ LPCTSTR Val)
/*++

Routine Description:

    Get a multi-sz from registry
    and return as an array of strings

Arguments:

    hKey    - Registry Key
    Val     - Value to query

Return Value:

    array of strings. last entry+1 of array contains NULL
    returns NULL on failure

--*/
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    LPTSTR * array;
    DWORD szChars;
    LONG regErr;

    size = 8192; // initial guess, nothing magic about this
    buffer = new TCHAR[(size/sizeof(TCHAR))+2];
    if(!buffer) {
        return NULL;
    }
    reqSize = size;
    while(((regErr = RegQueryValueEx(hKey,Val,NULL,&dataType,(PBYTE)buffer,&reqSize)) != NO_ERROR)) {
        if(GetLastError() != ERROR_MORE_DATA) {
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+2];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    buffer[szChars+1] = TEXT('\0');

    array = GetMultiSzIndexArray(buffer);
    if(array) {
        return array;
    }

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

LPTSTR * CopyMultiSz(_In_opt_z_ PZPWSTR Array)
/*++

Routine Description:

    Creates a new array from old
    old array need not have been allocated by GetMultiSzIndexArray

Arguments:

    Array - array of strings, last entry is NULL

Return Value:

    MultiSz array allocated by GetMultiSzIndexArray

--*/
{
    LPTSTR multiSz = NULL;
    HRESULT hr = S_OK;
    int cchMultiSz = 0;
    int c = 0;
    if(Array) {
       for(c=0; Array[c]; c++) 
        {
           cchMultiSz+=lstrlen(Array[c])+1;
        }
    }
    cchMultiSz+=1; // final Null
    multiSz = new TCHAR[cchMultiSz];
    if(!multiSz) {
        return NULL;
    }
    int len = 0;
    if(Array) {
        for(c=0;Array[c];c++) {
            hr = StringCchCopy(multiSz+len,cchMultiSz-len,Array[c]);
            if(FAILED(hr)){
                if(multiSz)
                    delete [] multiSz;
                return NULL;
            }
            len+=lstrlen(multiSz+len)+1;
        }
    }
    
    if( len < cchMultiSz ){
        multiSz[len] = TEXT('\0');
    } else {
        // This should never happen!
        multiSz[cchMultiSz-1] = TEXT('\0');
    }
    
    LPTSTR * pRes = GetMultiSzIndexArray(multiSz);
    if(pRes) {
        return pRes;
    }
    delete [] multiSz;
    return NULL;
}
//Get deleted service name from hardware ID
//This will return "!<service>"
void GetDeletedService(TCHAR *out, int outLen) 
{
	if(out == NULL || outLen <=1)
		return; //invalid parameters

	//Check hardwareID and get its length
	size_t hardwareIDLen=_tcslen(_hardwareID);
	if(hardwareIDLen < 1)
		return; //empty hardware ID

	//Searching for the last '\'
	size_t backslashIndex = hardwareIDLen-1;
	for(; ; backslashIndex--) {
		if(_hardwareID[backslashIndex] == _T('\\'))
			break;
		if (backslashIndex == 0)
			break;
	}
	
	//After the last '\', it is the name of service
	//Copying it to the output buffer
	size_t j,i;
	for(i=backslashIndex+1, j=0; i<hardwareIDLen && j<static_cast<size_t>(outLen)-1; i++, j++)
		out[j]=_hardwareID[i];
	out[j]=_T('\0');
}
int EnumerateDevices(_In_ DWORD Flags, _In_ CallbackFunc Callback, _In_ LPVOID Context)
/*++

Routine Description:

    Generic enumerator for devices that will be passed the following arguments:
    <id> [<id>...]
    =<class> [<id>...]
    where <id> can either be @instance-id, or hardware-id and may contain wildcards
    <class> is a class name

Arguments:

    Flags    - extra enumeration flags (eg DIGCF_PRESENT)
    argc/argv - remaining arguments on command line
    Callback - function to call for each hit
    Context  - data to pass function for each hit

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    IdEntry idEntry;
   // DWORD err;
    int failcode = EXIT_FAIL;
    int retcode;
  //  int argIndex;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
//    BOOL doSearch = FALSE;
    BOOL match =FALSE;
    GUID cls;
    DWORD numClass = 0;

    idEntry.InstanceId = FALSE;
    idEntry.Wild = NULL;
    idEntry.String = _hardwareID;
    if(idEntry.String[0] == QUOTE_PREFIX_CHAR) {
        //
        // prefix to treat rest of string literally
        //
        idEntry.String = CharNext(idEntry.String);
    } else {
        //
        // see if any wild characters exist
        //
        idEntry.Wild = _tcschr(idEntry.String,WILD_CHAR);
    }

    //
    // add all id's to list
    // if there's a class, filter on specified class
    //
    devs = SetupDiGetClassDevsEx(numClass ? &cls : NULL,
                                 NULL,
                                 NULL,
                                (numClass ? 0 : DIGCF_ALLCLASSES) | Flags,
                                 NULL,
                                 NULL,
                                 NULL);

    if(devs == INVALID_HANDLE_VALUE) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: EnumerateDevices: SetupDiGetClassDevsEx: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }
 
    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: EnumerateDevices: SetupDiGetDeviceInfoListDetail: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // now enumerate them
    //
    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {
	//	TCHAR devID[MAX_DEVICE_ID_LEN];
        LPTSTR *hwIds = NULL;
        LPTSTR *compatIds = NULL;
 
		//
        // determine hardware ID's
        // and search for matches
        //
        hwIds = GetDevMultiSz(devs,&devInfo,SPDRP_HARDWAREID);
        compatIds = GetDevMultiSz(devs,&devInfo,SPDRP_COMPATIBLEIDS);

		if(WildCompareHwIds(hwIds,idEntry) ||
			WildCompareHwIds(compatIds,idEntry)) {
			match = TRUE;
		}
                
		DelMultiSz(hwIds);
		DelMultiSz(compatIds);

		if(match) {
			retcode = Callback(devs,&devInfo,devIndex,Context);
			if(retcode) {
				failcode = retcode;
				_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: EnumerateDevices: Callback: retcode=%d\n", retcode);
				CELOG_LOG(CELOG_DEBUG, debugStr);
				goto final;
			}
			match = FALSE;
		}
	}

    failcode = EXIT_OK;

final:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }
    return failcode;

}

int RemoveCallback(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Index, _In_ LPVOID Context)
/*++

Routine Description:

    Callback for use by Remove
    Invokes DIF_REMOVE
    uses SetupDiCallClassInstaller so cannot be done for remote devices
    Don't use CM_xxx API's, they bypass class/co-installers and this is bad.

Arguments:

    Devs    )_ uniquely identify the device
    DevInfo )
    Index    - index of device
    Context  - GenericContext

Return Value:

    EXIT_xxxx

--*/
{
    SP_REMOVEDEVICE_PARAMS rmdParams;
    GenericContext *pControlContext = (GenericContext*)Context;
    SP_DEVINSTALL_PARAMS devParams;
    LPCTSTR action = NULL;

    //
    // need hardware ID before trying to remove, as we wont have it after
    //
    TCHAR devID[MAX_DEVICE_ID_LEN];
  //  LPTSTR desc;
 //   BOOL b = TRUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        //
        // skip this
        //
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveCallback: SetupDiGetDeviceInfoListDetail: retcode=%d: skip this device\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        return EXIT_OK;
    }

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&rmdParams.ClassInstallHeader,sizeof(rmdParams)) ||
       !SetupDiCallClassInstaller(DIF_REMOVE,Devs,DevInfo)) {
        //
        // failed to invoke DIF_REMOVE
        //
        action = L"Fail"; //pControlContext->strFail;
    } else {
        //
        // see if device needs reboot
        //
        devParams.cbSize = sizeof(devParams);
        if(SetupDiGetDeviceInstallParams(Devs,DevInfo,&devParams) && (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT))) {
            //
            // reboot required
            //
            action = L"Reboot";// pControlContext->strReboot;
            pControlContext->reboot = TRUE;
        } else {
            //
            // appears to have succeeded
            //
            action = L"Success"; //pControlContext->strSuccess;
        }
        pControlContext->count++;
    }
    _tprintf(TEXT("%-60s: %ls\n"),devID,action);
	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveCallback: Remove device %-60ls: %ls\n", devID, action);
	CELOG_LOG(CELOG_DEBUG, debugStr);

    return EXIT_OK;
}

int DoDeleteDriverPackage()
/*++

Routine Description:
    dp_delete 
    Deletes a driver package to the machine.

Arguments:

Return Value:

    EXIT_xxxx

--*/
{
    int failcode = EXIT_FAIL;
    DWORD res;
    TCHAR InfFileName[MAX_PATH];
    PTSTR FilePart = NULL;
    HMODULE setupapiMod = NULL;
    SetupUninstallOEMInfProto SUOIFn;

    res = GetFullPathName(_infFile,
                          ARRAYSIZE(InfFileName),
                          InfFileName,
                          &FilePart);
    if ((!res) || (!FilePart)) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoDeleteDriverPackage: Invalid INF file %ws\n", _infFile);
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
	} else {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoDeleteDriverPackage: INF file %ws\n", _infFile);
		CELOG_LOG(CELOG_DEBUG, debugStr);
	}

    setupapiMod = LoadLibrary(TEXT("setupapi.dll"));
    if(!setupapiMod) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoDeleteDriverPackage: setupapi.dll is not found");
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }
    SUOIFn = (SetupUninstallOEMInfProto)GetProcAddress(setupapiMod,SETUPUNINSTALLOEMINF);
    if(!SUOIFn)
    {		
		_snwprintf_s(debugStr, 1024, _TRUNCATE,
				L"Failure: DoDeleteDriverPackage: can't find function %hs. We are on Windows2000 and ignore this. Skip removing inf file\n",
				SETUPUNINSTALLOEMINF);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		failcode=EXIT_OK;
        goto final;
    }

    if (!SUOIFn(FilePart,
                1,
                NULL)) {
        if (GetLastError() == ERROR_INF_IN_USE_BY_DEVICES) {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoDeleteDriverPackage: the INF file %ws is in used\n", _infFile);
			CELOG_LOG(CELOG_DEBUG, debugStr);
        } else if (GetLastError() == ERROR_NOT_AN_INSTALLED_OEM_INF) {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoDeleteDriverPackage: the file %ws is not an OEM INF file\n", _infFile);
			CELOG_LOG(CELOG_DEBUG, debugStr);
        } else {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoDeleteDriverPackage: skip deleting INF %ws: retcode=%d\n", 
					_infFile, GetLastError());
			CELOG_LOG(CELOG_DEBUG, debugStr);
			failcode=EXIT_OK;
       }
        goto final;
    }

    failcode = EXIT_OK;

final:
    if (setupapiMod) {
        FreeLibrary(setupapiMod);
    }

    return failcode;
}


int DoUpdate()
/*++

Routine Description:
    UPDATE
    update driver for existing device(s)

Arguments:
	None

Return Value:

    EXIT_xxxx

--*/
{
	HMODULE newdevMod = NULL;
//	HMODULE setupapiMod = NULL;
	int failcode = EXIT_FAIL;
    UpdateDriverForPlugAndPlayDevicesProto UpdateFn;
//	SetupCopyOEMInfProto copyInfFn;
    BOOL reboot = FALSE;
    LPCTSTR inf = NULL;
    DWORD flags = 0;
    DWORD res;
    TCHAR InfPath[MAX_PATH];

    //
    // Inf must be a full pathname
    //
    res = GetFullPathName(_infFile,MAX_PATH,InfPath,NULL);
    if((res >= MAX_PATH) || (res == 0)) {
        //
        // inf pathname too long
        //
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoUpdate: INF file path name is invalid: error=%d\n", res);
		CELOG_LOG(CELOG_DEBUG, debugStr);
        return EXIT_FAIL;
    }
    if(GetFileAttributes(InfPath)==(DWORD)(-1)) {
        //
        // inf doesn't exist
        //
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoUpdate: INF file %ws doesn't exist\n", _infFile);
		CELOG_LOG(CELOG_DEBUG, debugStr);
        return EXIT_FAIL;
    }
    inf = InfPath;
    flags |= INSTALLFLAG_FORCE ;

    //
    // make use of UpdateDriverForPlugAndPlayDevices
    //
    newdevMod = LoadLibrary(TEXT("newdev.dll"));
    if(!newdevMod) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoUpdate: can't load library newdev.dll\n");
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }
    UpdateFn = (UpdateDriverForPlugAndPlayDevicesProto)GetProcAddress(newdevMod,UPDATEDRIVERFORPLUGANDPLAYDEVICES);
    if(!UpdateFn) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoUpdate: Can't find function %hs\n", UPDATEDRIVERFORPLUGANDPLAYDEVICES);
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    if(!UpdateFn(NULL,_hardwareID,inf,flags,&reboot)) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoUpdate: update drivers for %ws from %ws failed: error=%d\n", 
				_hardwareID, inf, GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
		goto final;
    }
    failcode = reboot ? EXIT_REBOOT : EXIT_OK;
final:

    if(newdevMod) {
        FreeLibrary(newdevMod);
    }
    return failcode;
}

int DoInstallation()
/*++

Routine Description:

    CREATE
    Creates a root enumerated devnode and installs drivers on it

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO DeviceInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID ClassGUID;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    TCHAR hwIdList[LINE_LEN+4];
    TCHAR InfPath[MAX_PATH];
 //   DWORD err;
    int failcode = EXIT_FAIL;
 //   BOOL reboot = FALSE;
//    DWORD flags = 0;
//    DWORD len;
	
    //
    // Inf must be a full pathname
    //
    if(GetFullPathName(_infFile,MAX_PATH,InfPath,NULL) >= MAX_PATH) {
        //
        // inf pathname too long
        //
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: INF file path name is too long (>=%d)\n",MAX_PATH);
		CELOG_LOG(CELOG_DEBUG, debugStr);
        return EXIT_FAIL;
	} else {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoInstallation: INF file full path is %ws\n", InfPath);
		CELOG_LOG(CELOG_DEBUG, debugStr);
	}
    //
    // List of hardware ID's must be double zero-terminated
    //
    ZeroMemory(hwIdList,sizeof(hwIdList));
    if (FAILED(StringCchCopy(hwIdList,LINE_LEN,_hardwareID))) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: StringCchCopy failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // Use the INF File to extract the Class GUID.
    //
    if (!SetupDiGetINFClass(InfPath,&ClassGUID,ClassName,sizeof(ClassName)/sizeof(ClassName[0]),0))
    {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: SetupDiGetINFClass failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // Create the container for the to-be-created Device Information Element.
    //
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: SetupDiCreateDeviceINfoList failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // Now create the element.
    // Use the Class GUID and Name from the INF file.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet,
        ClassName,
        &ClassGUID,
        NULL,
        0,
        DICD_GENERATE_ID,
        &DeviceInfoData))
    {	
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: SetupDiCreateDeviceInfo failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // Add the HardwareID to the Device's HardwareID property.
    //
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
        &DeviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)hwIdList,
        (lstrlen(hwIdList)+1+1)*sizeof(TCHAR)))
    {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: SetupDiSetDeviceRegistryProperty failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

    //
    // Transform the registry element into an actual devnode
    // in the PnP HW tree.
    //
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        DeviceInfoSet,
        &DeviceInfoData))
    {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: SetupDiCallClassInstaller failed: error=%d\n", GetLastError());
		CELOG_LOG(CELOG_DEBUG, debugStr);
        goto final;
    }

	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoInstallation: Device node created. Install drivers ...\n");
	CELOG_LOG(CELOG_DEBUG, debugStr);

    //
    // update the driver for the device we just created
    //
    failcode = DoUpdate();

final:

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

	if(failcode == EXIT_FAIL) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Failure: DoInstallation: installation failed\n");
		CELOG_LOG(CELOG_DEBUG, debugStr);
		return failcode;
	}

	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoInstallation: Installation successful\n");
	CELOG_LOG(CELOG_DEBUG, debugStr);
	return EXIT_OK;
}
int DoRemoveDeviceNode()
/*++

Routine Description:

    REMOVE
    remove devices

Arguments:
	None

Return Value:

    EXIT_xxxx

--*/
{
    GenericContext context;
    TCHAR strRemove[80];
    TCHAR strReboot[80];
    TCHAR strFail[80];
    int failcode = EXIT_FAIL;

    context.reboot = FALSE;
    context.count = 0;
    context.strReboot = strReboot;
    context.strSuccess = strRemove;
    context.strFail = strFail;
    failcode = EnumerateDevices(DIGCF_PRESENT,RemoveCallback,&context);

    if(failcode == EXIT_OK) {
		if(!context.count) {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoRemoveDeviceNode: No Device removed\n");
			CELOG_LOG(CELOG_DEBUG, debugStr);
        } else if(!context.reboot) {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoRemoveDeviceNode: %d device(s) were removed\n", context.count);
			CELOG_LOG(CELOG_DEBUG, debugStr);
        } else {
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoRemoveDeviceNode: %d dvice(s) are ready to be removed. Reboot the system to remove them.\n", 
					context.count);
			CELOG_LOG(CELOG_DEBUG, debugStr);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}

int RemoveFromClassFilter(LPTSTR regval, PCWSTR className)
/*++

Routine Description:

    Remove the class filter driver from "lower" filter driver of class 
Arguments:


Return Value:

    EXIT_xxxx

--*/
{
    int failcode = EXIT_FAIL;
  //  int argIndex;
    DWORD numGuids = 0;
    GUID guid;
    HKEY hk = (HKEY)INVALID_HANDLE_VALUE;
    LPTSTR * multiVal = NULL;
    int after;
//    bool modified = false;
    int span;
//    SC_HANDLE SCMHandle = NULL;
//    SC_HANDLE ServHandle = NULL;
	TCHAR deletedService[125];

	//Get deleted service name from hardware ID
	deletedService[0]=_T('\0');
	GetDeletedService(deletedService, 125);
	if(deletedService[0] == _T('\0')) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, 
				L"Message: RemoveFromClassFilter: the name (%ls) of service is invalid\n",
				_hardwareID);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		goto final;
	}


	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: Attempt to remove service %ls from %ls of class %ls\n",
			deletedService, regval, className);
	CELOG_LOG(CELOG_DEBUG, debugStr);

    //
    // just take the first guid for the name
    // Return the pointer to the GUID of the class whose registry key is to be opened at the following
    if(!SetupDiClassGuidsFromNameEx(className,&guid,1,&numGuids,NULL,NULL)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto final;
        }
    }
    if(numGuids == 0) {
	 	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: No filter class %ls on this system and nothing need to do\n",
				className);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		goto final;
    }

	//opens the device setup class registry key with "KEY_READ" and "KEY_WRITE" permission
    hk = SetupDiOpenClassRegKeyEx(&guid,
                                  KEY_READ | KEY_WRITE,
                                  DIOCR_INSTALLER,
                                  NULL,
                                  NULL
                                 );
    if(hk == INVALID_HANDLE_VALUE) {
	 	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: can't open registry key of class %ls\n",
				className);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		goto final;
    }
    multiVal = GetRegMultiSz(hk,regval);

    after = -1; // for the @service expressions
    span =  1;

    if(!multiVal) {
        multiVal = CopyMultiSz(NULL);
        if(!multiVal) {
            goto final;
        }
    }

	//
    // need to find specified service in list
    //
    for(after+=span;multiVal[after];after++) {
		if(_tcsicmp(multiVal[after],deletedService)==0) {
			break;
		}
    }
    if(!multiVal[after]) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: Can't find service %ls from %ls of class %ls. Nothing need to be done\n",
				deletedService, regval, className);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		failcode=EXIT_OK;
        goto final;
	}

    //
    // we're modifying
    //
    int c;
    for(c = after;multiVal[c];c++) {
		multiVal[c] = multiVal[c+1];
	}

    LPTSTR * newArray = CopyMultiSz(multiVal);
    if(!newArray) {
		goto final;
    }
    DelMultiSz(multiVal);
    multiVal = newArray;	

    if(multiVal[0]) {
		int len = 0;
        LPTSTR multiSz = multiVal[-1];
        LPTSTR p = multiSz;
        while(*p) {
			p+=lstrlen(p)+1;
        }
		
        p++; // skip past null
        len = static_cast<int>((p-multiSz)*sizeof(TCHAR));
        LONG err = RegSetValueEx(hk,regval,0,REG_MULTI_SZ,(LPBYTE)multiSz,len);
        if(err==NO_ERROR) {
			failcode = EXIT_REBOOT;
 			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: remove service %ls from %ls filter of class %ls. Reboot the system.\n",
					deletedService, regval, className);
			CELOG_LOG(CELOG_DEBUG, debugStr);
		}
	} else {
		LONG err = RegDeleteValue(hk,regval);
        if((err == NO_ERROR) || (err == ERROR_FILE_NOT_FOUND)) {
			failcode = EXIT_REBOOT;
			_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: RemoveFromClassFilter: remove service %ls from %ls filter of class %ls. Reboot the system.\n",
					deletedService, regval, className);
			CELOG_LOG(CELOG_DEBUG, debugStr);
		}
	}
final:
    if(multiVal) {
        DelMultiSz(multiVal);
    }
    if(hk != (HKEY)INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }
    return failcode;
}


int DoUninstallation()
{
	int failCode=EXIT_OK;
	int classFilterRetCode=EXIT_OK;

	//Deletes the device stack and removes the devnode
    failCode=DoRemoveDeviceNode();
	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoUninstallation: DoRemoveDeviceNode: ret=%d\n", failCode);
	CELOG_LOG(CELOG_DEBUG, debugStr);

	if(failCode != EXIT_FAIL) {
		//If the driver is a kind of filter driver, it needs to be removed
		//from its setup class.
		TCHAR *classNames[7]={_T("net"), _T("usb"), _T("bluetooth"), _T("image"), 
							_T("avc"), _T("sbp2"), _T("61883")};
		int removeRetCode;

		for(int i=0; i<7; i++) {
			//Check the upper filter
			removeRetCode=RemoveFromClassFilter(REGSTR_VAL_UPPERFILTERS, classNames[i]);
			classFilterRetCode=(classFilterRetCode==EXIT_REBOOT)?EXIT_REBOOT:removeRetCode;
			//Check the lower filter
			removeRetCode=RemoveFromClassFilter(REGSTR_VAL_LOWERFILTERS, classNames[i]);
			classFilterRetCode=(classFilterRetCode==EXIT_REBOOT)?EXIT_REBOOT:removeRetCode;
		}
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoUninstallation: RemoveFromClassFilter: ret=%d\n", classFilterRetCode);
		CELOG_LOG(CELOG_DEBUG, debugStr);
	}

	if(failCode != EXIT_FAIL) {
		//Delete driver binaries or the driver and INF from the driver store
		int deleteDriverPackRetCode=DoDeleteDriverPackage();
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: DoDeleteDriverPackage: ret=%d\n", deleteDriverPackRetCode);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		if(deleteDriverPackRetCode == EXIT_FAIL)
			failCode=EXIT_FAIL;
		else {
			if(failCode == EXIT_REBOOT || deleteDriverPackRetCode == EXIT_REBOOT)
				failCode=EXIT_REBOOT;
		}
	}

	if(failCode != EXIT_FAIL) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: Uninstall driver successfully\n");
		CELOG_LOG(CELOG_DEBUG, debugStr);
		if(failCode == EXIT_REBOOT || classFilterRetCode==EXIT_REBOOT)
			return EXIT_REBOOT;
		return EXIT_OK;
	}
	return failCode;
}
}

/*
Function:
	nlPnpDriverInstall
Description:
	Install or remove a plug-and-play device driver
Params:
      bInstall (in)  : true if it is installation; false if it is removal
      infFile (in)   : The name (including full path) of INF file
      hardwareID (in): Hardware ID for the device.  Supplied by Enforcer team.
                       Use "root\NLDevEnf".
Return:
0: OK
1: (requres) REBOOT
2: Fail
*/
//extern "C" __declspec(dllexport) int nlPnpDriverInstall(BOOLEAN bInstall, LPCTSTR infFile, LPCTSTR hardwareID)
int nlPnpDriverInstall(BOOLEAN bInstall, LPCTSTR infFile, LPCTSTR hardwareID)
{
    int failCode = EXIT_FAIL;

	//Checking input parameters
	_bInstall=bInstall;

	if(infFile == NULL) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"FAILURE: The INF file is not defined\n");
		CELOG_LOG(CELOG_DEBUG, debugStr);
		return EXIT_FAIL;
	} else
		_infFile=infFile;

	if(hardwareID == NULL) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: The hardware ID is not defined\n");
		CELOG_LOG(CELOG_DEBUG, debugStr);
		return EXIT_FAIL;
	} else
		_hardwareID=hardwareID;
	//--end--Checking input parameters

    if (_bInstall) {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: INSTALL %ls. The INF file is located at %ls\n", 
			hardwareID, infFile);
		CELOG_LOG(CELOG_DEBUG, debugStr);
		failCode=DoInstallation();
    } else {
		_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: REMOVE %ls.\n", hardwareID);
		CELOG_LOG(CELOG_DEBUG, debugStr);
	    failCode=DoUninstallation();
    }

	_snwprintf_s(debugStr, 1024, _TRUNCATE, L"Message: %ls %ls returns %d\n", bInstall?L"INSTALL":L"REMOVE", 
			hardwareID, failCode);
	CELOG_LOG(CELOG_DEBUG, debugStr);
    return failCode;
}
