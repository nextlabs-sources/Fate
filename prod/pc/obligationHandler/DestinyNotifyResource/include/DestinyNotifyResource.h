// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DESTINYNOTIFYRESOURCE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DESTINYNOTIFYRESOURCE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DESTINYNOTIFYRESOURCE_EXPORTS
#define DESTINYNOTIFYRESOURCE_API __declspec(dllexport)
#else
#define DESTINYNOTIFYRESOURCE_API __declspec(dllimport)
#endif

// This class is exported from the DestinyNotifyResource.dll
class DESTINYNOTIFYRESOURCE_API CDestinyNotifyResource {
public:
	CDestinyNotifyResource(void);
	// TODO: add your methods here.
};

extern DESTINYNOTIFYRESOURCE_API int nDestinyNotifyResource;

DESTINYNOTIFYRESOURCE_API int fnDestinyNotifyResource(void);
