#include <stdlib.h>
#include <fstream>
#include <list>
#include <string>
#include <jni.h>
#include "JavaConstants.h"
#include "nlthread.h"
#include "nlthreadpool.h"
#include "brain.h"
#include "transport.h"
#define  NL_KIF_USER
#if defined (WIN32) || defined (_WIN64)
#include "cekif.h"
#include "cekif_user.h"
#endif
#include "TransCtrl.h"
#include "marshal.h"
#include "cetype.h"
#include "CEsdk.h"
#include "CESDK_private.h"
#include "PDPConn.h"
#include "PDPEval.h"
#include "PDPPrivate.h"
#include "PDPProtect.h"
#include "PDPLog.h"
#if defined (WIN32) || defined (_WIN64)
#include "PDPGeneric.h"
#include "PDPSec.h"
#endif
#include "celog.h"
#include "celog_policy_file.hpp"
#include "celog_policy_windbg.hpp"
#if defined (WIN32) || defined (_WIN64)
#include "nlconfig.hpp"
#include "quenchhash.h"
#endif

#if defined (WIN32) || defined (_WIN64)
#include <direct.h>   //for chdir under win32
#include <dbghelp.h>
#include <wchar.h>
#else
#include "linux_win.h"
#endif

#if defined (WIN32) 
#include <windows.h>
#include "securityattributesfactory.h"
#endif


#include "process.h"
#include <Ntsecapi.h>
#include <shlwapi.h>
#include <tlhelp32.h>

using namespace std;

/*==========================================================================*
 * External variables and functions used in this file.                      *
 *==========================================================================*/
//parsing function implemented in the files parseargsXXX.cpp
char **getJavaArgs(char **lpszArgs, int* pdwLen, int dwArgc, char **lpszArgv);
char* getWorkingDirectory(int dwArgc, char* *lpszArgv);
void parser_freeworkdir(char *dir);
void pareseargs_free(char **args, int argc);
bool getPDPType(int dwArgc, char**lpszArgv);
//Start/stop bootstrap injection service in BootstrapInjectionService.cpp
void StartBootstrapInjection();
void StopBootstrapInjection();
bool  StartEdpManagerAsLogonUser(DWORD dwRDSeessionID);
std::string GetEDPMgrExePath();

// global variables used by other files
JavaVM *PDP_jvm = NULL;
nlthread_sem_t pdpInitializedLock;


/*==========================================================================*
 * Interanl Global variables and functions scoped in this file.             *
 *==========================================================================*/

namespace {
  //Variables for JVM/JNI invoke
  jobject  g_scmEventMgr = NULL;
  jobject  g_cmStub = NULL;
  JNIEnv *PDP_env = NULL;
  jclass g_cmStubClass = NULL;

  bool   bDesktop=true;

  // Critical section to protect global plug-in information
  nlthread_cs_t PDP_plugin_cs;

  //Variables to stop the PDP
  nlthread_mutex_t  PDP_stop_mutex; //mutex for notifying the stop of PDP
  nlthread_cond_t   PDP_stop_cond;  //condition variable for notifying the stop 
  bool PDP_stopped=false;           //stop flag for PDP

  const int MAX_DISPOSE_THREAD_NUM=10;  //Maximium number of dispose threads

  //Tamperproof variables
  nlthread_t regKeyGuardTid;

  //Plugin 
  typedef int (*plugin_entry_t)( void** context );  /* PluginEntry */
  typedef int (*plugin_unload_t)( void* context );  /* PluginUnload */
  /** PluginEntry
   *
   *  \brief Instance of a plug-in that will be controlled by the
   *         Policy Controller.
   */
  typedef struct
  {
    std::wstring libpath;     /* Library name */
    void* context;            /* Context */
    HMODULE hlib;             /* Library handle */
    plugin_entry_t entry;     /* PluginEntry */
    plugin_unload_t unload;   /* PluginUnload */
  } PluginEntry;
  std::list<PluginEntry> plugin_list;
  //--End--Plugin

  //Check file nlQuench.txt exists under NextLabs folder.
  //If yes, cepdpman.exe quits. This works around the GPO installation 
  //problem that installer hangs for unknown reason. We need to replace this
  //method in the near future.
  bool NLQuench(char *workdir)
  {
#if defined (WIN32) || defined (_WIN64)
    bool bQuench=false;
    volatile int dummy;

    if(workdir != NULL) {
      char qFile[512]; /* MAX_PATH */
      _snprintf_s(qFile,_countof(qFile), _TRUNCATE, "%s..\\nlQuench.txt",workdir);  

      std::string data;
      std::ifstream input(qFile, std::ifstream::in);

      if (std::getline(input, data))
      {
        bQuench = (validateData(data, time(NULL), QUENCH_SHARED_SECRET) == ERROR_SUCCESS);
      }

      input.close();

      if (!DeleteFileA(qFile))
      {
        dummy = 0;      // just to keep Veracode happy
      }
    }

    return bQuench;
#else
    return true;
#endif
  }

#if defined (WIN32) || defined (_WIN64)
  //Get all the configuratil files under: 
  //[NextLabs]/Policy Controller/config/plugin/
  void GetAllPluginConfigFiles(WCHAR *pcRoot, std::vector<nlstring> &cFiles)
  {
    if(pcRoot == NULL)
      return;

    WCHAR cDir[MAX_PATH];
    WCHAR sDir[MAX_PATH];

    _snwprintf_s(cDir,
                 sizeof(cDir)/sizeof(cDir[0]),
                 _TRUNCATE,
                 L"%sconfig\\plugin\\",
                 pcRoot);
    _snwprintf_s(sDir,
                 sizeof(sDir)/sizeof(sDir[0]),
                 _TRUNCATE,
                 L"%sconfig\\plugin\\*",
                 pcRoot);
    TRACE(CELOG_DEBUG, _T("GetAllPluginConfigFile: from %s\n"),cDir);

    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError=0;

    // Find the first file in the directory.
    hFind = FindFirstFile(sDir, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {

      TRACE(CELOG_ERR, 
            _T("GetAllPluginConfigFile: FindFirstFile failed: %d\n"), 
            GetLastError());
      return;
    } 
   
    nlstring fullPath;
    // retrieve the directory 
    do {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        //only care about file not directory
        fullPath=cDir;
        fullPath+=ffd.cFileName;
        cFiles.push_back(fullPath);
      }
    } while (FindNextFile(hFind, &ffd) != 0);
 
    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) 
      TRACE(CELOG_ERR, 
            _T("GetAllPluginConfigFile: FindFirstFile failed: %d\n"), dwError);

    FindClose(hFind);
    return;  
  }

  //Get all the plugin Dlls from plugin configuration files
  void GetAllPluginDlls(std::vector<nlstring> &cFiles, 
                        std::vector<nlstring> &pluginDlls,
                        TCHAR *nlDir)
  {
    size_t numConfigFiles=cFiles.size();

    if(numConfigFiles <= 0)
      return; //no plugin cofiguration files specified

    TCHAR libpath[MAX_PATH];
    for(int i=0; i<static_cast<int>(numConfigFiles); i++) {
      std::wfstream mfile;
      mfile.open(cFiles[i].c_str(),std::ios_base::in);
      if( mfile.is_open() == false ) {
        TRACE(CELOG_ERR, 
              L"GetAllPluginDlls: cannot read plug-in configuration: %s\n",
              cFiles[i].c_str());
        continue;
      }

      TCHAR *nlp=NULL;
      TCHAR *tmpP;
      while( mfile.getline(libpath,_countof(libpath))) {
        nlp=_tcsstr(libpath, _T("[NextLabs]"));
        if(nlp && nlp==libpath) { //format [NextLabs]???

          //advance to after "[NextLabs]"
          tmpP=libpath+10;
          size_t endIndex=_tcslen(tmpP);
          if(!(endIndex <= 0 || endIndex == 1)) {
            //format: [NextLabs]\xxx
            tmpP+=1; //after '\'
            nlstring f(nlDir);  
            f+=tmpP;
            pluginDlls.push_back(f.c_str());
          }   
        } else
          pluginDlls.push_back(libpath);
      }/* while */   
      mfile.close();
    }
  }

  static int getPolicyControllerArg(const WCHAR *arg, DWORD defaultValue)
  {
    HKEY hKey = NULL; 
    LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                 TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
                                 0,KEY_QUERY_VALUE,&hKey);
    
    if (rstatus == ERROR_SUCCESS) {
      DWORD dwType = REG_DWORD;
      DWORD dwSize = sizeof(DWORD);
      DWORD dwData;
      rstatus = RegQueryValueExW(hKey,
                                 arg,
                                 NULL,
                                 &dwType,
                                 (LPBYTE)&dwData,
                                 &dwSize);

      if (rstatus == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return (int)dwData;
      }
    }

    RegCloseKey(hKey);

    return defaultValue;
  }

    

  // If hardcoded configuration file names for Outlook Enforcer and Removable Device Enforcer are not in cFiles,
  // add OE and RDE DLLs to pluginDLLs
  //
  // Note: This assumes both ce_root and nlDir end with "\".
  //       The same assumption is made in both GetAllPluginConfigFiles() and GetAllPluginDlls().
  // 
  // IN cFiles: vector of config file names
  // INOUT pluginDLLs: vector of DLL paths
  // IN ce_root: Policy Controller installation directory, e.g., c:\Program Files\NextLabs\Policy Controller
  // IN nlDir: NextLabs products installation directory, e.g., c:\Program Files\NextLabs
  void AddHardCodedPluginDLLs(std::vector<nlstring> &cFiles, std::vector<nlstring> &pluginDlls, WCHAR* ce_root, WCHAR *nlDir)
  {
    // Hardcoded Plugin entries
    typedef struct
    {
      nlstring configFile;
      nlstring dllPath;
    } HardCodedPluginEntry;
    HardCodedPluginEntry hardCodedPluginTable[] =
      {
        {nlstring(L"nl_OE_plugin.cfg"), nlstring(L"Outlook Enforcer\\bin\\OEService.dll")},
        {nlstring(L"rde_plugin.cfg"), nlstring(L"Removable Device Enforcer\\bin\\nl_devenf_plugin.dll")}
      };

    // prefix strings in hardCodedPluginTable
    for (int i=0; i<sizeof(hardCodedPluginTable)/sizeof(HardCodedPluginEntry) ; i++ ) {
      WCHAR path[MAX_PATH];

      // prefix config file name with ce_root.  
      _snwprintf_s(path, MAX_PATH-1, _TRUNCATE, L"%sconfig\\plugin\\%s", ce_root, hardCodedPluginTable[i].configFile.c_str());
      hardCodedPluginTable[i].configFile = nlstring(path);

      // prefix DLL path with nlDir
      _snwprintf_s(path, MAX_PATH-1, _TRUNCATE, L"%s%s", nlDir, hardCodedPluginTable[i].dllPath.c_str());
      hardCodedPluginTable[i].dllPath = nlstring(path);
    }

    // check if hard coded config files exist in cFiles
    // note that this uses case sensitive string comparison
    // These are considered different paths: path and Path, folder\path and folder\\path
    for (int i=0; i<sizeof(hardCodedPluginTable)/sizeof(HardCodedPluginEntry) ; i++ ) {
      vector<nlstring>::iterator iter = find(cFiles.begin(), cFiles.end(), hardCodedPluginTable[i].configFile);
      if (iter == cFiles.end()) {
        // match not found - add DLL path to pluginDlls, if it doesn't exist in pluginDlls
        iter = find(pluginDlls.begin(), pluginDlls.end(), hardCodedPluginTable[i].dllPath);
        if (iter == pluginDlls.end()) {
          pluginDlls.push_back(hardCodedPluginTable[i].dllPath);
        }
      }
    }
  }

  /** PCM_PluginLoad
   *
   *  Load all plug-ins.
   */
  int PCM_PluginLoad(void)
  {
    /* Determine NextLabs root directory */
    LONG rstatus;
    HKEY hKey = NULL; 
    rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
                            0,KEY_QUERY_VALUE,&hKey);
    if( rstatus != ERROR_SUCCESS )
    {
      TRACE(CELOG_ERR, L"PCM_PluginLoad: cannot read root path\n");
      return 0;
    }

    WCHAR ce_root[MAX_PATH];
    DWORD ce_root_size = sizeof(ce_root);
    rstatus = RegQueryValueExW(hKey,TEXT("PolicyControllerDir"),NULL,NULL,
                               (LPBYTE)ce_root,&ce_root_size);
    if( rstatus != ERROR_SUCCESS ) {
      TRACE(CELOG_ERR, L"PCM_PluginLoad: cannot read PolicyControllerDir\n");
      RegCloseKey(hKey);
      return 0;
    }

    //Get InstallDir
    WCHAR nlDir[MAX_PATH];  
    DWORD nlDir_size = sizeof(nlDir);
    rstatus = RegQueryValueExW(hKey,               /* handle to reg */      
                               L"InstallDir",      /* key to read */
                               NULL,               /* reserved */
                               NULL,               /* type */
                               (LPBYTE)nlDir,    /* InstallDir */
                               &nlDir_size       /* size (in/out) */
                               );
    RegCloseKey(hKey);
    if( rstatus != ERROR_SUCCESS ) {
      TRACE(CELOG_ERR, 
            _T("PCM_PluginLoad: cannot read InstallDir.\n"));
      return 0;
    }

    //Get all the plugin configuration files
    std::vector<nlstring> cFiles;
    GetAllPluginConfigFiles(ce_root, cFiles);

    //Get all the plugin Dlls
    std::vector<nlstring> pluginDlls;
    GetAllPluginDlls(cFiles, pluginDlls, nlDir);

    // Add hardcoded plugin DLLs
    AddHardCodedPluginDLLs(cFiles, pluginDlls, ce_root, nlDir);

    size_t numPlugins=pluginDlls.size();
    nlthread_cs_enter(&PDP_plugin_cs);
    for(int i=0; i<static_cast<int>(numPlugins); i++) {
      PluginEntry pe;
      pe.libpath = pluginDlls[i];
      pe.hlib = LoadLibraryW(pluginDlls[i].c_str());
      TRACE(CELOG_INFO, L"PCM_PluginLoad: Loading %s\n",pluginDlls[i].c_str());
      if( pe.hlib == NULL ) {
        TRACE(CELOG_WARNING, 
              L"PCM_PluginLoad: LoadLibray (%s) failed (%d)\n",
              pluginDlls[i].c_str(),GetLastError());
        continue;
      }

      // PluginEntry
      pe.entry  = (plugin_entry_t)GetProcAddress(pe.hlib,"PluginEntry"); 
      // PluginUnload
      pe.unload = (plugin_unload_t)GetProcAddress(pe.hlib,"PluginUnload");  

      TRACE(CELOG_INFO, 
            L"PCM_PluginLoad: PluginEntry @ 0x%p PluginUnload @ 0x%p\n",
            pe.entry, pe.unload);

      /* Unload the library if both entry points are not exported */
      if( pe.entry == NULL || pe.unload == NULL ) {
        TRACE(CELOG_ERR, 
              L"PCM_PluginLoad: missing entry point(s) (%d)\n",GetLastError());
        FreeLibrary(pe.hlib);
        continue;
      }

      int entry_result = 0;
      try {
        /* plug-in entry with address of its context */
        entry_result = pe.entry(&pe.context);  
      }catch(...) {
        entry_result = 0;
      }

      //If the PluginEntry call failed, unload the library and 
      //reset entry points.
      if( entry_result == 0 ) {
        TRACE(CELOG_ERR, "PCM_PluginLoad: PluginEntry failed\n");
        FreeLibrary(pe.hlib);
        continue;
      }

      /* Plug-in loading successful so add to plug-in list. */
      plugin_list.push_back(pe);
    }/* for */
    nlthread_cs_leave(&PDP_plugin_cs);
    TRACE(CELOG_INFO, "PCM_PluginLoad: complete\n");

    return 1;
  }/* PCM_PluginLoad */

  /** PCM_PluginUnload
   *
   *  Unload all plug-ins.
   */
  int PCM_PluginUnload(void)
  {
    nlthread_cs_enter(&PDP_plugin_cs);
    std::list<PluginEntry>::const_iterator plugin;
    for( plugin = plugin_list.begin() ; plugin != plugin_list.end() ; ++plugin )
    {
      assert( plugin->hlib != NULL );
      TRACE(CELOG_INFO,L"PCM_PluginUnload: Unload %s (context 0x%p)\n", 
            plugin->libpath.c_str(), plugin->context);
      try
      {
        if( plugin->unload(plugin->context) == 0 )
        {
          TRACE(CELOG_ERR,"PCM_PluginUnload: PluginUnload failed\n");
        }
        FreeLibrary(plugin->hlib);
      }
      catch(...)
      {
        TRACE(CELOG_ERR,L"PluginUnload in %s caused an exception\n",
              plugin->libpath.c_str());
      }
    }/* for */
    plugin_list.clear();
    nlthread_cs_leave(&PDP_plugin_cs);
    return 0;
  }/* PCM_Stop */
#endif
  /*==========================================================================*
   * The backend thread to mornitor and restore the protected registration    *
   * keys.                                                                    *
   *==========================================================================*/
  extern "C" 
  void *RegKeyGuardThread(void *arg)
  {
#if defined (WIN32) || defined (_WIN64)
    TRACE(CELOG_INFO, _T("Start registration keys guardian\n"));
    PDP_CEPROTECT_RegKeyGuard_Init(bDesktop);

    PDP_CEPROTECT_RegKeyGuard_Run();
  
    TRACE(CELOG_INFO, _T("End registration keys guardian thread\n"));
    nlthread_end();  
#endif
    return NULL;
  }

  void ParseNLArguments(int argc, char **argv)
  {
    for (int i=0; i < argc; ++i)
    {
      if (strcmp(argv[i], "-NLClient") == 0) {
        // Argument is no longer used. Java 7 no longer has the client jvm
        break;
      }
    }
    return;
  }

  //Get all the arguments to control manager
  char **GetControlMgrArgs(int argc, char **argv, int &ctrMgrArgc)
  {
    for(int i=0; i<argc; i++) {
      if(strstr(argv[i], "-D")==0 && strstr(argv[i], "-d")==0 &&
         strstr(argv[i], "/D")==0 &&
         strstr(argv[i], "-X")==0 && strstr(argv[i], "-x")==0 && 
         strstr(argv[i], "/X")==0 && strstr(argv[i], "cepdpman")==0 &&
         strstr(argv[i], "wrkdir=")==0 && strstr(argv[i], "-w")==0 &&
         strstr(argv[i], "-c")==0 && strstr(argv[i], "/c")==0 &&
         strstr(argv[i], "-C")==0 && strstr(argv[i], "/C")==0 &&
         strstr(argv[i], "-NL")==0) {
        ctrMgrArgc=argc-i;
        return &argv[i];
      }
    }
    ctrMgrArgc=0;
    return NULL;
  }

  //Marshal the reply and send it over the socket to PEP 
  CEResult_t Pack_Send_Reply(const nlchar * rpcName,
                             nlsocket sockid,
                             CEResult_t rpcResult,
                             vector<void *> &rpcOutArgs)
  {
	TRACE(CELOG_INFO, _T("Enter Pack_Send_Reply, rcpName=%s\n"), rpcName );
    CEResult_t ret=CE_RESULT_SUCCESS;
    size_t replyLen;
    char *reply=Marshal_PackFuncReply(rpcName,
                                      rpcResult,
                                      rpcOutArgs, 
                                      replyLen);
    if(reply) {
      //Send out the reply
      if(TRANSPORT_Sendn(sockid, replyLen, reply) != CE_RESULT_SUCCESS)
      {
        TRACE(CELOG_ERR,_T("TRANSPORT_Sendn failed\n"));
        ret = CE_RESULT_CONN_FAILED;
      }
	  else
	  {
		  TRACE(CELOG_INFO, _T("TRANSPORT_Sendn Success: len=%d\n"), replyLen);
	  }
      Marshal_PackFree(reply); 
    } else {
      TRACE(CELOG_ERR, _T("Marshal_PackFuncReply (%s) failed\n"), rpcName);
      ret = CE_RESULT_GENERAL_FAILED;
    }
      
	TRACE(CELOG_INFO, _T("Leave Pack_Send_Reply\n"));
    return ret;
  }

  void SignalPDPStopped()
  {
    nlthread_mutex_lock(&PDP_stop_mutex);
    PDP_stopped=true;
    nlthread_cond_signal(&PDP_stop_cond);
    nlthread_mutex_unlock(&PDP_stop_mutex); 
  }

  //thread function: process each incoming request, call Java function 
  //if necessary  
  void* PDP_Disposer(task_t t)
  {
    vector<void *> argv;
    vector<void *> rpcOutArgs;
    nlstring funcname(100u, ' ');
    CEResult_t ret;
    bool bReplyNow=true;
    bool bDoStop=false;
    TRANSPORT_QUEUE_ITEM *sitem = NULL;
#if defined (WIN32) || defined(_WIN64)
    NL_KIF_QUEUE_ITEM *kitem = NULL;
#else
    CEString req_id;
#endif
    // PDP_CECONN_Initialize puts a pointer to an unsigned long long in rpcOutArgs,
    // which are used further down.  We want this pointer to remain in scope, so it
    // has to reference something outside PDP_CECONN_Initialize
    unsigned long long inScopeCEHandleWrapper;

    //Initial local variables
    argv.reserve(CE_MAX_NUM_REQUESTS * 4 + 10);
    rpcOutArgs.reserve(CE_MAX_NUM_REQUESTS * 4 + 10);

    //unpack request
    NL_TRANSCTRL_QUEUE_ITEM* qitem = (NL_TRANSCTRL_QUEUE_ITEM*)(t.taskdata);

    if(qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET) {
      sitem=(TRANSPORT_QUEUE_ITEM *)(qitem->item);
      ret = Marshal_UnPackReqFunc(sitem->buf, funcname,argv);
    } else { //Then the request from KIF
      //Currently, the request from KIF is only of policy evaluation 
#if defined (WIN32) || defined(_WIN64)
      kitem=(NL_KIF_QUEUE_ITEM *)(qitem->item); 
#endif
      funcname=_T("CEEVALUATE_CheckMetadata");
      ret=CE_RESULT_SUCCESS;
    }

	TRACE(CELOG_DEBUG, _T("Enter PDP_Disposer: funcname:%s, unpackReq Result:%d\n"), funcname.c_str(), ret);

    //Process request
    if(ret==CE_RESULT_SUCCESS) {
      //get func id
      CEMarshalFunc* mf = Marsh_GetFuncSignature(funcname.c_str());

      //process each function HERE
      switch(mf->requestID) {
        case FUNCID_CONN_INITIALIZE_Q:
          {
            TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CONN_INITIALIZE_Q\n");
            if(NULL != sitem)
            {
              ret=PDP_CECONN_Initialize (sitem->sock,
                                         PDP_jvm,
                                         g_cmStub,
                                         &inScopeCEHandleWrapper,
                                         argv,
                                         rpcOutArgs);
              TRACE(CELOG_DEBUG,
                    "PDP_Disposer: FUNCID_CONN_INITIALIZE_Q result %d\n", ret);
              break;
            }
            break;
          }
        case FUNCID_CONN_CLOSE_Q:
          TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CONN_CLOSE_Q\n");
          ret=PDP_CECONN_Close (argv, rpcOutArgs);
          TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CONN_CLOSE_Q result %d\n", ret);
          break;
        case FUNCID_PROTECT_LOCKKEY_Q:
#if defined (WIN32) || defined (_WIN64)
          ret=PDP_CEPROTECT_LockKey(argv, rpcOutArgs);
#else
          req_id = (CEString)argv[0];
          rpcOutArgs.push_back(req_id);
          ret = CE_RESULT_FUNCTION_NOT_AVAILBLE;
#endif
          break;
        case FUNCID_PROTECT_UNLOCKKEY_Q:
#if defined (WIN32) || defined (_WIN64)
          ret=PDP_CEPROTECT_UnlockKey(argv, rpcOutArgs);
#else
          req_id = (CEString)argv[0];
          rpcOutArgs.push_back(req_id);
          ret = CE_RESULT_FUNCTION_NOT_AVAILBLE;
#endif
          break;
        case FUNCID_EVALUATE_CHECKMETADATA_Q:
          if(qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET) {
            ret=PDP_CEEVALUATE_CheckMetadata (g_cmStub,
                                              g_cmStubClass,
                                              PDP_jvm,
                                              argv,
                                              sitem->sock);
          } else {//Then the request from KIF
#if defined (WIN32) || defined (_WIN64)
            ret=PDP_CEEVALUATE_CheckMetaFromKIF(g_cmStub,
                                                g_cmStubClass,
                                                PDP_jvm,
                                                kitem);       
#endif
          }
          //Reply of CEEVAL_CheckMetadata is sent back via pdpjni function
          //Java_com_bluejungle_destiny_agent_controlmanager_PDPJni_SendPolicyResponse
          bReplyNow = false;
          break;
        case FUNCID_EVALUATE_CHECKMULTIRESOURCES_Q:
		  TRACE(CELOG_DEBUG, "PDP_Disposer: FUNCID_EVALUATE_CHECKMULTIRESOURCES_Q\n");
          ret = PDP_CEEVALUATE_CheckMulti(g_cmStub,
                                          g_cmStubClass,
                                          PDP_jvm,
                                          argv,
                                          sitem->sock);
          bReplyNow = false;
          break;
        case FUNCID_CEP_STOPPDP_Q:
          {
            TRACE(CELOG_INFO,"PDP_Disposer: FUNCID_CEP_STOPPDP_Q\n");
            /* Stop Policy Controller - verify password */
            ret=PDP_CEP_StopPDP(PDP_jvm, g_cmStub, g_cmStubClass, 
                                argv, rpcOutArgs);
            TRACE(CELOG_INFO,
                  "PDP_Disposer: FUNCID_CEP_STOPPDP_Q result %d\n", ret);

            if(ret == CE_RESULT_SUCCESS)
            {
              bDoStop=true;
#if defined (WIN32) || defined (_WIN64)
              PCM_PluginUnload();
#endif
            }
          }
          break;
        case FUNCID_CELOG_LOGDECISION_Q:
          {
            TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CELOG_LOGDECISION_Q\n");
            /* Logging the user decision */
            ret=PDP_CELOG_LogDecision(PDP_jvm, g_cmStub, g_cmStubClass, 
                                      argv, rpcOutArgs);
            TRACE(CELOG_DEBUG,
                  "PDP_Disposer: FUNCID_CELOG_LOGDECISION_Q result %d\n", ret);
          }
          break;
        case FUNCID_CELOG_LOGASSISTANTDATA_Q:
          {
            TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CELOG_LOGASSISTANTDATA_Q\n");
            /* Logging assistant data */
            ret=PDP_CELOG_LogAssistantData(PDP_jvm, g_cmStub, g_cmStubClass, 
                                           argv, rpcOutArgs);
            TRACE(CELOG_DEBUG,
                  "PDP_Disposer: FUNCID_CELOG_LOGASSISTANTDATA_Q result %d\n", 
                  ret);
          }
          break;
        case FUNCID_GENERIC_FUNCCALL_Q:
          {
            TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_GENERIC_FUNCCALL_Q\n");
#if defined (WIN32) || defined (_WIN64)
            if( NULL != sitem )
            {
              ret=PDP_GenericCall(PDP_jvm, g_cmStub, g_cmStubClass, sitem->sock, argv);
            }
#endif
            TRACE(CELOG_DEBUG,
                  "PDP_Disposer: FUNCID_GENERIC_FUNCCALL_Q result %d\n", 
                  ret);
            bReplyNow = false;
          }
          break;
        case FUNCID_CESEC_MAKETRUSTED_Q:
          {
            TRACE(CELOG_DEBUG,"PDP_Disposer: FUNCID_CESEC_MAKETRUSTED_Q\n");
#if defined (WIN32) || defined (_WIN64)
            ret=PDP_CESEC_MakeTrusted(PDP_jvm, g_cmStub, g_cmStubClass, 
                                      argv, rpcOutArgs);
#endif
            TRACE(CELOG_DEBUG, "PDP_Disposer: FUNCID_CESEC_MAKETRUSTED_Q result %d\n", ret);
          }
                    
          break;
        default:
          TRACE(CELOG_ERR,"PDP_Disposer: FUNCTION_NOT_AVAILBLE\n");
          CEString reqID = (CEString)argv[0];
          rpcOutArgs.push_back(reqID);
          ret = CE_RESULT_FUNCTION_NOT_AVAILBLE;
          break;
      }

	  TRACE(CELOG_ERR, "PDP_Disposer: finished process, bReplyNow:%s\n", bReplyNow ? "True" : "False");

      if(bReplyNow && (NULL != sitem)) {
        //Send back the result over the socket only now
        ret=Pack_Send_Reply(funcname.c_str(),
                            sitem->sock,
                            ret,
                            rpcOutArgs);

        if (mf->requestID == FUNCID_CONN_INITIALIZE_Q) {
          // Free up handle object allocated by PDP_CECONN_INITIALIZE. We don't need it any more and some enforcers (Sharepoint, for example),
          // have no way to free it themselves.
          CEHandle handle = (CEHandle)*((unsigned long long *)rpcOutArgs[1]);
          PDP_CECONN_FreeHandle(handle);
        }
      }

      //Free the memory allocated for request
      if(qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET) {
        Marshal_UnPackFree(funcname.c_str(), argv, true); 
      }
    
    } else {
#if defined (WIN32) || defined (_WIN64)
      TRACE(CELOG_INFO, _T("Invalid request (over sockid=%d): errorno=%d.\n"),
            (qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET)?"socketid":"kifid", 
            (qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET)?sitem->sock:kitem->index,
            ret);
#else
      TRACE(CELOG_INFO, _T("Invalid request (over sockid=%d): errorno=%d.\n"),
            (qitem->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET)?"socketid":"kifid", 
            sitem->sock,
            ret);
#endif
    }
 
    //Free the memory allocated for the thread pool task structure
    NL_TRANSCTRL_MemoryFree(qitem);

    if(bDoStop) {
#if defined(_WIN32) || defined(_WIN64)
      TRACE(CELOG_INFO,"PDP_Disposer: Stopping injection\n");
      /* Unload WDE and dynamic injection (ceBootstrapInjection) */
      StopBootstrapInjection();
#endif
      SignalPDPStopped();
    }

    return NULL;
  }/* PDP_Disposer */

  //thread function: keeping getting request from transport layer, 
  //unpacking it, and pass it to a processing thread
  void* PDP_master(void* arg)
  {
    JNIEnv *PDP_env_l = NULL;

    nlthread_detach(nlthread_self());
 
    //attach env to current thread
    jint res = PDP_jvm->AttachCurrentThread((void**)&PDP_env_l,NULL);
    if(res<0) {
      TRACE(CELOG_ERR, _T("Cannot attach JNI to PDP master thread.\n"));
      SignalPDPStopped();
      return NULL;
    }
  
    NL_TRANSCTRL_QUEUE_ITEM *qitem = NULL;
    char buf[1024];
    jmethodID getInstanceMethod;
    int taskidx = 0;

    //initalize the ControlManagerStub.java
    g_cmStubClass = PDP_env_l->FindClass(CONTROLMGR_STUB_CLASS);
    if(!g_cmStubClass) {
      TRACE(CELOG_ERR, _T("Cannot get control manager stub class.\n"));
      SignalPDPStopped();
      return NULL;
    }

    _snprintf_s (buf, _countof(buf), _TRUNCATE, "()L%s;", CONTROLMGR_STUB_CLASS);  
    getInstanceMethod = PDP_env_l->GetStaticMethodID (g_cmStubClass, 
                                                      CM_STUB_GETINSTANCE_M, 
                                                      buf);
    if(!getInstanceMethod) {
      TRACE(CELOG_ERR, _T("Cannot get instance of control manager stub.\n"));
      SignalPDPStopped();
      return NULL;
    }

    //Get an instance for the java server stub class
    g_cmStub=PDP_env_l->NewGlobalRef(PDP_env_l->CallStaticObjectMethod(
                                       g_cmStubClass,
                                     getInstanceMethod));
    if(!g_cmStub) {
      TRACE(CELOG_ERR, _T("Cannot get global reference of control manager stub.\n"));
      SignalPDPStopped();
      return NULL;
    }

    //initialize the thread pool
    TRACE(CELOG_INFO, _T("PDP_master: initialize nl threadpool.\n"));
    NLTP_init(MAX_DISPOSE_THREAD_NUM,&PDP_Disposer, true);

    //initalize the transport layer
    TRACE(CELOG_INFO, _T("PDP_master: initialize nl transport layer.\n"));
    NL_TRANSCTRL_Serv_Initialize();
  
    //debug
    TRACE(CELOG_INFO, _T("PDP_master: Ready to accept request.\n"));

    //wait for the traffic from transport layer
    for(;;) {
      //Check the stop flag before wait for the next request
      nlthread_mutex_lock(&PDP_stop_mutex);
      if(PDP_stopped) {

        nlthread_mutex_unlock(&PDP_stop_mutex);
        // 7/30/2010 Nao
        // There used to be code to free qitem here.
        // However, that caused cepdpman.exe crash during stopping cepdpman.exe.
        // I deleted the free here.  It will leak qitem, but cepdpman.exe is shutting down anyway.

        break;
      }
      nlthread_mutex_unlock(&PDP_stop_mutex);

      //get request
      NL_TRANSCTRL_Serv_GetNextRequest(&qitem);

      //Check the stop flag before try to handle the new request
      nlthread_mutex_lock(&PDP_stop_mutex);
      if(PDP_stopped) {

        nlthread_mutex_unlock(&PDP_stop_mutex);
        //Free the memory allocated for the thread pool task structure
        NL_TRANSCTRL_MemoryFree(qitem);
        break;
      }
      nlthread_mutex_unlock(&PDP_stop_mutex);

      TRACE(CELOG_DEBUG,_T("PDP master get next\n"));
      
      task_t task;
      task.taskid = taskidx++;
      task.taskdata = (void*)qitem;

      NLTP_doWork(task);
    }
  
    nlthread_detach_end();  
    //debug
    TRACE(CELOG_INFO, _T("PDP_master: exit.\n"));
    return NULL;
  }

  int setup_additional_args(char ***additional_args)
  {
#if defined (WIN32) || defined (_WIN64)
    int cacheTimeout = getPolicyControllerArg(L"CacheHintTimeOut", 300);
    if (cacheTimeout < 0)
    {
      cacheTimeout = 300;
    }

    int trustedProcessTimeout = getPolicyControllerArg(L"TrustedProcessTTL", 600);
    if (trustedProcessTimeout < 0)
    {
      trustedProcessTimeout = 600;
    }

    int trustedProcessDisable = getPolicyControllerArg(L"TrustedProcessDisable", 0);
    if (trustedProcessDisable > 0)
    {
      // Disable via a special timeout value
      trustedProcessTimeout = -1;
    }
#else
    int cacheTimeout = 300;
    int trustedProcessTimeout = 600;
    int trustedProcessDisable = -1;
#endif

    *additional_args = (char **)malloc(2 * sizeof(char *));
    if (*additional_args == NULL)
    {
      return 0;
    }
    
    (*additional_args)[0] = (char *)malloc(128);
    if ((*additional_args)[0] == NULL)
    {
      free(*additional_args);
      return 0;
    }

    (*additional_args)[1] = (char *)malloc(128);
    if ((*additional_args)[1] == NULL)
    {
      free((*additional_args)[0]);
      free(*additional_args);
      return 0;
    }

    _snprintf_s((*additional_args)[0], 128, _TRUNCATE, "SDKCacheHintValue=%d", cacheTimeout);
    _snprintf_s((*additional_args)[1], 128, _TRUNCATE, "TrustedProcessTimeout=%d", trustedProcessTimeout);
    // Number of additional args
    return 2;
  }

  void free_additional_args(char *additional_args[], int cnt)
  {
    if (additional_args != NULL)
    {
      for (int i = 0; i < cnt; i++)
      {
        free(additional_args[i]);
      }

      free(additional_args);
    }
  }

  // ---------------------------------------------------------------------------
  // Function to start the JVM for the Control Module
  // Arguments
  //   - args     : arguments to the Control Module
  //   - args_cnt : number of argruments to the Control Module
  // 
  // Returns
  //   - -1       : Fail to start the JVM Control Module
  //   -  0       : Control Module started, but fail to initialized
  //   -  1       : Control Module started and initialized successfully
  // ---------------------------------------------------------------------------

  int startControlMgr (char ** args, int args_cnt) 
  {
    jclass       agentEventManClass = NULL;
    jclass       agentMainClass = NULL;
    jmethodID    mainMethod;
    jmethodID    initMethod;
    jmethodID    getInstanceMethod;
    jobjectArray jargs;
    char         buf[1024]; 

    // Get all the necessary classes first
    agentMainClass     = PDP_env->FindClass (CM_MAIN_CLASS);
    agentEventManClass = PDP_env->FindClass (CM_EVENTMAN_CLASS);

    if (!agentMainClass || !agentEventManClass) {
      if (PDP_env->ExceptionOccurred()) {
        PDP_env->ExceptionDescribe();
      }
      TRACE(CELOG_ERR,_T("Failed to load the event class.\n"));
      return -1;
    }  

    // Get all the methods from the classes 
    _snprintf_s(buf, _countof(buf), _TRUNCATE, "()L%s;", CM_EVENTMAN_CLASS);
    mainMethod = PDP_env->GetStaticMethodID (agentMainClass, 
                                             CM_MAIN_MAIN_M,
                                             "([Ljava/lang/String;)V");
    initMethod = PDP_env->GetStaticMethodID (agentMainClass,
                                             CM_MAIN_ISINITIALIZED_M, "()Z");
    getInstanceMethod = PDP_env->GetStaticMethodID (agentEventManClass, 
                                                    CM_EVENTMAN_GETINSTANCE_M,
                                                    buf);
                                              
    if (!mainMethod || !initMethod || !getInstanceMethod) {
      TRACE(CELOG_ERR,_T("Failed to find the methods.\n"));
      return -1;
    }

    char **additional_args = NULL;
    int additional_args_cnt = setup_additional_args(&additional_args);

    // Get all the necessary stuffs for calling the methods
    jargs = PDP_env->NewObjectArray (args_cnt + additional_args_cnt, 
                                     PDP_env->FindClass("java/lang/String"), 
                                     NULL);
    if (jargs == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the arguments to run main\n"));
      return -1;
    }

    for (int i = 0; i < args_cnt; i++) {    
      jstring jarg = PDP_env->NewStringUTF(args[i]);

      if (jarg == NULL) {
        TRACE(CELOG_ERR,_T("Failed to build the arg list"));
        return -1;
      }
      PDP_env->SetObjectArrayElement (jargs, i, jarg);
    }

    for (int i = 0; i < additional_args_cnt; i++) {
      jstring jarg = PDP_env->NewStringUTF(additional_args[i]);

      if (jarg == NULL) {
        TRACE(CELOG_ERR,_T("Failed to build the additional_arg list"));
        return -1;
      }
      PDP_env->SetObjectArrayElement(jargs, i+args_cnt, jarg);
    }

    free_additional_args(additional_args, additional_args_cnt);

    // Get an instance for the Event Manager
    g_scmEventMgr = PDP_env->NewGlobalRef (PDP_env->CallStaticObjectMethod 
                                           (agentEventManClass, 
                                            getInstanceMethod));
    if (g_scmEventMgr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot get an Instance from SCM EventMgr\n"));
    }

    // Calling entry point of the control manager
    PDP_env->CallStaticVoidMethod  (agentMainClass, mainMethod, jargs);
  
    // Check if it has been initialized successfully
    if (! PDP_env->CallStaticBooleanMethod (agentMainClass, initMethod)) {
      TRACE(CELOG_ERR,_T("Control manager failed to initialized.\n"));
      return false;
    }
  
    TRACE(CELOG_INFO,_T("Agent has started and initialized successfully.\n"));
    nlthread_sem_post(&pdpInitializedLock);

    return true;
  }

string GetPolicyDir()
{
	LONG rstatus;
	HKEY hKey = NULL; 
	rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		TRACE(CELOG_ERR, L"GetOEInstallDir: cannot read root path\n");
		return "";
	}
	//Get PolicyControllerDir
	char nlDir[MAX_PATH];  
	DWORD nlDir_size = sizeof(nlDir);
	rstatus = RegQueryValueExA(hKey,               /* handle to reg */      
		"PolicyControllerDir",      /* key to read */
		NULL,               /* reserved */
		NULL,               /* type */
		(LPBYTE)nlDir,    /* InstallDir */
		&nlDir_size       /* size (in/out) */
		);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS ) {
		TRACE(CELOG_ERR, 
			_T("GetOEInstallDir: cannot read InstallDir.\n"));
		return "";
	}
	string strOEInstallDir = nlDir;

	if(strOEInstallDir.length() > 1)
	{
		if (strOEInstallDir[strOEInstallDir.length() -1] == '\\')
		{
			strOEInstallDir += "jre\\bin\\";
		}
		else
		{
			strOEInstallDir += "\\jre\\bin\\";
		}
	}
	return strOEInstallDir;
}
 

  // ---------------------------------------------------------------------------
  // Function to create a JVM for some Java class to run
  // Arguments
  //   - options     : options to the JVM
  //   - options_cnt : number of options
  // 
  // Returns
  //   - NULL : Fail to create the JVM
  //   -      : JVM env pointer
  // ---------------------------------------------------------------------------
  JNIEnv * invokeJVM (char **argv, int argc, char ** options, int options_cnt)
  {
    JNIEnv       * jenv;
    JavaVMInitArgs vm_args;
    JavaVMOption   joptions[MAX_JVM_OPTIONS];
    jint           rc;
    int            i, j;

    memset (joptions, 0x0, sizeof(joptions));
  
    for (i = 0; i < options_cnt; i++) {
      joptions[i].optionString = options[i];
    }

    for (j = 0; j < argc; j++) {
      if (strncmp(argv[j], "-NLJVM", 6) == 0) {
        // Arguments of the form -NLJVM-randomjavaargument will be passed as -randomjavaargument
        joptions[i++].optionString = argv[j]+6;
      }
    }

    vm_args.version             = JNI_VERSION_1_2;
    vm_args.options             = joptions;
    vm_args.nOptions            = i;
    vm_args.ignoreUnrecognized  = JNI_FALSE;

#if defined (WIN32) || defined (_WIN64)
    typedef jint  (JNICALL *CreateJavaVM)(JavaVM **, void **, void *);

    HMODULE hJniDll = NULL;

#if defined(_WIN64)
	hJniDll = ::LoadLibraryA ("jre\\bin\\server\\jvm.dll");
#else
    hJniDll = ::LoadLibraryA ("jre\\bin\\client\\jvm.dll");
#endif
    
    if (hJniDll == NULL) {
      hJniDll = ::LoadLibraryA ("jvm.dll");
    }

    if (hJniDll == NULL) {
		DWORD dwDllLength = 2048;
		char szDllPath[2048] = {0};
		GetDllDirectoryA(dwDllLength,szDllPath);
		string strDllPath = GetPolicyDir();
		SetDllDirectoryA(strDllPath.c_str());
#if defined(_WIN64)
		string strJvmdll = strDllPath + "server\\jvm.dll";
#else
		string strJvmdll = strDllPath + "client\\jvm.dll";
#endif
    
		hJniDll = ::LoadLibraryA (strJvmdll.c_str());
		SetDllDirectoryA(szDllPath);
		if (hJniDll == NULL)
		{
			TRACE(CELOG_ERR, _T("jvm.dll could not be loaded. Make sure wrkdir is set correctly or jvm.dll is in your path."));
			return NULL;
		}
	}

    // QTP and other tools can put options here that can kill us.  Anyway, I don't think we actually should let random
    // environment variables affect how we run
    TRACE(CELOG_ERR, _T("Overriding JAVA_TOOL_OPTIONS for cepdpman\n"));
    (void)_putenv("JAVA_TOOL_OPTIONS=");
        
    CreateJavaVM pfnCreateJavaVM=(CreateJavaVM)::GetProcAddress(hJniDll, 
                                                                "JNI_CreateJavaVM");

    rc = (pfnCreateJavaVM) (&PDP_jvm, (void **)&jenv, &vm_args);
#else
    rc = JNI_CreateJavaVM (&PDP_jvm, (void **) &jenv, &vm_args);
#endif

    if (rc < 0) {
      TRACE(CELOG_ERR, _T("Cannot create Java VM.\n"));
      return NULL;
    }

    return jenv;
  }

  //Start tamper resistance
  bool StartTamperResistances()
  {
    //Launch Registry Key protection for Windows platform
#if defined (WIN32) || defined (_WIN64)
    bool bResult=true;
    bResult=nlthread_create(&regKeyGuardTid, 
                            (nlthread_func)(&RegKeyGuardThread), NULL);
    if(!bResult) {
      TRACE(CELOG_ERR, _T("Failed to create RegKey gradian thread\n"));
      return false;
    } 
#endif  
    return true;
  }

  //Shut down tamper resistance
  void StopTamperResistance() 
  {
    //1. Stop registration key guard on Windows platform
#if defined (WIN32) || defined (_WIN64)
    PDP_CEPROTECT_RegKeyGuard_Stop();
    nlthread_join(regKeyGuardTid);
#endif 
  }
}

#if defined(_WIN32) || defined(_WIN64)
/** efilter
 *
 *  Excecption filter.  When called a minidump in the Policy Controller
 *  root directory name 'cepdpman.dmp' is created.
 *
 *  \param exception_pointers (in) Exception pointers provided by
 *                                 OS for the exception that occurred.
 *
 *
 */
static LONG WINAPI efilter( LPEXCEPTION_POINTERS exception_pointers )
{
  HANDLE ph = INVALID_HANDLE_VALUE;
  HANDLE fh = INVALID_HANDLE_VALUE;

  ph = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());

  if( ph == NULL )
  {
    goto dump_complete;
  }

  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
                          0,
                          KEY_QUERY_VALUE,
                          &hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    goto dump_complete;
  }

  /* Policy Controller root directory (InstallDir) */
  WCHAR pc_root[MAX_PATH];
  DWORD pc_root_size = sizeof(pc_root);

  rstatus = RegQueryValueExW(hKey,L"InstallDir",NULL,NULL,(LPBYTE)pc_root,&pc_root_size);
  RegCloseKey(hKey);
  if( rstatus != ERROR_SUCCESS )
  {
    goto dump_complete;
  }

  WCHAR dumpfile[MAX_PATH];
  _snwprintf_s(dumpfile,sizeof(dumpfile)/sizeof(dumpfile[0]),_TRUNCATE,L"%s\\cepdpman.dmp",pc_root);

  fh = CreateFileW(dumpfile,
                   GENERIC_READ|GENERIC_WRITE,
                   0,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

  if( fh == INVALID_HANDLE_VALUE )
  {
    goto dump_complete;
  }

  MINIDUMP_EXCEPTION_INFORMATION mn_exception_info;
  int type = (int)(MiniDumpWithDataSegs |
                   MiniDumpWithHandleData |
                   MiniDumpWithProcessThreadData);

  /********************************************************************************************
   * Debug mode will generate a complete process dump.
   *******************************************************************************************/
  if( NLConfig::IsDebugMode() == true )
  {
    type |= MiniDumpWithFullMemory;
  }

  mn_exception_info.ThreadId = GetCurrentThreadId();
  mn_exception_info.ExceptionPointers = exception_pointers;
  mn_exception_info.ClientPointers = FALSE;

  /* Win2K does not have MiniDumpWriteDump but this source must support platforms
     which have this function.  Look up the function and call it if it can be
     found.
  */
  typedef BOOL (WINAPI *MiniDumpWriteDump_fn_t)( HANDLE hProcess,
                                                 DWORD ProcessId,
                                                 HANDLE hFile,
                                                 MINIDUMP_TYPE DumpType,
                                                 PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                                 PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                                 PMINIDUMP_CALLBACK_INFORMATION CallbackParam );

  HMODULE hlib = LoadLibraryA("dbghelp.dll");

  if( hlib == NULL )
  {
    goto dump_complete;
  }

  MiniDumpWriteDump_fn_t MiniDumpWriteDump_fn = (MiniDumpWriteDump_fn_t)GetProcAddress(hlib,"MiniDumpWriteDump");

  if( MiniDumpWriteDump_fn != NULL )
  {
    MiniDumpWriteDump_fn(ph,GetCurrentProcessId(),fh,(MINIDUMP_TYPE)type,
                         &mn_exception_info,NULL,NULL);
  }
  FreeLibrary(hlib);

 dump_complete:

  if( ph != NULL )
  {
    CloseHandle(ph);
  }

  if( fh != INVALID_HANDLE_VALUE )
  {
    CloseHandle(fh);
  }

  /* This will supress the pop-up to the user.  The process is quietly
     terminated.
  */
  return EXCEPTION_EXECUTE_HANDLER;
}/* efilter */
#endif /* defined(_WIN32) || defined(_WIN32) */


class CRegsvr
{
public:
	CRegsvr();
	~CRegsvr();
public:
	static unsigned __stdcall RegThread( void* pArguments );
	static void CallNLRegisterPlugIn();
	static void CallWDERegisterPlugIn();
	static wstring GetPolicyControlInstallDir();
	static wstring GetDesktopEnforcerInstallDir();

	HANDLE m_hThread;
    unsigned m_threadID ;
	static HANDLE m_hEvent;  
};

HANDLE CRegsvr::m_hEvent = NULL;


CRegsvr::CRegsvr()
{
	m_hThread = NULL;
	m_threadID = 0;
	CRegsvr::m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(CRegsvr::m_hEvent != NULL)
	{
    // Create the thread.
		m_hThread = (HANDLE)_beginthreadex( NULL, 0, &RegThread, NULL, 0, &m_threadID );
	}
}
CRegsvr::~CRegsvr()
{
	if(CRegsvr::m_hEvent != NULL)
		CloseHandle(CRegsvr::m_hEvent);
}



wstring CRegsvr::GetPolicyControlInstallDir()
{
	LONG rstatus;
	HKEY hKey = NULL; 
	rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"),
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		TRACE(CELOG_ERR, L"GetOEInstallDir: cannot read root path\n");
		return L"";
	}
	//Get PolicyControllerDir
	WCHAR nlDir[MAX_PATH];  
	DWORD nlDir_size = sizeof(nlDir);
	rstatus = RegQueryValueExW(hKey,               /* handle to reg */      
		L"PolicyControllerDir",      /* key to read */
		NULL,               /* reserved */
		NULL,               /* type */
		(LPBYTE)nlDir,    /* InstallDir */
		&nlDir_size       /* size (in/out) */
		);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS ) {
		TRACE(CELOG_ERR, 
			_T("GetOEInstallDir: cannot read InstallDir.\n"));
		return L"";
	}
	wstring strOEInstallDir = nlDir;

	if(strOEInstallDir.length() > 1)
	{
		if (strOEInstallDir[strOEInstallDir.length() -1] == '\\')
		{
			strOEInstallDir += L"bin\\";
		}
		else
		{
			strOEInstallDir += L"\\bin\\";
		}
	}
	

	OutputDebugString(strOEInstallDir.c_str());
	 
	return strOEInstallDir;
}

wstring CRegsvr::GetDesktopEnforcerInstallDir()
{
	LONG rstatus;
	HKEY hKey = NULL; 
	rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer"),
		0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		TRACE(CELOG_ERR, L"GetDesktopEnforcerInstallDir: cannot read root path\n");
		return L"";
	}
	//Get DesktopEnforcerInstallDir
	WCHAR nlDir[MAX_PATH];  
	DWORD nlDir_size = sizeof(nlDir);
	rstatus = RegQueryValueExW(hKey,               /* handle to reg */      
		L"InstallDir",      /* key to read */
		NULL,               /* reserved */
		NULL,               /* type */
		(LPBYTE)nlDir,    /* InstallDir */
		&nlDir_size       /* size (in/out) */
		);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS ) {
		TRACE(CELOG_ERR, 
			_T("GetDesktopEnforcerInstallDir: cannot read InstallDir.\n"));
		return L"";
	}
	wstring strDesktopEnforcerInstallDir = nlDir;

	if(strDesktopEnforcerInstallDir.length() > 1)
	{
		if (strDesktopEnforcerInstallDir[strDesktopEnforcerInstallDir.length() -1] == '\\')
		{
			strDesktopEnforcerInstallDir += L"bin\\";
		}
		else
		{
			strDesktopEnforcerInstallDir += L"\\bin\\";
		}
	}


	OutputDebugString(strDesktopEnforcerInstallDir.c_str());

	return strDesktopEnforcerInstallDir;
}

void CRegsvr::CallWDERegisterPlugIn()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	TRACE(CELOG_INFO, _T("CallWDERegisterPlugIn in PDPMan.cpp\n"));
	wstring wstrInstallDir = GetDesktopEnforcerInstallDir();
	wstring wstrNLRegPath = wstrInstallDir + L"NlRegisterPlugins.exe --wde --register";
	WCHAR wcNLRegPath[MAX_PATH] = { 0 };
	wcscpy_s(wcNLRegPath, MAX_PATH, wstrNLRegPath.c_str());
	wchar_t szLog[MAX_PATH*2] = {0}; 
	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		wcNLRegPath,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,  // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		_stprintf_s(szLog, _T("CreateProcess:%s failed. module can't be load,GetLastError = [%d]\n"), wstrNLRegPath.c_str(),GetLastError());
		OutputDebugString(szLog);
		return ;
	}

	WaitForSingleObject( pi.hProcess, INFINITE );

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );


	return ;

}

void CRegsvr::CallNLRegisterPlugIn()
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	TRACE(CELOG_INFO, _T("CallNLRegisterPlugIn in PDPMan.cpp\n"));
	wstring wstrInstallDir = GetPolicyControlInstallDir();
	wstring wstrNLRegPath = wstrInstallDir + L"NLRegisterPlugIn.exe";
	wchar_t szLog[MAX_PATH*2] = {0}; 
    // Start the child process. 
    if( !CreateProcess( wstrNLRegPath.c_str(),   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,  // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
		_stprintf_s(szLog, _T("CreateProcess:%s failed. module can't be load,GetLastError = [%d]\n"), wstrNLRegPath.c_str(),GetLastError());
		OutputDebugString(szLog);
       return ;
    }

	 WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );


	return ;
	
}

unsigned __stdcall CRegsvr::RegThread( void* pArguments )
{
	while(1)
	{
		CallNLRegisterPlugIn();
		CallWDERegisterPlugIn();
		DWORD dwRet = WaitForSingleObject( CRegsvr::m_hEvent, 86400000 );
		if(dwRet==WAIT_TIMEOUT)
		{
			TRACE(CELOG_INFO, _T("RegThread WaitForSingleObject is WAIT_TIMEOUT\n"));
			continue;
		}
		else if(dwRet==WAIT_OBJECT_0)  
		{
			TRACE(CELOG_INFO, _T("RegThread WaitForSingleObject is WAIT_OBJECT_0\n"));
			break;
		}
		else
		{
			TRACE(CELOG_INFO, _T("RegThread WaitForSingleObject dwRet is [%d]\n"),dwRet);
			break;
		}
	}
	
    _endthreadex( 0);
    return 0;
} 


void GetAllWindowsSessionID(std::list<unsigned long>& lstSessionID)
{
	lstSessionID.clear();

	//enum all logon users
	unsigned long uSessionCount = 0;
	LUID* pLUIDs = NULL;
	NTSTATUS  ntStatus = LsaEnumerateLogonSessions(&uSessionCount, &pLUIDs);

	TRACE(CELOG_INFO, L"LsaEnumerateLogonSessions result:%s\n", ntStatus == 0x00000000 ? L"Success" : L"failed");

	for (unsigned long iLogonSession = 0; iLogonSession < uSessionCount; iLogonSession++)
	{
		SECURITY_LOGON_SESSION_DATA* pSessionData = NULL;
		LsaGetLogonSessionData(&pLUIDs[iLogonSession], &pSessionData);
		if (pSessionData != NULL)
		{		
			if ((pSessionData->Session > 0) && 
				(std::find(lstSessionID.begin(), lstSessionID.end(), pSessionData->Session)==lstSessionID.end()) )
			{
				lstSessionID.push_back(pSessionData->Session);
			}

			LsaFreeReturnBuffer(pSessionData);
		}
	}

	LsaFreeReturnBuffer(pLUIDs);
}

bool IsEdpManagerRunning(unsigned long uSessionID)
{
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(L"CreateToolhelp32Snapshot  fail!\n");
		return false;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	bool bRunning = false;
	std::list<DWORD> lstDwEdpMgrProcessID;
	do
	{
		if (_tcsstr(pe32.szExeFile, _T("edpmanager")))
		{
			lstDwEdpMgrProcessID.push_back(pe32.th32ProcessID);

			DWORD dwSessionID = 0;
			BOOL bGetSessionID = ProcessIdToSessionId(pe32.th32ProcessID, &dwSessionID);
			if (bGetSessionID && (dwSessionID==uSessionID))
			{
				bRunning = true;
				break;
			}
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	hProcessSnap = NULL;

	return bRunning;
}

void StartEdpManagerForAllUsers()
{
	//get all sessions
	std::list<unsigned long> lstSessions;
	GetAllWindowsSessionID(lstSessions);

	//start edpmanager for all sessions
	std::list<unsigned long>::iterator itSession = lstSessions.begin();
	while (itSession != lstSessions.end())
	{
		//is this session alread have an edpmanager.exe running
		if (!IsEdpManagerRunning(*itSession))
		{
			StartEdpManagerAsLogonUser(*itSession);
		}
		
		itSession++;
	}
}


void KillCAService()
{
    HANDLE hProcessSnap = NULL;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return;
    }
    DWORD dwPId = 0;
    do
    {
        if (_tcsstr(pe32.szExeFile, _T("nlca_service.exe")))
        {
            dwPId = pe32.th32ProcessID;
            break;
        }

    } while (Process32Next(hProcessSnap, &pe32));
    if (dwPId != 0)
    {
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPId);
        if (hProc != NULL)
        {
            TerminateProcess(hProc, 0);
            CloseHandle(hProc);
        }
    }
    CloseHandle(hProcessSnap);
}
/*==========================================================================*
 * Exported functions implemented in this file.                             *
 *==========================================================================*/
void ServiceStart(int argc, char** argv)
{
  //parse the command line arguments
  char** jvm_options = NULL; 
  int jvm_options_cnt = 0;
  char* workdir = NULL;

  nlthread_sem_init(&pdpInitializedLock, 0);

#if defined(_WIN32) || defined(_WIN64)
  /* Install exception filter for minidump generation */
  SetUnhandledExceptionFilter(efilter);
#endif

  jvm_options = getJavaArgs(jvm_options, &jvm_options_cnt, argc, argv);
  workdir = getWorkingDirectory(argc,argv);
  bDesktop= getPDPType(argc, argv);

  CELogS::Instance()->SetLevel(CELOG_INFO); /* default */
#if defined(WIN32) || defined(_WIN64)
  if( NLConfig::IsDebugMode() == true )
#else
    if(true)
#endif
    {
      CELogS::Instance()->SetLevel(CELOG_DEBUG);
#if defined(WIN32) || defined(_WIN64)
      CELogS::Instance()->SetPolicy( new CELogPolicy_WinDbg() );
#endif
    }

  /* If the working directory is set, generate a log file path to create a
     file log policy instance.
  */
  if( workdir != NULL )
  {
    char logfile[512]; /* MAX_PATH */
    _snprintf_s(logfile,512,_TRUNCATE,"%s//agentLog//pclog.txt",workdir);
    CELogPolicy_File* log_policy = new CELogPolicy_File(logfile);
    log_policy->SetMaxLogSize( 1024 * 1024 * 5 ); /* 5MB */
    CELogS::Instance()->SetPolicy(log_policy);
  }
  else
  {
    TRACE(CELOG_ERR,_T("No working directory supplied.  Exiting\n"));

    // No wrkdir is a huge problem.  Desperately log something and then exit
    // The file will appear in windows/system32
#if defined (WIN32) || defined (_WIN64)
    FILE *outfile = NULL;
    fopen_s(&outfile, "./cepdpman.log", "a+");
#else
    FILE *outfile = fopen("./cepdpman.log", "a+");
#endif
    if (outfile != NULL)
    {
      fprintf(outfile, "No working directory supplied.  cepdpman.exe exiting\n");
      fclose(outfile);
    }
    return;
  }
  
#if defined (WIN32) || defined (_WIN64)
  if(_chdir(workdir)!=0) {
    TRACE(CELOG_ERR,_T("Cannot change working directory to %s\n"),workdir);
    return;
  }
#else
  if(chdir(workdir)!=0) {
    TRACE(CELOG_ERR,_T("Cannot change working directory to %s\n"),workdir);
    return;
  }
#endif

  //Check file nlQuench.txt exists under NextLabs folder.
  //If yes, quit. This works around the GPO installation problem that
  //installer hangs for unknown reason. We need to replace this
  //method in the near future.
  if(NLQuench(workdir)) {
    nlthread_cs_init(&PDP_plugin_cs);
    PCM_PluginLoad();

    PCM_PluginUnload();
    TRACE(CELOG_INFO,_T("File nlQuench.txt exists. Quit!\n"));
    return;
  }  

  //Start tamperproof
  TRACE(CELOG_INFO,"ServiceStart: Starting Tamper Resistance\n");
  if(!StartTamperResistances()) {
    TRACE(CELOG_ERR,"Fail to launch tamper resistance.\n");
    return;
  }

  // -NL arguments used for various, nefarious, purposes
  ParseNLArguments(argc, argv);

  //Invoke JVM
  TRACE(CELOG_INFO,"ServiceStart: Starting JVM\n");
  PDP_env = invokeJVM(argv, argc, jvm_options,jvm_options_cnt);
  if (PDP_env == NULL) {
    TRACE(CELOG_ERR,"Fail to invoke JVM.\n");
    return;
  }
  //Start Control Manager
  TRACE(CELOG_INFO,"ServiceStart: Starting Contrl Manager\n");
  int ctrlMgrArgc;
  char **ctrlMgrArgv=GetControlMgrArgs(argc, argv, ctrlMgrArgc);
  TRACE(CELOG_DEBUG,_T("CM opts=%d\n"),ctrlMgrArgc);
  startControlMgr(ctrlMgrArgv, ctrlMgrArgc);

#if defined (WIN32)
  TRACE(CELOG_INFO,_T("ServiceStart: Store Policy Controller PID.\n"));

  /* Before installing/configuration any injection scheme, the PID of the
     Policy Controller must be set in the shared memory identified by the
     PC UID.  The file mapping is released when the Policy Controller is
     shutting down.
  */
  HANDLE hPIDFileMapping;
  hPIDFileMapping = CreateFileMappingA(INVALID_HANDLE_VALUE,    // current file handle 
                                       SecurityAttributesFactory::GetSecurityAttributes(), // security attributes
                                       PAGE_READWRITE,                          // read/write permission 
                                       0,                                       // max. object size 
                                       sizeof(DWORD),                           // size of hFile 
                                       "b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"); // Policy Controller UID
  
  if( hPIDFileMapping == NULL || GetLastError() == ERROR_ALREADY_EXISTS )
  {
    //Most likely there is already an agent running.
    TRACE(CELOG_WARNING, _T("Policy Controller already running.\n"));
  }
  else
  {
    DWORD* pid = NULL;
    pid = (DWORD*)MapViewOfFile(hPIDFileMapping,     // handle to mapping object 
                                FILE_MAP_ALL_ACCESS, // read/write permission 
                                0,                   // max. object size 
                                0,                   // size of hFile 
                                0);                  // map entire file  

    if( pid != NULL )
    {
      /* Assign PID of current process which is the policy controller. */
      *pid = GetCurrentProcessId();
      UnmapViewOfFile(pid);
    }
  }

  TRACE(CELOG_INFO,"ServiceStart: Starting plug-ins\n");
  nlthread_cs_init(&PDP_plugin_cs);
  PCM_PluginLoad();

  TRACE(CELOG_INFO,"ServiceStart: Start process injection\n");
  StartBootstrapInjection();
  TRACE(CELOG_INFO,"ServiceStart: injection complete\n");
#endif /* WIN32 */

  // The "generic" call handler needs some initialization.  Called after
  // jvmInvoke()
#if defined(WIN32) || defined(_WIN64)
  PDP_GenericCallInit(PDP_jvm);
#endif

  CRegsvr Reg;	// help install to register Nextlabs Plugin
  //initialize the condition and mutex variable 
  nlthread_cond_init(&PDP_stop_cond);
  nlthread_mutex_init(&PDP_stop_mutex);

  //After the JVM starts, start a master thread to listen on socket
  TRACE(CELOG_INFO, 
        _T("ServiceStart: Create PDP master thread that serves request\n"));
  nlthread_t  tid;
  nlthread_detach_create(&tid,
                         (nlthread_detach_func)(&PDP_master),
                         NULL);

  HKEY hKey = 0;
  LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
  if(ERROR_SUCCESS == lRet)
  {
	  char EDPManager[MAX_PATH] = { 0 };
	  DWORD BufferSize = sizeof EDPManager;
	  if(ERROR_SUCCESS != RegQueryValueExA(hKey, "EDPManager", NULL, NULL, (LPBYTE)EDPManager, &BufferSize))
	  {
		  std::string strEDPMgrExePath = "\"" + GetEDPMgrExePath() + "\"";

		  RegSetValueExA(hKey, "EDPManager", 0, REG_SZ, (BYTE*)strEDPMgrExePath.c_str(), strEDPMgrExePath.size() + sizeof(char));

		  //start edpmanager for all logon users
		  StartEdpManagerForAllUsers();
	  }

	  RegCloseKey(hKey);
  }

  TRACE(CELOG_INFO, _T("ServiceStart: Ready\n"));
  //Wait to stop
  nlthread_mutex_lock(&PDP_stop_mutex);
  while(!PDP_stopped)
  {
    nlthread_cond_wait(&PDP_stop_cond,&PDP_stop_mutex);
  }
  nlthread_mutex_unlock(&PDP_stop_mutex);
  TRACE(CELOG_INFO, _T("ServiceStart: Shutting Down\n"));

  SetEvent(CRegsvr::m_hEvent);

#if defined (WIN32) 
  /* Mapping for PID file must persist until the policy controller is
     terminated.  The stop signal has been received here so the map
     file is removed.
  */
  if( hPIDFileMapping != NULL )
  {
    CloseHandle (hPIDFileMapping);
  }
#endif /* WIN32 */

  //Wait all PDP disposer threads to be idle then exit from PDP 
  NLTP_WaitThreadAllIdle();
  NLTP_Free();

  //Shut down transport control layer
  NL_TRANSCTRL_Shutdown();

  //Shut down tamper resistance
  StopTamperResistance();

  //Destruct the condition variable 
  nlthread_cond_destroy(&PDP_stop_cond);
  nlthread_mutex_destroy(&PDP_stop_mutex);

  // Kill CA_Service.exe if it can't be shutdown
  // by normal way.
  KillCAService();

  //Cleaning up memory on heap
  pareseargs_free(jvm_options,jvm_options_cnt);
  parser_freeworkdir(workdir);

  TRACE(CELOG_INFO,"ServiceStart: Policy controller shut down.\n");

  // Should not exit here since this is not in the highest level
  // otherwise, the system is treating this as a crash
}/* ServiceStart */

#if defined(WIN32)
/* StartEnfrocerDriver
 *
 * Start the ProcDetect driver for trapping creationg of processes.
 */
static bool StartEnforcerDriver(void)
{

  SC_HANDLE schService;
  SC_HANDLE schSCManager;

  TRACE(CELOG_INFO,"StartEnforcerDriver\n");

  schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
  if( schSCManager == NULL )
  {
    TRACE(CELOG_INFO,"StartEnforcerDriver: failed\n");
    return false;
  }

  schService = OpenServiceA(schSCManager,"ProcDetect",SERVICE_START);
  if( schService == NULL )
  {
    TRACE(CELOG_INFO,"StartEnforcerDriver: cannot open ProcDetect\n");
    CloseServiceHandle(schSCManager);
    return false;
  }

  BOOL rv = StartService(schService,NULL,NULL);

  CloseServiceHandle(schService);
  CloseServiceHandle(schSCManager);

  if( rv )
  {
    return false;
  }

  TRACE(CELOG_INFO,"StartEnforcerDriver: started\n");

  return true;
}/* StartEnforcerDriver */
#endif /* defined(WIN32) */

