
#ifndef __WDESDKSUPPORT_H__
#define __WDESDKSUPPORT_H__

#include <windows.h>
#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <string>
#include <list>
#include <map>

#include "CEsdk.h"
#include "CEsdk_helper.hpp"

namespace CESDK
{

  /* CECONN */
  typedef CEResult_t (*CECONN_Initialize_t)(CEApplication,CEUser,CEString,CEHandle*,CEint32);
  typedef CEResult_t (*CECONN_DLL_Activate_t)(CEApplication app, 
					      CEUser user, 
					      CEString pdpHostName,
					      CEHandle * connectHandle,
					      CEint32 timeout_in_millisec);
  typedef CEResult_t (*CECONN_Close_t)(CEHandle,CEint32);
  typedef CEResult_t (*CECONN_DLL_Deactivate_t)(CEHandle handle, CEint32 timeout_in_millisec);
  
  typedef CEResult_t (*CEEVALUATE_CheckFile_t)(CEHandle,CEAction_t,CEString,CEAttributes*,CEString,
					       CEAttributes*,CEint32,CEUser*,CEApplication*,
					       CEBoolean,CENoiseLevel_t,CEEnforcement_t*,CEint32);

  typedef CEResult_t (*CEEVALUATE_CheckResources_t)(CEHandle handle, 
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

  typedef CEResult_t (*CEEVALUATE_FreeEnforcement_t)(CEEnforcement_t);
  typedef CEString   (*CEM_AllocateString_t)(const TCHAR*);
  typedef const TCHAR* (*CEM_GetString_t)(CEString);
  typedef CEResult_t (*CEM_FreeString_t)(CEString);
  typedef CEResource* (*CEM_CreateResourceW_t)(const wchar_t*,const wchar_t*);
  typedef void (*CEM_FreeResource_t)(CEResource*);

  typedef struct
  {
    const WCHAR* libname;  // library name (e.g. cebrain.dll)
  } sdk_lib_t;

  #define CEBRAIN      L"cebrain.dll"
  #define CECEM        L"cecem.dll"
  #define CEMARSHAL    L"cemarshal.dll"
  #define CETRANSPORT  L"cetransport.dll"
  #define CEPEPMAN     L"cepepman.dll"
  #define CECONN       L"ceconn.dll"
  #define CEEVAL       L"ceeval.dll"

  typedef struct
  {
    /* CECONN */
    CECONN_Initialize_t     CECONN_Initialize;
    CECONN_DLL_Activate_t   CECONN_DLL_Activate;
    CECONN_Close_t          CECONN_Close;
    CECONN_DLL_Deactivate_t CECONN_DLL_Deactivate;

    /* CEEVAL */
    CEEVALUATE_CheckFile_t       CEEVALUATE_CheckFile;
    CEEVALUATE_CheckResources_t  CEEVALUATE_CheckResources;
    CEEVALUATE_FreeEnforcement_t CEEVALUATE_FreeEnforcement;

    /* CEM */
    CEM_AllocateString_t  CEM_AllocateString;
    CEM_GetString_t       CEM_GetString;
    CEM_FreeString_t      CEM_FreeString;
    CEM_CreateResourceW_t CEM_CreateResourceW;
    CEM_FreeResource_t    CEM_FreeResource;

    std::map<std::wstring,HMODULE> libmap;

    /***********************************************************************
     * Connection oriented state
     **********************************************************************/
    CRITICAL_SECTION cs;  /* Protect connections state */
    bool connected;       /* Current connection state */
    CEHandle connHandle;  /* Connection handle */
  } Handle;	

  /** Load
   *
   *  Load the SDK into the given handle.  The SDK connection is not initiated from
   *  this method.
   *
   *  \return true on success, otherwise false.
   */
  bool Load( __in Handle* handle );

  /** Unload
   *
   *  \brief Unload the SDK at the given handle.
   */
  void Unload( __in Handle* handle );

  /** SetupConnection
   *
   *  \brief Setup connection to policy controller.
   *
   *  \param handle (in)      CESDK handle.
   *  \param application (in) Application.
   *  \param timeout_ms (in)  Timeout in milliseconds for call to succeed.
   *
   *  \return true if connection is successful, otherwise false.
   */
  bool SetupConnection( Handle& handle ,
			const WCHAR* application ,
			int timeout_ms );

  /** DoPolicyEvaluation
   *
   *  \brief Perform policy evaluation.  If a connection the Policy Controller has not yet
   *         been made or is not active a connection attempt will be made.
   *
   *  \param sdk (in)            CESDK handle.
   *  \param action (in)         Action being performed.
   *  \param application (in)    Application performing the action.
   *  \param usid (in)           User's SID.
   *  \param source (in)         Source resource.
   *  \param target (in-opt)     Target resource.
   *  \param obligations (out)   Obligations.
   *  \param timeout_ms (in-opt) Timeout for evaluation call in milliseconds.
   *
   *  \return TRUE when action is permitted, otherwise FALSE.
   */
  BOOL DoPolicyEvaluation(Handle &sdk,
			  CEAction_t action,
			  CENoiseLevel_t noise_level,
			  __in const WCHAR *application,
			  __in const WCHAR *usid,
			  __in const WCHAR *source, 
			  __in const WCHAR *target,
			  bool ignore_obligations ,
			  int timeout_ms ,
			  double& sdk_time );

}/* namespace CESDK */

#endif /* __WDESDKSUPPORT_H__ */
