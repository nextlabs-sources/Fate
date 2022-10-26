#ifndef CERESATTRMGR_H
#define CERESATTRMGR_H

#ifndef TAGLIB_WINDOWS
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define TAGLIB_WINDOWS
#include <windows.h>
#endif
#endif

#include "resattrlib.h"

enum TagType
{
	TagTypeDefault = 0, TagTypeNTFS,
	TagSummary
};

struct ResourceAttributeManager;

#ifdef TAGLIB_WINDOWS
	#ifdef BUILDING_RESATTRMGR
		#define RESATTRMGR_EXPORT __declspec(dllexport)
	#else
		#define RESATTRMGR_EXPORT
	#endif
#else
#define RESATTRMGR_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

RESATTRMGR_EXPORT int CreateAttributeManager(ResourceAttributeManager **mgr);
RESATTRMGR_EXPORT int ReadResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int ReadResourceAttributesForNTFSA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesForNTFSA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int RemoveResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT void CloseAttributeManager(ResourceAttributeManager *mgr);
RESATTRMGR_EXPORT int Convert_Raw_2_PC_For_Non_Office(ResourceAttributes *raw_attrs,ResourceAttributes* PC_attrs);
RESATTRMGR_EXPORT int Convert_PC_2_RAW_For_Non_Office(ResourceAttributes *PC_attrs,ResourceAttributes* raw_attrs);
RESATTRMGR_EXPORT int Convert_GetAttributes(ResourceAttributes *attrs, ResourceAttributes* existing_attrs);
RESATTRMGR_EXPORT int Convert_SetAttributes(ResourceAttributes* attrs_to_set, ResourceAttributes* merged_attrs);

#ifdef TAGLIB_WINDOWS
RESATTRMGR_EXPORT int ReadResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int ReadResourceAttributesForNTFSW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int WriteResourceAttributesForNTFSW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
RESATTRMGR_EXPORT int RemoveResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
#endif

RESATTRMGR_EXPORT bool IsNxlFormat(const WCHAR *filename);
RESATTRMGR_EXPORT int ReadResrcSummaryAttr(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

#ifdef UNICODE
	#define ReadResourceAttributes ReadResourceAttributesW
	#define ReadResourceAttributesForNTFS ReadResourceAttributesForNTFSW
	#define WriteResourceAttributes WriteResourceAttributesW
	#define WriteResourceAttributesForNTFS WriteResourceAttributesForNTFSW
	#define RemoveResourceAttributes RemoveResourceAttributesW
#else
	#define ReadResourceAttributes ReadResourceAttributesA
	#define ReadResourceAttributesForNTFS ReadResourceAttributesForNTFSA
	#define WriteResourceAttributes WriteResourceAttributesA
	#define WriteResourceAttributesForNTFS WriteResourceAttributesForNTFSA
	#define RemoveResourceAttributes RemoveResourceAttributesA
#endif

#ifdef __cplusplus
}
#endif

#endif



