/***************************************************************************************************
 *
 * SDK Loader
 *
 * Load and unload support for the CE SDK.
 *
 **************************************************************************************************/

#ifndef __CESDK_LOADER_HPP__
#define __CESDK_LOADER_HPP__

#include <windows.h>
#include <cassert>
#include <string>
#include <list>
#include <map>

#include "CEsdk.h"
#include "ceservice.h"

namespace nextlabs
{

  /******************************************************************************************
   * Type definitions to match CE SDK (see CEsdk.h)
   *****************************************************************************************/

  typedef CEResult_t (*CECONN_Initialize_t)
    (_In_ CEApplication,_In_ CEUser,_In_opt_ CEString,_Out_ CEHandle*,_In_ CEint32);

  typedef CEResult_t (*CECONN_DLL_Activate_t)
    (_In_ CEApplication app, 
     _In_ CEUser user, 
     _In_opt_ CEString pdpHostName,
     _Out_ CEHandle * connectHandle,
     _In_ CEint32 timeout_in_millisec);

  typedef CEResult_t (*CECONN_Close_t)
    (_In_opt_ CEHandle,_In_ CEint32);

  typedef CEResult_t (*CECONN_DLL_Deactivate_t)
    (_In_opt_ CEHandle handle,
     _In_ CEint32 timeout_in_millisec);

  typedef CEResult_t (*CEEVALUATE_CheckFile_t)
    (_In_opt_ CEHandle,
     _In_ CEAction_t,
     _In_opt_ CEString,
     _In_opt_ CEAttributes*,
     _In_opt_ CEString,
     _In_ CEAttributes*,
     _In_ CEint32,
     _In_opt_ CEUser*,
     _In_ CEApplication*,
     _In_ CEBoolean,
     _In_ CENoiseLevel_t,
     _Out_ CEEnforcement_t*,
     _In_ CEint32);

  typedef CEResult_t (*CEEVALUATE_CheckResources_t)
    (_In_opt_ CEHandle handle, 
     _In_opt_ const CEString operation,                 
     _In_opt_ const CEResource* source,           
     _In_ const CEAttributes * sourceAttributes,
     _In_ const CEResource* target,           
     _In_ const CEAttributes * targetAttributes,
     _In_opt_ const CEUser  *user,
     _In_ CEAttributes * userAttributes,
     _In_ CEApplication *app,
     _In_ CEAttributes * appAttributes,
     _In_opt_count_(numRecipients) CEString *recipients,
     _In_ CEint32 numRecipients,
     _In_ const CEint32 ipNumber,
     _In_ const CEBoolean performObligation,
     _In_ const CENoiseLevel_t noiseLevel,
     _Out_ CEEnforcement_t * enforcement,
     _In_ const CEint32 timeout_in_millisec);

  typedef CEResult_t (*CEEVALUATE_CheckMessageAttachment_t)
    (_In_opt_ CEHandle handle, 
     _In_ CEAction_t operation, 
     _In_opt_ CEString sourceFullFileName, 
     _In_opt_ CEAttributes * sourceAttributes,
     _In_ CEint32 numRecipients,
     _In_opt_count_(numRecipients) CEString *recipients,
     _In_ CEint32 ipNumber,
     _In_opt_ CEUser  *user,
     _In_ CEAttributes * userAttributes,
     _In_ CEApplication *app,
     _In_ CEAttributes * appAttributes,
     _In_ CEBoolean performObligation,
     _In_ CENoiseLevel_t noiseLevel,
     _Out_ CEEnforcement_t * enforcement,
     _In_ CEint32 timeout_in_millisec);

  typedef CEResult_t (*CEEVALUATE_FreeEnforcement_t)
    (_In_ CEEnforcement_t);

  typedef CEString (*CEM_AllocateString_t)
    (_In_opt_z_ const TCHAR*);

  typedef const TCHAR* (*CEM_GetString_t)
    (_In_opt_  CEString);

  typedef CEResult_t (*CEM_FreeString_t)
    (_In_opt_ CEString);

  typedef CEResource* (*CEM_CreateResourceW_t)
    (_In_opt_z_ const wchar_t*,
     _In_opt_z_ const wchar_t*);

  typedef void (*CEM_FreeResource_t)
    (_In_opt_ CEResource*);

  typedef CEResult_t (*ServiceInvoke_t)(_In_opt_ CEHandle h,
					_In_opt_ CEString serviceName,
					_In_opt_  CEString fmt,
					_In_ void **request,
					_Deref_out_ void ***response,
					_In_ CEint32 timeout);

  typedef CEResult_t (*ServiceResponseFree_t)(_In_opt_ void **response);

  typedef CEResult_t (*CELOGGING_LogDecision_t)( _In_opt_ CEHandle handle,
						 _In_ CEString cookie,
						 _In_ CEResponse_t userResponse,
						 _In_ CEAttributes* optAttributes );

  typedef CEResult_t (*CELOGGING_LogObligationData_t)( _In_opt_ CEHandle handle,
						       _In_ CEString logIdentifier,
						       _In_ CEString obligationName,
						       _In_ CEAttributes* attributes );
						       
  /******************************************************************************************
   * Function structure for CE SDK function pointers
   *****************************************************************************************/

  typedef struct
  {
    CECONN_Initialize_t     CECONN_Initialize;
    CECONN_DLL_Activate_t   CECONN_DLL_Activate;
    CECONN_Close_t          CECONN_Close;
    CECONN_DLL_Deactivate_t CECONN_DLL_Deactivate;

    CEEVALUATE_CheckFile_t               CEEVALUATE_CheckFile;
    CEEVALUATE_CheckResources_t          CEEVALUATE_CheckResources;
    CEEVALUATE_CheckMessageAttachment_t  CEEVALUATE_CheckMessageAttachment;
    CEEVALUATE_FreeEnforcement_t         CEEVALUATE_FreeEnforcement;

    CEM_AllocateString_t  CEM_AllocateString;
    CEM_GetString_t       CEM_GetString;
    CEM_FreeString_t      CEM_FreeString;
    CEM_CreateResourceW_t CEM_CreateResourceW;
    CEM_FreeResource_t    CEM_FreeResource;

    ServiceInvoke_t ServiceInvoke;
    ServiceResponseFree_t ServiceResponseFree;

    CELOGGING_LogDecision_t CELOGGING_LogDecision;
    CELOGGING_LogObligationData_t CELOGGING_LogObligationData;

  } sdk_functions_t;

  typedef struct
  {
    const WCHAR* libname;  // library name (e.g. cebrain.dll)
  } sdk_lib_t;

  /******************************************************************************************
   * cesdk_loader
   *****************************************************************************************/

  /** cesdk_loader
   *
   *  \brief Class to abstract dynamic loading of the CE SDK.
   */
  class cesdk_loader
  {
    public:

    cesdk_loader(void) :
      loaded(false)
    {
      memset(&fns,0x00,sizeof(fns));
    }/* cesdk_loader */

    /** load
     *
     *  \brief Load the SDK and make member fns available to call.  When this method succeeds
     *         all members of fns are non-NULL and safe to call.
     *
     *  \param path (in) Path to load the CE SDK from (i.e. C:/Program Files/NextLabs/Desktop Enforcer/bin).
     *
     *  \return true on success, otherwise false.
     */
   _Check_return_
    virtual bool load( _In_opt_z_ const wchar_t* path ) throw()
    {
      assert( path != NULL );
      if( path == NULL )
      {
	return false;
      }

      /****************************************************************
       * Load SDK libraries.  Order is critical due to link depends.
       ***************************************************************/
      bool status = false;
      WCHAR libpath[MAX_PATH] = {0};

#if defined(_M_IX86)
      _snwprintf_s(libpath,_countof(libpath), _TRUNCATE,L"%s\\cesdk32.dll",path);
#elif defined(_M_X64)
      _snwprintf_s(libpath,_countof(libpath), _TRUNCATE,L"%s\\cesdk.dll",path);
#endif
      hlib = LoadLibraryW(libpath);
      if( hlib == NULL )
      {
	return false;
      }

      /****************************************************************
       * Retrieve all SDK function addresses.
       ***************************************************************/
      /* CESDK */
      fns.CECONN_Initialize          = (CECONN_Initialize_t)GetProcAddress(hlib,"CECONN_Initialize");
      fns.CECONN_DLL_Activate        = (CECONN_DLL_Activate_t)GetProcAddress(hlib,"CECONN_DLL_Activate");
      fns.CECONN_Close               = (CECONN_Close_t)GetProcAddress(hlib,"CECONN_Close");
      fns.CECONN_DLL_Deactivate      = (CECONN_DLL_Deactivate_t)GetProcAddress(hlib,"CECONN_DLL_Deactivate");
      fns.CEEVALUATE_CheckFile       = (CEEVALUATE_CheckFile_t)GetProcAddress(hlib,"CEEVALUATE_CheckFile");
      fns.CEEVALUATE_CheckResources  = (CEEVALUATE_CheckResources_t)GetProcAddress(hlib,"CEEVALUATE_CheckResources");
      fns.CEEVALUATE_CheckMessageAttachment  = (CEEVALUATE_CheckMessageAttachment_t)GetProcAddress(hlib,"CEEVALUATE_CheckMessageAttachment");
      fns.CEEVALUATE_FreeEnforcement = (CEEVALUATE_FreeEnforcement_t)GetProcAddress(hlib,"CEEVALUATE_FreeEnforcement");
      fns.CEM_AllocateString         = (CEM_AllocateString_t)GetProcAddress(hlib,"CEM_AllocateString");
      fns.CEM_GetString              = (CEM_GetString_t)GetProcAddress(hlib,"CEM_GetString");
      fns.CEM_FreeString             = (CEM_FreeString_t)GetProcAddress(hlib,"CEM_FreeString");
      fns.CEM_CreateResourceW        = (CEM_CreateResourceW_t)GetProcAddress(hlib,"CEM_CreateResourceW");
      fns.CEM_FreeResource           = (CEM_FreeResource_t)GetProcAddress(hlib,"CEM_FreeResource");
      fns.CELOGGING_LogDecision       = (CELOGGING_LogDecision_t)GetProcAddress(hlib,"CELOGGING_LogDecision");
      fns.CELOGGING_LogObligationData = (CELOGGING_LogObligationData_t)GetProcAddress(hlib,"CELOGGING_LogObligationData");

      /* CESERVICE */
      fns.ServiceInvoke              = (ServiceInvoke_t)GetProcAddress(hlib,"ServiceInvoke");
      fns.ServiceResponseFree        = (ServiceResponseFree_t)GetProcAddress(hlib,"ServiceResponseFree");

      status = true;

      if( fns.CECONN_Initialize == NULL ||
	  fns.CECONN_DLL_Activate == NULL ||
	  fns.CECONN_Close == NULL ||
	  fns.CECONN_DLL_Deactivate == NULL ||
	  fns.CEEVALUATE_CheckFile == NULL ||
	  fns.CEEVALUATE_CheckResources == NULL ||
	  fns.CEEVALUATE_CheckMessageAttachment == NULL ||
	  fns.CEEVALUATE_FreeEnforcement == NULL ||
	  fns.CEM_AllocateString == NULL ||
	  fns.CEM_GetString == NULL ||
	  fns.CEM_FreeString == NULL ||
	  fns.CEM_CreateResourceW == NULL ||
	  fns.CEM_FreeResource == NULL ||
	  fns.CELOGGING_LogDecision == NULL ||
	  fns.CELOGGING_LogObligationData == NULL )
      {
	status = false;
      }

      loaded = true; /* load successful */

      /* If any of the functions cannot be found the SDK cannot be used. */
      if( status == false )
      {
	unload();
      }/* if failed */
  
      return status;
    }/* load */

    /** is_loaded
     *
     *  \brief Determine if the CE SDK has been loaded.  Initialize must be called before this
     *         may be called.
     *
     *  \return true if load was successful, otherwise false.
     *  \sa load, unload
     */
    bool is_loaded(void) const throw()
    {
      return loaded;
    }/* is_loaded */

    /** unload
     *
     *  \brief Unload the CE SDK.  It is unsafe to call any method in the fns structure after
     *         unload has been called.
     *
     *  \return None
     *  \sa load
     */
    void unload(void) throw()
    {
      FreeLibrary(hlib);
      loaded = false;
      memset(&fns,0x00,sizeof(fns));
    }/* unload */

    /* CE SDK methods */
    sdk_functions_t fns;

    protected:

      /* CE SDK has been loaded */
      bool loaded;
      HMODULE hlib;

  };/* class cesdk_loader */

}/* namespace nextlabs */

#endif /* __CESDK_LOADER_HPP__ */
