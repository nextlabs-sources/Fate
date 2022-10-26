/***********************************************************************
 *
 * Compliant Enterprise Logging - Test Application
 *
 **********************************************************************/

#ifdef WIN32
  #include <windows.h>
  #include <tchar.h>
#endif

#include <cstdio>
#include <cstring>

#include "celog.h"

/* Include policies to test */
#include "celog_policy_file.hpp"
#include "celog_policy_stderr.hpp"
#include "celog_policy_cbuf.hpp"
#include "celog_policy_windbg.hpp"
#include "celog_policy_cstyle.hpp"

/* C-style log method for use with CStyle policy.
 */
int myclog( const wchar_t* msg )
{
  return fwprintf(stdout,msg);
}

int main(void)
{
  fprintf(stdout, "CELogTest\n");

  CELog mylog;

  mylog.Enable();

  /* Set an instance of each default policy type.
   */
  mylog.SetPolicy( new CELogPolicy_File("celog.txt") );
  mylog.SetPolicy( new CELogPolicy_Stderr() );
#if defined(_WIN32) || defined(_WIN64)
  mylog.SetPolicy( new CELogPolicy_CBuf() );
  mylog.SetPolicy( new CELogPolicy_WinDbg() );
#endif
  mylog.SetPolicy( new CELogPolicy_CStyle(myclog) );

  celog_Enable();

  /* Default to CELOG_INFO */
  mylog.SetLevel(CELOG_INFO);
  CELogS::Instance()->SetLevel(CELOG_INFO);

  /***********************************************************************************************
   * Basic funcationality test for various interfaces.
   **********************************************************************************************/
  for( int i = 0 ; i < 32 ; i++ )
  {
    fprintf(stdout, "Level %d\n", mylog.Level()); // confirm current level.

    // Test explicit file information interface
    mylog.Log(CELOG_INFO,__FILE__,__LINE__, "mylog.Log()      %d %s == %ls\n", i, "bar", L"bar");
    mylog.Log(CELOG_INFO,__FILE__,__LINE__,L"mylog.Log() wide %d %s\n", i, L"bar");

    // Test standard ::Log() interface
    mylog.Log(CELOG_ERR,"mylog.Log(): level %d\n", i);
    mylog.Log(CELOG_WARNING, "mylog.Log(): level default (current %d)\n", mylog.Level());

    // Test macro interface for singleton
    TRACE(CELOG_INFO,"TRACE()      %d %s\n", i, "foo");
    TRACE(CELOG_INFO,"TRACE() wide %d %s\n", i, L"foo");

    CELogS::Instance()->Log(1,__FILE__,__LINE__," CELogS::Instance()->Log()\n");
  }

  /************************************************************************************************
   * Attempt to force stack overrun or out of bounds memory access.
   *
   * Create a large buffer and print it.  Sizes range from 0 - 32KB in 1KB segments.
   * CELog interface should accept any length but truncate based on maximum implemention size
   * of log buffer.
   ***********************************************************************************************/
  wchar_t* big_buf = new wchar_t[ 32 * 1024 ];
  for( int i = 0 ; i < 32 ; i++ )
  {
    memset(big_buf,0x00,32*1024);
    for( int j = 0 ; j < i * 1024 ; j++ ) // [0,32KB] in 1KB sizes
    {
      big_buf[j] = L'x';
    }
    mylog.Log(CELOG_ERR,      "mylog.Log(): big_buf = %ws\n", big_buf);
    mylog.Log(CELOG_WARNING, L"mylog.Log(): big_buf = %s\n", big_buf);
  }

  return 0;
}/* main */
