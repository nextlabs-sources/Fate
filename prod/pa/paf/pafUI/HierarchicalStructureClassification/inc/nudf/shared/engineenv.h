

#ifndef __NUDF_SHARE_ENGINE_CONTEXT_H__
#define __NUDF_SHARE_ENGINE_CONTEXT_H__

#include <Windows.h>
#include <nudf\shared\logdef.h>
#include <nudf\shared\moddef.h>
#include <nudf\log.hpp>
#include <nudf\exception.hpp>

#ifdef __cplusplus
extern "C" {
#endif

//
//  Declare Engine Callbacks
//


//
//  Declare Structs
//

typedef struct _NXRM_ENGINE_ENVBLOCK {
    GUID    Id; // Example: {3163C566-D381-4467-87BC-A65A18D5B649}
    WCHAR   IdName[40];
    WCHAR   RootDir[MAX_PATH];
} NXRM_ENGINE_ENVBLOCK, *PNXRM_ENGINE_ENVBLOCK;




#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_ENGINE_CONTEXT_H__