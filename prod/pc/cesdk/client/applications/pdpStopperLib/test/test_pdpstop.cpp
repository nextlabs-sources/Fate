#if defined (WIN32) || defined (_WIN64)
#include <tchar.h>
#else
#include "unistd.h"
#include "linux_win.h"
#include "brain.h"
#include <fcntl.h>
#endif
#include <stdio.h>
#include "CEsdk.h"

#if defined (WIN32) || defined (_WIN64)
extern CEResult_t  __stdcall stopAgentServiceWithoutPassword (TCHAR * lpszPassword);
#else
extern CEResult_t  stopAgentService(char * lpszPassword);
#endif

int main(int argc, char **argv) 
{
#ifdef Linux
  int fd = open("/opt/PolicyController/.pdpstop", O_CREAT | O_WRONLY);
  close(fd);
  char *password = getpass("Policy Controller password: ");
  CEResult_t ret = stopAgentService(password);
#else
  CEResult_t ret=stopAgentServiceWithoutPassword(_T("password"));
#endif
  printf("StopAgentService return %d\n", ret);
  return 0;
}
