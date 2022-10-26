//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle Inc, 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 5/23/2006
// Note   : This is a OS abstraction layer so that we don't have to
//          deal with which OS we are running on.
//
// $Id$
// 
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->


#ifndef BRAIN_H
#define BRAIN_H

typedef enum {
  BJ_ERROR = -1,
  BJ_OK     = 0
} BJ_error_t;

typedef enum {
  BJ_WAIT_FAILED  = -1,
  BJ_WAIT_SUCCESS = 0,
  BJ_WAIT_TIMEOUT = 1
} BJ_waitResult_t;

#include <stdarg.h>

#include "nlstrings.h"

#if defined (DEBUG) || defined (_DEBUG)

#ifndef NLMODULE
#define NLMODULE _nl_module
#endif

/* Nesting Marco expansion */
#define TVAR(s)       s##tracelevel
#define TRACEVAR(s)   TVAR(s)
#define TCLS(s)       s##nltrace
#define TRACECLASS(s) TCLS(s)

#ifndef NLTRACELEVEL
#define NLTRACELEVEL 1
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)
typedef __int64 NLint64;
#elif defined(Linux) || defined(Darwin)
typedef long long NLint64;
#else
#error "Unknown platform"
#endif

// --------------------------------------------------------------------------
//  Common Utility Functions available for all platforms 
// --------------------------------------------------------------------------

/* ------------------------------------------------------------------------- */
/* NL_getUserId                                                              */
/* Getting the user ID of the CURRENT process                                */
/*                                                                           */
/* Input : id_buf: user supply storage for the id                            */
/*         size  : size of id_buf, in NLCHAR count                           */
/* ------------------------------------------------------------------------- */

BJ_error_t NL_getUserId (_Out_z_cap_(size) nlchar * id_buf, _In_ size_t size);

#if defined(_WIN32) || defined(_WIN64)
#define NL_UNKNOWN_USER_ID "S-1-0-0"
#elif defined(Linux) || defined(Darwin)
#define NL_UNKNOWN_USER_ID "-1"
#else
#error "Unknown platform"
#endif
/* ------------------------------------------------------------------------- */
/* NL_sleep                                                                  */
/* Cause the current process to be suspended for 'milliseconds' milliseconds */
/*                                                                           */
/* Input : milliseconds: the specified sleep time in milliseconds.           */
/* ------------------------------------------------------------------------- */
void NL_sleep(_In_ unsigned long milliseconds);

/* ------------------------------------------------------------------------- */
/* NL_GetCurrentTimeInMillisec                                               */
/* On Linux, this function will return the current time in milliseconds.     */
/* On Window, this function will return the number of milliseconds that have */
/* elapsed since the system was started, up to 49.7 days.                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */
_Check_return_ double NL_GetCurrentTimeInMillisec();

/* ------------------------------------------------------------------------- */
/* NL_GetFingerPrint                                                         */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the application finger print.        */
/* ------------------------------------------------------------------------- */
bool NL_GetFingerprint(_In_ const nlchar *appNameWithPath, 
		       _Out_z_cap_(bufSize) nlchar *fingerprintBuf,
		       _In_ int bufSize); 
/* ------------------------------------------------------------------------- */
/* NL_GetFQDN                                                                */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the fully qualified DNS name that    */
/* uniquely identifies the local computer. application finger print.         */
/* Parameter:                                                                */
/* fqdnBuf (OUT): the buffer for the FQDN string                             */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetFQDN(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fqdnBuf, _Inout_ int *bufSize);

/* ------------------------------------------------------------------------- */
/* NL_GetMyDesktopFolder                                                     */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the name of my desktop folder        */
/* Parameter:                                                                */
/* fBuf (OUT): the buffer for the FQDN string                                */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDesktopFolder(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fBuf, _Inout_ int *bufSize);

/* ------------------------------------------------------------------------- */
/* NL_GetMyDocumentsFolder                                                   */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the name of my documents folder      */
/* Parameter:                                                                */
/* fBuf (OUT): the buffer for the FQDN string                                */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDocumentsFolder(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fBuf, _Inout_ int *bufSize);

/* ------------------------------------------------------------------------- */
/* NL_IsRemovableMedia                                                       */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will check if the file is on removable media     */
/* ------------------------------------------------------------------------- */
_Check_return_ bool NL_IsRemovableMedia(_In_ const nlchar *fileName);

/* ------------------------------------------------------------------------- */
/* NL_LogEvent                                                               */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will log the event                               */
/* ------------------------------------------------------------------------- */
void NL_LogEvent(_In_ int type, _In_ int eventId, _In_ int paramSize, _In_opt_count_(paramSize)nlchar **ppStrArray);

/* ------------------------------------------------------------------------- */
/* NL_GetUserName                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the user name.                       */
/* ------------------------------------------------------------------------- */
bool NL_GetUserName(_In_ const nlchar *userSid, _Out_ nlchar **userName);
/* ------------------------------------------------------------------------- */
/* NL_GetUserName_Free                                                       */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will free the memory allocated for the user name.*/
/* ------------------------------------------------------------------------- */
void NL_GetUserName_Free(_In_ nlchar *userName);

/* ------------------------------------------------------------------------- */
/* NL_OpenProcessToken                                                       */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will opens the access token associated with a    */
/*            process.                                                       */
/* ------------------------------------------------------------------------- */
void *NL_OpenProcessToken();

/* ------------------------------------------------------------------------- */
/* NL_GetFileModifiedTime                                                    */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last modified time of a file.       */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileModifiedTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken);

/* ------------------------------------------------------------------------- */
/* NL_GetFileAccessTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last access time of a file.         */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileAccessTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken);

/* ------------------------------------------------------------------------- */
/* NL_GetFileCreateTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the create time of a file.              */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileCreateTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken);

/* ------------------------------------------------------------------------- */
/* NL_GetOwnerSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the file owner's sid.                   */
/* ------------------------------------------------------------------------- */
bool NL_GetOwnerSID(_In_ const nlchar *fileName, _In_ int agentType, 
		    _In_ void *processTokenHandle, _Out_z_cap_(bufSize) nlchar *sidBuf, _In_ int bufSize);
/* ------------------------------------------------------------------------- */
/* NL_GetGroupSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the group sid.                          */
/* ------------------------------------------------------------------------- */
bool NL_GetGroupSID(_In_ const nlchar *fileName, _Out_z_cap_(bufSize) nlchar *sidBuf, _In_ int bufSize);

/* ------------------------------------------------------------------------- */
/* NL_GetRdpSessionAddress                                                   */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the RDP (remot desktop) session      */
/* address associate with the given pid (or the current process if pid = 0)  */
/* ------------------------------------------------------------------------- */
long NL_GetRdpSessionAddress(_In_ int pid);

/* ------------------------------------------------------------------------- */
/* NL_GetFilePhysicalPath                                                    */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the physical path of a file. For        */
/* example,  if file "T:\heidiz\SDK\eval.cpp" is actually located at         */
/* \\bluejungle.com\share\public\heidiz\SDK\eval.cpp. Then, this fucntion    */
/* will return the latter as ouput.                                          */
/* Parameter:                                                                */
/* input (IN): the logical path and name of the file                         */
/* output (OUT): the returned physical path                                  */
/* outputBufLen (IN): the length of output buffer                            */
/* return value: true, if the physical path is different from the logical one*/
/*               false, if the physical path is same as the logical one.     */
/* ------------------------------------------------------------------------- */
bool NL_GetFilePhysicalPath(_In_ const nlchar *input, _Out_z_cap_(outputBufLen) nlchar *output, 
			    _In_ size_t outputBufLen);

/* ---------------------------------------------------- */
/* Linux Specific defines                               */
/* ---------------------------------------------------- */

#if defined (Linux) || defined (Darwin)

/* OS specific include section */
#include <time.h>
#include <string>
#include  <netinet/in.h>

using namespace std;

/* Utility functions signuatures */
void BJ_millisec2timespec      (int ms, timespec *ts);
void BJ_trimLeadingWhitespace  (string & s);
void BJ_trimTrailingWhitespace (string & s);
int  BJ_isValidIPAddress       (const char * ip);


/* OS specific defines */

/* Wide char, denote L"...", but we don't want that for different string */
/* literal.  Window has _T to takes care of that, so on Linux we use the */
/* same idea.                                                            */ 

/* For Linux,  we use UTF-8 to encode unicode, which is variable byte wide char */
#define _T(x)           x
#define WHITESPACES     "\t \r\n"

#ifdef DEBUG

#define TRACE TRACECLASS(NLMODULE) (__FILE__,__LINE__)

#define ASSERT(e)   assert(e)

#ifndef TRACEFUNC
#define TRACEFUNC

// In the future, we can use this for the module level trace, for now
// We will use the NLTRACELEVEL for the file scope trace
// static int TRACEVAR (NLMODULE) = 1;

// Using a class so that we can pass the correct FILE/LINE

class TRACECLASS(NLMODULE) {
 private :
  char * file;
  int    line;
 public:
  TRACECLASS(NLMODULE) (char * f, int l): file(f), line(l) {}
  
  inline void operator() (int level, nlchar *fmt, ...) {
    if (level <= NLTRACELEVEL) {
      printf ("%s [%d]:", file, line);
      va_list args;
      va_start (args, fmt);
      vprintf (fmt, args);
      va_end (args);
    }
  }
};

#endif  /* TRACEFUNC */

#else
#define TRACE nltrace

#ifndef TRACEFUNC
#define TRACEFUNC

static inline void nltrace (int level, nlchar * fmt, ...)
{
  // Mapping to something else, nothing for release mode now
}

#endif
#define ASSERT(e)
#endif

#else

/* ---------------------------------------------------- */
/* Win32 Specific defines                               */
/* ---------------------------------------------------- */

/* OS specific defines */

#include <winsock2.h>

/* Visual Studio 2003 doesn't support (...), so here is the mess to deal */
/* with the variadic macro                                               */

#ifdef _DEBUG

#define TRACE TRACECLASS(NLMODULE) (_T(__FILE__),__LINE__)


#ifndef TRACEFUNC
#define TRACEFUNC

#define TRACE_BUF_LEN 1024
class TRACECLASS(NLMODULE) {
 private :
  nlchar * file;
  int      line;
 public:
  TRACECLASS(NLMODULE) (nlchar * f, int l): file(f), line(l) {}
  void operator() (int level, nlchar *fmt, ...) {

    if (level <= NLTRACELEVEL) {
      TCHAR buf[TRACE_BUF_LEN * 2];
      TCHAR* pbuf = buf;

      /* Prefix {file,line} and offset 'pbuf' for next write */
      pbuf += _stprintf_s (pbuf, sizeof(buf) / sizeof(TCHAR), _T("%s [%d]:"), file, line);

      va_list args;
      va_start (args, fmt);

      /* Place trace into 'buf' at the pbuf offset and calculate size available
	 for writing: sizeof(buf)/sizeof(TCHAR) less elements already written.
       */
      _vstprintf_s (pbuf, (sizeof(buf) / sizeof(TCHAR)) - (pbuf - buf), fmt, args);

      va_end (args);

      _tprintf (buf);
    }
  }
};


#endif  /* TRACEFUNC */


#define timedTRACE(s)
#else

#ifdef TRACE
#undef TRACE
#endif
#define TRACE nltrace
#define timedTRACE

#ifndef TRACEFUNC
#define TRACEFUNC

static inline void nltrace (int level, nlchar * fmt, ...)
{
  // Mapping to something else, nothing for release mode now
}
#endif

#endif

#endif


#endif  /* BRAIN_H */
