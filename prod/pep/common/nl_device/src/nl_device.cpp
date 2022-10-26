
//#define NL_DEVICE_TEST

/* Disable warnings for Windows SDK/WDK */
#pragma warning( push )
#pragma warning( disable : 4200 )

#include <windows.h>
#include <winioctl.h>
#include <setupapi.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <INITGUID.H>
#include <winioctl.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <usbiodef.h>
#include <usbioctl.h>
#include <usb100.h>
#include <usb200.h>

#pragma warning(pop)

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

#include "nl_device.h"

// Structure used to build a linked list of String Descriptors
// retrieved from a device.
typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE *Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
#pragma warning( push )
#pragma warning( disable : 4200 )
    USB_STRING_DESCRIPTOR           StringDescriptor[0];
#pragma warning( pop )
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;

static VOID EnumerateHubPorts( _In_ HANDLE hHubDevice, ULONG NumPorts, _In_ USBDevice* udev );

PSTRING_DESCRIPTOR_NODE GetAllStringDescriptors ( _In_ HANDLE hHubDevice,
						  _In_ ULONG ConnectionIndex,
						  _In_ PUSB_DEVICE_DESCRIPTOR DeviceDesc,
						  _In_ USBDevice* udev );

PSTRING_DESCRIPTOR_NODE GetStringDescriptor( _In_ HANDLE hHubDevice,
					     _In_ ULONG ConnectionIndex,
					     _In_ UCHAR DescriptorIndex,
					     _In_ USHORT LanguageID );

static VOID EnumerateHub ( _In_ wchar_t* HubName, _In_ USBDevice* udev )
{
  assert( HubName != NULL );
  assert( udev != NULL );

  PUSB_NODE_INFORMATION hubInfo = NULL;
  HANDLE hHubDevice = INVALID_HANDLE_VALUE;
  wchar_t* deviceName = NULL;
  BOOL success;
  ULONG nBytes = 0;

  // Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
  hubInfo = (PUSB_NODE_INFORMATION)malloc(sizeof(USB_NODE_INFORMATION));
  if( hubInfo == NULL )
  {
    goto EnumerateHubError;
  }

  // Allocate a temp buffer for the full hub device name.
  deviceName = (wchar_t*)malloc(1024*sizeof(wchar_t));
  if (deviceName == NULL)
  {
    goto EnumerateHubError;
  }

  // Create the full hub device name
  _snwprintf_s(deviceName,1024, _TRUNCATE,L"\\\\.\\%s",HubName);

  //fprintf(stdout, "EnumerateHub: Open: %ws\n", deviceName);
  hHubDevice = CreateFile(deviceName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,
			  OPEN_EXISTING,0,NULL);

  // Done with temp buffer for full hub device name
  free(deviceName);

  if (hHubDevice == INVALID_HANDLE_VALUE)
  {
    goto EnumerateHubError;
  }

  // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
  // This will tell us the number of downstream ports to enumerate, among
  // other things.
  success = DeviceIoControl(hHubDevice,
			    IOCTL_USB_GET_NODE_INFORMATION,
			    hubInfo,sizeof(USB_NODE_INFORMATION),
			    hubInfo,sizeof(USB_NODE_INFORMATION),
			    &nBytes,NULL);

  if (!success)
  {
    goto EnumerateHubError;
  }

  EnumerateHubPorts(hHubDevice,hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts,udev);

  CloseHandle(hHubDevice);
  return;

EnumerateHubError:

    if (hHubDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }

    if (hubInfo)
    {
        free(hubInfo);
    }
}

//*****************************************************************************
//
// EnumerateHubPorts()
//
// hHubDevice - Handle of the hub device to enumerate.
//
// NumPorts - Number of ports on the hub.
//
//*****************************************************************************
static VOID EnumerateHubPorts ( _In_ HANDLE hHubDevice, ULONG NumPorts, USBDevice* udev )
{
  assert( udev != NULL );
  ULONG       index = 0;
  BOOL        success;
  PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx = NULL;
  PSTRING_DESCRIPTOR_NODE stringDescs = NULL;

  // Port indices are 1 based, not 0 based.
  for (index=1; index <= NumPorts; index++)
  {
    ULONG nBytesEx = 0;

    // Allocate space to hold the connection info for this port.
    // For now, allocate it big enough to hold info for 30 pipes.
    //
    // Endpoint numbers are 0-15.  Endpoint number 0 is the standard
    // control endpoint which is not explicitly listed in the Configuration
    // Descriptor.  There can be an IN endpoint and an OUT endpoint at
    // endpoint numbers 1-15 so there can be a maximum of 30 endpoints
    // per device configuration.
    //
    // Should probably size this dynamically at some point.
    nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30;

    connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)malloc(nBytesEx);
    if (connectionInfoEx == NULL)
    {
      break;
    }

    // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION_EX structure
    // for this port.  This will tell us if a device is attached to this
    // port, among other things.
    connectionInfoEx->ConnectionIndex = index;

    success = DeviceIoControl(hHubDevice,IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
			      connectionInfoEx,nBytesEx,
			      connectionInfoEx,nBytesEx,
			      &nBytesEx,NULL);
    if( !success )
    {
      continue;
    }
    stringDescs = NULL;
    if( connectionInfoEx->DeviceDescriptor.iManufacturer ||
        connectionInfoEx->DeviceDescriptor.iProduct ||
        connectionInfoEx->DeviceDescriptor.iSerialNumber )
    {
      stringDescs = GetAllStringDescriptors(hHubDevice,index,&connectionInfoEx->DeviceDescriptor,udev);
    }
  }
}/* EnumerateHubPorts */

//*****************************************************************************
//
// GetAllStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptors will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptors will be requested.
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// requested.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE GetAllStringDescriptors ( _In_ HANDLE hHubDevice,
						  _In_ ULONG ConnectionIndex,
						  _In_ PUSB_DEVICE_DESCRIPTOR DeviceDesc,
						  _In_ USBDevice* udev )
{
  PSTRING_DESCRIPTOR_NODE stringDescNodeTail = NULL;
  wchar_t iMan[512] = {0};
  wchar_t iProd[512] = {0};

  if (DeviceDesc->iManufacturer)
  {
    stringDescNodeTail = GetStringDescriptor(hHubDevice,ConnectionIndex,DeviceDesc->iManufacturer,0x408);
    if( stringDescNodeTail != NULL )
    {
      wcsncpy_s(iMan,_countof(iMan),stringDescNodeTail->StringDescriptor->bString, _TRUNCATE);
    }
  }

  if (DeviceDesc->iProduct)
  {
    stringDescNodeTail = GetStringDescriptor(hHubDevice,ConnectionIndex,DeviceDesc->iProduct,0x408);
    if( stringDescNodeTail != NULL )
    {
      wcsncpy_s(iProd,_countof(iProd),stringDescNodeTail->StringDescriptor->bString, _TRUNCATE);
    }
  }

  /* Vendor and Product ID match */
  if( udev->vendor_id == DeviceDesc->idVendor && udev->product_id == DeviceDesc->idProduct )
  {
    wcsncpy_s(udev->vendor_string, _countof(udev->vendor_string), iMan, _TRUNCATE);
    wcsncpy_s(udev->product_string, _countof(udev->product_string), iProd, _TRUNCATE);
  }
  return stringDescNodeTail;
}


//*****************************************************************************
//
// GetStringDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// LanguageID - Language in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE GetStringDescriptor( _In_ HANDLE hHubDevice,
					     _In_ ULONG ConnectionIndex,
					     _In_ UCHAR DescriptorIndex,
					     _In_ USHORT LanguageID )
{
  BOOL    success;
  ULONG   nBytes = 0;
  ULONG   nBytesReturned = 0;
  UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) + MAXIMUM_USB_STRING_LENGTH] = {0};

  PUSB_DESCRIPTOR_REQUEST stringDescReq = NULL;
  PUSB_STRING_DESCRIPTOR  stringDesc = NULL;
  PSTRING_DESCRIPTOR_NODE stringDescNode = NULL;

  nBytes = sizeof(stringDescReqBuf);

  stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
  stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

  // Indicate the port from which the descriptor will be requested
  stringDescReq->ConnectionIndex = ConnectionIndex;

  // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
  // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
  //
  // USBD will automatically initialize these fields:
  //     bmRequest = 0x80
  //     bRequest  = 0x06
  //
  // We must inititialize these fields:
  //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
  //     wIndex    = Zero (or Language ID for String Descriptors)
  //     wLength   = Length of descriptor buffer
  //
  stringDescReq->SetupPacket.wValue  = (USB_STRING_DESCRIPTOR_TYPE << 8) | DescriptorIndex;
  stringDescReq->SetupPacket.wIndex  = LanguageID;
  stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

  // Now issue the get descriptor request.
  success = DeviceIoControl(hHubDevice,
			    IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
			    stringDescReq,nBytes,
			    stringDescReq,nBytes,
			    &nBytesReturned,NULL);

  if (!success)
  {
    return NULL;
  }

  if (nBytesReturned < 2)
  {
    return NULL;
  }

  if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
  {
    return NULL;
  }

  if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
  {
    return NULL;
  }

  if (stringDesc->bLength % 2 != 0)
  {
    return NULL;
  }

  // Looks good, allocate some (zero filled) space for the string descriptor
  // node and copy the string descriptor to it.
  stringDescNode = (PSTRING_DESCRIPTOR_NODE)malloc(sizeof(STRING_DESCRIPTOR_NODE) +
						   stringDesc->bLength);
  if (stringDescNode == NULL)
  {
    return NULL;
  }

  stringDescNode->DescriptorIndex = DescriptorIndex;

  memcpy(stringDescNode->StringDescriptor,stringDesc,stringDesc->bLength);

  return stringDescNode;
}

/* Is this WinXP? */
static bool is_xp(void)
{
  OSVERSIONINFO os_info;
  memset(&os_info,0x00,sizeof(os_info));
  os_info.dwOSVersionInfoSize = sizeof(os_info);
  if( GetVersionEx(&os_info) == FALSE )
  {
    return false;
  }

  if( os_info.dwMajorVersion == 5 )
  {
    return true;
  }
  return false;
}/* is_xp */

/** nl_device_to_device_instance_id
 *
 *  \brief Given a device name provide the device instance ID.
 *
 *  \param device_name       Device name
 *  \param volume_name       Volume name
 *  \param device_out        Device name out (instance ID)
 *  \param device_out_count  Device name count in chars.
 *
 *  \return true if the name has successfuly been translated to a device
 *          instance ID.  Otherwise, false.
 */
static bool nl_device_to_device_instance_id( const wchar_t* device_name ,
					     const wchar_t* volume_name ,
					     wchar_t* device_out , size_t device_out_count )
{
  HDEVINFO hDevInfo;
  SP_DEVINFO_DATA DeviceInfoData;
  DWORD i = 0;
  std::wstring serial;

  if( is_xp() == false )
  {
    HKEY hKey = NULL;
    LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SYSTEM\\MountedDevices",
				 0,KEY_QUERY_VALUE,&hKey);
    if( rstatus != ERROR_SUCCESS )
    {
      //fprintf(stdout, "RegOpenKeyEx faile (le %d)\n", GetLastError());
      return false;
    }

    std::wstring dev(volume_name);
    boost::algorithm::replace_all(dev,L"\\\\?\\",L"\\??\\");

    wchar_t in_value[512] = {0};
    int in_value_size;
    DWORD result_size = in_value_size = _countof(in_value);
    rstatus = RegQueryValueExW(hKey,dev.c_str(),NULL,NULL,(LPBYTE)in_value,&result_size);
    RegCloseKey(hKey);
    if( rstatus != ERROR_SUCCESS )
    {
      //fprintf(stdout, "RegQueryValueEx failed for %ws (le %d)\n", dev.c_str(), GetLastError());
      return false;
    }

    dev.assign(in_value);

    /* Split
     *
     *  _??_USBSTOR#Disk&Ven_Kingston&Prod_DataTraveler_2.0&Rev_PMAP#5B821700152A&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
     */
    std::wstring::size_type i0 = dev.find_first_of(L"#");

    if( i0 == std::wstring::npos )
    {
      return false;
    }
    i0 = dev.find_first_of(L"#",i0+1);
    if( i0 == std::wstring::npos )
    {
      return false;
    }

    std::wstring::size_type i1 = dev.find_first_of(L"&",i0+1);
    if( i1 == std::wstring::npos )
    {
      return false;
    }

    serial.assign(dev,i0+1,i1-i0-1);
  }

  /* Consider only storage devices */
  if( is_xp() == true )
  {
    hDevInfo = SetupDiGetClassDevsA(NULL,"STORAGE",0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
  }
  else
  {
    hDevInfo = SetupDiGetClassDevsA(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
  }

  if( hDevInfo == INVALID_HANDLE_VALUE )
  {
    return false;
  }

  wchar_t buffer[512] = {0};
  DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
  bool found = false;
  for( i = 0 ; SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData) && found == false ; i++ )
  {
    BOOL rv = FALSE;
    DWORD buffersize = sizeof(buffer);
    DWORD DataT;

    buffersize = sizeof(buffer);
    rv = SetupDiGetDeviceRegistryProperty(hDevInfo,&DeviceInfoData,SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,&DataT,
					  (PBYTE)buffer,buffersize,&buffersize);
    if( rv != TRUE )
    {
      continue;
    }

    /* This is the device? */
    wchar_t device[512] = {0};
    int dev_rv = 0;
    if( is_xp() == false )
    {
      DEVINST dev_inst;
      for( dev_rv = CM_Get_Parent(&dev_inst,DeviceInfoData.DevInst,0) ;
	   dev_rv == CR_SUCCESS ;
	   dev_rv = CM_Get_Parent(&dev_inst,dev_inst,0) )
      {
	if( CM_Get_Device_IDW(dev_inst,device,_countof(device),0) == CR_SUCCESS )
	{
	  if( wcsstr(device,serial.c_str()) )
	  {
	    wcsncpy_s(device_out,device_out_count,device,_TRUNCATE);
	    found = true;
	    break;
	  }
	}
      }/* enum parents */
    }
    else /* handle XP */
    {
      if( wcscmp(device_name,buffer) == 0 )
      {
	DEVINST parent;

	/* Move to USB mass storage device */
	dev_rv = CM_Get_Parent(&parent,DeviceInfoData.DevInst,0); /* Volume */
	dev_rv = CM_Get_Parent(&parent,parent,0);                 /* USB mass storage */

	dev_rv = CM_Get_Device_IDW(parent,device,_countof(device),0);
	if( dev_rv == CR_SUCCESS )
	{
	  wcsncpy_s(device_out,device_out_count,device,_TRUNCATE);
	  found = true;
	  break;
	}
      }
    }/* is xp? */
  }/* enum dev info */
       
  SetupDiDestroyDeviceInfoList(hDevInfo);

  return found;
}/* nl_device_to_device_instance_id */

bool nl_device_get_info( int idVendor , int idProduct ,
			 wchar_t* idVendorString , size_t idVendorString_count ,
			 wchar_t* idProductString , size_t idProductString_count  )
{
  assert( idVendorString != NULL );
  assert( idProductString != NULL );
  if( idVendorString == NULL || idProductString == NULL )
  {
    return false;
  }

  bool found = false;
  HDEVINFO hDevInfoIf;
  DWORD i = 0;
  SP_DEVICE_INTERFACE_DATA deviceInfoData;

  /* Interested in device interfaces for USB hubs */
  hDevInfoIf = SetupDiGetClassDevsA((LPGUID)&GUID_DEVINTERFACE_USB_HUB,NULL,NULL,(DIGCF_PRESENT|DIGCF_DEVICEINTERFACE));
  if( hDevInfoIf == INVALID_HANDLE_VALUE )
  {
    return false;
  }

  deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
  for( i = 0 ; SetupDiEnumDeviceInterfaces(hDevInfoIf,0,(LPGUID)&GUID_DEVINTERFACE_USB_HUB,i,&deviceInfoData) ; i++ )
  {
    DWORD requiredLength = 0;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = NULL;

    /* Determine reuqired size */
    deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    SetupDiGetDeviceInterfaceDetail(hDevInfoIf,&deviceInfoData,NULL,0,&requiredLength,NULL);

    deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc( sizeof(char) * requiredLength );
    if( deviceDetailData == NULL )
    {
      break;
    }
    memset(deviceDetailData,0x00,requiredLength);

    deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    SetupDiGetDeviceInterfaceDetail(hDevInfoIf,&deviceInfoData,deviceDetailData,requiredLength,&requiredLength,NULL);

    deviceDetailData->DevicePath[2] = '.';

    USBDevice udev;
    memset(&udev,0x00,sizeof(udev));
    udev.vendor_id = idVendor;
    udev.product_id = idProduct;

    /* Iterate over the hub and search for the given device */
    EnumerateHub(deviceDetailData->DevicePath,&udev);

    free(deviceDetailData);

    /* If the device has been found set the descriptor values for the caller */
    if( wcslen(udev.vendor_string) > 0 )
    {
      wcsncpy_s(idVendorString,idVendorString_count,udev.vendor_string,_TRUNCATE);
      wcsncpy_s(idProductString,idProductString_count,udev.product_string,_TRUNCATE);
      found = true;
      break;
    }
  }

  SetupDiDestroyDeviceInfoList(hDevInfoIf);

  return found;
}/* nl_device_get_info */

bool nl_device_from_letter( _In_ wchar_t root_drive , _Out_ USBDevice* udev )
{
  assert( udev != NULL );
  if( udev == NULL )
  {
    return false;
  }

  wchar_t dletter[32] = {0};
  wchar_t device_name[512] = {0};
  _snwprintf_s(dletter,_countof(dletter), _TRUNCATE,L"%c:",root_drive);

  wchar_t temp[1024] = {0};
  wchar_t dletters[32] = {0};
  _snwprintf_s(dletters,_countof(dletters), _TRUNCATE,L"%c:\\",root_drive);
  if( GetVolumeNameForVolumeMountPointW(dletters,temp,_countof(temp)) != TRUE )
  {
    //fprintf(stderr, "GetVolumeNameForVolumeMountPoint failed for %ws (le %d)\n",dletters,GetLastError());
    return false;
  }
  temp[wcslen(temp)-1] = (wchar_t)NULL;

  std::wstring vname(temp);

  /* Retreive device name of the storage device */
  if( QueryDosDeviceW(dletter,device_name,_countof(device_name)) == 0 )
  {
    return false;
  }

  /* Map the device name to the device instance ID */
  bool found = false;
  wchar_t device[512] = {0};
  if( nl_device_to_device_instance_id(device_name,temp,device,_countof(device)) == true )
  {
    int scan_count = 0;
    scan_count = swscanf_s(device,L"USB\\VID_%04x&PID_%04x\\%hs",
			   &udev->vendor_id,&udev->product_id,&udev->sn,sizeof(udev->sn));
    if( scan_count == 3 )
    {
      /* Map <idVendor,idProduct> to <iManufacturer,iProduct> */
      bool rv = false;
      rv = nl_device_get_info(udev->vendor_id,udev->product_id,
			      udev->vendor_string,_countof(udev->vendor_string),     /* idVendor */
			      udev->product_string,_countof(udev->product_string));  /* idProduct */  
      /* Was the device config read? */
      if( rv == true )
      {
	found = true;
      }
    }
  }/* devie -> instance id */
  return found;
}/* nl_device_from_letter */

#ifdef NL_DEVICE_TEST
int main(void)
{
  for( wchar_t i = L'f' ; i < L'z' ; i++ )
  {
    USBDevice udev;
    wchar_t root[32] = {0};
    _snwprintf_s(root,_countof(root), _TRUNCATE,L"%c:\\",L'a' + i);

    if( GetDriveTypeW(root) != DRIVE_REMOVABLE )
    {
      continue;
    }

    if( nl_device_from_letter(i,&udev) == true )
    {
      fprintf(stdout, "%c: Vendor = <%x,%ws> Product = <%x,%ws>\n",
	      i,
	      udev.vendor_id, udev.vendor_string,
	      udev.product_id, udev.product_string);
    }
    else
    {
      fprintf(stderr, "cannot map %c: to device\n", i);
    }
  }
  return 0;
}
#endif
