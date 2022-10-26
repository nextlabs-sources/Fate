
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <winioctl.h>
#include <iostream>

#pragma warning( disable : 4510 )
#pragma warning( disable : 4512 )
#pragma warning( disable : 4610 )
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include <eframework/timer/timer_high_resolution.hpp>
#include <eframework/policy/policy.hpp>

using namespace boost::accumulators;

bool op_verbose = false;
size_t op_count = 5;
bool op_monitor = false;

int main( int argc , char** argv )
{
  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    char* option = strstr(argv[i],"=");
    if( option != NULL )
    {
      option++;
    }

    if( strcmp(argv[i],"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
    else if( strcmp(argv[i],"--monitor") == 0 )
    {
      op_monitor = true;
      continue;
    }
    else if( strncmp(argv[i],"--count=",strlen("--count=")) == 0 )
    {
      op_count = atoi(option);
      continue;
    }
  }/* for */

  nextlabs::policy_connection pconn;
  if( pconn.connect() == false )
  {
    fprintf(stderr, "policy_test: cannot connect\n");
    return 1;
  }

  if( op_monitor )
  {
    bool result;
    result = nextlabs::policy_monitor_application::is_ignored();
    return 0;
  }

  accumulator_set< double , features< tag::min, tag::max > > acc;
  accumulator_set< double , stats<tag::mean,tag::variance > > acc_stats;
  double min_time = INT_MAX, max_time = 0;
  double seq = 0;
  for( size_t i = 0 ; i < op_count ; i++ , seq++ )
  {
    nextlabs::high_resolution_timer timer;
    timer.start();

    nextlabs::policy_query pquery;

    pquery.set_action(L"OPEN");
    pquery.set_application(L"c:\\foobar.exe");

    /* Set source */
    pquery.set_source(L"c:\\newfile.txt",L"fso");
    pquery.add_source_attribute(L"Are you for real?",L"Word");
    pquery.add_source_attribute(L"Who's the man?",L"You're the man!");

    /* Set target */
    pquery.set_target(L"c:\\newfile_to.txt",L"fso");
    pquery.add_target_attribute(L"Huh?",L"What?");

    if( pconn.query(pquery) == false )
    {
      DWORD le = GetLastError();
      fprintf(stderr, "query failed (le %d)\n", le);

      switch( le )
      {
      case ERROR_TIMEOUT:
	fprintf(stderr, "ERROR_TIMEOUT\n");
	break;
      case ERROR_SERVICE_NOT_FOUND:
	fprintf(stderr, "ERROR_SERVICE_NOT_FOUND\n");
	break;
      default:
	fprintf(stderr, "Unknown error value (%d)\n", le);
	break;
      }
      break;
    }

    timer.stop();

    double diff = timer.diff();

    if( op_verbose == true )
    {
      double query_time = pquery.query_time();
      fprintf(stdout, "TIME: Query %fms Total %fms : Diff %fms (%s)\n",
	      query_time,
	      diff,
	      diff - query_time,
	      pquery.is_deny() == true ? "deny" : "allow" );
    }

    double curr_time = diff;

    min_time = curr_time < min_time ? curr_time : min_time;  // running stat for min
    max_time = curr_time > max_time ? curr_time : max_time;  // running stat for max

    acc(diff);
    acc_stats(diff);

    if( op_verbose == true )
    {
      const nextlabs::policy_obligations& obs = pquery.get_obligations();
      nextlabs::policy_obligation_list::const_iterator it;
      for( it = obs.begin() ; it != obs.end() ; ++it )
      {
	fprintf(stdout, "Policy: %ws\n", it->get_policy());
	fprintf(stdout, "Name  : %ws\n", it->get_name());

	nextlabs::policy_obligation_options::const_iterator it_opt;
	for( it_opt = it->get_options().begin() ; it_opt != it->get_options().end() ; ++it_opt )
	{
	  fprintf(stdout, "  <%ws,%ws>\n", it_opt->first.c_str(), it_opt->second.c_str());
	}
      }
    }/* verbose */
  }/* for */

  pconn.disconnect();

  double u   = mean(acc_stats);
  double var = boost::accumulators::variance(acc_stats);
  double k   = pow(var,0.5) / pow(seq,0.5);

  double lower_bound = (u - 1.96 * k);
  double upper_bound = (u + 1.96 * k);

  // Summary stats for current run
  std::cout << "statistics for total time:\n";
  std::cout << "   elapsed = " << boost::accumulators::sum(acc_stats) << "ms , count = " << seq << "\n";
  std::cout << "       min = " << min_time << " , max = " << max_time << "\n";
  std::cout << "         u = " << u << " , var = " << var << "\n";
  std::cout << "  interval = [" << lower_bound << "," << upper_bound << "] (p = 0.95)\n";

  fprintf(stdout, "complete\n");

  return 0;
}/* main */
