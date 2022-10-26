/*************************************************************************
 *
 * Compliant Enterprise Logging
 *
 * Implementation file for logging interface described in celog.h
 *
 * Notes:
 *   There are many Log() interfaces but there is only one that writes
 *   directly to a policy.  The 'wchar*' method performs the write.  All
 *   others reduce to this call after generating intermediate log message.
 *
 ************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cassert>
#include <cstring>

#include <sys/types.h>
#include <sys/timeb.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #include <winsock2.h>
#else
  #include <sys/time.h>
  #include "dstypes.h"
#endif

#include "celog.h"
#include "celog_policy_cstyle.hpp"

/* log instance */
CELog* CELogS::log = NULL;

/****************************************************************************
 * CELog Singleton Implementation
 ***************************************************************************/
CELOG_EXPORT CELog* CELogS::Instance(void)
{
  if( log == NULL )
  {
    log = new CELog();
  }
  return log;
}/* CELogS::Instance */

/**************************************************************************
 * C linkage support
 *************************************************************************/
extern "C" CELOG_EXPORT int celog( int lvl , 
				   const char* file ,
				   int line ,
				   const char* fmt ,
				   ... )
{
  char sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
  char* p = sline;            /* cursor for log write */

  /* File may be NULL to indicate there is not file/line information */
  if( file != NULL )
  {
#if defined(_WIN32) || defined(_WIN64)
    p += _snprintf_s(p, sizeof(sline), _TRUNCATE, "%s [%d]: ", file, line);
#else
    p += _snprintf_s(p, sizeof(sline), _TRUNCATE, "%s [%d]: ", file, line);
#endif
  }

  va_list args;
  va_start(args,fmt);
  p += vsnprintf_s(p,sizeof(sline) - (p-sline),_TRUNCATE,fmt,args);
  va_end(args);

  return CELogS::Instance()->Log(lvl,sline);
}/* celog */

extern "C" CELOG_EXPORT int celogW( int lvl , 
				    const char* file ,
				    int line ,
				    const wchar_t* fmt ,
				    ... )
{
  wchar_t sline[CELOG_MAX_MESSAGE_SIZE_CHARS];
  wchar_t* p = sline; /* cursor for log write */

  /* File may be NULL to indicate there is not file/line information */
  if( file != NULL )
  {
    p += _snwprintf_s(p, sizeof(sline)/sizeof(WCHAR), _TRUNCATE, L"%s [%d]: ", file, line);
  }

  va_list args;
  va_start(args,fmt);
  p += vswprintf(p,sizeof(sline)/sizeof(WCHAR) - (p-sline),fmt,args);
  va_end(args);

  return CELogS::Instance()->Log(lvl,sline);
}/* celogW */

extern "C" CELOG_EXPORT int celog_SetLevel( int level )
{
  return CELogS::Instance()->SetLevel(level);
}/* celog_SetLevel */

extern "C" CELOG_EXPORT int celog_IsEnabled(void)
{
  return (int)CELogS::Instance()->IsEnabled();
}/* celog_IsEnabled */

extern "C" CELOG_EXPORT void celog_Enable(void)
{
  CELogS::Instance()->Enable();
}/* celog_Enable */

extern "C" CELOG_EXPORT void celog_Disable(void)
{
  CELogS::Instance()->Disable();
}/* celog_Disable */

extern "C" CELOG_EXPORT int celog_Level(void)
{
  return CELogS::Instance()->Level();
}/* celog_SetLevel */

extern "C" CELOG_EXPORT void celog_SetPolicy( int (*policy)(const wchar_t*) )
{
  CELogS::Instance()->SetPolicy( new CELogPolicy_CStyle(policy) );
}/* celog_SetPolicy */
