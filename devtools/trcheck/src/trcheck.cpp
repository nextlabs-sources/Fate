

#include <windows.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>

#include "CEsdk.h"
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_connection.hpp"

HANDLE tamper_down;
HANDLE thread_started;
HANDLE thread_stop;

typedef CEResult_t (*CEP_StopPDPWithoutPassword_t)(CEHandle handle, 
						   CEString password,
						   CEint32 timeout_in_milliseconds) ;

void __cdecl monitor_tamper( void* arg )
{
  FILE* fp;
  for( ; ; )
  {
    if( WaitForSingleObject(thread_stop,0) == WAIT_OBJECT_0 )
    {
      fprintf(stderr, "Stopping thread\n");
      break;
    }

    errno_t err = fopen_s(&fp, "C:\\Program Files\\NextLabs\\Policy Controller\\agentLog\\pclog.txt", "r");
    if ((err == 0) && (fp != NULL))
    {
      SetEvent(tamper_down);
      fclose(fp);
    }
    Sleep(1);
    SetEvent(thread_started);
  }
}

int main(void)
{
  nextlabs::cesdk_loader cesdk;
  nextlabs::cesdk_connection cesdk_conn;

  if( cesdk.load(L"C:\\Program Files\\NextLabs\\Policy Controller\\bin") == false )
  {
    fprintf(stderr, "Cannot load CE SDK\n");
    return 1;
  }
  cesdk_conn.set_sdk(&cesdk);

  if( cesdk_conn.connect() == false )
  {
    fprintf(stderr, "Cannot connect to the PC\n");
    return 1;
  }

  HMODULE hmod;
  hmod = LoadLibrary(L"C:\\Program Files\\NextLabs\\Policy Controller\\bin\\ceprivate32.dll");
  if( hmod == NULL )
  {
    fprintf(stderr, "Cannot load ceprivate32.dll\n");
    return 1;
  }
  CEP_StopPDPWithoutPassword_t stop_pc = NULL;

  stop_pc = (CEP_StopPDPWithoutPassword_t)GetProcAddress(hmod,"CEP_StopPDPWithoutPassword");

  fprintf(stdout, "StopPC @ 0x%p\n", stop_pc);

  if( stop_pc == NULL )
  {
    fprintf(stderr, "Cannot find CEP_StopPDPWithoutPassword\n");
    exit(1);
  }

  tamper_down = CreateEventW(NULL,TRUE,FALSE,NULL);
  thread_started = CreateEventW(NULL,TRUE,FALSE,NULL);
  thread_stop = CreateEventW(NULL,TRUE,FALSE,NULL);
  if( tamper_down == NULL || thread_started == NULL || thread_stop == NULL )
  {
    fprintf(stderr, "Cannot create events!\n");
    return 1;
  }

  DWORD start_time, end_time;
  _beginthread(monitor_tamper,0,NULL);

  /* wait for thread to start */
  WaitForSingleObject(thread_started,INFINITE);
  fprintf(stdout, "Thread started\n");

  CEString password = CEM_AllocateString(L"123blue!");
  start_time = GetTickCount();
  stop_pc(cesdk_conn.get_connection_handle(),password,30 * 1000);
  end_time = GetTickCount();
  fprintf(stdout, "Waited %d ms for Stop PC\n", end_time - start_time);

  if( WaitForSingleObject(tamper_down,0) == WAIT_TIMEOUT )
  {
    fprintf(stdout, "Tamper is NOT down!\n");
  }

  /* wait for tamper to come down */
  start_time = GetTickCount();
  for( ; ; )
  {
    if( WaitForSingleObject(tamper_down,0) == WAIT_OBJECT_0 )
    {
      break;
    }
  }
  end_time = GetTickCount();

  fprintf(stdout, "Waited %d ms\n", end_time - start_time);

  return 0;
}
