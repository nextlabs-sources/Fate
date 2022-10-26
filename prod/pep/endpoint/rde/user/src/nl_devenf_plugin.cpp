/*****************************************************************************
 *
 * Removable Device Enforcer Plug-in
 *
 * Provides arbitrary context for tasks.
 *
 ****************************************************************************/

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

#include "eframework/platform/cesdk_loader.hpp"
#include "nlexcept.h"
#include "nlconfig.hpp"
#include "nl_device.h"

/* CELog & CELog Policies */
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"

#include "nl_devenf_lib.h"
#include "nl_devenf_device.h"
#include "nl_devenf_sdk_wrapper.h"

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

#include <string>
#include <set>

/** NL_DEVENF_PLUGIN_EVENT_NAME
 *
 *  Global event object used to determine if there are any logged-in users on the system.
 *  When the event is in a signaled state there are users, otherwise there is *no* user
 *  on the system.
 */
#define NL_DEVENF_PLUGIN_EVENT_NAME L"Global\\NLDevEnfPluginUsersEvent"

using namespace std;

nextlabs::cesdk_loader cesdk;

/** NLDevEnfPluginContext
 *
 *  Context information for NLDevEnf plug-in.
 */
typedef struct
{
  HANDLE cancel_event;  /* Event cancel support of device arrival */
  HANDLE th;            /* Thread handle */
} NLDevEnfPluginContext;

CELog RdeLog;

/************************************************************
 * Private 
 ***********************************************************/
namespace {

/* Device path types used for evaluation */
enum
{
  DEVICE_PATH_FULL,                    /* bus + class + Vid + Pid + SN */
  DEVICE_PATH_MANUFACTURER_PRODUCT,    /* bus + class + Vid + Pid      */
  DEVICE_PATH_GENERIC                  /* bus + class                  */
};

/** NLExistLoggedInUsers
 *
 *  \brief Determine if there is at least one user logged-in to the system.
 *  \return true if there is at least one logged-in user, otherwise false.
 */
bool NLExistLoggedInUsers(void)
{
  HANDLE h = CreateEventW(NULL,TRUE,FALSE,NL_DEVENF_PLUGIN_EVENT_NAME);
  if( h != NULL && WaitForSingleObject(h,0) == WAIT_OBJECT_0 )
  {
    return true;  /* event is signaled - there is at least one user */
  }
  return false;   /* event not signaled - no user logged-in */
}/* NLExistLoggedInUsers */

/** NLIsWellKnownSid
 *
 *  \brief Determine if the given SID is well known such as local/network service.
 *  \return true when the SID is well known, otherwise false.
 */
bool NLIsWellKnownSid( _In_ const WCHAR* sid )
{
  assert( sid != NULL );
  if( sid == NULL )
  {
    return false;
  }
  static const WCHAR* known_sids[] =
    {
      L"S-1-5-18",   /* Local System : OS Account      */
      L"S-1-5-19",   /* NT Authority : Local Service   */
      L"S-1-5-20"    /* NT Authority : Network Service */
    };

  for( int i = 0 ; i < _countof(known_sids) ; i++ )
  {
    if( wcscmp(sid,known_sids[i]) == 0 )
    {
      return true;
    }
  }
  return false;
}/* NLIsWellKnownSid */

/** Anonymous user's SID **/
void AddAnonymousUser(set<wstring> &users)
{
  users.insert(L"S-1-5-7");
}/* AddAnonymousUser */

/** AddAllProcessUsers
 *
 *  \brief Add the set of all users who own a process on the system.
 */
void AddAllProcessUsers( set<wstring>& users )
{
  DWORD pids[512] = {0};
  DWORD out_size = sizeof(pids);

  if( EnumProcesses(pids,sizeof(pids),&out_size) != TRUE )
  {
    RdeLog.Log(CELOG_CRIT,L"AddAllProcessUsers: EnumProcesses failed (le %d)\n",GetLastError());
    return;
  }

  for( size_t i = 0 ; i < (out_size / sizeof(DWORD)) ; i++ )
  {
    HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pids[i]);
    if( ph == NULL )
    {
      RdeLog.Log(CELOG_CRIT,L"AddAllProcessUsers: OpenProcess failed (le %d)\n",GetLastError());
      continue;
    }

    /* Open the process token to read the owner's SID. */
    HANDLE hToken = NULL;
    if( OpenProcessToken(ph,TOKEN_READ,&hToken) != FALSE )
    {
      PTOKEN_USER pTokenUser = NULL;
      WCHAR *pSid=NULL;
      DWORD len;
      GetTokenInformation(hToken,TokenUser,NULL,0,&len);
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
	pTokenUser = (PTOKEN_USER) malloc (len);
	if( pTokenUser && GetTokenInformation(hToken,TokenUser,pTokenUser,len,&len) != FALSE )
	{
	  if( ConvertSidToStringSidW(pTokenUser->User.Sid,&pSid) != FALSE )
	  {
	    users.insert(pSid);  /* add process owner's SID into the users set */
	  }
	}
	if( pTokenUser != NULL )
	{
	  free(pTokenUser);
	}
      }
      CloseHandle(hToken);
    }/* process token */
    CloseHandle(ph);
  }  
}/* AddAllProcessUsers */

void display_device( const DeviceInfo* device )
{
  RdeLog.Log(CELOG_INFO, L"Interface        = %s\n", device->InterfaceName);
  RdeLog.Log(CELOG_INFO, L"Setup Class GUID = %s\n", device->SetupClass);
  RdeLog.Log(CELOG_INFO, L"Class Name       = %s\n", device->ClassName);
  RdeLog.Log(CELOG_INFO, L"Enumerator Name  = %s\n", device->EnumeratorName);
  const WCHAR* p = device->CompatibleIDs;
  while( *p != (WCHAR)NULL )
  {
    fwprintf(stdout, L"Compatible ID    = %s\n", p);
    p += wcslen(p) + 1;
  }
  WCHAR buf[512] = {0};
  StringFromGUID2(device->BusTypeGuid,buf,_countof(buf));
  RdeLog.Log(CELOG_INFO, L"BusTypeGuid      = %s\n", buf);
  RdeLog.Log(CELOG_INFO, L"State            = 0x%x\n", device->state);
}/* display_device */

/** eval_device
 *
 *  \brief Evaluate if a device can be used.
 *
 *  \param device (in)    Device.
 *  \param user_name (in) Name of user for evaluation.
 *  \param user_sid (in)  SID of user for evaluation.
 *  \param deny (out)     Result of policy evaluation.  true if the result was
 *                        deny, otherwise false.
 *
 *  \return Return true if the device can be used, otherwise
 *          false.
 */
bool eval_device( _In_ const DeviceInfo* device , 
		  _In_ const WCHAR *user_name , 
		  _In_ const WCHAR *user_sid , 
		  bool& deny )
{
  assert( device != NULL && user_name != NULL && user_sid != NULL );
  if( device == NULL || user_name == NULL || user_sid == NULL )
  {
    return false;
  }

  bool result;
  WCHAR serial_number[512] = {0};
  int product_id = 0, vendor_id = 0;
  int device_path = DEVICE_PATH_GENERIC;

  RdeLog.Log(CELOG_DEBUG,L"eval_device: reading device attributes\n");
  result = read_device(device,&vendor_id,&product_id,serial_number,_countof(serial_number));
  if( result == true )
  {
    device_path = DEVICE_PATH_FULL;
  }

  const WCHAR* bus_type   = L"usb";               /* default bus type */
  const WCHAR* class_name = device->ClassName;    /* default class name */

  /******************************************************************************
   * Bus Type
   *****************************************************************************/
  if( is_pcmcia(device) == true )
  {
    bus_type = L"pcmcia";
  }
  else if( is_firewire(device) == true )
  {
    bus_type = L"firewire"; /* IEEE 1394 */
    device_path = DEVICE_PATH_GENERIC;
  }
  else if( is_usb(device) == true )
  {
    bus_type = L"usb";
  }

  /******************************************************************************
   * Class Type
   *
   * Override default class name since some devices are enumerated only as USB.
   *  For example, USB mass storage devices are USB class name.
   *
   *  Compatible ID
   *   Class_06 -> image
   *   Class_08 -> storage
   *   Class_09 -> hub (WinXP)
   *   HubClass -> hub (Win2K)
   *
   *  ClassName
   *   BTW -> Bluetooth on Win2K.  This is the Broadcomm Win2K driver for support
   *   of Bluetooth devices.
   *****************************************************************************/
  if( wcsstr(device->CompatibleIDs,L"\\Class_06") )            /* Image devices */
  {
    class_name = L"image";
  }
  else if( wcsstr(device->CompatibleIDs,L"\\Class_08") )       /* USBSTOR */
  {
    class_name = L"storage";
  }
  else if( wcsstr(device->CompatibleIDs,L"\\Class_09") ||      /* USB Hub (Generic) */
	   wcsstr(device->CompatibleIDs,L"\\HubClass") )
  {
    class_name = L"hub";
    device_path = DEVICE_PATH_MANUFACTURER_PRODUCT;
  }

  if( _wcsicmp(class_name,L"BTW") == 0 )  /* BTW = Win2K Broadcomm driver */
  {
    class_name = L"Bluetooth";
  }

  /* Failed to map device to any proper class.  This means that the class could
   * not be determined by the Compatible IDs reported.  The class mapping is
   * handled in the above source.
   */
  if( _wcsicmp(class_name,L"USB") == 0 )
  {
    return false;
  }

  /******************************************************************************
   * Generate device resource name
   *****************************************************************************/
  WCHAR file[MAX_PATH] = {0};
  const WCHAR* device_prefix = L"device://";
  switch( device_path )
  {
    case DEVICE_PATH_FULL:
      // FORMAT: device://[bus]/[class]/[vendor_id]/[product_id]/[serial_number]
      _snwprintf_s(file, _countof(file), _TRUNCATE, L"%s%s/%s/%04x/%04x/%s",
	       device_prefix,bus_type,class_name,vendor_id,product_id,serial_number);
      break;

    case DEVICE_PATH_MANUFACTURER_PRODUCT:
      // FORMAT: device://[bus]/[class]/[vendor_id]/[product_id]/GENERIC
      _snwprintf_s(file, _countof(file), _TRUNCATE, L"%s%s/%s/%04x/%04x/GENERIC",
	       device_prefix,bus_type,class_name,vendor_id,product_id);
      break;

    case DEVICE_PATH_GENERIC:
      // FORMAT: device://[bus]/[class]/GENERIC
      _snwprintf_s(file, _countof(file), _TRUNCATE, L"%s%s/%s/GENERIC",device_prefix,bus_type,class_name);
      break;
  }/* switch */

  wchar_t idVendorString[64] = {0};
  wchar_t idProductString[64] = {0};
  bool info_result = false;

  RdeLog.Log(CELOG_DEBUG,L"eval_device: reading vendor/product strings\n");
  info_result = nl_device_get_info(vendor_id,product_id,                        /* IDs */
				   idVendorString,_countof(idVendorString),     /* Vendor */
				   idProductString,_countof(idProductString));  /* Product */

  RdeLog.Log(CELOG_DEBUG,L"eval_device: %s : '%s' (%d) : '%s' (%d)\n", file,
	     idVendorString, vendor_id, idProductString, product_id);

  RdeDevice dev;
  memset(&dev,0x00,sizeof(dev));

  wcsncpy_s(dev.bus_name,_countof(dev.bus_name),bus_type,_TRUNCATE);
  wcsncpy_s(dev.class_name,_countof(dev.class_name),class_name,_TRUNCATE);
  dev.id_vendor = vendor_id;
  dev.id_product = product_id;
  wcsncpy_s(dev.id_vendor_string,_countof(dev.id_vendor_string),idVendorString,_TRUNCATE);
  wcsncpy_s(dev.id_product_string,_countof(dev.id_product_string),idProductString,_TRUNCATE);
  wcsncpy_s(dev.serial_number,_countof(dev.serial_number),serial_number,_TRUNCATE);

  PolicyEval(file,&dev,user_name,user_sid,deny);

  return deny;
}/* eval_device */

}/*--end--namespace */

/** evaluate_devices
 *
 *  \brief Evaluate attached devices against a set of users.
 */
static void evaluate_devices( const set<wstring>& users )
{
  DeviceInfo* devices = NULL;
  int status = 0, num_devices = 0;

  /* Determine required storage for devices. */
  status = Device_GetDevices(devices,&num_devices);
  if( status == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
  {
    /* Allocate 4 additional device records to avoid iteration when a device
     * comes on to the system between query for size and retreival of list.
     */
    devices = new (std::nothrow) DeviceInfo[num_devices + 4];
    if( devices == NULL )
    {
      return;
    }
  }
  else
  {
    RdeLog.Log(CELOG_DEBUG,L"nl_devenf_plugin_worker: Device_GetDevices failed (status %d)\n",status);
    return;  /* Device_GetDevices failed - expected out size calculation */
  }

  status = Device_GetDevices(devices,&num_devices);
  RdeLog.Log(CELOG_DEBUG,L"nl_devenf_plugin_worker: num_devices = %d (status %d)\n",num_devices,status);
  for( int i = 0 ; status && i < num_devices ; i++ )
  {
    bool result = false, deny = false;

    /* If the device is already disabled ignore it. */
    if( (devices[i].state & NL_DEVENF_DEVICE_STATE_ENABLED) == 0 )
    {
      continue;
    }

#if defined(_DEBUG) || defined(DEBUG)
    display_device(&devices[i]);
#endif

    /* Iterate each user and evaluate if each user is allowed to use this device */
    set<wstring>::const_iterator it;
    for( it = users.begin() ; it != users.end() ; it++ )
    {
      /* Skip well known accounts such as local/network service */
      if( NLIsWellKnownSid(it->c_str()) == true )
      {
	continue;
      }

      RdeLog.Log(CELOG_DEBUG,L"NLDevEnf-Plugin: Evaluate device USER = (%s)\n",it->c_str());

      /********************************************************************************
       * Retreive the account name and domain.
       *******************************************************************************/
      WCHAR user_name[512] = {0};
      WCHAR user_domain[512] = {0};
      PSID sid = (PSID)NULL;
      BOOL rv = FALSE;

      if( ConvertStringSidToSidW(it->c_str(),&sid) != FALSE )
      {
	SID_NAME_USE name_use;
	DWORD user_name_size = _countof(user_name);
	DWORD user_domain_size = _countof(user_domain);
	rv = LookupAccountSidW(NULL,sid,user_name,&user_name_size,user_domain,&user_domain_size,&name_use);
	LocalFree(sid);
      }
      if( rv == FALSE )  /* cannot retreive account name? */
      {
	continue;
      }

      /********************************************************************************
       * Build fully qualified name using account name and domain.  Evaluate the given
       * device with the fully qualified name.
       *******************************************************************************/
      wstring full_user_name;
      full_user_name  = user_domain;
      full_user_name += L"\\";
      full_user_name += user_name;

      /* Composite devices are ignored.  These devices are caught because their class is
       * USB which is also used for storage.
       */
      if( is_composite_device(&devices[i]) == true )
      {
	RdeLog.Log(CELOG_DEBUG,L"evaluate_devices: ignoring composite device\n");
	continue;
      }

      result = eval_device(&devices[i],full_user_name.c_str(),it->c_str(),deny);
      if( result == true && deny == true )
      {
	RdeLog.Log(CELOG_DEBUG,L"evaluate_devices: disable device\n");
	Device_Disable(&devices[i]);
      }
    }/*end of user interation */
  }/*end of device interation */
  delete [] devices;
}/* evaluate_devices */

/** evaluate_devices_against_users
 *
 *  \brief Evaluate all devices against all system users.
 *
 *  \return true.
 */
static bool evaluate_devices_against_users(void)
{
  set<wstring> users; 

  /* If there is not a user present include the 'anonymous' user */
  if( NLExistLoggedInUsers() == false )
  {
    AddAnonymousUser(users);
  }
  AddAllProcessUsers(users);  /* any user that owns a process */

  if( users.size() > 0 )
  {
    evaluate_devices(users);
  }
  return true;
}/* evaluate_devices_against_users */

/** nl_devenf_plugin_worker
 *
 *  \brief Worker thread for device enforcement.  Wait for device arrival, collect
 *         system users and evaluate the device against each user.
 */
static DWORD WINAPI nl_devenf_plugin_worker( LPVOID in_context )
{
  assert( in_context != NULL );
  NLDevEnfPluginContext* context = (NLDevEnfPluginContext*)in_context;

  RdeLog.Log(CELOG_INFO,L"nl_devenf_plugin_worker: starting\n");

  evaluate_devices_against_users();
  for( ; context ; )
  {
    /* Wait for device arrival.  If a device has arrived before this thread was
     * started Device_WaitForArrival will not block.
     */
    if( Device_WaitForArrival(context->cancel_event) == 0 )
    {
      break;
    }
    RdeLog.Log(CELOG_INFO,L"nl_devenf_plugin_worker: device arrival\n");

    /* If cancellation occurs from the unload callback the cancel
     * event will be in a signaled stage.  Test for signaled state.
     */
    if( WaitForSingleObject(context->cancel_event,0) == WAIT_OBJECT_0 )
    {
      RdeLog.Log(CELOG_INFO,L"nl_devenf_plugin_worker: cancel\n");
      break;
    }

    RdeLog.Log(CELOG_INFO,L"nl_devenf_plugin_worker: evaluate devices\n");
    evaluate_devices_against_users();
  }/* for */
  return 0;
}/* nl_devenf_plugin_worker */

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
  assert( in_context != NULL );
  if( in_context == NULL )
  {
    return 0;
  }

  int result = 0; /* default to failed */
  NLDevEnfPluginContext* context = NULL;

  try
  {
    context = new NLDevEnfPluginContext;
  }
  catch(...)
  {
  	if (NULL != context)
  		delete context;
    return 0; /* failed */
  }


  std::wstring install_path;
  wchar_t comp_root[MAX_PATH] = {0}; // component root for RDE

  /* Determine install path for RDE */
  if( NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Removable Device Enforcer",comp_root,_countof(comp_root)) == true )
  {
    install_path.assign(comp_root);
  }

  if( NLConfig::IsDebugMode() == true )
  {
    char file[MAX_PATH] = {0};
    _snprintf_s(file,_countof(file), _TRUNCATE,"%ws\\diags\\logs\\debug.log",install_path.c_str());
    CELogPolicy_File* file_policy = NULL;
    try
    {
      file_policy = new CELogPolicy_File(file);
    }
    catch(...)
    {
    	if (NULL != file_policy)
    	delete file_policy;
      return 0; /* failed */
    }

    RdeLog.SetPolicy(file_policy);
    RdeLog.SetLevel(CELOG_DEBUG);
    
  }
  
 
   CELogPolicy_WinDbg* dbgPolicy = NULL;
   try
   {
   		dbgPolicy =	new CELogPolicy_WinDbg();
  }
   catch(...)
   {
   	if (NULL != dbgPolicy)
   		delete dbgPolicy;
   	return 0;
   }
  RdeLog.SetPolicy( /*new CELogPolicy_WinDbg()*/dbgPolicy );

  RdeLog.Log(CELOG_INFO,L"PluginEntry: starting\n");

  /* If the device control feature is not present, then fail the plug-in load. */
  nextlabs::feature_manager feat;
  feat.open();
  if( feat.is_enabled(NEXTLABS_FEATURE_REMOVABLE_DEVICE) == false )
  {
    RdeLog.Log(CELOG_INFO,L"PluginEntry: Removable device control is disabled.\n");
    return 1;
  }

  *in_context = (void*)context;
  memset(context,0x00,sizeof(NLDevEnfPluginContext));

  context->cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);
  if( context->cancel_event == NULL )
  {
    RdeLog.Log(CELOG_CRIT,L"PluginEntry: CreateEvent failed (le %d)\n",GetLastError());
    goto PluginEntry_complete;
  }

  if( PolicyInit() == false )
  {
    RdeLog.Log(CELOG_CRIT,L"PluginEntry: SDK load failed\n");
    goto PluginEntry_complete;
  }

  context->th = CreateThread(NULL,0,nl_devenf_plugin_worker,*in_context,0,NULL);
  if( context->th == NULL )
  {
    RdeLog.Log(CELOG_CRIT,L"PluginEntry: CreateThread failed (le %d)\n",GetLastError());
    goto PluginEntry_complete;
    
  }

  result = 1; /* success */

 PluginEntry_complete:

  RdeLog.Log(CELOG_INFO,L"PluginEntry: complete (result %d)\n",result);

  if( result == 0 ) /* failed */
  {
    if( context->cancel_event != NULL )
    {
      CloseHandle(context->cancel_event);
    }
  }
  
  return result;
}/* PluginEntry */

extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
  assert( in_context != NULL );
  if( in_context == NULL )
  {
    RdeLog.Log(CELOG_INFO,L"PluginUnload: context is NULL\n");
    return 0;
  }

  RdeLog.Log(CELOG_INFO,L"PluginUnload\n");

  NLDevEnfPluginContext* context = (NLDevEnfPluginContext*)in_context;

  SetEvent(context->cancel_event);            /* signal thread to cancel */

  RdeLog.Log(CELOG_INFO,L"PluginUnload: waiting for thread\n");
  WaitForSingleObject(context->th,INFINITE);  /* wait for thread */
  CloseHandle(context->th);

  RdeLog.Log(CELOG_INFO,L"PluginUnload: unloading SDK\n");
  cesdk.unload();

  delete in_context;

  RdeLog.Log(CELOG_INFO,L"PluginUnload: complete\n");

  return 1;
}/* PluginUnload */

/**********************************************************************************
 *
 * Events for Winlogon service.
 *
 * All event callbacks are wrapped with "try_" to avoid an exception.  If the case
 * of any exceptions a minidump should be generated.
 *
 *********************************************************************************/

/* Counter for number of users on the system.  Winlogon service loads this library
 * and issues Logon/Logoff event which increments and decrements the logon counter,
 * respectively.
 */
static LONG logon_user_count = 0L;

static void try_LogonEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  InterlockedIncrement(&logon_user_count);

  HANDLE event = CreateEventW(NULL,TRUE,FALSE,NL_DEVENF_PLUGIN_EVENT_NAME);
  if( event != NULL )
  {
    SetEvent(event);
  }
}/* try_LogonEvent */

extern "C" __declspec(dllexport) void LogonEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  __try
  {
    try_LogonEvent(pInfo);
  }
  __except( NLEXCEPT_FILTER() )
  {
    /* empty */
  }
}/* LogonEvent */

static void try_StartShellEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  if( PolicyInit() == false )
  {
    RdeLog.Log(CELOG_CRIT,L"try_StartShellEvent: PolicyInit failed\n");
    return;
  }

  if( pInfo->hToken != NULL && pInfo->UserName != NULL )
  {
    PTOKEN_USER pTokenUser = NULL;
    WCHAR *pSid=NULL;
    DWORD len;

    /*Get logged user's SID */
    GetTokenInformation(pInfo->hToken, TokenUser, NULL, 0, &len);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      pTokenUser = (PTOKEN_USER) malloc (len);
      if( pTokenUser && GetTokenInformation(pInfo->hToken,TokenUser,pTokenUser,len,&len) != FALSE )
      {
	if( ConvertSidToStringSidW(pTokenUser->User.Sid,&pSid) != FALSE )
	{
	  set<wstring> users;
	  users.insert(pSid);
	  evaluate_devices(users);  
	}
      }
      if( pTokenUser != NULL )
      {
	free(pTokenUser);
      }
    }
  }/* token != NULL */
  cesdk.unload();
}/* try_StartShellEvent */

/* The SDK must be dynamically loaded for policy evaluation.  Use of PolicyInit and
 * PolicyUnload are symmetric about the evaluation.
 */
extern "C" __declspec(dllexport) void StartShellEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  __try
  {
    try_StartShellEvent(pInfo);
  }
  __except( NLEXCEPT_FILTER() )
  {
    /* empty */
  }
}/* StartShellEvent */

void try_device_query(LPCWSTR lpszSid)
{
	if(!lpszSid)
	{
		return;
	}

	if( PolicyInit() == false )
	{
		RdeLog.Log(CELOG_CRIT,L"try_StartShellEvent: PolicyInit failed\n");
		return;
	}

	
	//Try to do evaluation with the SID from parameter.
	set<wstring> users;
	users.insert(lpszSid);

	RdeLog.Log(CELOG_CRIT, L"RDE::This was called by logon_detection_win7.exe, SID: %s", lpszSid);

	evaluate_devices(users);  
	
	cesdk.unload();
}

extern "C" __declspec(dllexport) void device_query( LPCWSTR lpszSid )
{
	__try
	{
		try_device_query(lpszSid);
	}
	__except( NLEXCEPT_FILTER() )
	{
	
	}
}

static void try_LogoffEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  if( InterlockedDecrement(&logon_user_count) == 0 )
  {
    /* Set signaled state for event to indicate htere are no logged-in users */
    HANDLE h = CreateEventW(NULL,TRUE,FALSE,NL_DEVENF_PLUGIN_EVENT_NAME);
    if( h != NULL )
    {
      ResetEvent(h);
    }
  }
}/* try_LogoffEvent */

extern "C" __declspec(dllexport) void LogoffEvent( PWLX_NOTIFICATION_INFO pInfo )
{
  __try
  {
    try_LogoffEvent(pInfo);
  }
  __except( NLEXCEPT_FILTER() )
  {
    /* empty */
  }
}/* LogoffEvent */
