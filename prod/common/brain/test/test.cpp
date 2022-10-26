
#include <iostream>

#ifdef Linux

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

using namespace std;

// Defining different TRACE level for different file
#define NLMODULE mybraintest
#define NLTRACELEVEL 2

#include "brain.h"

#ifdef Linux
#include "osServLinux.h"
#endif

extern void testnlstring();
extern void testnlprintf();
extern void testnlthreadpool();


int main (int argc, char ** argv)
{
  //test NL_GetFilePhysicalPath
  nlchar output[1024];
  int c;

  c = getchar();

  if(NL_GetFilePhysicalPath(_T("T:\\heidiz\\SDK\\eval.cpp"), output, 1024)) {
    TRACE (1, _T("NL_GetFilePhysicalPath: T:\\heidiz\\SDK\\eval.cpp <--> %s\n"), 
	   output);
  }

  
  if(NL_GetFilePhysicalPath(_T("V:\\readme.htm"), output, 1024)) {
    TRACE (1, _T("NL_GetFilePhysicalPath: V:\\readme.htm <--> %s\n"), 
	   output);
  }

  NL_GetFilePhysicalPath(_T("C:\\TEMP\\PDPStop.dll"), output, 1024);
  TRACE (1, _T("NL_GetFilePhysicalPath: C:\\TEMP\\PDPStop.dll <--> %s\n"), 
	   output);

  nlchar owner[256];
  NL_getUserId (owner, 256);

  TRACE (1, _T("This test is being run by user %s\n"), owner);

  // Show case the TRACE, use the macro above
  int i = 10;

  TRACE (1, _T("In test.cpp 1 %d\n"), i);
  TRACE (3, _T("Won't print this message with level 2\n"));

  testnlstring();

  TRACE (2, _T("In test.cpp 2 %p\n"), &i);

  testnlprintf ();

  testnlthreadpool();
}
  
