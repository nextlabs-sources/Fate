//#include "servstub.h"
#include <string>
#include <iostream>
#include <vector>
#include "nlthread.h"
#include "nlthreadpool.h"
#include "transport.h"
#include "marshal.h"
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "CESDK_private.h"

namespace {
char *parseArgsWorkdir=NULL;
}
// ---------------------------------------------------------------------------
// Usage print out of the control module
// ---------------------------------------------------------------------------

void print_usage (char * program)
{
  cout << "Usage: " << program << endl;
  cout << "   -?                  : Display this message"   << endl;
  cout << "   -D <name>=<value>   : JVM properties"         << endl;   
  cout << "   -X debug flags      : JVM properties"         << endl;
  cout << "   -w work directory   : Working directory"      << endl;
  // Not printing this out, treat out as undocumented feature.
  cout << "   -C                  : Interactive mode, not as deamon" << endl;
}


char** getJavaArgs(char** targetstr, int* cnt, int argc, char** argv)
{
  vector<string> javaArgs;
  string s;
  char c;
  *cnt = 0;
  bool bDaemonMode=true;

  while ((c = getopt (argc, argv, "c?d:D:x:X:w:C")) != -1) {
    switch (c) {
    case '?':
      print_usage (argv[0]);
      bDaemonMode=false;
      exit(0);
      break;
    case 'd':
    case 'D':
      s = "-D" + string(optarg);
      javaArgs.push_back(s.c_str());
      break;      
    case 'x':
    case 'X':
      s = "-X" + string(optarg);
      javaArgs.push_back(s.c_str());
      break;
    case 'w':
      {
	int len = strlen(optarg) +1;
	parseArgsWorkdir = (char*)malloc(len);
	_snprintf_s(parseArgsWorkdir,len, _TRUNCATE, "%s",optarg);
      }
      break;
    case 'C':
    case 'c':
      bDaemonMode = false;
      break;      
    default:
      cout << "Bad argument: "<<c<<". Program aborted." << endl;
      exit (1);
    }
  }
  *cnt=javaArgs.size();

  //Allocate an array to hold the arguments
  targetstr= new char *[*cnt];

  for (int i = 0; i < *cnt; i++) {
    targetstr[i] = strdup (javaArgs[i].c_str());
    //cout << "option " << i << " = " <<targetstr[i] << endl;
  }

  if(bDaemonMode)
    daemon(1, 0); //run as daemon

  return targetstr;
}


char* getWorkingDirectory(int argc, char** argv)
{  
  return parseArgsWorkdir;
}

void pareseargs_free(char **args, int argc)
{
  if(args == NULL)
    return;

  for(int i=0; i<argc; i++)
    free(args[i]);

  delete [] args;
}

void parser_freeworkdir(char *dir)
{
  if(dir)
    free(dir);
}
//Only file server enforcer on Linux, return false
bool getPDPType(int dwArgc, char* *lpszArgv)
{
  return false;
}
//Of course, we don't support WDE on Linux
bool CheckWDESupport(int dwArgc, char* *lpszArgv)
{
  return false;
}
//We don't support bootstrap injection service on Linux now
bool CheckBootstrapInjectionEnable(int dwArgc, char* *lpszArgv)
{
  return false;
}
