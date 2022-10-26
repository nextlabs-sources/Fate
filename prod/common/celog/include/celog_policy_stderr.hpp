/**********************************************************************
 *
 * celog_policy_file.hpp
 *
 * Default file logging policy.
 *
 *********************************************************************/

#ifndef __CELOG_POLICY_STDERR_HPP__
#define __CELOG_POLICY_STDERR_HPP__

#include "celog.h"

/** CELogPolicy_Stderr
 *
 *  \brief Policy for logging to stderr.
 */
class CELogPolicy_Stderr : public CELogPolicy
{
  public:
    /** Log
     *
     *  \brief Log to stderr.
     */
    int Log( const wchar_t* string )
    {
      return fwprintf(stderr,L"%s",string);
    }/* Log */

    int Log( const char* string )
    {
      return fprintf(stderr,"%s",string);
    }
};/* CELogPolicy_Stderr */

#endif /* __CELOG_POLICY_STDERR_HPP__ */
