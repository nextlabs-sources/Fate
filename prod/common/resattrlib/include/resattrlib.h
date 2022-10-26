#ifndef CERESATTRLIB_H
#define CERESATTRLIB_H

#ifndef TAGLIB_WINDOWS
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define TAGLIB_WINDOWS
#endif
#endif

struct ResourceAttributes;

#ifdef TAGLIB_WINDOWS
	#ifdef BUILDING_RESATTRLIB
		#define RESATTRLIB_EXPORT __declspec(dllexport)
	#else
		#define RESATTRLIB_EXPORT
	#endif
#endif //WIN32 or _WIN64

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TAGLIB_WINDOWS
RESATTRLIB_EXPORT int 	AllocAttributes(ResourceAttributes **attrs);
RESATTRLIB_EXPORT void 	AddAttributeA(ResourceAttributes *attrs, const char *name, const char *value);
RESATTRLIB_EXPORT void 	AddAttributeW(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
RESATTRLIB_EXPORT void 	FreeAttributes(ResourceAttributes *attrs);
RESATTRLIB_EXPORT int 	GetAttributeCount(const ResourceAttributes *attrs);
RESATTRLIB_EXPORT const WCHAR *GetAttributeName(const ResourceAttributes *attrs, int index);
RESATTRLIB_EXPORT const WCHAR *GetAttributeValue(const ResourceAttributes *attrs, int index);

RESATTRLIB_EXPORT void 	SetAttributeValueA(ResourceAttributes *attrs, int index,const char* value);
RESATTRLIB_EXPORT void 	SetAttributeValueW(ResourceAttributes *attrs, int index,const WCHAR* value);
RESATTRLIB_EXPORT int	FindAttributeA(ResourceAttributes *attrs, const char* name,int& idx);
RESATTRLIB_EXPORT int	FindAttributeW(ResourceAttributes *attrs, const WCHAR* name,int& idx);


#ifdef UNICODE
	#define AddAttribute 		AddAttributeW
	#define SetAttributeValue	SetAttributeValueW
	#define FindAttribute		FindAttributeW
#else
	#define AddAttribute 		AddAttributeA
	#define SetAttributeValue 	SetAttributeValueA
	#define FindAttribute 		FindAttributeA
#endif

#else

int 		AllocAttributes(ResourceAttributes **attrs);
void 		AddAttributeA(ResourceAttributes *attrs, const char *name, const char *value);
void 		FreeAttributes(ResourceAttributes *attrs);
int 		GetAttributeCount(const ResourceAttributes *attrs);
const char *GetAttributeName(const ResourceAttributes *attrs, int index);
const char *GetAttributeValue(const ResourceAttributes *attrs, int index);
void 		SetAttributeValueA(ResourceAttributes *attrs, int index,const char* value);
int 		FindAttributeA(ResourceAttributes *attrs, const char* name,int& idx);

#define AddAttribute 		AddAttributeA
#define SetAttributeValue 	SetAttributeValueA
#define FindAttribute 		FindAttributeA
	
#endif

#ifdef __cplusplus
}
#endif

#endif
