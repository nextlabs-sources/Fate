#ifndef CERESATTRLIB_H
#define CERESATTRLIB_H

struct ResourceAttributes;

#ifdef BUILDING_RESATTRLIB
#define RESATTRLIB_EXPORT __declspec(dllexport)
#else
#define RESATTRLIB_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

RESATTRLIB_EXPORT int AllocAttributes(ResourceAttributes **attrs);
RESATTRLIB_EXPORT void AddAttributeA(ResourceAttributes *attrs, const char *name, const char *value);
RESATTRLIB_EXPORT void AddAttributeW(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
RESATTRLIB_EXPORT void FreeAttributes(ResourceAttributes *attrs);
RESATTRLIB_EXPORT int GetAttributeCount(const ResourceAttributes *attrs);
RESATTRLIB_EXPORT const WCHAR *GetAttributeName(const ResourceAttributes *attrs, int index);
RESATTRLIB_EXPORT const WCHAR *GetAttributeValue(const ResourceAttributes *attrs, int index);

#ifdef UNICODE
#define AddAttribute AddAttributeW
#else
#define AddAttribute AddAttributeA
#endif

#ifdef __cplusplus
}
#endif

#endif
