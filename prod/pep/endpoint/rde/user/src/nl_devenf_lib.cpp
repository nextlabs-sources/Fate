#include <windows.h>
#include <winioctl.h>
#include <cassert>
#include <cstdio>
#include <cstring>

#include "nlconfig.hpp"
#include "nl_devenf_lib.h"

extern "C" void* Device_GetContext( const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return 0;
  }

  return (void*)&device->context;
}/* Device_GetContext */

extern "C" int Device_GetDevices( DeviceInfo* devices ,
				  int* num_devices )
{
  assert( num_devices != NULL );
  if( num_devices == NULL )
  {
    return 0;
  }

  HANDLE devh;
  BOOL rv;
  int status = 0;
  DWORD out_size = 0;

  devh = CreateFileW(NL_DEVENF_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,
		     OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( devh == INVALID_HANDLE_VALUE )
  {
    status = 0;
    goto Device_GetDevices_complete;
  }

  rv = DeviceIoControl(devh,IOCTL_NL_DEVENF_GET_ALL_DEVICES,
		       NULL,0,
		       (PVOID)devices,sizeof(DeviceInfo) * (*num_devices),
		       &out_size,
		       NULL);
  if( rv != FALSE )
  {
    status = 1; /* success */
    /* If the output size is larger than supplied there was insufficient space.  The
     *   out_size indicates the required space when this happens.
     */
    if( *num_devices < (int)(out_size / sizeof(DeviceInfo)) )
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      status = 0; /* failure */
    }
    *num_devices = out_size / sizeof(DeviceInfo);
  }

  CloseHandle(devh);

 Device_GetDevices_complete:

  return status;
}/* Device_GetDevices */

extern "C" int Device_WaitForArrival( HANDLE cancel_event )
{
  assert( cancel_event != NULL && cancel_event != INVALID_HANDLE_VALUE );
  if( cancel_event == NULL || cancel_event == INVALID_HANDLE_VALUE )
  {
    return 0;
  }

  HANDLE devh;
  BOOL rv;
  int result, status = 0;
  DWORD out_size = 0;

  devh = CreateFileW(NL_DEVENF_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,
		     OPEN_EXISTING,
		     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		     NULL);

  if( devh == INVALID_HANDLE_VALUE )
  {
    if( NLConfig::IsDebugMode() == true )
    {
      fwprintf(stdout, L"Device_WaitForArrival: CreateFileW failed (le %d) %s\n",
	       GetLastError(), NL_DEVENF_DEVICE);
    }
    goto Device_WaitForArrival_complete;
  }

  OVERLAPPED io;
  memset(&io,0x00,sizeof(io));
  io.hEvent = CreateEventA(NULL,TRUE,FALSE,NULL);

  if( io.hEvent == NULL )
  {
    goto Device_WaitForArrival_complete;
  }

  rv = DeviceIoControl(devh,IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE,
		       NULL,0,                   /* input */
		       &result,sizeof(result),   /* output */
		       &out_size,
		       &io);

  DWORD wait_result = 0;
  HANDLE wait_handles[2];

  wait_handles[0] = cancel_event;     /* caller's cancel event */
  wait_handles[1] = io.hEvent;        /* I/O from DeviceIoControl */
  wait_result = WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE);

  if( wait_result == WAIT_OBJECT_0 )           /* cancel event */
  {
    /* Wait has been cancelled */
    CancelIo(devh);
  }

  if( wait_result == (WAIT_OBJECT_0 + 1) )     /* IO event */
  {
    if( GetOverlappedResult(devh,&io,&out_size,FALSE) == TRUE )
    {
      status = 1;  /* IO completed with success */
    }
  }

  CloseHandle(io.hEvent);

 Device_WaitForArrival_complete:

  if( devh != INVALID_HANDLE_VALUE )
  {
    CloseHandle(devh);
  }

  return status;
}/* Device_WaitForArrival */

extern "C" int Device_TestForArrival( int* device_arrived )
{
  assert( device_arrived != NULL );

  if( device_arrived == NULL )
  {
    return 0;
  }

  HANDLE devh;
  BOOL rv;
  DWORD out_size = 0;
  int result, status = 0;

  devh = CreateFileW(NL_DEVENF_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,
		     OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( devh == INVALID_HANDLE_VALUE )
  {
    if( NLConfig::IsDebugMode() == true )
    {
      fwprintf(stdout, L"Device_WaitForArrival: CreateFileW failed (le %d) %s\n",
	       GetLastError(), NL_DEVENF_DEVICE);
    }
    status = 0;
    goto Device_TestForArrival_complete;
  }

  rv = DeviceIoControl(devh,IOCTL_NL_DEVENF_TEST_DEVICE_ARRIVE,
		       NULL,0,                   /* input */
		       &result,sizeof(result),   /* output */
		       &out_size,
		       NULL);
  CloseHandle(devh);

  status = 1;            /* default success */
  if( rv != TRUE )
  {
    status = 0;          /* DeviceIoControl failed */
  }

  *device_arrived = 0;   /* default non-signaled arrival state */
  if( result )
  {
    *device_arrived = 1; /* output of DeviceIoControl indicates signaled state */
  }

 Device_TestForArrival_complete:

  return status;
}/* Device_TestForArrival */

extern "C" int Device_SetDevice( const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return 0;
  }

  HANDLE devh = INVALID_HANDLE_VALUE;
  BOOL rv;
  DWORD out_size = 0;
  int status = 0;

  devh = CreateFileW(NL_DEVENF_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,
		     OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( devh == INVALID_HANDLE_VALUE )
  {
    status = 0;
    goto Device_SetDevice_complete;
  }

  rv = DeviceIoControl(devh,IOCTL_NL_DEVENF_SET_DEVICE_STATE,
		       (PVOID)device,sizeof(DeviceInfo),  /* input */
		       NULL,0,                            /* output */
		       &out_size,
		       NULL);
  status = 0;         /* default failed */
  if( rv != FALSE )
  {
    status = 1;       /* DeviceIoControl okay */
  }

  CloseHandle(devh);

 Device_SetDevice_complete:

  return status;
}/* Device_SetDevice */

extern "C" int Device_Disable( DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return 0;
  }

  device->state &= ~NL_DEVENF_DEVICE_STATE_ENABLED; /* clear enabled state */
  return Device_SetDevice(device);
}/* Device_Disable */

extern "C" int Device_SetContext( DeviceInfo* device ,
				  const void* context,
				  size_t context_size )
{
  assert( device != NULL );
  assert( context != NULL );
  assert( context_size <= Device_GetContextSize() );

  if( device == NULL ||
      context == NULL ||
      context_size > Device_GetContextSize() )
  {
    return 0;
  }

  /* place user-mode context into device struct and set the device */
  memcpy((void*)&device->context,context,context_size);
  return Device_SetDevice(device);

}/* Device_SetContext */

extern "C" size_t Device_GetContextSize(void)
{
  return sizeof(DeviceInfoContext);
}/* Device_GetContextSize */
