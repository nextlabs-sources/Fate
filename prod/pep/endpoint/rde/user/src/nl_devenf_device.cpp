
#include <windows.h>
#include <cstdio>
#include <cassert>
#include <time.h>
#include <wdmguid.h>
#include <WtsApi32.h>
#include <psapi.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <aclapi.h>
#include <imagehlp.h>
#include <winwlx.h>
#include <objbase.h>

#include "celog.h"
#include "nl_devenf_device.h"

extern CELog RdeLog;

bool is_firewire( _In_ const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return false;
  }
#if 0 /* Requires > WIN2K but should work based on string value of class below. */
  if( IsEqualGUID(device->BusTypeGuid,GUID_BUS_TYPE_1394) == TRUE )
  {
    return true;
  }
#endif
  /* It is possible that the BusTypeGuid is not correct.  For example, a Canon video
     camera (ZR200) will report as present on a USB bus per WdfDeviceQueryProperty
     when it is actually present on the IEEE 1394 bus.  The following classes are
     exclusive to IEEE 1394, so they are used to determine presence of that type of
     device.

     Canon ZR200 reports setup class of 61883 but a class name of AVC.  This is due
     to the AVC device being present on the 61883 which is itself present on the
     IEEE 1394 bus.  Any device present on the IEEE 1394 bus is considered a FireWire
     device.

     Bus:  IEEE 1394
               |--------- 61883
                             |--------- AVC (Canon ZR200)
  */
  const WCHAR* classes[] = { L"61883" , L"AVC" , L"SBP2" , L"1394" };
  for( int i = 0 ; i < _countof(classes) ; i++ )
  {
    if( _wcsnicmp(device->ClassName,classes[i],_countof(device->ClassName)) == 0 )
    {
      return true;
    }
  }
  return false;
}/* is_firewire */

bool is_usb( _In_ const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return false;
  }
  if( IsEqualGUID(device->BusTypeGuid,GUID_BUS_TYPE_USB) == TRUE )
  {
    return true;
  }
  return false;
}/* is_usb */

bool is_pcmcia( _In_ const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return false;
  }
  const WCHAR* enums[] = { L"PCMCIA" };
  for( int i = 0 ; i < _countof(enums) ; i++ )
  {
    if( _wcsnicmp(device->EnumeratorName,enums[i],_countof(device->EnumeratorName)) == 0 )
    {
      return true;
    }
  }
  return false;
}/* is_pcmcia */

bool is_composite_device( _In_ const DeviceInfo* device )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return false;
  }
  const WCHAR* p = device->CompatibleIDs;
  while( *p != (WCHAR)NULL )
  {
    if( wcsstr(p,L"COMPOSITE") != NULL )
    {
      return true;
    }
    p += wcslen(p) + 1;
  }
  return false;
}/* is_composite_device */

bool read_device( _In_ const DeviceInfo* device ,
		  _Out_ int* out_vendor_id ,
		  _Out_ int* out_product_id ,
		  _Out_cap_(in_serial_number_count) WCHAR* in_serial_number ,
		  _In_ size_t in_serial_number_count )
{
  assert( device != NULL );
  if( device == NULL )
  {
    return false;
  }
  bool result = false;
  WCHAR InterfaceName[512] = {0};

  *out_vendor_id = 0x0;
  *out_product_id = 0x0;

  wcsncpy_s(InterfaceName,_countof(InterfaceName),device->InterfaceName,_TRUNCATE);

  /* Interface format:
   *
   *  \??\USB#Vid_0781&Pid_5506#0000173EA1802201#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
   */
  WCHAR* next_token        = NULL;
  WCHAR* enumerator_name   = NULL;
  WCHAR* vendor_product_id = NULL;
  WCHAR* serial_number     = NULL;
  WCHAR* interface_class   = NULL;

  enumerator_name   = wcstok_s(InterfaceName,L"#",&next_token);
  vendor_product_id = wcstok_s(NULL,L"#",&next_token);
  serial_number     = wcstok_s(NULL,L"#",&next_token);
  interface_class   = wcstok_s(NULL,L"#",&next_token);

  if( enumerator_name == NULL || vendor_product_id == NULL ||
      serial_number == NULL || interface_class == NULL )
  {
    RdeLog.Log(CELOG_WARNING,L"read_device: failed '%ws'\n",InterfaceName);
    goto read_device_complete;
  }

  /* Format:
   *   Vid_0781&Pid_5506
   */
  WCHAR* vid_string = NULL;
  WCHAR* pid_string = NULL;

  next_token = NULL;
  vid_string = wcstok_s(vendor_product_id,L"&_",&next_token);   /* 'Vid' */
  vid_string = wcstok_s(NULL,L"&_",&next_token);                /* Vendor ID */
  pid_string = wcstok_s(NULL,L"&_",&next_token);                /* 'Pid' */
  pid_string = wcstok_s(NULL,L"&",&next_token);                 /* Product ID */

  if( vid_string == NULL || pid_string == NULL )
  {
    goto read_device_complete;
  }

  /* Extract vendor/product ID */
  if( swscanf_s(vid_string,L"%04x",out_vendor_id) != 1 ||
      swscanf_s(pid_string,L"%04x",out_product_id) != 1 )
  {
    RdeLog.Log(CELOG_WARNING,L"read_device: failed to read vendor and product IDs\n");
    goto read_device_complete;
  }

  wcsncpy_s(in_serial_number,in_serial_number_count,serial_number,_TRUNCATE);

  result = true;

 read_device_complete:

  return result;
}/* read_device */
