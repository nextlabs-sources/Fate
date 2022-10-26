/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

_WdfVersionBuild_

Module Name:

    WdfInstaller.h

Abstract:

    Contains prototypes for the WDF installer support.

Author:

Environment:

    kernel mode only

Revision History:

--*/

#ifndef _WDFINSTALLER_H_
#define _WDFINSTALLER_H_



#if (NTDDI_VERSION >= NTDDI_WIN2K)



//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// To be called before (your) WDF driver is installed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPreDeviceInstall(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPREDEVICEINSTALL)(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called after (your) WDF driver is installed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPostDeviceInstall(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPOSTDEVICEINSTALL)(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called before (your) WDF driver is removed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPreDeviceRemove(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName

    );

typedef
ULONG
(WINAPI *PFN_WDFPREDEVICEREMOVE)(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called after (your) WDF driver is removed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPostDeviceRemove(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPOSTDEVICEREMOVE)(
    _In_ PWCHAR  InfPath,
    _In_opt_ PWCHAR  InfSectionName

    );



#endif // (NTDDI_VERSION >= NTDDI_WIN2K)


#endif // _WDFINSTALLER_H_

