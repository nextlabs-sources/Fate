//
// All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle Inc, 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 5/31/2006
// Note   : A collection of utilities for Linux
//          I guess it's an overkill to do an object class here
//
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include "brain.h"

// Little function converting millisecond to timespec
void BJ_millisec2timespec (int ms, timespec *ts)
{
  if (!ts) return;
  ts->tv_sec  = ms / 1000;
  ts->tv_nsec = (ms % 1000) * 1000000;
  return;
}

void BJ_trimLeadingWhitespace (string & s)
{
  char const * whitespace = WHITESPACES;

  string::size_type lpos = s.find_first_not_of (whitespace);

  if (lpos == string::npos) { // only white space
    s.erase();
  } else {
    s.erase(0,lpos);
  }

}

void BJ_trimTrailingWhitespace (string & s)
{
  char const * whitespace = WHITESPACES;

  string::size_type rpos = s.find_last_not_of (whitespace);
  
  if (rpos == string::npos) {   // only white space
    s.erase ();
  } else {
    s.erase (rpos + 1);
  }

}

/* Return 1 if it's a valid IPv4 xx.xx.xx.xx IP address format. 0 otherwise */
int BJ_isValidIPAddress (const char *ip)
{
  int i;
  
  if (ip == NULL) return 0;

  for (i = 0; i < 4 && *ip != '\0'; i++) {
    while (*ip != '\0' && isdigit (*ip))
      ip++;

    if (*ip != '\0') {
      if (*ip == '.' && i < 3)
        ip++;
      else
        break;
    }
  }

  if (i == 4 && *ip == '\0')
    return 1;

  return 0;
}
// ---------------------------------------------------------------------------
// Getting the user ID of the current process
//
// Input : id_buf: user supply storage for the id
//         size  : size of id_buf, in nlchar count
//
// ---------------------------------------------------------------------------

BJ_error_t NL_getUserId (nlchar * id_buf, size_t size)
{

  if (!id_buf)    return BJ_ERROR;
  if (size <= 11) return BJ_ERROR; // 11 char to hold a 32bit value

  memset (id_buf, 0x0, size * sizeof(nlchar));

  uid_t uid;
  uid = getuid ();

  _snprintf_s (id_buf, size, _TRUNCATE, "%d", uid);
  return BJ_OK;
}

//Cause the current process to be suspended for 'milliseconds' milliseconds
void NL_sleep(unsigned long milliseconds)
{
  struct timeval delay;

  //convert to second
  delay.tv_sec = milliseconds / 1000;

  //convert to microsecond
  delay.tv_usec = (milliseconds % 1000)*1000;

  select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &delay );
}

/* ------------------------------------------------------------------------- */
/* NL_GetCurrentTimeInMillisec                                               */
/* On Linux, this function will return the current time in milliseconds.     */
/* On Window, this function will return the number of milliseconds that have */
/* elapsed since the system was started, up to 49.7 days.                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */
double NL_GetCurrentTimeInMillisec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  double result=t.tv_sec*1000.0;
  result+=((double)t.tv_usec/1000.0);
  return result;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFingerprint                                                         */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the application fingerprint.         */
/* ------------------------------------------------------------------------- */
bool NL_GetFingerprint(const nlchar *appNameWithPath, 
		       nlchar *fingerPrintBuf,
		       int bufSize) 
{
  return false;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFingerPrint                                                         */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the application finger print.        */
/* ------------------------------------------------------------------------- */
bool NL_GetFQDN(nlchar *fqdnBuf, int *bufSize)
{
  int ret = gethostname(fqdnBuf, *bufSize);
  if(ret != 0) {
    *fqdnBuf = 0;
    *bufSize = 0;
    return false;
  }

  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetMyDesktopFolder                                                     */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the name of my desktop folder        */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDesktopFolder(nlchar *fBuf, int *bufSize)
{
  if(getcwd(fBuf, *bufSize) == NULL ) {
    *fBuf = 0;
    *bufSize = 0;
    TRACE(0, _T("GetMyDesktopFolder failed due to: errno=%d\n"), errno);
    return false;
  }
  *bufSize=nlstrlen(fBuf);
  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetMyDocumentsFolder                                                   */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the name of my documents folder      */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDocumentsFolder(nlchar *fBuf, int *bufSize)
{
  if(getcwd(fBuf, *bufSize)==NULL) {
    *fBuf = 0;
    *bufSize = 0;
    TRACE(0, _T("GetMyDocumentsFolder failed due to: errno=%d\n"), errno);
    return false;
  }
  *bufSize=nlstrlen(fBuf);
  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_IsRemovableMedia                                                       */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will check if the file is on removable media     */
/* ------------------------------------------------------------------------- */
bool NL_IsRemovableMedia(const nlchar *fileName)
{
  return false;
}
/* ------------------------------------------------------------------------- */
/* NL_LogEvent                                                               */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will log the event                               */
/* ------------------------------------------------------------------------- */
void NL_LogEvent(int type, int eventId, int paramSize, nlchar **ppStrArray)
{
}
/* ------------------------------------------------------------------------- */
/* NL_GetUserName                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the user name.                       */
/* ------------------------------------------------------------------------- */
bool NL_GetUserName(const nlchar *userSid, nlchar **userName)
{
  if(userSid==NULL)
    return false;

  struct passwd *p=getpwuid(atoi(userSid));
  if(p==NULL) {
    TRACE(0, _T("GetUserName failed due to: errno=%d\n"), errno);
    return false;
  }
  
  int nlen=nlstrlen(p->pw_name);
  if(nlen==0)
    return false;

  *userName=new nlchar[nlen+1];
  nlsprintf(*userName, "%s", p->pw_name);
  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileModifiedTime                                                    */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last modified time of a file.       */
/* ------------------------------------------------------------------------- */
NLint64 NL_GetFileModifiedTime(const nlchar *fileName, void *processToken)
{
  NLint64 ret = -1;
  struct stat s;

  if(fileName==NULL)
    return ret;
  
  if(stat(fileName, &s) != 0) {
    TRACE(0, _T("GetFileModifiedTime (file %s) failed due to: errno=%d\n"), 
	  fileName, errno);
    return ret;
  }

  //Convert to millisecond
  ret=s.st_mtime*1000;

  return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileAccessTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last access time of a file.         */
/* ------------------------------------------------------------------------- */
NLint64 NL_GetFileAccessTime(const nlchar *fileName, void *processToken)
{
  NLint64 ret = -1;
  struct stat s;

  if(fileName==NULL)
    return ret;
  
  if(stat(fileName, &s) != 0) {
    TRACE(0, _T("GetFileAccessTime (file %s) failed due to: errno=%d\n"), 
	  fileName, errno);
    return ret;
  }

  //Convert to millisecond
  ret=s.st_atime*1000;

  return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileCreateTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the create time of a file.              */
/* ------------------------------------------------------------------------- */
NLint64 NL_GetFileCreateTime(const nlchar *fileName, void *processToken)
{
  NLint64 ret = -1;
  return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_OpenProcessToken                                                       */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will opens the access token associated with a    */
/*            process.                                                       */
/* ------------------------------------------------------------------------- */
void *NL_OpenProcessToken()
{
  return NULL;
}
/* ------------------------------------------------------------------------- */
/* NL_GetOwnerSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the file owner's sid.                   */
/* ------------------------------------------------------------------------- */
bool NL_GetOwnerSID(const nlchar *fileName, int agentType, 
		    void *processTokenHandle, nlchar *sidBuf, int bufSize)
{
  struct stat s;

  if(fileName==NULL)
    return false;
  
  if(stat(fileName, &s) != 0) {
    TRACE(0, _T("GetOwnerSID (file %s) failed due to: errno=%d\n"), 
	  fileName, errno);
    return false;
  }

  nlchar uidBuf[100];
  nlsprintf(uidBuf, "%d", s.st_uid);
  if(nlstrlen(uidBuf) >= bufSize) {
    TRACE(0, 
     _T("GetOwnerSID failed due to: small input buffer (real=%d, needed=%d\n"),
	  bufSize, nlstrlen(uidBuf));
    return false;
  }
  nlsprintf(sidBuf, "%d", s.st_uid);
  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetGroupSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the group sid.                          */
/* ------------------------------------------------------------------------- */
bool NL_GetGroupSID(const nlchar *fileName, nlchar *sidBuf, int bufSize)
{
  struct stat s;

  if(fileName==NULL)
    return false;
  
  if(stat(fileName, &s) != 0) {
    TRACE(0, _T("GetGroupSID (file %s) failed due to: errno=%d\n"), 
	  fileName, errno);
    return false;
  }

  nlchar uidBuf[100];
  nlsprintf(uidBuf, "%d", s.st_gid);
  if(nlstrlen(uidBuf) >= bufSize) {
    TRACE(0, 
     _T("GetGroupSID failed due to: small input buffer (real=%d, needed=%d\n"),
	  bufSize, nlstrlen(uidBuf));
    return false;
  }
  nlsprintf(sidBuf, "%d", s.st_gid);
  return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetUserName_Free                                                       */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will free the memory allocated for the user name.*/
/* ------------------------------------------------------------------------- */
void NL_GetUserName_Free(nlchar *userName)
{
  if(userName)
    delete [] userName;
  return;
}
/* ------------------------------------------------------------------------- */
/* NL_GetRdpSessionAddress                                                   */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the RDP (remot desktop) session      */
/* address.                                                                  */
/* ------------------------------------------------------------------------- */
long NL_GetRdpSessionAddress(int pid)
{
  return 0;
}
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
bool NL_GetFilePhysicalPath(const nlchar *input, nlchar *output, 
			    size_t outputBufLen)
{
  return false;
}
