
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

#include <eframework/os/operating_system.hpp>

bool op_verbose = false;

int main( int argc , char** argv )
{
  fprintf(stdout,"is_system_process = %d\n", nextlabs::os::is_system_process());
  fprintf(stdout,"is_x86            = %d\n", nextlabs::os::is_process_x86());
  fprintf(stdout,"is_x64            = %d\n", nextlabs::os::is_process_x64());

  if( nextlabs::os::is_process_x86() == nextlabs::os::is_process_x64() )
  {
    fprintf(stderr, "FATAL: process cannot be both x86 and x64\n");
    return 1;
  }

  return 0;
}/* main */
