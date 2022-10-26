

#ifndef __NUDF_SHARE_MODDEF_H__
#define __NUDF_SHARE_MODDEF_H__

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif


extern const ULONG __NXRMModuleId;
extern const ULONG __NXRMDrvModId;

#define NXMODID()               __NXRMModuleId
#define NXDRVID()               __NXRMDrvModId
#define NXMODNAME()             ((const CHAR*)(&__NXRMModuleId))
#define NXDRVNAME()             ((const CHAR*)(&__NXRMDrvModId))
#define DECLARE_NXRM_MODULE(id) const ULONG __NXRMModuleId = (id)
#define DECLARE_NXRM_DRVMOD(id) const ULONG __NXRMDrvModId = (id)


//
//  Define Module IDs
//
#define MOD_UNKNOWN     '\0KNU' // "UNK" -- Unknown
#define MOD_AUTOUPDATE  '\0DUA' // "AUD" -- AutoUpdate              (nxrmau.exe)
#define MOD_CORE        '\0ROC' // "COR" -- Core Lib                (nxrmcore.dll)
#define MOD_DRV         '\0VRD' // "DRV" -- Core Driver             (nxrmdrv.sys)
#define MOD_DRVMAN      '\0MRD' // "DRM" -- Core Driver Manager     (nxrmdrvman.dll)
#define MOD_FLT         '\0TLF' // "FLT" -- Filter Driver           (nxrmflt.sys)
#define MOD_FLTMAN      '\0MLF' // "FLM" -- Filter Driver Manager   (nxrmfltman.dll)
#define MOD_SERVER      '\0VRS' // "SRV" -- Server                  (nxrmserv.exe)
#define MOD_ENGINE      '\0GNE' // "ENG" -- Engine Lib              (nxrmeng.dll)
#define MOD_TRAY        '\0YRT' // "TRY" -- Tray App                (nxrmtray.exe)
#define MOD_SHELL       '\0LHS' // "SHL" -- Shell Plug-in           (nxrmshell.dll)


#define DECLARE_NXRM_MODULE_AUTOUPDATE()    const ULONG __NXRMModuleId = MOD_AUTOUPDATE
#define DECLARE_NXRM_MODULE_CORE()          const ULONG __NXRMModuleId = MOD_CORE
#define DECLARE_NXRM_MODULE_DRV()           const ULONG __NXRMDrvModId = MOD_DRV
#define DECLARE_NXRM_MODULE_DRVMAN()        const ULONG __NXRMModuleId = MOD_DRVMAN
#define DECLARE_NXRM_MODULE_FLT()           const ULONG __NXRMDrvModId = MOD_FLT
#define DECLARE_NXRM_MODULE_FLTMAN()        const ULONG __NXRMModuleId = MOD_FLTMAN
#define DECLARE_NXRM_MODULE_SERVER()        const ULONG __NXRMModuleId = MOD_SERVER
#define DECLARE_NXRM_MODULE_ENGINE()        const ULONG __NXRMModuleId = MOD_ENGINE
#define DECLARE_NXRM_MODULE_TRAYAPP()       const ULONG __NXRMModuleId = MOD_TRAY
#define DECLARE_NXRM_MODULE_SHELL()         const ULONG __NXRMModuleId = MOD_SHELL


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_MODDEF_H__