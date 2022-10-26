/**********************************************************************
 *
 * celog_policy_cbuf.hpp
 *
 * Default circular buffer logging policy.
 *
 * Allow logging to a bounded memory region to avoid complications of
 * directed messages to the file system.  The log message array can
 * be viewed via a debugger.
 *
 *********************************************************************/

#ifndef __CELOG_POLICY_CBUF_HPP__
#define __CELOG_POLICY_CBUF_HPP__

#include "celog.h"

#if defined (_WIN32) || defined(_WIN64)

#include <windows.h>

/** CELogPolicy_CBuf
 *
 *  \brief Policy for logging to a circular buffer.
 */
class CELogPolicy_CBuf : public CELogPolicy
{
  public:
    static const int MAX_MSG_LEN = 512;  /* Maximum message length */
    static const int MAX_MSGS = 2048;    /* Maximum number of messages */

    CELogPolicy_CBuf(void) :
      cirBufCurIndex(0),
      cirBufWrapCount(0)
    {
      memset(cirBuf,0x00,sizeof(cirBuf));
      InitializeCriticalSection(&mutex);
    }

    virtual ~CELogPolicy_CBuf(void)
    {
      memset(cirBuf,0x00,sizeof(cirBuf));
      DeleteCriticalSection(&mutex);
    }

    /** Log
     *
     *  \brief Log to the circular buffer.
     */
    virtual int Log( const wchar_t* string )
    {
      EnterCriticalSection(&mutex);

      wcsncpy_s(cirBuf[cirBufCurIndex],string,_TRUNCATE);

      cirBuf[cirBufCurIndex][MAX_MSG_LEN - 1] = L'\0';

      /* adjust index into bounded buffer */
      cirBufCurIndex = (cirBufCurIndex + 1) % MAX_MSGS;

      if (cirBufCurIndex == 0)
      {
        cirBufWrapCount++;
      }

      LeaveCriticalSection(&mutex);
      return MAX_MSG_LEN;
    }/* Log */

    virtual int Log( const char* string )
    {
      EnterCriticalSection(&mutex);

      strncpy_s(cirBuf[cirBufCurIndex], _countof(cirBuf[cirBufCurIndex]), string,_TRUNCATE);

      /* adjust index into bounded buffer */
      cirBufCurIndex = (cirBufCurIndex + 1) % MAX_MSGS;

      if (cirBufCurIndex == 0)
      {
        cirBufWrapCount++;
      }

      LeaveCriticalSection(&mutex);
      return MAX_MSG_LEN;
    }

  private:
    CRITICAL_SECTION mutex;               /** Mutex for internals */
    WCHAR cirBuf[MAX_MSGS][MAX_MSG_LEN];  /** Bounded buffer */
    int cirBufCurIndex;                   /** Current buffer index */
    int cirBufWrapCount;                  /** Buffer wrap count */
};/* CELogPolicy_CBuf */

#endif /* _WIN32 || _WIN64 */

#endif /* __CELOG_POLICY_CBUF_HPP__ */
