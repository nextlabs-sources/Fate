/***************************************************************************************************
 *
 * resattr Loader
 *
 * Load and unload support for resattrmgr and resattrlib
 *
 **************************************************************************************************/

#ifndef __RESATTR_LOADER_HPP__
#define __RESATTR_LOADER_HPP__

#include "nlconfig.hpp"
#include "resattrmgr.h"



namespace nextlabs
{
  /******************************************************************************************
   * Type definitions to match resattrmgr and resattrlib (see resattrmgr.h and resattrlib.h)
   *****************************************************************************************/
  typedef int (*CreateAttributeManager_t)(ResourceAttributeManager **mgr);
  typedef void (*CloseAttributeManager_t)(ResourceAttributeManager *mgr);

  typedef int (*AllocAttributes_t)(ResourceAttributes **attrs);
  typedef int (*ReadResourceAttributesW_t)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
  typedef int (*ReadResourceAttributesForNTFSW_t)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
  typedef int (*GetAttributeCount_t)(const ResourceAttributes *attrs);
  typedef const WCHAR *(*GetAttributeName_t)(const ResourceAttributes *attrs, int index);
  typedef const WCHAR * (*GetAttributeValue_t)(const ResourceAttributes *attrs, int index);
  typedef void (*AddAttributeW_t)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
  typedef int (*WriteResourceAttributesW_t)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
  typedef int (*WriteResourceAttributesForNTFSW_t)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
  typedef void (*FreeAttributes_t)(ResourceAttributes *attrs);

  /******************************************************************************************
   * Function structure for resattmgr and resattrlib function pointers
   *****************************************************************************************/
  typedef struct
  {
    CreateAttributeManager_t    CreateAttributeManager;
    CloseAttributeManager_t     CloseAttributeManager;

    AllocAttributes_t           AllocAttributes;
    ReadResourceAttributesW_t   ReadResourceAttributesW;
	ReadResourceAttributesForNTFSW_t   ReadResourceAttributesForNTFSW;
    GetAttributeCount_t         GetAttributeCount;
    GetAttributeName_t          GetAttributeName;
    GetAttributeValue_t         GetAttributeValue;
    AddAttributeW_t             AddAttributeW;
    WriteResourceAttributesW_t  WriteResourceAttributesW;
	WriteResourceAttributesForNTFSW_t  WriteResourceAttributesForNTFSW;
    FreeAttributes_t            FreeAttributes;
  } resattr_functions_t;

  typedef struct
  {
    HMODULE     celog;
    HMODULE     nl_sysenc_lib;
    HMODULE     libtiff;
#ifdef _WIN64
    HMODULE     zlibwapi;
#else
    HMODULE     zlib1;
    HMODULE     freetype6;
#endif
    HMODULE     PoDoFoLib;
    HMODULE     resattrlib;
    HMODULE     resattrmgr;
  } resattr_modules_t;

  /******************************************************************************************
   * resattr_loader
   *****************************************************************************************/

  /** resattr_loader
   *
   *  \brief Class to abstract dynamic loading of resattrmgr and resattrlib
   */
  class resattr_loader
  {
    std::wstring GetCommonComponentsDir()
    {
      wchar_t szDir[MAX_PATH] = {0};
      if (NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir",
                            szDir, MAX_PATH))
      {
#ifdef _WIN64
        wcsncat_s(szDir, MAX_PATH, L"bin64", _TRUNCATE);
#else
        wcsncat_s(szDir, MAX_PATH, L"bin32", _TRUNCATE);
#endif
        return szDir;
      }

      return L"";
    }

  public:
    resattr_loader(void)
    {
      m_loaded = false;
      m_mods.celog              = NULL;
      m_mods.nl_sysenc_lib      = NULL;
      m_mods.libtiff            = NULL;
#ifdef _WIN64
      m_mods.zlibwapi           = NULL;
#else
      m_mods.zlib1              = NULL;
      m_mods.freetype6          = NULL;
#endif
      m_mods.PoDoFoLib          = NULL;
      m_mods.resattrlib         = NULL;
      m_mods.resattrmgr         = NULL;
    } /* resattr_loader */

    ~resattr_loader(void)
    {
      if (m_loaded)
      {
        unload();
      }
    }

    /** load
     *
     *  \brief Load the resattr-related libs and make member fns available to
     *         call.  When this method succeeds all members of m_fns are
     *         non-NULL and safe to call.
     *
     *  \return true on success, otherwise false.
     */
    _Check_return_
    bool load(void) throw()
    {
      /****************************************************************
       * Load resattr-related libs.  Order is critical due to link depends.
       ***************************************************************/
      std::wstring strCommonPath = GetCommonComponentsDir();

#ifdef _WIN64
      std::wstring strcelog = strCommonPath + L"\\celog.dll";
      std::wstring strnl_sysenc_lib = strCommonPath + L"\\nl_sysenc_lib.dll";
#else
      std::wstring strcelog = strCommonPath + L"\\celog32.dll";
      std::wstring strnl_sysenc_lib = strCommonPath + L"\\nl_sysenc_lib32.dll";
#endif

      std::wstring strlibtiff = strCommonPath + L"\\libtiff.dll";

#ifdef _WIN64
      std::wstring strzlibwapi = strCommonPath + L"\\zlibwapi.dll";
#else
      std::wstring strzlib1 = strCommonPath + L"\\zlib1.dll";
      std::wstring strfreetype6 = strCommonPath + L"\\freetype6.dll";
#endif

      std::wstring strPoDoFoLib = strCommonPath + L"\\PoDoFoLib.dll";

#ifdef _WIN64
      std::wstring strLib = strCommonPath + L"\\resattrlib.dll";
      std::wstring strMgr = strCommonPath + L"\\resattrmgr.dll";
#else
      std::wstring strLib = strCommonPath + L"\\resattrlib32.dll";
      std::wstring strMgr = strCommonPath + L"\\resattrmgr32.dll";
#endif

      m_mods.nl_sysenc_lib = LoadLibraryW(strnl_sysenc_lib.c_str());
      m_mods.libtiff = LoadLibraryW(strlibtiff.c_str());

#ifdef _WIN64
      m_mods.zlibwapi = LoadLibraryW(strzlibwapi.c_str());
#else
      m_mods.zlib1 = LoadLibraryW(strzlib1.c_str());
      m_mods.freetype6 = LoadLibraryW(strfreetype6.c_str());
#endif

      m_mods.PoDoFoLib = LoadLibraryW(strPoDoFoLib.c_str());

      m_mods.resattrlib = (HMODULE)LoadLibraryW(strLib.c_str());
      m_mods.resattrmgr = (HMODULE)LoadLibraryW(strMgr.c_str());

      if (!m_mods.resattrlib || !m_mods.resattrmgr)
      {
        wchar_t szBuf[MAX_PATH] = {0};
        _snwprintf_s(szBuf,MAX_PATH, _TRUNCATE,L"Load error:%p,%p,%d",
                   m_mods.resattrlib, m_mods.resattrmgr, ::GetLastError());
        return false;
      }

      m_fns.CreateAttributeManager = (CreateAttributeManager_t)GetProcAddress(m_mods.resattrmgr, "CreateAttributeManager");
      m_fns.AllocAttributes = (AllocAttributes_t)GetProcAddress(m_mods.resattrlib, "AllocAttributes");
      m_fns.ReadResourceAttributesW = (ReadResourceAttributesW_t)GetProcAddress(m_mods.resattrmgr, "ReadResourceAttributesW");
	  m_fns.ReadResourceAttributesForNTFSW = (ReadResourceAttributesForNTFSW_t)GetProcAddress(m_mods.resattrmgr, "ReadResourceAttributesForNTFSW");
      m_fns.GetAttributeCount = (GetAttributeCount_t)GetProcAddress(m_mods.resattrlib, "GetAttributeCount");
      m_fns.FreeAttributes = (FreeAttributes_t)GetProcAddress(m_mods.resattrlib, "FreeAttributes");
      m_fns.CloseAttributeManager = (CloseAttributeManager_t)GetProcAddress(m_mods.resattrmgr, "CloseAttributeManager");
      m_fns.AddAttributeW = (AddAttributeW_t)GetProcAddress(m_mods.resattrlib, "AddAttributeW");
      m_fns.GetAttributeName = (GetAttributeName_t)GetProcAddress(m_mods.resattrlib, "GetAttributeName");
      m_fns.GetAttributeValue = (GetAttributeValue_t)GetProcAddress(m_mods.resattrlib, "GetAttributeValue");
      m_fns.WriteResourceAttributesW = (WriteResourceAttributesW_t)GetProcAddress(m_mods.resattrmgr, "WriteResourceAttributesW");
	  m_fns.WriteResourceAttributesForNTFSW = (WriteResourceAttributesForNTFSW_t)GetProcAddress(m_mods.resattrmgr, "WriteResourceAttributesForNTFSW");

      if (!(m_fns.CreateAttributeManager && m_fns.AllocAttributes &&
            m_fns.ReadResourceAttributesW && m_fns.ReadResourceAttributesForNTFSW && m_fns.GetAttributeCount &&
            m_fns.FreeAttributes && m_fns.CloseAttributeManager &&
            m_fns.AddAttributeW && m_fns.GetAttributeName &&
            m_fns.GetAttributeValue && m_fns.WriteResourceAttributesW && m_fns.WriteResourceAttributesForNTFSW ))
      {
        return false;
      }

      m_loaded = true;
      return true;
    }

    void unload() throw()
    {
      if (m_mods.resattrmgr != NULL)
      {
        FreeLibrary(m_mods.resattrmgr); m_mods.resattrmgr = NULL;
      }
      if (m_mods.resattrlib != NULL)
      {
        FreeLibrary(m_mods.resattrlib); m_mods.resattrlib = NULL;
      }
      if (m_mods.PoDoFoLib != NULL)
      {
        FreeLibrary(m_mods.PoDoFoLib); m_mods.PoDoFoLib = NULL;
      }
#ifdef _WIN64
      if (m_mods.zlibwapi != NULL)
      {
        FreeLibrary(m_mods.zlibwapi); m_mods.zlibwapi = NULL;
      }
#else
      if (m_mods.freetype6 != NULL)
      {
        FreeLibrary(m_mods.freetype6); m_mods.freetype6 = NULL;
      }
      if (m_mods.zlib1 != NULL)
      {
        FreeLibrary(m_mods.zlib1); m_mods.zlib1 = NULL;
      }
#endif
      if (m_mods.libtiff != NULL)
      {
        FreeLibrary(m_mods.libtiff); m_mods.libtiff = NULL;
      }
      if (m_mods.nl_sysenc_lib != NULL)
      {
        FreeLibrary(m_mods.nl_sysenc_lib); m_mods.nl_sysenc_lib = NULL;
      }
      if (m_mods.celog != NULL)
      {
        FreeLibrary(m_mods.celog); m_mods.celog = NULL;
      }

      m_loaded = false;
    }

    bool is_loaded(void) throw()
    {
      return m_loaded;
    }

    bool ensure_loaded(void) throw()
    {
      if (m_loaded)
      {
        return true;
      }

      return load();
    }

    resattr_modules_t m_mods;
    resattr_functions_t m_fns;

  protected:
    bool m_loaded;
  }; /* class resattr_loader */
} /* namespace nextlabs */



#endif /* __RESATTR_LOADER_HPP__ */
