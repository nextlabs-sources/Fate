/**********************************************************************
 *
 * celog_policy_windbg.hpp
 *
 *********************************************************************/

#ifndef __CELOG_POLICY_WINDBG_HPP__
#define __CELOG_POLICY_WINDBG_HPP__

#include "celog.h"

#if defined (_WIN32) || defined(_WIN64)

#include <windows.h>

/** CELogPolicy_WinDbg
 *
 *  \brief Policy for logging to windows debugger.
 */
class CELogPolicy_WinDbg : public CELogPolicy
{
  public:
    /** Log
     *
     *  \brief Log to the windows debugger.
     */
    virtual int Log( const wchar_t* string )
    {
      OutputDebugStringW(string);
      return 0;
    }/* Log */

    virtual int Log( const char* string )
    {
      OutputDebugStringA(string);
      return 0;
    }

};/* CELogPolicy_WinDbg */

#endif /* _WIN32 || _WIN64 */

#endif /* __CELOG_POLICY_WINDBG_HPP__ */
