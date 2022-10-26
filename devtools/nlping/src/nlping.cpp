/****************************************************************************************
 *
 * NextLabs Policy Ping
 *
 * Measure roundtrip policy decision time.
 *
 ***************************************************************************************/

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string>
#include <algorithm>

#pragma warning( disable : 4510 )
#pragma warning( disable : 4512 )
#pragma warning( disable : 4610 )

#include <iostream>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include "eframework/platform/cesdk.hpp"
#include "eframework/timer/timer_high_resolution.hpp"

#include "nlping_sdk_support.hpp"

using namespace boost::accumulators;

int wmain( int argc , wchar_t** argv )
{
  std::size_t  op_count = 5;      // default ping count
  bool op_verbose       = false;  // verbose output?
  std::size_t op_delay  = 500;    // 500ms
  std::wstring op_file(L"c:\\nlping");

  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

    if( wcsncmp(argv[i],L"--file=",wcslen(L"--file=")) == 0 )
    {
      op_file.assign(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--count=",wcslen(L"--count=")) == 0 )
    {
      if( option != NULL )
      {
	op_count = _wtoi(option);
      }
      continue;
    }
    if( wcsncmp(argv[i],L"--delay=",wcslen(L"--delay=")) == 0 )
    {
      if( option != NULL )
      {
	op_delay = _wtoi(option);
      }
      continue;
    }
    if( _wcsicmp(argv[i],L"--time") == 0 )
    {
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
  }/* for */

  /* There must be at least one option */
  if( argc == 2 && (_wcsicmp(argv[1],L"--help") == 0 ||
		    _wcsicmp(argv[1],L"-h") == 0 ||
		    _wcsicmp(argv[1],L"/?") == 0) )
  {
    fprintf(stdout, "NextLabs Policy Ping (Built %s %s)\n", __DATE__, __TIME__);
    fprintf(stdout, "usage: nlping [option] ...\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "   --file=[file path]               File path to access.  Default path is\n");
    fprintf(stdout, "                                    c:\\nlping.\n");
    fprintf(stdout, "   --count=[iterations]             Number of iterations to perform.\n");
    fprintf(stdout, "                                    Default is 5.  0 means infinite.\n");
    fprintf(stdout, "   --delay=[ms]                     Delay between operations in milliseconds.\n");
    fprintf(stdout, "   --verbose                        Verbose output.\n");
    fprintf(stdout, "   --help, -h, /?                   This screen.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "A sequence of file accesses is generated where each file has the sequence\n");
    fprintf(stdout, "number added as a suffix.  For example, the default file path c:\\nlping will\n");
    fprintf(stdout, "result in c:\\nlping.1, c:\\nlping.2, ...\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  nlping --count=5000 --file=\\\\ts01\\transfer\\dmustaine\\nlping\n");
    fprintf(stdout, "  nlping --count=0 --verbose --delay=100\n");
    return 0;
  }

  accumulator_set< double , features< tag::min, tag::max > > acc;
  accumulator_set< double , stats<tag::mean,tag::variance > > acc_stats;

  double min_time = INT_MAX, max_time = 0;

  nextlabs::cesdk_connection cesdk_conn;
  nextlabs::cesdk_loader cesdk;

  bool rv;
  rv = cesdk.load(L"c:\\cesdk");
  if( rv == false )
  {
    fprintf(stderr, "cesdk.load failed\n");
    return 1;
  }
  cesdk_conn.set_sdk(&cesdk);
  rv = cesdk_conn.connect();
  if (rv != true)
  {
    fprintf(stderr, "cesdk_conn.connect failed\n");
    return 1;
  }

  std::size_t seq = 0;
  for( ; ; )
  {
    wchar_t temp[MAX_PATH] = {0};
    double sdk_time = 0;

    swprintf(temp,_countof(temp),L"%s.%d",op_file.c_str(),seq);

    nextlabs::cesdk_query q(cesdk);
    rv = q.set_action(L"OPEN");
    if (rv != true)
    {
        fprintf(stderr, "set_action failed\n");
        return 1;
    }
    rv = q.set_source(temp,L"fso",NULL);
    if (rv != true)
    {
        fprintf(stderr, "set_source failed\n");
        return 1;
    }
    nextlabs::high_resolution_timer ht;
    rv = q.query(cesdk_conn.get_connection_handle());
    if (rv != true)
    {
        fprintf(stderr, "query failed\n");
        return 1;
    }

    double curr_time = ht.diff();

    min_time = curr_time < min_time ? curr_time : min_time;  // running stat for min
    max_time = curr_time > max_time ? curr_time : max_time;  // running stat for max

    acc(curr_time);
    acc_stats(curr_time);

    if( op_verbose == true )
    {
      struct _timeb timebuffer;
      char timeline[32] = {0};
      if( _ftime64_s(&timebuffer) == 0 && 
	  ctime_s(timeline,_countof(timeline),&timebuffer.time) == 0 )
      {
	timeline[strlen(timeline)-1] = (char)NULL;  /* remove newline */
	swprintf(temp,_countof(temp), L"%.19hs.%03hu %hs: seq=%d ",timeline,timebuffer.millitm,&timeline[20],seq);
	wprintf(temp);
      }
    }
    fprintf(stdout, "total_time %fms,sdk_time %fms (dt %f)\n", curr_time, sdk_time, curr_time-sdk_time);

    seq++;
    if( op_count > 0 && seq >= op_count )
    {
      break;
    }

    Sleep(static_cast<DWORD>(op_delay));
  }/* for */

  double u   = mean(acc_stats);
  double var = boost::accumulators::variance(acc_stats);
  double k   = pow(var,0.5) / pow(static_cast<double>(seq),0.5);

  double lower_bound = (u - 1.96 * k);
  double upper_bound = (u + 1.96 * k);

  // Summary stats for current run
  std::cout << "statistics: elapsed time = " << boost::accumulators::sum(acc_stats) << "ms , count = " << seq << "\n";
  std::cout << "            min = " << min_time << " , max = " << max_time << "\n";
  std::cout << "            u   = " << u << " , var = " << var << "\n";
  std::cout << "            [" << lower_bound << "," << upper_bound << "] (p = 0.95)\n";

  return 0;
}/* main */
