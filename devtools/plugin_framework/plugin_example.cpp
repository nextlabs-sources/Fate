/*****************************************************************************
 *
 * Example Plug-in
 *
 * Provides an example of plug-in implementation for the Policy Controller.
 * A worker thread is created in the PluginEntry to handle IO.  When
 * PluginUnload is called the thread is signaled to exit.
 *
 ****************************************************************************/
#include <windows.h>
#include <cstdio>
#include <cassert>
#include <time.h>
#include <tchar.h>

/** ExamplePluginContext
 *
 *  Context information for Example plug-in.
 */
typedef struct
{
  HANDLE cancel_event;  /* Event cancel support of device arrival */
  HANDLE th;            /* Thread handle */
} ExamplePluginContext;

static DWORD WINAPI wde_plugin_worker( LPVOID in_context )
{
  assert( in_context != NULL );
  ExamplePluginContext* context = (ExamplePluginContext*)in_context;

  fprintf(stdout, "plugin_example: worker thread started\n");

  /* Auto reset synchronization event for IO */
  HANDLE io_event = CreateEventA(NULL,FALSE,FALSE,NULL);

  if( io_event == NULL )
  {
    return 0;
  }

  /* Perform IO until cancelled by PluginUnload */
  for( ;; )
  {
    /************************************************************
     * Wait for IO or until cancel event is signaled
     ***********************************************************/
    DWORD wait_result;
    HANDLE wait_handles[] = { context->cancel_event , io_event };
    wait_result = WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE);

    if( wait_result == WAIT_OBJECT_0 )           /* cancel event signaled */
    {
      /* Wait has been cancelled.  Cancel any IO state that is pending. */
      fprintf(stdout, "plugin_example: cancel state has been signaled\n");
      break;
    }

    if( wait_result == (WAIT_OBJECT_0 + 1) )     /* IO event signaled */
    {
      /* Do IO here */
      fprintf(stdout, "plugin_example: IO signaled\n");
    }
  }/* for */
  CloseHandle(io_event);
  fprintf(stdout, "plugin_example: worker thread complete\n");
  return 0;
}/* example_plugin_worker */

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
  assert( in_context != NULL );

  int result = 0;
  if( in_context == NULL )
  {
    return 0;
  }

  ExamplePluginContext* context = new ExamplePluginContext;
  *in_context = (void*)context;

  memset(context,0x00,sizeof(ExamplePluginContext));
  context->cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);

  if( context->cancel_event != NULL )
  {
    /* Create a worker thread for handling IO */
    context->th = CreateThread(NULL,0,wde_plugin_worker,*in_context,0,NULL);

    if( context->th != NULL )
    {
      result = 1;
    }
  }
  return result;
}/* PluginEntry */

extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
  assert( in_context != NULL );

  if( in_context == NULL )
  {
    return 0;
  }

  ExamplePluginContext* context = (ExamplePluginContext*)in_context;

  /* Signal thread to complete */
  SetEvent(context->cancel_event);
  WaitForSingleObject(context->th,INFINITE);
  CloseHandle(context->th);

  delete in_context;

  return 1;
}/* PluginUnload */
