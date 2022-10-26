/***************************************************************************************************
 *
 * SDK Loader
 *
 * Load and unload support for the CE SDK.
 *
 **************************************************************************************************/

#ifndef __CESDK_LOADER_HPP__
#define __CESDK_LOADER_HPP__

#include <cassert>
#include <string>
#include <list>
#include <map>

#include "CEsdk.h"

namespace nextlabs
{

  /******************************************************************************************
   * Type definitions to match CE SDK (see CEsdk.h)
   *****************************************************************************************/

  /* CECONN */
  typedef CEResult_t (*CECONN_Initialize_t)
    (CEApplication,CEUser,CEString,CEHandle*,CEint32);

  typedef CEResult_t (*CECONN_DLL_Activate_t)
    (CEApplication app, 
     CEUser user, 
     CEString pdpHostName,
     CEHandle * connectHandle,
     CEint32 timeout_in_millisec);

  typedef CEResult_t (*CECONN_Close_t)
    (CEHandle,CEint32);

  typedef CEResult_t (*CECONN_DLL_Deactivate_t)
    (CEHandle handle,
     CEint32 timeout_in_millisec);
  
  /* CEEVAL */
  typedef CEResult_t (*CEEVALUATE_CheckFile_t)
    (CEHandle,
     CEAction_t,
     CEString,
     CEAttributes*,
     CEString,
     CEAttributes*,
     CEint32,
     CEUser*,
     CEApplication*,
     CEBoolean,
     CENoiseLevel_t,
     CEEnforcement_t*,
     CEint32);

  typedef CEResult_t (*CEEVALUATE_CheckResources_t)
    (CEHandle handle, 
     const CEString operation,                 
     const CEResource* source,           
     const CEAttributes * sourceAttributes,
     const CEResource* target,           
     const CEAttributes * targetAttributes,
     const CEUser  *user,
     CEAttributes * userAttributes,
     CEApplication *app,
     CEAttributes * appAttributes,
     CEString *recipients,
     CEint32 numRecipients,
     const CEint32 ipNumber,
     const CEBoolean performObligation,
     const CENoiseLevel_t noiseLevel,
     CEEnforcement_t * enforcement,
     const CEint32 timeout_in_millisec);

  typedef CEResult_t (*CEEVALUATE_CheckMessageAttachment_t)
    (CEHandle handle, 
     CEAction_t operation, 
     CEString sourceFullFileName, 
     CEAttributes * sourceAttributes,
     CEint32 numRecipients,
     CEString *recipients,
     CEint32 ipNumber,
     CEUser  *user,
     CEAttributes * userAttributes,
     CEApplication *app,
     CEAttributes * appAttributes,
     CEBoolean performObligation,
     CENoiseLevel_t noiseLevel,
     CEEnforcement_t * enforcement,
     CEint32 timeout_in_millisec);

  typedef CEResult_t (*CEEVALUATE_FreeEnforcement_t)
    (CEEnforcement_t);

  /* CECEM */
  typedef CEString (*CEM_AllocateString_t)
    (const TCHAR*);

  typedef const TCHAR* (*CEM_GetString_t)
    (CEString);

  typedef CEResult_t (*CEM_FreeString_t)
    (CEString);

  typedef CEResource* (*CEM_CreateResourceW_t)
    (const wchar_t*,
     const wchar_t*);

  typedef void (*CEM_FreeResource_t)
    (CEResource*);

  /******************************************************************************************
   * Function structure for CE SDK function pointers
   *****************************************************************************************/

  static const wchar_t* CEBRAIN     = L"cebrain.dll";
  static const wchar_t* CECEM       = L"cecem.dll";
  static const wchar_t* CEMARSHAL   = L"cemarshal50.dll";
  static const wchar_t* CETRANSPORT = L"cetransport.dll";
  static const wchar_t* CEPEPMAN    = L"cepepman.dll";
  static const wchar_t* CECONN      = L"ceconn.dll";
  static const wchar_t* CEEVAL      = L"ceeval.dll";

  typedef struct
  {
    /* CECONN */
    CECONN_Initialize_t     CECONN_Initialize;
    CECONN_DLL_Activate_t   CECONN_DLL_Activate;
    CECONN_Close_t          CECONN_Close;
    CECONN_DLL_Deactivate_t CECONN_DLL_Deactivate;

    /* CEEVAL */
    CEEVALUATE_CheckFile_t               CEEVALUATE_CheckFile;
    CEEVALUATE_CheckResources_t          CEEVALUATE_CheckResources;
    CEEVALUATE_CheckMessageAttachment_t  CEEVALUATE_CheckMessageAttachment;
    CEEVALUATE_FreeEnforcement_t         CEEVALUATE_FreeEnforcement;

    /* CEM */
    CEM_AllocateString_t  CEM_AllocateString;
    CEM_GetString_t       CEM_GetString;
    CEM_FreeString_t      CEM_FreeString;
    CEM_CreateResourceW_t CEM_CreateResourceW;
    CEM_FreeResource_t    CEM_FreeResource;

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
      libmap(),
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
    virtual bool load( _In_ const wchar_t* path ) throw()
    {
      assert( path != NULL );
      if( path == NULL )
      {
	return false;
      }

      /****************************************************************
       * Load SDK libraries.  Order is critical due to link depends.
       ***************************************************************/
      sdk_lib_t sdk_libs[] = {L"cebrain.dll",L"cecem.dll",L"cemarshal50.dll",
			      L"cetransport.dll",L"cepepman.dll",L"ceconn.dll",
			      L"ceeval.dll"};

      bool status = false;
      for( size_t i = 0 ; i < _countof(sdk_libs) ; i++ )
      {
	HMODULE hlib = NULL;
	WCHAR libpath[MAX_PATH] = {0};
	_snwprintf_s(libpath,_countof(libpath), _TRUNCATE,L"%s\\%s",path,sdk_libs[i].libname);
	hlib = LoadLibraryW(libpath);
	if( hlib == NULL )
	{
	  goto Load_done;
	}
	libmap[sdk_libs[i].libname] = hlib;
      }

      /****************************************************************
       * Retrieve all SDK function addresses.
       ***************************************************************/
      fns.CECONN_Initialize          = (CECONN_Initialize_t)GetProcAddress(libmap[CECONN],"CECONN_Initialize");
      fns.CECONN_DLL_Activate        = (CECONN_DLL_Activate_t)GetProcAddress(libmap[CECONN],"CECONN_DLL_Activate");
      fns.CECONN_Close               = (CECONN_Close_t)GetProcAddress(libmap[CECONN],"CECONN_Close");
      fns.CECONN_DLL_Deactivate      = (CECONN_DLL_Deactivate_t)GetProcAddress(libmap[CECONN],"CECONN_DLL_Deactivate");
      
      fns.CEEVALUATE_CheckFile               = (CEEVALUATE_CheckFile_t)GetProcAddress(libmap[CEEVAL],"CEEVALUATE_CheckFile");
      fns.CEEVALUATE_CheckResources          = (CEEVALUATE_CheckResources_t)GetProcAddress(libmap[CEEVAL],"CEEVALUATE_CheckResources");
      fns.CEEVALUATE_CheckMessageAttachment  = (CEEVALUATE_CheckMessageAttachment_t)GetProcAddress(libmap[CEEVAL],"CEEVALUATE_CheckMessageAttachment");
      fns.CEEVALUATE_FreeEnforcement         = (CEEVALUATE_FreeEnforcement_t)GetProcAddress(libmap[CEEVAL],"CEEVALUATE_FreeEnforcement");

      fns.CEM_AllocateString         = (CEM_AllocateString_t)GetProcAddress(libmap[CECEM],"CEM_AllocateString");
      fns.CEM_GetString              = (CEM_GetString_t)GetProcAddress(libmap[CECEM],"CEM_GetString");
      fns.CEM_FreeString             = (CEM_FreeString_t)GetProcAddress(libmap[CECEM],"CEM_FreeString");
      fns.CEM_CreateResourceW        = (CEM_CreateResourceW_t)GetProcAddress(libmap[CECEM],"CEM_CreateResourceW");
      fns.CEM_FreeResource           = (CEM_FreeResource_t)GetProcAddress(libmap[CECEM],"CEM_FreeResource");

      status = true;

      /* CECONN */
      if( fns.CECONN_Initialize == NULL ||
	  fns.CECONN_DLL_Activate == NULL ||
	  fns.CECONN_Close == NULL ||
	  fns.CECONN_DLL_Deactivate == NULL )
      {
	status = false;
      }

      /* CEEVALUATE */
      if( fns.CEEVALUATE_CheckFile == NULL ||
	  fns.CEEVALUATE_CheckResources == NULL ||
	  fns.CEEVALUATE_CheckMessageAttachment == NULL ||
	  fns.CEEVALUATE_FreeEnforcement == NULL )
      {
	status = false;
      }

      /* CEM */
      if( fns.CEM_AllocateString == NULL ||
	  fns.CEM_GetString == NULL ||
	  fns.CEM_FreeString == NULL ||
	  fns.CEM_CreateResourceW == NULL ||
	  fns.CEM_FreeResource == NULL )
      {
	status = false;
      }

      loaded = true; /* load successful */

      Load_done:
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
      std::map<std::wstring,HMODULE>::iterator it;
      for( it = libmap.begin() ; it != libmap.end() ; ++it )
      {
	FreeLibrary(it->second);
	it->second = NULL;
      }
      loaded = false;
      memset(&fns,0x00,sizeof(fns));
    }/* unload */

    /* CE SDK methods */
    sdk_functions_t fns;

    protected:

      /* Library map of names to handles */
      std::map<std::wstring,HMODULE> libmap;

      /* CE SDK has been loaded */
      bool loaded;

  };/* class cesdk_loader */

}/* namespace nextlabs */

#endif /* __CESDK_LOADER_HPP__ */
