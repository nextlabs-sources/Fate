

#ifndef __NUDF_SHARE_LOGDEF_H__
#define __NUDF_SHARE_LOGDEF_H__

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif



//
//  Log Level
//
typedef enum _LOGLEVEL {
    LOGCRITICAL = 0,
    LOGERROR,
    LOGWARNING,
    LOGINFO,
    LOGDEBUG,
    LOGDETAIL,
    LOGUSER = 200
} LOGLEVEL;

#define LOGNUNKNOWN     "UKN"
#define LOGNCRITICAL    "CRI"
#define LOGNERROR       "ERR"
#define LOGNWARNING     "WAR"
#define LOGNINFO        "INF"
#define LOGNDEBUG       "DBG"
#define LOGNDETAIL      "DTL"
#define LOGNUSER        "USR"

//
//  Log Functions
//

typedef BOOL (WINAPI *LOGAPI_ACCEPT)(_In_ ULONG Level);
typedef LONG (WINAPI *LOGAPI_LOG)(_In_ LPCWSTR Info);

#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_LOGDEF_H__