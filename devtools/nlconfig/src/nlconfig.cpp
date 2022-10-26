/****************************************************************************************
 *
 * NextLabs Diagnostics/Debug Configuration Tool
 *
 ***************************************************************************************/

#include <windows.h>
#include <process.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <string>
#include <list>

#include "nlconfig.hpp"

// Debug enablement for NextLabs software.
#define NEXTLABS_DEBUG_KEY L"SOFTWARE\\NextLabs\\DebugMode"

// Debug log level for WdeLog (CELog) instance.
#define WDE_LOG_LEVEL_KEY L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer\\DebugLogLevel"

// Debug log size for this process in bytes.
#define WDE_LOG_SIZE_KEY  L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer\\DebugLogSize"

void print_help(void)
{
  fprintf(stdout, "NextLabs Configuration (Built %s %s)\n", __DATE__, __TIME__);
  fprintf(stdout, "usage: nlconfig [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "   --debug=yes|no|true|false           Enable or disable debug mode.\n");
  fprintf(stdout, "   --help,/?,-h                        This screen.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  nlconfig --debug=yes\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Product Keys\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Windows Desktop Enforcer\n");
  fprintf(stdout, "  DebugLogLevel (DWORD)\n");
  fprintf(stdout, "    %ws\n",WDE_LOG_LEVEL_KEY);
  fprintf(stdout, "  DebugLogSize (DWORD)\n");
  fprintf(stdout, "    %ws\n",WDE_LOG_SIZE_KEY);
}/* print_help */

int wmain( int argc , wchar_t** argv )
{
  if( argc == 2 )
  {
    if( _wcsicmp(argv[1],L"--help") == 0 ||
	_wcsicmp(argv[1],L"-?") == 0 ||
	_wcsicmp(argv[1],L"/?") == 0 ||
	_wcsicmp(argv[1],L"/h") == 0 )
    {
      print_help();
      return 0;
    }
  }

  bool op_display = true;

  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

    if( wcsncmp(argv[i],L"--prefix=",wcslen(L"--prefix=")) == 0 )
    {
      //op_prefix = option;
      continue;
    }
    else if( wcsncmp(argv[i],L"--debug=",_countof(L"--debug=")-1) == 0 )
    {
      if( _wcsicmp(argv[i],L"--debug=yes") == 0 || _wcsicmp(argv[i],L"--debug=true") == 0 )
      {
	NLConfig::WriteKey(NEXTLABS_DEBUG_KEY,1);
	fprintf(stdout, "nlconfig: debug mode is enabled\n");
      }
      else if( _wcsicmp(argv[i],L"--debug=no") == 0 || _wcsicmp(argv[i],L"--debug=false") == 0 )
      {
	NLConfig::WriteKey(NEXTLABS_DEBUG_KEY,0);
	fprintf(stdout, "nlconfig: debug mode is disabled\n");
      }
      else
      {
	fprintf(stderr, "nlconfig: invalid option\n");
	exit(1);
      }
      exit(0);
    }

  }/* for */

  bool is_debug_mode = NLConfig::IsDebugMode();
  if( op_display == true )
  {
    fprintf(stdout, "NextLabs Configuration (Built %s %s)\n\n", __DATE__, __TIME__);
    fprintf(stdout, "Configuration:\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "NextLabs (Base)\n\n");
    fprintf(stdout, "  DebugMode %d (%s)\n", is_debug_mode, is_debug_mode == true ? "on" : "off" );
    fprintf(stdout, "\n");
    fprintf(stdout, "Windows Desktop Enforcer (WDE)\n");
    fprintf(stdout, "\n");

    int log_level = 0;
    int log_size = 0;
    NLConfig::ReadKey(WDE_LOG_LEVEL_KEY,&log_level);
    NLConfig::ReadKey(WDE_LOG_SIZE_KEY,&log_size);
    fprintf(stdout, "  DebugLogLevel %d (CELog Type)\n", log_level);
    fprintf(stdout, "  DebugLogSize  %d (bytes)\n", log_size);
  }

  return 0;
}/* main */
