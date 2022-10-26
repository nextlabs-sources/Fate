/******************************************************************************
 *
 * NLDevEnf - Test Tool
 *
 *****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <basetyps.h>
#include <INITGUID.H>
#include <winioctl.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <usbiodef.h>
#include <usbioctl.h>
#include <usb100.h>
#include <usb200.h>

#include "nl_devenf_lib.h"
#include "nl_device.h"

/** display_device
 *
 *  Display device information to stdout.
 */
static void display_device( const DeviceInfo* device )
{
  WCHAR buf[512];
  StringFromGUID2(device->BusTypeGuid,buf,sizeof(buf));

  fwprintf(stdout, L"Interface       = %s\n", device->InterfaceName);
  fwprintf(stdout, L"BusTypeGuid     = %s\n", buf);
  fwprintf(stdout, L"Setup Class     = %s\n", device->SetupClass);
  fwprintf(stdout, L"Class Name      = %s\n", device->ClassName);
  fwprintf(stdout, L"Enumerator Name = %s\n", device->EnumeratorName);
  const WCHAR* p = device->CompatibleIDs;
  while( *p != (WCHAR)NULL )
  {
    fwprintf(stdout, L"Compatible ID   = %s\n", p);
    p += wcslen(p) + 1;
  }
  fwprintf(stdout, L"State           = 0x%x (%s)\n", 
	   device->state,
	   (device->state & NL_DEVENF_DEVICE_STATE_ENABLED) ? L"enabled" : L"disable" );

  fwprintf(stdout, L"Device Number   = %d\n", device->device_number);
  fwprintf(stdout, L"Arrival Time    = %I64d\n", device->sys_time_arrival.QuadPart);
  fwprintf(stdout, L"Change  Time    = %I64d\n", device->sys_time_changed.QuadPart);

}/* display_device */

static void print_help(void)
{
  fprintf(stdout, "usage: nldevenf [optional parameter]\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "       wait          Wait for device arrival.\n");
  fprintf(stdout, "       test          Test for device arrival.  Show arrival state.\n");
  fprintf(stdout, "       disable [no.] Disable device by number.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "example: nldevenf wait\n");
  fprintf(stdout, "         nldevenf test\n");
  fprintf(stdout, "         nldevenf disable 3\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "By default all devices are shown.\n");
}/* print_help */

/** disable_device
 *
 *  \brief Disable a device by its number.
 *  \param device_number (in) Device number.
 *  \return true on success, false on error.
 */
static bool disable_device( int device_number )
{
  DeviceInfo devices[16];
  int num_devices = sizeof(devices) / sizeof(DeviceInfo);

  if( Device_GetDevices(devices,&num_devices) == 0 )
  {
    return false;
  }

  for( int i = 0 ; i < num_devices ; i++ )
  {
    if( devices[i].device_number == (LONG)device_number )  /* match? */
    {
      if( Device_Disable(&devices[i]) == 0 )               /* disable device */
      {
	return false;
      }
      break;
    }
  }/* for */
  return true;
}/* disable_device */

/* Cancel event for Device_WaitForArrival
 */
HANDLE cancel_event = NULL;

/** NLDevEnfCtrlHandler
 *
 *  \brief Control handler.  When break occurs cancel pending wait for device
 *         arrival.
 */
static BOOL WINAPI NLDevEnfCtrlHandler( DWORD fdwCtrlType ) 
{
  if( cancel_event != NULL )
  {
    SetEvent(cancel_event);
  }
  return TRUE;
}/* NLDevEnfCtrlHandler */

int main( int argc , char** argv )
{
  /****************************************************************************
   * Display help
   ***************************************************************************/
  if( argc == 2 && 
      ( strcmp(argv[1],"/?") == 0 ||
	strcmp(argv[1],"?") == 0 ||
	strcmp(argv[1],"-h") == 0 ||
	strcmp(argv[1],"--help") == 0) )
  {
    print_help();
    return 0;
  }

  /****************************************************************************
   * Disable device by number
   ***************************************************************************/
  if( argc == 3 && strcmp(argv[1],"disable") == 0 )
  {
    int device_number = atoi(argv[2]);
    if( disable_device(device_number) == false )
    {
      fprintf(stdout, "Cannot disable device.  It does not exist or cannot be disabled\n");
      return 1;
    }
    return 0;
  }

  int status, device_arrived = 0;
  /****************************************************************************
   * Wait (block) for device arrival
   ***************************************************************************/
  if( argc == 2 && strcmp(argv[1],"wait") == 0 )
  {
    cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);

    SetConsoleCtrlHandler(NLDevEnfCtrlHandler,TRUE);

    int result = Device_WaitForArrival(cancel_event);
    CloseHandle(cancel_event);
    if( result == 0 )
    {
      fprintf(stderr, "Device_WaitForArrival failed (%d)\n",GetLastError());
      return 1;
    }
    return 0;
  }

  /****************************************************************************
   * Test for device arrival
   ***************************************************************************/
  if( argc == 2 && strcmp(argv[1],"test") == 0 )
  {
    if( Device_TestForArrival(&device_arrived) == 0 )
    {
      fprintf(stderr, "Device_TestForArrival failed (le %d)\n",GetLastError());
      return 1;
    }
    fprintf(stdout, "Device_TestForArrival: arrival state is %s\n",
	    device_arrived ? "signaled" : "not signaled");
    return 0;
  }
#if 0
  /****************************************************************************
   * Display devices
   ***************************************************************************/
  if( Device_TestForArrival(&device_arrived) == 0 )
  {
    fprintf(stderr, "Device_TestForArrival failed (le %d)\n",GetLastError());
    return 1;
  }

  /* Determine size required to retreive device list */
  DeviceInfo* devices = NULL;
  int num_devices = 0;
  status = Device_GetDevices(devices,&num_devices);
  if( status == 0 && GetLastError() != ERROR_INSUFFICIENT_BUFFER )
  {
    fprintf(stderr, "Device_GetDevices failed (le %d)\n", GetLastError());
    return 1;
  }

  if( num_devices == 0 )
  {
    fprintf(stdout, "no devices\n");
    return 0;
  }

  devices = new DeviceInfo[num_devices];
  status = Device_GetDevices(devices,&num_devices);
  if( status != 0 )
  {
    fprintf(stdout, "%d devices (arrival state is %s)\n", num_devices,
	    device_arrived ? "signaled" : "not signaled");
    for( int i = 0 ; i < num_devices ; i++ )
    {
      display_device(&devices[i]);
    }
  }

  delete [] devices;
#endif

  char device[512] = {0};
  USBDevice udev;

  for( size_t i = 0 ; i < 4 ; i++ )
  {
    memset(&udev,0x00,sizeof(udev));
    if( nl_device_from_letter(L'c' + i,&udev) == true )
    {
      fprintf(stdout, "Device: %wc Vendor=%ws %04x Product=%ws %04x SerialNumber=%s\n",
	      L'c' + i,
	      udev.vendor_string, udev.dd.idVendor,
	      udev.product_string, udev.dd.idProduct,
	      udev.sn);
    }
  }

  return 0;
}/* main */
