// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HTTPE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HTTPE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef HTTPE_EXPORTS
#define HTTPE_API __declspec(dllexport)
#else
#define HTTPE_API __declspec(dllimport)
#endif

// This is an example of an exported function.
extern "C" HTTPE_API int fnHTTPE(void);
