#ifndef LINUX_WIN_H
#define LINUX_WIN_H

#define _TRUNCATE  -1

#define vsnprintf_s(buffer,sizeOfBuffer,count,format,...) \
        vsnprintf(buffer,sizeOfBuffer,format,__VA_ARGS__)

#define sprintf_s  snprintf

#define _tcsncpy  strncpy

#define _tcsstr  strstr

#define _stprintf  sprintf

#define _tcscpy  strcpy

#define _tcscmp  strcmp

#define _tcsicmp  strcasecmp

#define _tcscat  strcat

#define _tcslen  strlen

#define _tprintf  printf

#define _stprintf  sprintf

#define OutputDebugString(str)  

#define OutputDebugStringW(str)

#define RevertToSelf()  

#define LoadLibrary(str)  dlopen(str,RTLD_NOW)

#define GetProcAddress(h,name)  dlsym(h,name)

#define ImpersonateLoggedOnUser(handle) 

#define _i64toa(val,buf,size)  _snprintf_s(buf,size, _TRUNCATE,"%lld",val)  

#define _timeb  timeb
#define _ftime64_s  ftime
#define ctime_s(buffer,numberOfElements,time) buffer = ctime(time) 

#define _countof(ar)  (sizeof(ar)/sizeof(ar[0]))

#define towlower  tolower

#define TCHAR  char

#define LPCTSTR  const char*

#define LPTSTR  char*

#define ERROR_SUCCESS  0
#define ERROR_FILE_NOT_FOUND  2
#define ERROR_INVALID_FUNCTION  3

#define _fsopen(name,mode,shflag)  fopen(name,mode)

#define _fileno  fileno

#define _chsize  ftruncate  

#define MAX_PATH  260

typedef void *HMODULE;
typedef long __int64;
typedef void *HINSTANCE;

#endif  /* LINUX_WIN_H */
