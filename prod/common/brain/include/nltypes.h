#ifndef NLTYPES_H
#define NLTYPES_H

#ifdef Linux
#include <linux/limits.h>
#endif //end ifdef Linux

#if defined (WIN32) || defined (_WIN64)
#include <windows.h>
#include <tchar.h>
#include <winsock2.h>
#endif //end ifdef WIN32 || WIN64

#if defined (Linux) || defined (Darwin)

#ifndef NL_PATH_MAX
#define NL_PATH_MAX     PATH_MAX
#endif

#ifndef NLSTRING
#define NLSTRING
#define nlstring        string
#define nlprintf        printf
#define nlfprintf       fprintf
#define nlsprintf       sprintf
#endif

#ifndef NLSOCKET
#define NLSOCKET
#define nlsocket        int
#endif

typedef char            nlchar;
typedef unsigned char   nluchar;
typedef short           nlint16;
typedef unsigned short  nluint16;
typedef int             nlint32;
typedef unsigned int    nluint32;
typedef nlint32         nlint;    //general purpose definition
typedef nlint           nlDescriptor;
typedef unsigned long   nlulong;


#endif //end ifdef Linux

#if defined (WIN32) || defined (_WIN64)

#ifndef NL_PATH_MAX
#define NL_PATH_MAX     MAX_PATH
#endif

#ifndef NLSTRING
#define NLSTRING
#ifdef UNICODE
#define nlstring        wstring
#else 
#define nlstring        string
#endif
#endif

#ifndef NLPRINTF
#define NLPRINTF
#define nlprintf        _tprintf
#define nlfprintf       _ftprintf
#ifdef UNICODE
#define nlsprintf       swprintf
#else
#define nlsprintf       _stprintf
#endif
#endif

#ifndef NLSOCKET
#define NLSOCKET
#define nlsocket        SOCKET
#endif

typedef TCHAR           nlchar;
typedef _TUCHAR         nluchar;
typedef unsigned short  nluint16;
typedef short           nlint16;
typedef unsigned int    nluint32;
typedef int             nlint32;
typedef nlint32         nlint;    //general purpose definition
typedef HANDLE          nlDescriptor;
typedef ULONGLONG       nlulong;
#endif //end ifdef WIN32 || WIN64



#endif  //NLTYPES_H 
