################################################################################
#
# CELog README.TXT
#
# Basic documentation for CELog.  For detailed documentation see celog.h
#
################################################################################

CELog is a small portable logging library.  It supports both C and C++ linkage
and output to a user-defined policy.  The creation of policies is described
below.  This document outlines CELog and provides a sample use case for C and C++
linkage and the writing of custom policies.

Background
----------
CELog is small and portable.  It is intended to provide a simple logging
framework that can be easily extended.  For example, a policy could be written
to write to a network port of a log service.  The framework would not have to
change, only a new policy for network logging needs to be created.

Major features:
  * Support of ANSI and wide strings.
  * Portable.
  * Small.
  * Easily extended via policy interface.
  * Separated from all dependencies (i.e. cebrain, etc.).

Log Policies
------------
CELog accepts policies which describe how the logging library will direct log
output.  For example, a policy can be written to direct logging output to
stderr, file, memory buffer, or any user-defined space.

A policy is created by deriving from the base class CELogPolicy which defines
the Log() method.  The log method is called from the logging library with the
trace message to be written.  There is no limit to the number of policies that
may be associated with a CELog instance.  For each trace message each policy
will be called (no specified order).

Base Policies
-------------
The followin policies are provided as defaults.

include/celog_policy_file.hpp		Log to file with size limit.
include/celog_policy_stderr.hpp		Log to stderr.
intlude/celog_policy_cbuf.hpp		Log to circular bounded buffer.
intlude/celog_policy_windbg.hpp		Log to windows debugger using
                                        OutputDebugString().
intlude/celog_policy_cstyle.hpp		C-style adapter.  Adapter for c-linkage
                                        function that may be used with C++ policy.

CELogPolicy is an abstract base class which is used to construct log policies.
The above are examples of standard (base) policies provided, and the below
example illustrates how a new policy can be made.

Use Example
-----------
The following example shows how a new policy for encrypted logging is created
and the derived from for writting encrypted traces to a network port.

Classes:

           CELogPolicy         (abstract base)          Defines Log() interface
               |
        MyCryptoLogPolicy      (concrete crypto layer)  Implements Log()

Prototypes:

// MyCryptoLogPolicy to encrypt log data when writing to file
class MyCryptoLogPolicy : public CELogPolicy
{
  public:
    virtual int Log( const wchar_t* msg )
    {
      return CryptoLog(msg);
    }
};// MyCryptoLogPolicy

int policy_WriteFile( const wchar_t* msg )
{
  return fwprintf(fp,msg);
}/* policy_WriteFile( */

int policy_WriteStderr( const wchar_t* msg )
{
  return fwprintf(stderr,msg);
}/* policy_WriteStderr */

int policy_MyCryptoLogPolicy( const wchar_t* msg )
{
  wchar_t out_msg[MAX_MSG];
  return encrypt_write(msg,out_msg);
}/* policy_MyCryptoLogPolicy */

int main(void)
{
  // The following three sections are basically equivalent.

  ///////////////////////////////////////////////////////////
  // (1) Explicit use of CELog instance
  ///////////////////////////////////////////////////////////
  CELog mylog;

  mylog.SetPolicy( new CELogPolicy_File("log.txt") );
  mylog.SetPolicy( new CELogPolicy_Stderr() );
  mylog.SetPolicy( new MyCryptoLogPolicy() );
  mylog.Log(CELOG_INFO,"Log started\n");

  ///////////////////////////////////////////////////////////
  // (2) Use of TRACE macros which rely on CELog singleton
  ///////////////////////////////////////////////////////////
  CELogS::Instance()->SetPolicy( new CELogPolicy_File("log.txt") );
  CELogS::Instance()->SetPolicy( new CELogPolicy_Stderr() );
  CELogS::Instance()->SetPolicy( new MyCryptoLogPolicy() );
  TRACE(CELOG_INFO,"Log started\n");

  ///////////////////////////////////////////////////////////
  // (3) Use of C-linkage and TRACE macros
  ///////////////////////////////////////////////////////////
  celog_SetPolicy( policy_WriteFile );
  celog_SetPolicy( policy_WriteStderr );
  celog_SetPolicy( policy_MyCustomPolicy );
  TRACE(CELOG_INFO,"Log started\n");

  return 0;
}// main
