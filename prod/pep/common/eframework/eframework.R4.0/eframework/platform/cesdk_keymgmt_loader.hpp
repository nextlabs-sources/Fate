/***************************************************************************************************
 *
 * Key Management Client SDK Loader
 *
 * Load and unload support for the CE Key Management Client SDK.
 *
 **************************************************************************************************/

#ifndef __CESDK_KEYMGMT_LOADER_HPP__
#define __CESDK_KEYMGMT_LOADER_HPP__

#include <cassert>
#include <string>
#include <list>
#include <map>

#include "CEsdk.h"
#include "KeyManagementConsumer.h"

namespace nextlabs
{

  /******************************************************************************************
   * Type definitions to match CE Key Management Client SDK (see KeyManagementConsumer.h)
   *****************************************************************************************/

  typedef CEString (*CEKey_GetLastException_t)(void);

  typedef CEResult_t (*CEKey_GetKey_t)(_In_opt_ CEHandle handle,
				       _In_bytecount_(passwordLen) const unsigned char *password,
				       _In_ size_t passwordLen,
				       _In_opt_ CEString keyRingName,
				       _In_ CEKeyID id,
				       _Out_opt_ CEKey *key,
				       _In_ CEint32 processID,
				       _In_ CEint32 timeout);

  typedef CEResult_t (*CEKey_CreateKeyRing_t)(_In_opt_ CEHandle handle,
					      _In_bytecount_(passwordLen) const unsigned char *password,
					      _In_ size_t passwordLen,
					      _In_opt_ CEString keyRingName,
					      _In_ CEint32 processID,
					      _In_ CEint32 timeout);

  typedef CEResult_t (*CEKey_SetKey_t)(_In_opt_ CEHandle handle,
				      _In_bytecount_(passwordLen)  const unsigned char *password,
				      _In_ size_t passwordLen,
				      _In_opt_ CEString keyRingName,
				      _In_ CEKey key,
				      _In_ CEint32 processID,
				      _In_ CEint32 timeout);

  typedef CEResult_t(*CEKey_GenerateKey_t)(_In_opt_ CEHandle handle,
					   _In_bytecount_(passwordLen) const unsigned char *password,
					   _In_ size_t passwordLen,
					   _In_opt_ CEString keyRingName,
					   _In_ CEint32 keyLen,
					   _Out_opt_ CEKeyID *keyID,
					   _In_ CEint32 processID,
					   _In_ CEint32 timeout);

  typedef CEResult_t (*CEKey_DeleteKey_t)(_In_opt_ CEHandle handle,
					  _In_bytecount_(passwordLen) const unsigned char *password,
					  _In_ size_t passwordLen,
					  _In_opt_ CEString keyRingName,
					  _In_ CEKeyID id,
					  _In_ CEint32 processID,
					  _In_ CEint32 timeout); 

  typedef CEResult_t (*CEKey_DeleteKeyRing_t)(_In_opt_ CEHandle handle,
					     _In_bytecount_(passwordLen) const unsigned char *password,
					     _In_ size_t passwordLen,
					     _In_opt_ CEString keyRingName,
					     _In_ CEint32 processID,
					     _In_ CEint32 timeout);

  typedef CEResult_t (*CEKey_ListKeyRings_t)(_In_opt_ CEHandle handle,
					    _In_bytecount_(passwordLen) const unsigned char *password,
					    _In_ size_t passwordLen,
					    _Out_opt_ CEKeyRing *keyRings,
					    _Inout_opt_ CEint32 *size,
					    _In_ CEint32 processID,
					    _In_ CEint32 timeout);

  typedef CEResult_t (*CEKey_ListKeys_t)(_In_opt_ CEHandle handle,
					_In_bytecount_(passwordLen) const unsigned char *password,
					_In_ size_t passwordLen,
					_Inout_opt_ CEKeyRing *keyRing,
					_In_ CEint32 processID,
					_In_ CEint32 timeout);

  /******************************************************************************************
   * Function structure for CE SDK function pointers
   *****************************************************************************************/

#if defined(_M_IX86)
  static const wchar_t* CEKEYMGMTCONSUMER     = L"KeyManagementConsumer32.dll";
#elif defined(_M_X64)
  static const wchar_t* CEKEYMGMTCONSUMER     = L"KeyManagementConsumer.dll";
#endif

  typedef struct
  {
    CEKey_GetLastException_t CEKey_GetLastException;
    CEKey_GetKey_t           CEKey_GetKey;
    CEKey_CreateKeyRing_t    CEKey_CreateKeyRing;
    CEKey_SetKey_t           CEKey_SetKey;
    CEKey_GenerateKey_t      CEKey_GenerateKey;
    CEKey_DeleteKeyRing_t    CEKey_DeleteKeyRing;
    CEKey_ListKeyRings_t     CEKey_ListKeyRings;
    CEKey_ListKeys_t         CEKey_ListKeys;
  } sdk_keymgmt_functions_t;

  typedef struct
  {
    const WCHAR* libname;  // library name (e.g. xyz.dll)
  } sdk_keymgmt_lib_t;

  /******************************************************************************************
   * cesdk_keymgmt_loader
   *****************************************************************************************/

  /** cesdk_keymgmt_loader
   *
   *  \brief Class to abstract dynamic loading of the CE Key Management SDK.
   */
  class cesdk_keymgmt_loader
  {
    public:

    cesdk_keymgmt_loader(void) :
      libmap(),
      loaded(false)
    {
      memset(&fns,0x00,sizeof(fns));
    }/* cesdk_keymgmt_loader */

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
    virtual bool load( __in const wchar_t* path ) throw()
    {
      assert( path != NULL );
      if( path == NULL )
      {
	return false;
      }

      /****************************************************************
       * Load SDK libraries.  Order is critical due to link depends.
       ***************************************************************/
      sdk_keymgmt_lib_t sdk_libs[] =
	{
	  /* CE SDK 5.x must be used due to export changes to cemarshal. */
	  CEKEYMGMTCONSUMER
	};

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
       * Retrieve all Key Management Client SDK function addresses.
       ***************************************************************/
      fns.CEKey_GetLastException = (CEKey_GetLastException_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_GetLastException");
      fns.CEKey_GetKey           = (CEKey_GetKey_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_GetKey");
      fns.CEKey_CreateKeyRing    = (CEKey_CreateKeyRing_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_CreateKeyRing");
      fns.CEKey_SetKey           = (CEKey_SetKey_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_SetKey");
      fns.CEKey_GenerateKey      = (CEKey_GenerateKey_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_GenerateKey");
      fns.CEKey_DeleteKeyRing    = (CEKey_DeleteKeyRing_t )GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_DeleteKeyRing");
      fns.CEKey_ListKeyRings     = (CEKey_ListKeyRings_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_ListKeyRings");
      fns.CEKey_ListKeys         = (CEKey_ListKeys_t)GetProcAddress(libmap[CEKEYMGMTCONSUMER],"CEKey_ListKeys");

      status = true;

      if( fns.CEKey_GetLastException == NULL ||
	  fns.CEKey_GetKey == NULL ||
	  fns.CEKey_CreateKeyRing == NULL  ||
	  fns.CEKey_SetKey == NULL ||
	  fns.CEKey_GenerateKey == NULL ||
	  fns.CEKey_DeleteKeyRing == NULL ||
	  fns.CEKey_ListKeyRings == NULL ||
	  fns.CEKey_ListKeys == NULL )
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
    sdk_keymgmt_functions_t fns;

    protected:

      /* Library map of names to handles */
      std::map<std::wstring,HMODULE> libmap;

      /* CE SDK has been loaded */
      bool loaded;

  };/* class cesdk_keymgmt_loader */

}/* namespace nextlabs */

#endif /* __CESDK_KEYMGMT_LOADER_HPP__ */
