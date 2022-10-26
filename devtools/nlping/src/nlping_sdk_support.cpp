#include <windows.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <time.h>

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#  include <boost/scoped_ptr.hpp>
#pragma warning( pop )

#include "nlconfig.hpp"
#include "nlping_sdk_support.hpp"
#include "htimer.hpp"

static const wchar_t* WDE_PLATFORM_RESOURCE_DOCUMENT = L"fso";     // document resource
static const wchar_t* WDE_PLATFORM_RESOURCE_PORTAL   = L"portal";  // portal resource

/** GetResourceType
 *
 *  \brief Get the resource type of a given resource.
 *
 *  \return A string value of the resource type.  Default is document type
 *          WDE_PLATFORM_RESOURCE_DOCUMENT.
 */
static const __checkReturn wchar_t* GetResourceType( __in const std::wstring& in_resource )
{
  if( boost::algorithm::contains(in_resource,L"http:") == true )
  {
    return WDE_PLATFORM_RESOURCE_PORTAL;
  }
  return WDE_PLATFORM_RESOURCE_DOCUMENT; /* default when not portal */
}/* GetResourceType */

/** CreateResourceAttributes
 *
 *  \brief Create attributes based on the resource type and modification time.
 *
 *  \param sdk (in)              CE SDK handle for access to string management.
 *  \param in_resource (in)      Resource path.
 *  \param in_resource_type (in) Resource type (i.e. WDE_PLATFORM_RESOURCE_PORTAL)
 *  \param mod_time (opt-in)     Modification time if any.  This parameter may be NULL.
 *
 *  \return Attributes for the resource if there are any.  If the resource is empty
 *          there are no attributes and the return value is NULL.
 */
static __checkReturn CEAttributes* CreateResourceAttributes( const CESDK::Handle &sdk ,
							     __in const std::wstring& in_resource ,
							     __in const wchar_t* in_resource_type ,
							     const wchar_t* mod_time )
{
  assert( in_resource_type != NULL );

  if( in_resource.empty() == true )
  {
    return NULL;
  }

  CEAttributes* attr = new CEAttributes;
  memset(attr,0x00,sizeof(CEAttributes));

  attr->attrs = new CEAttribute[3];                /* 3 attributes by default */
  memset(attr->attrs,0x00,sizeof(CEAttribute)*3);

  attr->count = 0;
  /******************************************************************************
   * Modify time
   *****************************************************************************/
  if( mod_time != NULL )
  {
    attr->attrs[attr->count].key   = sdk.CEM_AllocateString(CE_ATTR_LASTWRITE_TIME);
    attr->attrs[attr->count].value = sdk.CEM_AllocateString(mod_time);
    attr->count++;
  }

  return attr;
}/* CreateResourceAttributes */

/** FreeResourceAttributes
 *
 *  \brief Free resource attributes.
 */
static void FreeResourceAttributes( const CESDK::Handle &sdk ,
				    __in CEAttributes* attrs )
{
  if( attrs != NULL )
  {
    for( int i = 0 ; i < attrs->count ; i++ )
    {
      sdk.CEM_FreeString(attrs->attrs[i].key);
      sdk.CEM_FreeString(attrs->attrs[i].value);
    }
    delete [] attrs->attrs;
    delete attrs;
  }
}/* FreeResourceAttributes */

time_t WinTime2JavaTime(SYSTEMTIME* pSysTime)
{
  assert( pSysTime != NULL );
  if( NULL == pSysTime)
  {
	  return 0;
  }

  time_t rtTime = 0;
  tm     rtTM;

  rtTM.tm_year = pSysTime->wYear - 1900;
  rtTM.tm_mon  = pSysTime->wMonth - 1;
  rtTM.tm_mday = pSysTime->wDay;
  rtTM.tm_hour = pSysTime->wHour;
  rtTM.tm_min  = pSysTime->wMinute;
  rtTM.tm_sec  = pSysTime->wSecond;
  rtTM.tm_wday = pSysTime->wDayOfWeek;
  rtTM.tm_isdst = -1;     // Let CRT Lib compute whether DST is in effect,
  // assuming US rules for DST.
  rtTime = mktime(&rtTM); // get the second from Jan. 1, 1970

  if (rtTime == (time_t) -1)
  {
    if (pSysTime->wYear <= 1970)
    {
      // Underflow.  Return the lowest number possible.
      rtTime = (time_t) 0;
    }
    else
    {
      // Overflow.  Return the highest number possible.
      rtTime = (time_t) _I64_MAX;
    }
  }
  else
  {
    rtTime*= 1000;          // get millisecond
  }
  
  return rtTime;
}

bool GetFileLastModifyTime(const WCHAR* wzFileName, WCHAR* wzDateTime, int BufSize, time_t& mod_time)
{
  assert( wzFileName != NULL && wzDateTime != NULL );
  if( (NULL == wzFileName) || (NULL == wzDateTime) )
  {
	  return false;
  }

  HANDLE hFile = INVALID_HANDLE_VALUE;
  memset(wzDateTime, 0, BufSize*sizeof(WCHAR));

  hFile = CreateFileW(wzFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE )
  {
    return false;
  }

  bool result = false;
  FILETIME ftLastModify;  memset(&ftLastModify, 0, sizeof(FILETIME));
  if(GetFileTime(hFile, NULL, NULL, &ftLastModify))
  {
    FILETIME ftLocalLastModify;  memset(&ftLocalLastModify, 0, sizeof(FILETIME));
    SYSTEMTIME stModifyTime;    memset(&stModifyTime, 0, sizeof(SYSTEMTIME));
    // We should pass UTC time, so don't convert it to local time
    //FileTimeToLocalFileTime(&ftLastModify, &ftLocalLastModify);
    if(FileTimeToSystemTime(&ftLastModify, &stModifyTime))
    {
      time_t tmModify;
      mod_time = tmModify = WinTime2JavaTime(&stModifyTime);
      swprintf(wzDateTime, BufSize, L"%I64d", tmModify);
      result = true;
    }
    CloseHandle(hFile);
  }
  return result;
}/* GetFileLastModifyTime */

bool CESDK::SetupConnection( Handle &sdk , const WCHAR* application , int timeout_ms )
{
  bool bSuccess=true;	

  CEResult_t ret;
  CEApplication app;      memset(&app, 0, sizeof(CEApplication));
  CEUser user;            memset(&user, 0, sizeof(CEUser));

  ret = sdk.CECONN_Initialize(app,user,NULL,&sdk.connHandle,timeout_ms);
  if(ret != CE_RESULT_SUCCESS)
  {
    bSuccess=false;
  }

  sdk.CEM_FreeString(app.appPath);

  sdk.connected = bSuccess;

  return bSuccess;
}/* CESDK::SetupConnection */

/* Hooks for the current thread are disabled for the entire call.
 */
BOOL CESDK::DoPolicyEvaluation(Handle &sdk, 
			       CEAction_t action,
			       CENoiseLevel_t noise_level,
			       const WCHAR *application,
			       const WCHAR *usid,
			       const WCHAR *in_source, 
			       const WCHAR *in_target,
			       bool ignore_obligations,
			       int timeout_ms ,
			       double& sdk_time )
{
  /* If not connected attempt to connect.  If connection does not succeed return
   * allow to prevent effective disablment of the current process by denies.
   */
  if( sdk.connected == false )
  {
    /* Prevent setting up SDK connection when SDK connect control is in a signaled state.  This
     * can occur when LoadLibraryXXX is called to avoid a deadlock in DllMain when an SDK
     * connection is initiated.
     */
    if( CESDK::SetupConnection(sdk,application,1000) != true )
    {
      return TRUE;
    }
    sdk.connected = true;
  }

  std::wstring final_source(in_source);  // default from caller
  std::wstring final_target;             // default empty - may be empty (NULL)

  if( in_target != NULL ) // caller provided target path?
  {
    final_target.assign(in_target);
  }

  const wchar_t* source_res_type = GetResourceType(final_source);  // source resource type
  const wchar_t* target_res_type = GetResourceType(final_target);  // target resource type
  assert( source_res_type != NULL && target_res_type != NULL );
  if(NULL==source_res_type || NULL==target_res_type)
      return TRUE;

  // Get modified date
  WCHAR wzDateTime[200] = {0};
  time_t mod_time = 0;
  bool have_mod_time = GetFileLastModifyTime(final_source.c_str(),wzDateTime,_countof(wzDateTime),mod_time);
  if( have_mod_time == false ) /* default when modify time cannot be determined */
  {
    wcsncpy_s(wzDateTime,_countof(wzDateTime),L"0123456789",_TRUNCATE);
  }

  const wchar_t* ce_action_name = L"OPEN";

  CEAttributes* source_attrs = CreateResourceAttributes(sdk,final_source.c_str(),source_res_type,wzDateTime);
  CEAttributes* target_attrs = CreateResourceAttributes(sdk,final_target.c_str(),target_res_type,NULL);

  /* Honor caller's request to either perform or ignore obligations. */
  CEBoolean perf_obs = CETrue;
  if( ignore_obligations == true )
  {
    perf_obs = CEFalse;
  }

  /* Create resource attributes for source and target */
  CEResource* res_source = sdk.CEM_CreateResourceW(final_source.c_str(),source_res_type);
  CEResource* res_target = sdk.CEM_CreateResourceW(final_target.c_str(),target_res_type);

  CEApplication* app = NULL;
  if( application != NULL )
  {
    app = new CEApplication;
    memset(app,0x00,sizeof(CEApplication));
    app->appPath = sdk.CEM_AllocateString(application);
  }

  CEString ce_string_action_name = sdk.CEM_AllocateString(ce_action_name);

  CEEnforcement_t enforcement;
  CEResult_t result;
  nextlabs::high_resolution_timer ht;
  ht.start();
  result = sdk.CEEVALUATE_CheckResources(sdk.connHandle,          /* connection handle */
					 ce_string_action_name,   /* action name */
					 res_source,              /* source */
					 source_attrs,            /* source - attributes */
					 res_target,              /* target */
					 target_attrs,            /* target - attributes */
					 NULL,                    /* user */
					 NULL,                    /* user - attributes */
					 app,                     /* application */
					 NULL,                    /* application attributes */
					 NULL,                    /* receipients */
					 0,                       /* number of receipients */
					 0,                       /* IP */
					 perf_obs,                /* perform obligations? */
					 noise_level,             /* noise level */
					 &enforcement,            /* enforcement */
					 timeout_ms);             /* timeout in milliseconds */

  sdk_time = ht.diff();
  sdk.CEM_FreeString(ce_string_action_name);
  FreeResourceAttributes(sdk,source_attrs);
  FreeResourceAttributes(sdk,target_attrs);
  if( app != NULL )
  {
    sdk.CEM_FreeString(app->appPath);
    delete app;
  }
  sdk.CEM_FreeResource(res_source);
  sdk.CEM_FreeResource(res_target);
 
  BOOL bAllow = TRUE; /* default */
  if (result == CE_RESULT_SUCCESS)
  {
    bAllow = (enforcement.result == CEAllow); // Enforcement is allow?
  }

  if( result == CE_RESULT_CONN_FAILED )
  {
    sdk.connected = false;
    sdk.CECONN_Close(sdk.connHandle,timeout_ms);
    memset(&sdk.connHandle,0x00,sizeof(sdk.connHandle));
  }

  sdk.CEEVALUATE_FreeEnforcement(enforcement);

  return bAllow;
}/* CESDK::DoPolicyEvaluation */

bool CESDK::Load( Handle* handle )
{
  assert( handle != NULL );
  if (NULL == handle)
  {
	return false;
  }

  WCHAR enforcer_root[MAX_PATH] = {0};
  if( NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller",enforcer_root,_countof(enforcer_root)) == false )
  {
    fprintf(stderr, "nlping: install path for SDK failed\n");
    return false;
  }

  wcscat_s(enforcer_root,L"Policy Controller\\");

  /****************************************************************
   * Load SDK libraries.  Order is critical due to link depends.
   ***************************************************************/
  sdk_lib_t sdk_libs[] =
  {
    CEBRAIN,
    CECEM,
    CEMARSHAL,
    CETRANSPORT,
    CEPEPMAN,
    CECONN,
    CEEVAL
  };

  bool status = false;
  handle->libmap.clear();
  for( int i = 0 ; i < _countof(sdk_libs) ; i++ )
  {
    HMODULE hlib = NULL;
    WCHAR libpath[MAX_PATH] = {0};
    _snwprintf_s(libpath,_countof(libpath),L"%sbin\\%s",enforcer_root,sdk_libs[i].libname);
    hlib = LoadLibraryW(libpath);
    if( hlib == NULL )
    {
      goto Load_done;
    }
    handle->libmap[sdk_libs[i].libname] = hlib;
  }

  /****************************************************************
   * Retrieve all SDK function addresses.
   ***************************************************************/
  handle->CECONN_Initialize          = (CECONN_Initialize_t)GetProcAddress(handle->libmap[CECONN],"CECONN_Initialize");
  handle->CECONN_DLL_Activate        = (CECONN_DLL_Activate_t)GetProcAddress(handle->libmap[CECONN],"CECONN_DLL_Activate");
  handle->CECONN_Close               = (CECONN_Close_t)GetProcAddress(handle->libmap[CECONN],"CECONN_Close");
  handle->CECONN_DLL_Deactivate      = (CECONN_DLL_Deactivate_t)GetProcAddress(handle->libmap[CECONN],"CECONN_DLL_Deactivate");
  
  handle->CEEVALUATE_CheckFile       = (CEEVALUATE_CheckFile_t)GetProcAddress(handle->libmap[CEEVAL],"CEEVALUATE_CheckFile");
  handle->CEEVALUATE_CheckResources  = (CEEVALUATE_CheckResources_t)GetProcAddress(handle->libmap[CEEVAL],"CEEVALUATE_CheckResources");
  handle->CEEVALUATE_FreeEnforcement = (CEEVALUATE_FreeEnforcement_t)GetProcAddress(handle->libmap[CEEVAL],"CEEVALUATE_FreeEnforcement");

  handle->CEM_AllocateString         = (CEM_AllocateString_t)GetProcAddress(handle->libmap[CECEM],"CEM_AllocateString");
  handle->CEM_GetString              = (CEM_GetString_t)GetProcAddress(handle->libmap[CECEM],"CEM_GetString");
  handle->CEM_FreeString             = (CEM_FreeString_t)GetProcAddress(handle->libmap[CECEM],"CEM_FreeString");
  handle->CEM_CreateResourceW        = (CEM_CreateResourceW_t)GetProcAddress(handle->libmap[CECEM],"CEM_CreateResourceW");
  handle->CEM_FreeResource           = (CEM_FreeResource_t)GetProcAddress(handle->libmap[CECEM],"CEM_FreeResource");
  
  if( handle->CECONN_Initialize != NULL &&
      handle->CECONN_DLL_Activate &&
      handle->CECONN_Close != NULL &&
      handle->CECONN_DLL_Deactivate &&
      handle->CEEVALUATE_CheckFile != NULL &&
      handle->CEEVALUATE_CheckFile != NULL &&
      handle->CEEVALUATE_FreeEnforcement != NULL &&
      handle->CEM_AllocateString != NULL &&
      handle->CEM_FreeString != NULL &&
      handle->CEM_FreeResource != NULL )
  {
    status = true;
  }

  handle->connected = false;

 Load_done:
  /* If any of the functions cannot be found the SDK cannot be used. */
  if( status == false )
  {
    Unload(handle);
  }/* if failed */
  
  return status;
}/* Load */

void CESDK::Unload( Handle* handle )
{
  assert( handle != NULL );
  if(NULL == handle)
  {
	  return;
  }
  std::map<std::wstring,HMODULE>::iterator it;
  for( it = handle->libmap.begin() ; it != handle->libmap.end() ; ++it )
  {
    FreeLibrary(it->second);
    it->second = NULL;
  }
}/* Unload */
