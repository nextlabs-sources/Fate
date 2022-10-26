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
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPREDEVICEINSTALL)(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called after (your) WDF driver is installed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPostDeviceInstall(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPOSTDEVICEINSTALL)(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called before (your) WDF driver is removed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPreDeviceRemove(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName

    );

typedef
ULONG
(WINAPI *PFN_WDFPREDEVICEREMOVE)(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

//----------------------------------------------------------------------------
// To be called after (your) WDF driver is removed.
//----------------------------------------------------------------------------
ULONG
WINAPI
WdfPostDeviceRemove(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName
    );

typedef
ULONG
(WINAPI *PFN_WDFPOSTDEVICEREMOVE)(
    __in PWCHAR  InfPath,
    __in_opt PWCHAR  InfSectionName

    );



#endif // (NTDDI_VERSION >= NTDDI_WIN2K)


#endif // _WDFINSTALLER_H_

