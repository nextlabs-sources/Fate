
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <tchar.h>
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"
#include "eframework/timer/timer_high_resolution.hpp"
#include "celog.h"
#include "CEsdk.h"
#include "nl_devenf_sdk_wrapper.h"
#include "nlconfig.hpp"

#define TIMEOUT_MS (1000 * 10)  /* 10 seconds */

extern CELog RdeLog;
extern nextlabs::cesdk_loader cesdk;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH) && L'\0'!=szDir[0])
	{
		if(szDir[wcslen(szDir) - 1] != L'\\')
		{
			wcsncat_s(szDir, MAX_PATH, L"\\", _TRUNCATE);
		}

#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

/** LoadSDK
 *
 *  Load the SDK libraries and functions for use.
 */
bool PolicyInit(void)
{
	if (cesdk.is_loaded() == false)
	{
		std::wstring strComDir = GetCommonComponentsDir();
		wchar_t buf[1024] = {0};
		_snwprintf_s(buf, 1024, _TRUNCATE, L"try to load SDK DLLs under folder: %s\n", strComDir.c_str());
		RdeLog.Log(CELOG_DEBUG,buf);

		if( cesdk.load(strComDir.c_str()) == false )
    return false;
  }
	RdeLog.Log(CELOG_DEBUG,L"devenf_plugin load sdk dlls successfully\n");
    return true;

}/* LoadSDK */

/** CreateResourceAttributes
 *
 *  \brief Create attributes based on the resource type and modification time.
 *
 *  \param sdk (in)              CE SDK handle for access to string management.
 *  \param in_resource (in)      Resource path.
 *  \param in_resource_type (in) Resource type (i.e. WDE_PLATFORM_RESOURCE_PORTAL)
 *
 *  \return Attributes for the resource if there are any.  If the resource is empty
 *          there are no attributes and the return value is NULL.
 */
_Check_return_ static CEAttributes* CreateResourceAttributes( _In_ const RdeDevice* device )
{
  wchar_t temp[64] = {0};
  CEAttributes* attr = nextlabs::cesdk_attributes::create();
  bool rv;

  rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Connection",device->bus_name);       /* Connection */
  RdeLog.Log(CELOG_DEBUG,L"CreateResourceAttributes: <Connection,%s>\n",device->bus_name);

  rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Class",device->class_name);          /* Class */
  RdeLog.Log(CELOG_DEBUG,L"CreateResourceAttributes: <Class,%s>\n",device->class_name);

  if( device->id_vendor != 0x0 && wcslen(device->id_vendor_string) > 0 )
  {
    rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Vendor",device->id_vendor_string); /* Vendor (string) */

    _snwprintf_s(temp,_countof(temp), _TRUNCATE,L"%04x",device->id_vendor);                           /* Vendor (ID) */
    rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Vendor ID",temp);

    RdeLog.Log(CELOG_DEBUG,L"CreateResourceAttributes: <Vendor,%s> <Vendor ID,%s>\n",device->id_vendor_string, temp);
  }

  if( device->id_vendor != 0x0 && wcslen(device->id_vendor_string) > 0 )
  {
    rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Product",device->id_product_string);    /* Product (string) */

    _snwprintf_s(temp,_countof(temp), _TRUNCATE,L"%04x",device->id_product);                               /* Product (ID) */
    rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Product ID",temp);                      

    RdeLog.Log(CELOG_DEBUG,L"CreateResourceAttributes: <Product,%s> <Product ID,%s>\n",device->id_product_string, temp);
  }

  if( wcslen(device->serial_number) > 0 )
  {
    rv = nextlabs::cesdk_attributes::add(cesdk,attr,L"Serial Number",device->serial_number); /* Serial Number */
    RdeLog.Log(CELOG_DEBUG,L"CreateResourceAttributes: <Serial Number,%s>\n",device->serial_number);
  }

  return attr;
}/* CreateResourceAttributes */

/** PolicyEval
 *
 *  \brief Perform policy evaluation for a device resource.
 *
 *  \param resource_from (in) (Device) resource.
 *  \param user_name (in)     User name for evaluation.
 *  \param user_sid (in)      User SID for evaluation.
 *  \param deny (out)         Result of evaluation.
 *
 *  \return false on error.
 */
bool PolicyEval( _In_ const wchar_t* resource_from ,
		 _In_ const RdeDevice* device ,
		 _In_ const wchar_t* user_name ,
		 _In_ const wchar_t* user_sid ,
		 bool& deny )
{
  assert( resource_from != NULL && user_name != NULL && user_sid != NULL );

  CEHandle handle = NULL;
  CEUser user;
  CEApplication app;
  CEString action = cesdk.fns.CEM_AllocateString(L"ATTACH_DEVICE");

  memset(&app,0x00,sizeof(app));
  app.appName = cesdk.fns.CEM_AllocateString(L"nl_devenf_plugin");  // Application
  app.appPath = cesdk.fns.CEM_AllocateString(L"");

  memset(&user,0x00,sizeof(user));
  user.userName = cesdk.fns.CEM_AllocateString(user_name);
  user.userID   = cesdk.fns.CEM_AllocateString(user_sid);

  CEResult_t res = CE_RESULT_SUCCESS;
 
  for (int i = 0; i < 30; i++)
  {
  res = cesdk.fns.CECONN_DLL_Activate(app,user,NULL,&handle,TIMEOUT_MS); 
	if (res == CE_RESULT_CONN_FAILED)
	{
		Sleep(1000);
		continue;
	}
	else
		break;
	
  }
  
  if( res != CE_RESULT_SUCCESS )
  {
    return false;
  }

  CEApplication appRun;
  CEResource* source = cesdk.fns.CEM_CreateResourceW(resource_from,L"device");
  CEAttributes* sourceAttributes = CreateResourceAttributes(device);

  memset(&appRun,0x00,sizeof(appRun));
  appRun.appName = cesdk.fns.CEM_AllocateString(L"URL");

  CEEnforcement_t enforcement;
  memset(&enforcement,0x00,sizeof(enforcement));

  RdeLog.Log(CELOG_INFO,L"PolicyInit: CheckResources\n");
  nextlabs::high_resolution_timer ht;
  res = cesdk.fns.CEEVALUATE_CheckResources(handle,action,
					    source,sourceAttributes,           /* source */
					    NULL,NULL,                         /* target */
					    &user,NULL,                        /* user */
					    &appRun,NULL,                      /* application */
					    NULL,0,                            /* recipients */
					    0,                                 /* IP */
					    CETrue,CE_NOISE_LEVEL_USER_ACTION,
					    &enforcement,TIMEOUT_MS);
  RdeLog.Log(CELOG_INFO,L"PolicyInit: CheckResources %fms (res %d)\n",ht.diff(),res);

  nextlabs::cesdk_attributes::destroy(cesdk,sourceAttributes);

  deny = false;
  if( res == CE_RESULT_SUCCESS && enforcement.result == CEDeny )
  {
    deny = true;
  }

  cesdk.fns.CEM_FreeResource(source);

  cesdk.fns.CEM_FreeString(appRun.appName);
  if( appRun.appURL != NULL )
  {
    cesdk.fns.CEM_FreeString(appRun.appURL);
  }

  cesdk.fns.CEM_FreeString(app.appName);
  cesdk.fns.CEM_FreeString(app.appPath);

  cesdk.fns.CEM_FreeString(user.userName);
  cesdk.fns.CEM_FreeString(user.userID);

  cesdk.fns.CEM_FreeString(action);

  cesdk.fns.CECONN_DLL_Deactivate(handle,TIMEOUT_MS);

  return true;
}/* PolicyEval */
