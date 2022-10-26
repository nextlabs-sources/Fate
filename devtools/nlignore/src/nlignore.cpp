#include <cstdio>
#include <cstdlib>
#include <string>
#include <winsock2.h>
#include <windows.h>

#include <eframework/policy/policy_monitor_application.hpp>

int wmain( int argc , wchar_t** argv )
{
  /* There must be at least one option */
  if( argc != 2 )
  {
    fprintf(stdout, "NextLabs Ignore Application (Built %s %s)\n", __DATE__, __TIME__);
    fprintf(stdout, "usage: nlignore [full image path]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "   --help, -h, /?          This screen.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  nlignore c:\\foo.exe\n");
    return 0;
  }

  bool is_ignored = false;

  fprintf(stdout, "QUERY : %ws\n", argv[1]);
  is_ignored = nextlabs::policy_monitor_application::is_ignored(argv[1]);
  fprintf(stdout, "RESULT: %ws\n",
	  is_ignored == true ? L"ignored" : L"not ignored" );

  return 0;
}/* main */
