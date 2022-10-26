
#include <Windows.h>
#include <list>
#include <boost/algorithm/string.hpp>
#include "celog.h"
#include "nlconfig.hpp"
#include "Detours.h"
#include "CEAdapter.h"
#include "CEAdapterConfig.h"

BOOL BJAnsiToUnicode(LPCSTR  pszAnsiBuff, LPWSTR lpWideCharStr, size_t cchDest);

/* The followings are temperal hooking configuration functions
 * Loading the dynamic injection service configuration information for
 * this process.
 *
 * LoadHookingConfiguration_free() must be called on a successful call to
 * this function.
 *
 * Return false when an error occurs.  A return of true indicates there
 * are [0,n] hooks.  It is possible there are no hooks to install.
 */
bool LoadHookingConfiguration(const WCHAR *m_szProcessName,
			      std::vector<HookDetour*>& hlist)
{
  /* Generate the path for injection configuration of this process.  The
     registry containts the Policy Controller root (InstallDir) which
     containts the application configuration below.

     [PolicyControllerDir]/service/injection/[exe_name].ini
   */
  LONG rstatus;
  HKEY hKey = NULL; 

  TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: enter\n"));

  rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,KEY_QUERY_VALUE,&hKey);
  if( rstatus != ERROR_SUCCESS )
  {
    TRACE(CELOG_ERR, _T("LoadHookingConfiguration: %d no reg.\n"), rstatus);
    return true;
  }

  WCHAR ce_root[MAX_PATH] = {0};                 /* InstallDir */
  DWORD ce_root_size = sizeof(ce_root);

  rstatus = RegQueryValueExW(hKey,                     /* handle to reg */      
			     L"PolicyControllerDir",   /* key to read */
			     NULL,                     /* reserved */
			     NULL,                     /* type */
			     (LPBYTE)ce_root,          /* InstallDir */
			     &ce_root_size             /* size (in/out) */
			     );
  RegCloseKey(hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: No configuration path.\n"));
    return true;
  }

  const WCHAR* p = wcsrchr(m_szProcessName,'\\');
  /* The image name must have been found and have at least on character. */
  if( p == NULL || wcslen(p) <= 1 )
  {
    return 0;
  }

  p++;  /* move past '\\' */

  /* Generate path to configuration file */

  WCHAR cfile[MAX_PATH] = {0};

  /* Read default config */

  _snwprintf_s(cfile,_countof(cfile), _TRUNCATE,L"%s\\service\\injection\\all.ini",ce_root);
  TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: %s\n"),cfile);
  LoadHookingConfigurationFile(cfile,hlist);

  /* Read process config */
  _snwprintf_s(cfile,_countof(cfile), _TRUNCATE,L"%s\\service\\injection\\%s.ini",ce_root,p);
  TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: %s\n"),cfile);
  LoadHookingConfigurationFile(cfile,hlist);

  return true;

}/* LoadHookingConfiguration */

bool LoadHookingConfigurationFile( const WCHAR* config_file ,
				   std::vector<HookDetour*>& hlist )
{
  FILE* fp;

  errno_t err = _wfopen_s(&fp, config_file,L"r");
  if (err != 0)
  {
    /* There are no hooks to install */
    TRACE(0, _T("LoadHookingConfiguration: No configuration.\n"));
    return true;
  }

  /* Line length is a possible full path (MAX_PATH) with library symbols (512)*/
  char temp[MAX_PATH + 512] = {0};

  /* Read file line by line and parse hook configuration for each target
     library.

     Params from the line in the follwing format:
       [hook.dll],[target.dll],[target function],[prehook],[posthook]

     Lines beginning with '#' or ';' are considered comments and ignored.
  */
  while( fgets(temp,sizeof(temp),fp) != NULL )
  {
    /* ignore line - empty or comment */
    if( strlen(temp) <= 1 || temp[0] == '#' || temp[0] == ';' )
    {
      TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: ignore empty/comment line\n"));
      continue; 
    }

    /* trim key assignment '[name]=' */
    char* p;

    p = (char*)strstr(temp,"=");

    if( p == NULL || strlen(p) <= 0 )
    {
      TRACE(CELOG_ERR, _T("LoadHookingConfiguration: ignore line\n"));
      continue; /* ignore line */
    }

    p++;  /* pass '=' delimiter */

    /* Pre-hook or post-hook function may be NULL to indicate there is no
       hook to install.
    */
    char* next_token = NULL;     /* parse context for strtok_s */
    char* delim = ",\n";         /* delimiter - '\n' is used to avoid manual trim */

    char* hookName = NULL;       /* hook parameters */
    char* target_libname = NULL;
    char* funcName = NULL;
    char* preDetourFunc = NULL;
    char* postDetourFunc = NULL;

    next_token = (char*)p;       /* set context for strtok_s */

    hookName = strtok_s(next_token,delim,&next_token);         /* hook library */
    target_libname = strtok_s(next_token,delim,&next_token);   /* target library */
    funcName = strtok_s(next_token,delim,&next_token);         /* target function */
    preDetourFunc = strtok_s(next_token,delim,&next_token);    /* pre-hook function */
    postDetourFunc = strtok_s(next_token,delim,&next_token);   /* post-hook function */

    /* At least the hook library parameter must be present (e.g. my_kernel32.dll) */
    if( hookName == NULL )
    {
      TRACE(CELOG_ERR, _T("LoadHookingConfiguration: malformed hook configuration\n"));
      continue;  /* malformed entry */
    }

    HookDetour* phook = new HookDetour;

    HookDetour& hook = *phook;          /* reference to current hook */
    size_t len = 0;

    memset(&hook,0x00,sizeof(HookDetour));

    len = strlen(hookName) + 1;                              /* hook library */
    BJAnsiToUnicode(hookName,hook.hook_libname,_countof(hook.hook_libname));

    /* Get NextLabs install directory and translate [NextLabs] paths. */
    wchar_t nl_root[MAX_PATH] = {0};
    if( NLConfig::GetComponentInstallPath(NULL,nl_root,_countof(nl_root)) == false )
    {
      wcsncpy_s(nl_root,_countof(nl_root),L"C:\\Program Files\\NextLabs",_TRUNCATE);
    }

    std::wstring libpath(hook.hook_libname);
    boost::ireplace_all(libpath,"[NextLabs]",nl_root);
    wcsncpy_s(hook.hook_libname,_countof(hook.hook_libname),libpath.c_str(),_TRUNCATE);

    TRACE(CELOG_INFO,L"LoadHookingConfigurationFile: lib = %s\n",hook.hook_libname);

    hook.level = CEADAPTER_DEFAULT_LEVEL;  /* default adapter level */
    if( target_libname != NULL )
    {
      len = strlen(target_libname) + 1;                       /* target library */
      BJAnsiToUnicode(target_libname,hook.target_libname,_countof(hook.target_libname));

      /* For general hook libraries, there may be a level defined as the second parameter. */
      hook.level = static_cast<int>( target_libname[0] - '0' );

      /* If level is not within bounds [min,max) default is used. */
      if( hook.level < CEADAPTER_MIN_LEVEL || hook.level >= CEADAPTER_MAX_LEVEL )
      {
	hook.level = CEADAPTER_DEFAULT_LEVEL;
      }
    }

    if( funcName != NULL )
    {
      len = strlen(funcName) + 1;                              /* target function */       
      hook.funcName = new char[ len ];
      strncpy_s(hook.funcName,len,funcName,_TRUNCATE);
    }

    if( preDetourFunc != NULL &&_stricmp(preDetourFunc,"NULL") != 0 )   /* pre-hook function */
    {
      len = strlen(preDetourFunc) + 1;
      hook.preDetourFunc = new char[ len ];
      strncpy_s(hook.preDetourFunc,len,preDetourFunc,_TRUNCATE);
    }

    if( postDetourFunc != NULL &&_stricmp(postDetourFunc,"NULL") != 0 ) /* post-hook function */
    {
      len = strlen(postDetourFunc) + 1;
      hook.postDetourFunc = new char[ len ];
      strncpy_s(hook.postDetourFunc,len,postDetourFunc,_TRUNCATE);
    }

    hlist.push_back(&hook);
  }/* while fgets */

  TRACE(CELOG_DEBUG, _T("LoadHookingConfiguration: %d hook(s) read\n"), hlist.size());

  fclose(fp);

  return true;
}/* LoadHookingConfigurationFile */

void LoadHookingConfiguration_free( std::vector<HookDetour*>& hlist )
{
  std::vector<HookDetour*>::iterator it;

  for( it = hlist.begin() ; it != hlist.end() ; ++it )
  {
    HookDetour* detour;

    detour = *it;

    delete detour->funcName;
    delete detour->preDetourFunc;
    delete detour->postDetourFunc;
    delete detour;
  }
}/* LoadHookingConfiguration_free */
