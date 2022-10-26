
#pragma once
#ifndef _NL_DEV_CON_HPP_
#define _NL_DEV_CON_HPP_
#include <Setupapi.h>
#include <Cfgmgr32.h>

// Error ode
#define NLDEVCON_HELP                   0
#define NLDEVCON_SUCCESS                1
#define NLDEVCON_SUCCESS_NEED_REBOOT    2
#define NLDEVCON_FAIL_TO_INSTALL        -1
#define NLDEVCON_FAIL_TO_UNINSTALL      -2
#define NLDEVCON_FAIL_TO_UPDATE         -3


#ifndef MAX_CLASS_NAME_LEN
#define MAX_CLASS_NAME_LEN  32
#endif

#ifndef MAX_DEVICE_ID_LEN
#define MAX_DEVICE_ID_LEN  32
#endif

// UpdateDriverForPlugAndPlayDevices
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(__in HWND hwndParent,
                                                              __in LPCTSTR HardwareId,
                                                              __in LPCTSTR FullInfPath,
                                                              __in DWORD InstallFlags,
                                                              __out_opt PBOOL bRebootRequired
                                                         );
typedef BOOL (WINAPI *SetupSetNonInteractiveModeProto)(__in BOOL NonInteractiveFlag
                                                      );
typedef BOOL (WINAPI *SetupUninstallOEMInfProto)(__in LPCTSTR InfFileName,
                                                 __in DWORD Flags,
                                                 __reserved PVOID Reserved
                                                 );

#if _SETUPAPI_VER >= _WIN32_WINNT_WINXP
typedef BOOL (WINAPI *SetupVerifyInfFileProto)(__in LPCTSTR InfName,
                                               __in_opt PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,
                                               __inout PSP_INF_SIGNER_INFO InfSignerInfo );
#endif

#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfA"
#endif
#define SETUPSETNONINTERACTIVEMODE "SetupSetNonInteractiveMode"
#define SETUPVERIFYINFFILE "SetupVerifyInfFile"

#endif