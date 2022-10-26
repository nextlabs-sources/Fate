#ifndef CERESATTRMGR_H
#define CERESATTRMGR_H

#include "resattrlib.h"

struct ResourceAttributeManager;

#ifdef BUILDING_RESATTRMGR
#define RESATTRMGR_EXPORT __declspec(dllexport)
#else
#define RESATTRMGR_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

RESATTRMGR_EXPORT int CreateAttributeManager(ResourceAttributeManager **mgr);
RESATTRMGR_EXPORT int ReadResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int ReadResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int RemoveResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int RemoveResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT void CloseAttributeManager(ResourceAttributeManager *mgr);


#ifdef UNICODE
#define ReadResourceAttributes ReadResourceAttributesW
#define WriteResourceAttributes WriteResourceAttributesW
#define RemoveResourceAttributes RemoveResourceAttributesW
#else
#define ReadResourceAttributes ReadResourceAttributesA
#define WriteResourceAttributes WriteResourceAttributesA
#define RemoveResourceAttributes RemoveResourceAttributesA
#endif

#ifdef __cplusplus
}
#endif

#endif



