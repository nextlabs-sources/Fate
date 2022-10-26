#include <jni.h>
#include <vector>
#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#pragma warning(push)
#pragma warning(disable : 6386)
#include <ws2tcpip.h>
#pragma warning(pop)
#include <psapi.h>
#endif
#if defined (Linux) || defined (Darwin)
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include "dstypes.h"
#endif
#include <errno.h>
#include "JavaConstants.h"
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "CESDK_private.h"
#include "transport.h"
#include "TransCtrl.h"
#include "marshal.h"
#define  NL_KIF_USER
#include "cekif.h"
#include "cekif_user.h"
#include "JavaConstants.h"
#include "celog.h"
#if defined (WIN32) || defined (_WIN64)
#include "nlTamperproofConfig.h"
#else
#ifdef nlsprintf
#undef nlsprintf
#define nlsprintf snprintf
#endif
#endif

using namespace std;

//define NewString func for JNI
#if defined (Linux) || defined (Darwin)
#define JNI_NEWSTRING(s,l)   env->NewStringUTF(s)
#endif
#if defined (WIN32) || defined (_WIN64)
#define JNI_NEWSTRING(s,l)   env->NewString((const jchar *)s,(jsize)l)
#endif

/*==========================================================================*
 * Interanl Global variables and functions scoped in this file.             *
 *==========================================================================*/
namespace {
#if defined (_WIN64) || defined (WIN32)
#define PDPEVAL_GetTime(r,h,l) if(h==0) r=l; else { r=h*0x100000000; r+=l;}
#else 
#define PDPEVAL_GetTime(r,h,l) r=l; 
#endif

  enum {PDPEVAL_MAX_BUF_LENGTH=1024};

  /* Make sure this table is in sync with the CEsdk.h action enumeration */
  /* Also, the strings must be defined in the JavaConstants.h            */
  /* The actions and corresponding strings need to be sync in the following
     files: JavaConstans.h, PDPEval.cpp, eval.cpp, CEsdk.h. */
  nlchar* actionTable[]={ NULL,
                          CE_ACTION_STRING_READ,                   
                          CE_ACTION_STRING_DELETE,                 
                          CE_ACTION_STRING_MOVE,                   
                          CE_ACTION_STRING_COPY,                   
                          CE_ACTION_STRING_WRITE,                  
                          CE_ACTION_STRING_RENAME,                 
                          CE_ACTION_STRING_CHANGE_ATTR_FILE,
                          CE_ACTION_STRING_CHANGE_SEC_FILE,        
                          CE_ACTION_STRING_PRINT_FILE,             
                          CE_ACTION_STRING_PASTE_FILE,             
                          CE_ACTION_STRING_EMAIL_FILE,             
                          CE_ACTION_STRING_IM_FILE,                
                          CE_ACTION_STRING_EXPORT,                 
                          CE_ACTION_STRING_IMPORT,                 
                          CE_ACTION_STRING_CHECKIN,                
                          CE_ACTION_STRING_CHECKOUT,
                          CE_ACTION_STRING_ATTACH,
                          CE_ACTION_STRING_RUN,
                          CE_ACTION_STRING_REPLY,
                          CE_ACTION_STRING_FORWARD,
                          CE_ACTION_STRING_NEW_EMAIL,
                          CE_ACTION_STRING_AVD,
                          CE_ACTION_STRING_MEETING,
                          CE_ACTION_STRING_PROCESS_TERMINATE,
                          CE_ACTION_STRING_WM_SHARE,
                          CE_ACTION_STRING_WM_RECORD,
                          CE_ACTION_STRING_WM_QUESTION,
                          CE_ACTION_STRING_WM_VOICE, 
                          CE_ACTION_STRING_WM_VIDEO,
                          CE_ACTION_STRING_WM_JOIN };

  std::set<nlstring> protectedProcess;

  inline void GetProtectedProcess()
  {
    if(protectedProcess.size() >0)
      return; //Loaded already

    //by default protecting policy controller
    protectedProcess.insert(_T("cepdpman.exe")); 

#if defined (WIN32) || defined (_WIN64)
    NLTamperproofMap *pP=NULL;
    if(NLTamperproofConfiguration_Load(NL_TAMPERPROOF_TYPE_PROCESS, &pP)) {
      if(pP) {
        NLTamperproofMap::iterator it=pP->begin();
        NLTamperproofMap::iterator eit=pP->end();
        for(; it!=eit; it++)
          protectedProcess.insert(it->first);

        NLTamperproofConfiguration_Free(pP);
      }       
    } 
#endif
  }

  bool GetProcessNamebyID(long processID, nlchar *procName, int nameLen )
  {
    bool bSuccess=false;
#if defined (WIN32) || defined (_WIN64)
  
    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.
    if (NULL != hProcess ) {
      HMODULE hMod;
      DWORD cbNeeded;
      if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
                               &cbNeeded) ) {
        GetModuleBaseName( hProcess, hMod, procName,nameLen);
        bSuccess=true;
      }
      CloseHandle( hProcess );
    }

    // Print the process name and identifier.
    if(bSuccess)
      TRACE(CELOG_INFO, _T("GetProcessNamebyID %s  (PID: %u)\n"), 
            procName, processID );
    else
      TRACE(CELOG_ERR, _T("Failed to get process name of PID: %u\n"), 
            processID );

#endif
    return bSuccess;
  }

  bool GetLocalHostInfo(nlchar *hostName, int len, struct in_addr &hostIPAddr)
  {
    struct addrinfo hints, *res;
    int err;
    char ha[PDPEVAL_MAX_BUF_LENGTH];

    if(gethostname(ha, PDPEVAL_MAX_BUF_LENGTH) != 0) {
      TRACE(CELOG_ERR, _T("Failed to get host name: error=%d\n"), errno);
      return false;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if ((err = getaddrinfo(ha, NULL, &hints, &res)) != 0) {
      TRACE(CELOG_ERR, _T("Failed to get host ip: error=%d\n"), err);
      return false;
    }

    hostIPAddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;

    //printf("ip address : %s %lu\n", inet_ntoa(hostIPAddr), hostIPAddr.s_addr);

    freeaddrinfo(res);

#if defined (WIN32) || defined (_WIN64)
    MultiByteToWideChar(CP_ACP, 0, ha, -1, hostName, len);
#else
    nlstrncpy_s(hostName, len, ha, _TRUNCATE);
#endif
    return true;  
  }

  HANDLE GetProcessTokenFromPID(int pid)
  {
#if defined (WIN32) || defined (_WIN64)
    BOOL rv;
    HANDLE hToken = NULL;
    HANDLE hTokenDup = NULL;
    HANDLE ph;

    ph = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pid);
    if( ph == NULL ) {
      TRACE(CELOG_ERR, _T("OpenProcess failed (le %d)\n"), 
            GetLastError());
      return 0;
    }
    rv = OpenProcessToken(ph,TOKEN_READ|TOKEN_DUPLICATE,&hToken);
    CloseHandle(ph);
    if( rv != TRUE ) {
      TRACE(CELOG_ERR, _T("OpenProcessToken failed (le %d)\n"),
            GetLastError());
      return 0;
    }
    rv = DuplicateTokenEx(hToken,
                          TOKEN_IMPERSONATE|TOKEN_READ|TOKEN_ASSIGN_PRIMARY|TOKEN_DUPLICATE,
                          NULL,SecurityImpersonation,TokenPrimary,&hTokenDup);
    CloseHandle(hToken);
    if( rv != TRUE ) {
      TRACE(CELOG_ERR, _T("DuplicateTokenEx failed (le %d)\n")
            ,GetLastError());
      return 0;
    }

    return hTokenDup;   
#else
    return NULL;
#endif
  }/* GetProcessTokenFromPID */

  HANDLE GetAppProcessToken(const CEAttributes *dimensions, 
                            const CEAttributes_Array *attributeMatrix)
  {
#if defined (WIN32) || defined (_WIN64)
    for(int i=0; i<dimensions->count; i++) {
      if(nlstrcmp((dimensions->attrs[i].value)->buf, _T("application"))==0) {
        CEAttributes * attrs=&(attributeMatrix->attrs_array[i]);
        for(int j = 0;j<attrs->count;j++) {
          if(nlstrcmp((attrs->attrs[j].key)->buf,_T("pid"))==0) {
            nlchar *pidStr=(attrs->attrs[j].value)->buf;
            int pid=_tstol(pidStr); 
            HANDLE hToken = GetProcessTokenFromPID(pid);
            return hToken;   
          }
        }
      }
    }
#endif
    return 0;
  }

  inline bool AddNonDefaultAttrs(nlchar *dimensionName,
                                 CEAttributes * attrs, JNIEnv *env,
                                 jobjectArray jArgs, int defaultAttrsNum)
  {
    jstring jkey, jvalue;

    for(int i = 0;i<attrs->count;i++) {
      //for each key/value pair
      if(attrs->attrs[i].key && (attrs->attrs[i].key)->buf && 
         attrs->attrs[i].value && (attrs->attrs[i].value)->buf) {
        TRACE(CELOG_DEBUG, _T("%s: %s\n"), 
              (attrs->attrs[i].key)->buf,
              (attrs->attrs[i].value)->buf);
        jkey = JNI_NEWSTRING((attrs->attrs[i].key)->buf,
                             nlstrlen((attrs->attrs[i].key)->buf));
        jvalue = JNI_NEWSTRING((attrs->attrs[i].value)->buf,
                               nlstrlen((attrs->attrs[i].value)->buf));
      } else {
        jkey = JNI_NEWSTRING(_T(""), nlstrlen(_T("")));
        jvalue = JNI_NEWSTRING(_T(""), nlstrlen(_T("")));
        if(attrs->attrs[i].key && attrs->attrs[i].value) {
          TRACE(CELOG_ERR, _T("Wrong %s attribute(%d) %s: %s\n"),
                dimensionName,i, 
                (attrs->attrs[i].key)->buf?(attrs->attrs[i].key)->buf:_T("NULL"),
                (attrs->attrs[i].value)->buf?(attrs->attrs[i].value)->buf:_T("NULL"));
        }else
          TRACE(CELOG_ERR, _T("Wrong %s attribute(%d)\n"),dimensionName, i);
      }
      if(jkey != NULL && jvalue != NULL) {
        env->SetObjectArrayElement(jArgs,defaultAttrsNum+2*i, jkey);
        env->SetObjectArrayElement(jArgs,defaultAttrsNum+2*i+1, jvalue);
        env->DeleteLocalRef(jkey);
        env->DeleteLocalRef(jvalue);
      } else {
        TRACE(CELOG_ERR, _T("JNI newstring failed\n"));
        return false;
      }
    }
    return true;
  }

  CEResult_t CallPolicyEval(jobject g_servStub,
                            jclass g_serverStubClass,
                            JNIEnv *env,
                            jobjectArray &jargs) 
  {
    //get the eval() method
    jmethodID evalMethod = env->GetMethodID (g_serverStubClass,
                                             SERVER_STUB_EVAL_M, 
                                             "([Ljava/lang/Object;)V");

    if (evalMethod == NULL) {
      TRACE(CELOG_ERR,_T("Cannot retrieve the eval method."));
      return CE_RESULT_GENERAL_FAILED;
    }

    //invoke the eval() method , start the java server stub thread
    env->CallVoidMethod (g_servStub, evalMethod,jargs);
  
    return CE_RESULT_SUCCESS;
  }

  CEResult_t CallMultiPolicyEval(jobject g_servStub,
                                 jclass g_serverStubClass,
                                 JNIEnv *env,
                                 jobjectArray &jargs)
  {
    jmethodID evalMethod = env->GetMethodID(g_serverStubClass,
                                            SERVER_STUB_MULTI_EVAL_M,
                                            "([Ljava/lang/Object;)V");

    if (evalMethod == NULL) {
      TRACE(CELOG_ERR, _T("Cannot retrive the multi-eval method"));
      return CE_RESULT_GENERAL_FAILED;
    }

	TRACE(CELOG_INFO, _T("Begin call java method to do checkresource.\n"));
    env->CallVoidMethod(g_servStub, evalMethod, jargs);
	TRACE(CELOG_INFO, _T("End call java method to do checkresource.\n"));

    return CE_RESULT_SUCCESS;
  }

  /** IsPlugInEnabled
   *
   *  \brief Determine if the given plug-in is enabled.
   *
   *  \param type (in) Plug-in type.
   */
  bool IsPlugInEnabled( const nlchar* type )
  {
    /* Call into Policy Controller here */
    return true;
  }/* IsPlugInEnabled */

  /** ReplyNow
   *
   *  \brief Send a reply to a CheckMetadata request.  This source
   *         is similar to that in PDPJNI (PDPJni_SendPolicyResponse).
   *
   *  \param handle (in) Connection handle.
   *  \param reqID (in)  Request ID for current call.
   *  \param r (in) Evaluation result sent back
   */
  void ReplyNow(nlsocket serverSfd , CEString reqID, CEResponse_t r)
  {
    CEEnforcement_t efmt;

    memset(&efmt,0x00,sizeof(efmt));
    efmt.result = r;

    vector<void*> argv;

    argv.push_back(reqID);
    argv.push_back(&efmt);

    char *reply= NULL;
    size_t replyLen = 0;

    reply = Marshal_PackFuncReply(_T("CEEVALUATE_CheckMetadata"),
                                  CE_RESULT_SUCCESS, argv, replyLen);

    if( reply != NULL )
    {
      TRANSPORT_Sendn(serverSfd,replyLen,reply);
      Marshal_PackFree(reply);
    }
  }/* ReplyNow */

  //Analyze the policy request and check if this request is about
  //any actions on process. If yes, the request is going to be
  //evaluated at here since the policy engine doesn't have process
  //concept. 
  // \param dimensions (in) request dimension.
  // \param attributeMatrix (in)  Request matrix.
  // \param evalRes (out) Evaluation result sent back
  bool IsProcessEval(CEAttributes * dimensions, 
                     CEAttributes_Array *attributeMatrix, 
                     CEResponse_t &evalRes) 
  {
    int fromIndex=-1;
    int actionIndex=-1;

    evalRes=CEAllow;

    for(int i=0; i<dimensions->count; i++) {
      if(nlstrlen((dimensions->attrs[i].value)->buf)==4 &&
         nlstrncmp((dimensions->attrs[i].value)->buf, _T("from"), 4)==0)
        fromIndex=i;
      else if(nlstrlen((dimensions->attrs[i].value)->buf)==6 &&
              nlstrncmp((dimensions->attrs[i].value)->buf, _T("action"), 6)==0)
        actionIndex=i;
    }

    //Not a request of action on process 
    if(fromIndex == -1 || actionIndex == -1) 
      return false;
  
    nlchar *processId=NULL;
    nlchar *actionStr=NULL;

    //Get process ID
    CEAttributes *attrs= &(attributeMatrix->attrs_array[fromIndex]); 
    for(int i = 0;i<attrs->count;i++) {
      if(nlstrlen((attrs->attrs[i].key)->buf)==6
         && nlstrncmp((attrs->attrs[i].key)->buf, _T("CE::id"), 6)==0) 
        processId=(attrs->attrs[i].value)->buf;
    }

    //Get action string 
    attrs= &(attributeMatrix->attrs_array[actionIndex]); 
    for(int i = 0;i<attrs->count;i++) {
      if(nlstrlen((attrs->attrs[i].key)->buf)==4
         && nlstrncmp((attrs->attrs[i].key)->buf, _T("name"), 4)==0) 
        actionStr=(attrs->attrs[i].value)->buf;
    }

    //Not a request of action on process 
    if(processId == NULL || actionStr == NULL) 
      return false;

    if(nlstrlen(actionStr)==nlstrlen(CE_ACTION_STRING_PROCESS_TERMINATE)
       && nlstrncmp(actionStr, CE_ACTION_STRING_PROCESS_TERMINATE,
                    nlstrlen(CE_ACTION_STRING_PROCESS_TERMINATE))==0) {
      //a request about the action of terminating a process

      GetProtectedProcess();

      char tmpBuf[128];
      long reqProcId;
      if(nlstrtoascii(processId, tmpBuf, 128)) {
        reqProcId=atol(tmpBuf);
        nlchar procName[MAX_PATH];
        if(GetProcessNamebyID(reqProcId, procName, MAX_PATH)) {
          if(protectedProcess.find(procName) != protectedProcess.end()) {
            TRACE(CELOG_INFO, 
                  _T("Try to terminate protected process(%s); not allow\n"),
                  procName);
            evalRes=CEDeny;
          }
        }
      }
      //End-Check if this action is going to terminate process
      return true;
    }
  
    return false;

  } /*IsProcessEval*/

  jclass getClass(JNIEnv *env, const char *className) {
    jclass localClass = env->FindClass (className);
    if (localClass == NULL) {
      TRACE(CELOG_ERR, _T("Cannot find '%s' class\n"), className);
      return NULL;
    }
    // create global reference
    jclass theClass = (jclass)env->NewGlobalRef(localClass);
    env->DeleteLocalRef(localClass);
    return theClass;
  }

  jobjectArray buildQueryArray(JNIEnv *env,
                               const CEString reqID, nlsocket serverSfd,
                               const CEAttributes *dimensions, const CEAttributes_Array *attributeMatrix,
                               CENoiseLevel_t noiseLevel, CEBoolean performObligation)
  {
    static jclass stringclass = NULL;
    static jclass objClass = NULL;

    if(stringclass == NULL) {
      stringclass = getClass(env, "java/lang/String");
      
      if (stringclass == NULL) {
        TRACE(CELOG_ERR, _T("Cannot create global reference for 'java/lang/String'\n"));
        return NULL;
      }
    }

    if(objClass == NULL) {
      objClass = getClass(env, "java/lang/Object");
      
      if (objClass == NULL) {
        TRACE(CELOG_ERR, _T("Cannot create global reference for 'java/lang/Object'\n"));
        return NULL;
      }
    }

    //Allocate vertical dimension
    //0:index, 1:socket-id, 
    //2:agent type, 3:timestamp, 4:ignore obligations flag, 5:log level, 
    //6:process token
    //7:keys
    int args_row_cnt=8;
    //8:from, 9:host, 10:app, 11:user, 12:action, [13:to], [14:sendto] 
    args_row_cnt+=dimensions->count;
    
    jobjectArray jargs= env->NewObjectArray (args_row_cnt, 
                                             objClass,
                                             NULL);
    if (jargs == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the arguments\n"));
      return NULL;
    }

    int index = 0;

    //Populate horizontal dimension
    //0:Index
    jstring jstr = JNI_NEWSTRING(reqID->buf,nlstrlen(reqID->buf));
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: index\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("0 REQUEST_ID_INDEX: %s\n"), reqID->buf);
    env->DeleteLocalRef(jstr);

    //1:sockethandle
    nlchar tmpbuf[PDPEVAL_MAX_BUF_LENGTH];
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%d"),serverSfd);
    jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: sockethandle\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("1 REQUEST_SOCKET_ID_INDEX: %s\n"), tmpbuf);
    env->DeleteLocalRef(jstr);

    //2:agent type 
    //ignored. legacy
    jstr = JNI_NEWSTRING(_T("sdk"), 3);
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: agent type\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("2 AGENT_TYPE_INDEX: sdk\n"));
    env->DeleteLocalRef(jstr);


    //3:timestamp
    nlsprintf(tmpbuf,PDPEVAL_MAX_BUF_LENGTH,
              _T("%d"),(int)NL_GetCurrentTimeInMillisec());
    jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: timestamp\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("3 TIMESTAMP_INDEX: %s\n"), tmpbuf);
    env->DeleteLocalRef(jstr);


    //4:ignore obligations flag, 
    if(performObligation==CEFalse) {
      jstr = JNI_NEWSTRING(_T("true"),nlstrlen(_T("true")));
      TRACE(CELOG_DEBUG, _T("4 IGNORE_OBLIGATIONS_INDEX: true\n"));
    } else {
      jstr = JNI_NEWSTRING(_T("false"),nlstrlen(_T("false")));
      TRACE(CELOG_DEBUG, _T("4 IGNORE_OBLIGATIONS_INDEX: false\n"));
    }
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: obligation flag\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    env->DeleteLocalRef(jstr);
  

  
    //5:log level, 
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%d"),noiseLevel);
    jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: log level\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("5 LOG_LEVEL_INDEX: %s\n"), tmpbuf);
    env->DeleteLocalRef(jstr);
    

    //6:process token, is freed by java side using oswrapper
    HANDLE pToken=GetAppProcessToken(dimensions, attributeMatrix);
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%ld"),(intptr_t)pToken);
    jstr = JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
    if(jstr == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: process token\n"));
      return NULL;
    }
    env->SetObjectArrayElement(jargs, index++, jstr);
    TRACE(CELOG_DEBUG, _T("6 PROCESS_TOKEN_INDEX: %s\n"), tmpbuf);
    env->DeleteLocalRef(jstr);

    //7:keys
    jobjectArray jkeys=env->NewObjectArray((args_row_cnt-8), stringclass, NULL);

    if(jkeys == NULL) {
      TRACE(CELOG_ERR,_T("Cannot allocate the argument: keys\n"));
      return NULL;
    }
    for(int i=0; i<dimensions->count; i++) {
      jstring jkeystr = JNI_NEWSTRING((dimensions->attrs[i].value)->buf,
                                      nlstrlen((dimensions->attrs[i].value)->buf));
      if(jkeystr == NULL) {
        TRACE(CELOG_ERR,_T("Cannot allocate the argument: key[%d] string(%s)\n"),
              i, (dimensions->attrs[i].value)->buf);
        env->DeleteLocalRef(jkeys);
        return NULL;
      } 
      env->SetObjectArrayElement(jkeys, i, jkeystr);
      env->DeleteLocalRef(jkeystr);
    }
    env->SetObjectArrayElement(jargs,index++, jkeys);
    env->DeleteLocalRef(jkeys);
    
    //Attributes
    for(int i=0; i<dimensions->count; i++) {
      TRACE(CELOG_DEBUG, _T("%s (%d)\n"), 
            (dimensions->attrs[i].value)->buf, 
            (attributeMatrix->attrs_array[i]).count);
      jobjectArray jAttrArgs= env->NewObjectArray ((attributeMatrix->attrs_array[i].count)*2,
                                                   stringclass , 
                                                   NULL);
      if(jAttrArgs == NULL) {
        TRACE(CELOG_ERR,
              _T("Cannot allocate the argument: attributes array: %s (count=%d)\n"),
              (dimensions->attrs[i].value)->buf, 
              (attributeMatrix->attrs_array[i]).count);
        return NULL;
      }     
      if(AddNonDefaultAttrs((dimensions->attrs[i].value)->buf,
                            &(attributeMatrix->attrs_array[i]), 
                            env, jAttrArgs, 0)) {
        env->SetObjectArrayElement(jargs,index++, jAttrArgs);
        env->DeleteLocalRef(jAttrArgs);
      } else {
        env->DeleteLocalRef(jAttrArgs);
        TRACE(CELOG_ERR,
              _T("AddNonDefaultAttrs failed for attribute array: %s\n"),
              (dimensions->attrs[i].value)->buf);
        return NULL;
      }
    }

    return jargs;
  }
}

CEResult_t PDP_CEEVALUATE_CheckMetadata (jobject g_servStub,
                                         jclass g_serverStubClass,
                                         JavaVM *PDP_jvm,
                                         std::vector<void *> &argv,
                                         nlsocket serverSfd)
{
  if(argv.size() != 6) {
    TRACE(CELOG_ERR, 
          _T("CheckMetadata: Wrong number(%d) of input parameters.\n"), argv.size());
    return CE_RESULT_INVALID_PARAMS;
  }

  CEString reqID = (CEString)argv[0];
  //CEHandle handle = *((CEHandle*)argv[1]); //which is a local pointer
  CEAttributes *dimensions = (CEAttributes *)argv[2];
  CEAttributes_Array *attributeMatrix = (CEAttributes_Array *)argv[3];
  CENoiseLevel_t noiseLevel = *((CENoiseLevel_t *)argv[4]);
  CEBoolean performObligation = *((CEBoolean *)argv[5]);

  /* Determine if the plug-in is enabled.  If not, the always allow to
     implement 'passive' mode.
  */
  if( IsPlugInEnabled(NULL) == false ) {
    ReplyNow(serverSfd,reqID, CEAllow);
    return CE_RESULT_SUCCESS;
  } else {
    CEResponse_t evalRes;
    if(IsProcessEval(dimensions, attributeMatrix, evalRes)) {
      //This is an evaluation request about an action on a process.
      //This request won't be sent over to policy engine for evaluation.
      ReplyNow(serverSfd,reqID, evalRes); 
      return CE_RESULT_SUCCESS;
    }
  }

  //attach env to this method of the current thread 
  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(CELOG_ERR, _T("Cannot attach JNI to disposer thread.\n"));
    //Failure; Allow the action
    ReplyNow(serverSfd,reqID, CEAllow);
    return CE_RESULT_GENERAL_FAILED;
  }

  jobjectArray jargs = buildQueryArray(env, reqID, serverSfd, dimensions, attributeMatrix, noiseLevel, performObligation);

  if (jargs == NULL) {
    TRACE(CELOG_ERR,_T("Error when building the arguments array\n"));
    //Failure; Allow the action
    ReplyNow(serverSfd,reqID, CEAllow);
    return CE_RESULT_GENERAL_FAILED;
  }

  CEResult_t ret=CallPolicyEval(g_servStub, g_serverStubClass, env, jargs);

  env->DeleteLocalRef(jargs);

  return ret;
}

CEResult_t PDP_CEEVALUATE_CheckMulti(jobject g_servStub,
                                     jclass g_serverStubClass,
                                     JavaVM *PDP_jvm,
                                     std::vector<void *> &argv,
                                     nlsocket serverSfd)
{
  static jclass objClass = NULL;

  CEString reqID = (CEString) argv[1];

  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(CELOG_ERR, _T("Cannot attach JNI to disposer thread.\n"));
    //Failure; Allow the action
    ReplyNow(serverSfd,reqID, CEAllow);
    return CE_RESULT_GENERAL_FAILED;
  }

  if(objClass == NULL) {
    objClass = getClass(env, "java/lang/Object");
    
    if (objClass == NULL) {
      TRACE(CELOG_ERR, _T("Cannot create global reference for 'java/lang/Object'\n"));
      return CE_RESULT_GENERAL_FAILED;
    }
  }

  // We make the query with an array of arrays. Each individual array
  // will look like a single query from PDP_CEEVALUATE_CheckMetaData.
  // Some of the information is comment between queries, but a little
  // redundancy makes it easier to share code.
  
  CEint32 numQueries = *(CEint32 *)argv[2];

  jobjectArray multiQuery = env->NewObjectArray(numQueries,
                                                objClass,
                                                NULL);

  for (int i = 0; i < numQueries; i++) {
    CEAttributes *dimensions = (CEAttributes *)argv[i*4 + 3];
    CEAttributes_Array *attributesMatrix = (CEAttributes_Array *)argv[i*4 + 4];
    CENoiseLevel_t noiseLevel = (CENoiseLevel_t) *((CEint32 *)argv[i*4 + 5]);
    CEBoolean performObligations = *((CEBoolean *)argv[i*4 + 6]);

    jobjectArray jargs = buildQueryArray(env, reqID, serverSfd, dimensions, attributesMatrix, noiseLevel, performObligations);

    env->SetObjectArrayElement(multiQuery, i, jargs);
    env->DeleteLocalRef(jargs);
  }

  TRACE(CELOG_INFO, _T("Call CallMultiPolicyEval reqID=%s\n"), reqID->buf);
  CEResult_t ret = CallMultiPolicyEval(g_servStub, g_serverStubClass, env, multiQuery);

  env->DeleteLocalRef(multiQuery);
  
  return ret;
}

#if defined (WIN32) || defined (_WIN64)
CEResult_t PDP_CEEVALUATE_CheckMetaFromKIF(jobject g_servStub,
                                           jclass g_serverStubClass,
                                           JavaVM *PDP_jvm,
                                           NL_KIF_QUEUE_ITEM *kitem)
{
  //attach env to this method of the current thread 
  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(CELOG_ERR, _T("Cannot attach JNI to disposer thread.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //construct the policy request matrix HERE
  int args_row_cnt;
  int index = 0;
  jstring jstr;
  jobjectArray jargs;
  nlchar tmpbuf[PDPEVAL_MAX_BUF_LENGTH];
  jclass stringclass = env->FindClass ("java/lang/String");
  jstring jkey, jvalue;

  //Allocate vertical dimension
  //0:index, 1:socket-id, 
  //2:agent type, 3:timestamp, 4:ignore obligations flag, 5:log level, 
  //6:process token
  //7:keys
  args_row_cnt=8;
  //8:from, 9:host, 10:app, 11:user, 12:action, [13:to] 
  args_row_cnt+=5;
  if(kitem->req.tofile.size!=0)
    args_row_cnt+=1;

  jargs= env->NewObjectArray (args_row_cnt, 
                              env->FindClass("java/lang/Object"),
                              NULL);
  if (jargs == NULL) {
    TRACE(CELOG_ERR,_T("Cannot allocate the arguments\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //Populate horizontal dimension
  //0:Index, in the format, "KIF+<request-index>"
  nlsprintf(tmpbuf,PDPEVAL_MAX_BUF_LENGTH, _T("KIF+%d"),(kitem->req).index);
  jstr = JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("0 REQUEST_ID_INDEX: %s\n"), tmpbuf);
  env->DeleteLocalRef(jstr);

  //1:KIF index which is equivalent to the sockethandle in UIF
  nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%d"),kitem->index);
  jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("1 REQUEST_SOCKET_ID_INDEX: %s\n"), tmpbuf);
  env->DeleteLocalRef(jstr);
 
  //2:agent type 
  //it identifies a policy structure for validation purposes, 
  //and will be ignored initially
  jstr = JNI_NEWSTRING(_T("sdk"), 3);
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("2 AGENT_TYPE_INDEX: sdk\n"));
  env->DeleteLocalRef(jstr);
  
  //3:timestamp
  nlsprintf(tmpbuf,PDPEVAL_MAX_BUF_LENGTH,
            _T("%d"),(int)NL_GetCurrentTimeInMillisec());
  jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("3 TIMESTAMP_INDEX: %s\n"), tmpbuf);
  env->DeleteLocalRef(jstr);

  //4:ignore obligations flag, 
  if(kitem->req.performOB==CEFalse) {
    jstr = JNI_NEWSTRING(_T("true"),nlstrlen(_T("true")));
    TRACE(CELOG_DEBUG, _T("4 IGNORE_OBLIGATIONS_INDEX: true\n"));
  } else {
    jstr = JNI_NEWSTRING(_T("false"),nlstrlen(_T("false")));
    TRACE(CELOG_DEBUG, _T("4 IGNORE_OBLIGATIONS_INDEX: false\n"));
  }
  env->SetObjectArrayElement(jargs, index++, jstr);
  env->DeleteLocalRef(jstr);

  //5:log level, 
  nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%d"),kitem->req.noiselevel);
  jstr = JNI_NEWSTRING(tmpbuf,nlstrlen(tmpbuf));
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("5 LOG_LEVEL_INDEX: %s\n"), tmpbuf);
  env->DeleteLocalRef(jstr);

  //6:process token (leave it blank temperally
  jstr = JNI_NEWSTRING(_T("0"),nlstrlen(_T("0")));
  env->SetObjectArrayElement(jargs, index++, jstr);
  TRACE(CELOG_DEBUG, _T("6 PROCESS_TOKEN_INDEX: 0\n"));
  env->DeleteLocalRef(jstr);

  //7:keys
  jobjectArray jkeys=env->NewObjectArray((args_row_cnt-8), stringclass,
                                         NULL);
  int subIndex=0;
  jstring jkeystr;
  //fromAttributes
  jkeystr = JNI_NEWSTRING(_T("from"), 4);
  env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
  env->DeleteLocalRef(jkeystr);

  //hostAttributes
  jkeystr = JNI_NEWSTRING(_T("host"), 4);
  env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
  env->DeleteLocalRef(jkeystr);
  
  //application attributes
  jkeystr = JNI_NEWSTRING(_T("application"), 11);
  env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
  env->DeleteLocalRef(jkeystr);
  
  //user Attributes
  jkeystr = JNI_NEWSTRING(_T("user"), 4);
  env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
  env->DeleteLocalRef(jkeystr);

  //action attributes
  jkeystr = JNI_NEWSTRING(_T("action"), 6);
  env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
  env->DeleteLocalRef(jkeystr);

  if(kitem->req.tofile.size != 0) {
    jkeystr = JNI_NEWSTRING(_T("to"), 2);
    env->SetObjectArrayElement(jkeys,subIndex++, jkeystr);
    env->DeleteLocalRef(jkeystr);
  }
  env->SetObjectArrayElement(jargs,index++, jkeys);
  env->DeleteLocalRef(jkeys);

  //8:fromAttributes
  jobjectArray jfromArgs;
  //destinytype, name, resolved name, creation time, 
  //last access time, last write time
  int fromArgc=3;
  NLint64 ct,at, mt;
  //fromAttributes: get creation time
  if(kitem->req.fromfileinfo.ulHighCreationTime != 0 ||
     kitem->req.fromfileinfo.ulLowCreationTime != 0) {
    PDPEVAL_GetTime(ct,kitem->req.fromfileinfo.ulHighCreationTime,
                    kitem->req.fromfileinfo.ulLowCreationTime);
    fromArgc+=1;
  } else {
    ct=NL_GetFileCreateTime(kitem->req.fromfile.content,
                            NULL);
    if(ct != -1) fromArgc+=1;
  }
  //fromAttributes: get access time
  if(kitem->req.fromfileinfo.ulHighLastAccessTime != 0 ||
     kitem->req.fromfileinfo.ulLowLastAccessTime != 0) {
    PDPEVAL_GetTime(at, kitem->req.fromfileinfo.ulHighLastAccessTime,
                    kitem->req.fromfileinfo.ulLowLastAccessTime);
    fromArgc+=1;
  } else {
    at=NL_GetFileAccessTime(kitem->req.fromfile.content, NULL); 
    if(at != -1) fromArgc+=1;
  }
  //fromAttributes: get modify time
  if(kitem->req.fromfileinfo.ulHighLastWriteTime != 0 ||
     kitem->req.fromfileinfo.ulLowLastWriteTime != 0) {
    PDPEVAL_GetTime(mt, kitem->req.fromfileinfo.ulHighLastWriteTime,
                    kitem->req.fromfileinfo.ulLowLastWriteTime);
    fromArgc+=1;
  } else {
    mt=NL_GetFileModifiedTime(kitem->req.fromfile.content,
                              NULL);
    if(mt != -1 ) fromArgc+=1;
  }
  //Compose the from attributes
  jfromArgs= env->NewObjectArray (fromArgc*2,
                                  stringclass , 
                                  NULL);
  //fromAttributes:destiny type
  jkey= JNI_NEWSTRING(_T("CE::destinytype"), nlstrlen(_T("CE::destinytype")));
  jvalue=JNI_NEWSTRING(_T("fso"), nlstrlen(_T("fso")));
  env->SetObjectArrayElement(jfromArgs,2*0, jkey);
  env->SetObjectArrayElement(jfromArgs,2*0+1, jvalue);
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  //fromAttributes:name
  jkey= JNI_NEWSTRING(_T("CE::id"), nlstrlen(_T("CE::id")));
  jvalue=JNI_NEWSTRING(kitem->req.fromfile.content,kitem->req.fromfile.size);
  env->SetObjectArrayElement(jfromArgs,2*1, jkey);
  env->SetObjectArrayElement(jfromArgs,2*1+1, jvalue);
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  //fromAttributes:resolved name
  jkey= JNI_NEWSTRING(_T("resolved_name"), nlstrlen(_T("resolved_name")));
  if(kitem->req.fromfileEQ.size != 0) {
    jvalue=JNI_NEWSTRING(kitem->req.fromfileEQ.content,
                         kitem->req.fromfileEQ.size);
  } else {
    jvalue=JNI_NEWSTRING(kitem->req.fromfile.content,kitem->req.fromfile.size);
  }
  env->SetObjectArrayElement(jfromArgs,2*2, jkey);
  env->SetObjectArrayElement(jfromArgs,2*2+1, jvalue);
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  //fromAttributes:creation time
  int fromIndex=3;
  if(ct != -1) {
    jkey= JNI_NEWSTRING(_T("created_date"), nlstrlen(_T("created_date")));
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%lu"),ct);  
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
    env->SetObjectArrayElement(jfromArgs,2*fromIndex, jkey);
    env->SetObjectArrayElement(jfromArgs,2*fromIndex+1, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    fromIndex++;
  }
  //fromAttributes:access time
  if(at != -1) {
    jkey= JNI_NEWSTRING(_T("access_date"), nlstrlen(_T("access_date")));
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%lu"),at);
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
    env->SetObjectArrayElement(jfromArgs,2*fromIndex, jkey);
    env->SetObjectArrayElement(jfromArgs,2*fromIndex+1, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    fromIndex++;
  }
  //fromAttributes:modify time
  if(mt != -1 ) {
    jkey= JNI_NEWSTRING(_T("modified_date"), nlstrlen(_T("modified_date")));
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%lu"),mt);
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
    env->SetObjectArrayElement(jfromArgs,2*fromIndex, jkey);
    env->SetObjectArrayElement(jfromArgs,2*fromIndex+1, jvalue);
    fromIndex++;
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
  }
  env->SetObjectArrayElement(jargs,index++, jfromArgs);
  env->DeleteLocalRef(jfromArgs);
  
  //9:hostAttributes: host name, host ip address
  jobjectArray jhostArgs;
  int hostAttrNum=2*2;
  struct in_addr  hostIPAddr; hostIPAddr.s_addr=0;
  jhostArgs = env->NewObjectArray(hostAttrNum, stringclass, NULL);
  //hostAttributes:host name
  jkey= JNI_NEWSTRING(_T("name"), nlstrlen(_T("name")));
  if(kitem->req.hostname.size != 0) 
    jvalue=JNI_NEWSTRING(kitem->req.hostname.content,kitem->req.hostname.size);
  else {
    GetLocalHostInfo(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, hostIPAddr);
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
  }
  env->SetObjectArrayElement(jhostArgs,2*0, jkey);
  env->SetObjectArrayElement(jhostArgs,2*0+1, jvalue);
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  //hostAttributes:host ip address
  jkey= JNI_NEWSTRING(_T("inet_address"), nlstrlen(_T("inet_address")));
  if(kitem->req.ipaddr != 0) {
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, _T("%d"),kitem->req.ipaddr);
  } else {
    if(hostIPAddr.s_addr==0)
      GetLocalHostInfo(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, hostIPAddr);
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH, 
              _T("%d"),ntohl(hostIPAddr.s_addr));    
  }
  jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
  env->SetObjectArrayElement(jhostArgs,2*1, jkey);
  env->SetObjectArrayElement(jhostArgs,2*1+1, jvalue);
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  env->SetObjectArrayElement(jargs,index++, jhostArgs);
  env->DeleteLocalRef(jhostArgs);
  
  //10:application Attributes: application name
  jobjectArray jappArgs;
  int appAttrNum=1*2;
  jappArgs = env->NewObjectArray (appAttrNum, stringclass, NULL);
  //Application Attributes:application name
  jkey= JNI_NEWSTRING(_T("name"), nlstrlen(_T("name")));
  jvalue=JNI_NEWSTRING(kitem->req.appname.content,kitem->req.appname.size);
  env->SetObjectArrayElement(jappArgs,2*0, jkey);
  env->SetObjectArrayElement(jappArgs,2*0+1, jvalue);   
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  env->SetObjectArrayElement(jargs,index++, jappArgs);
  env->DeleteLocalRef(jappArgs);

  //11:user Attributes
  jobjectArray juserArgs;
  int userAttrNum=1*2; 
  juserArgs = env->NewObjectArray (userAttrNum, stringclass, NULL);
  //user Attributes: SID
  jkey= JNI_NEWSTRING(_T("id"), nlstrlen(_T("id")));
#if defined (WIN32) || defined (_WIN64)
  if(kitem->req.coreSID.aUserSID.size != 0) {
    TRACE(CELOG_DEBUG, _T("user SID: %d %s\n"),
          kitem->req.coreSID.aUserSID.size,
          kitem->req.coreSID.aUserSID.content); 
    jvalue=JNI_NEWSTRING(kitem->req.coreSID.aUserSID.content, 
                         kitem->req.coreSID.aUserSID.size);
  } else {
    NL_getUserId(tmpbuf, PDPEVAL_MAX_BUF_LENGTH);
    TRACE(CELOG_DEBUG, _T("user ID: %s\n"),tmpbuf);
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
  }
#else
  if(kitem->req.coreSID.ulLinuxUid != 0) {
    nlsprintf(tmpbuf, PDPEVAL_MAX_BUF_LENGTH,
              _T("%d"),kitem->req.coreSID.ulLinuxUid);    
    jvalue=JNI_NEWSTRING(kitem->req.coreSID.aUserSID.content, 
                         kitem->req.coreSID.aUserSID.size);
  } else {
    NL_getUserId(tmpbuf, PDPEVAL_MAX_BUF_LENGTH);
    jvalue=JNI_NEWSTRING(tmpbuf, nlstrlen(tmpbuf));
  }
#endif
  env->SetObjectArrayElement(juserArgs,2*0, jkey);
  env->SetObjectArrayElement(juserArgs,2*0+1, jvalue);   
  env->DeleteLocalRef(jkey);
  env->DeleteLocalRef(jvalue);
  env->SetObjectArrayElement(jargs,index++, juserArgs);
  env->DeleteLocalRef(juserArgs);

  //12:action Attributes
  jobjectArray jactionArgs;
  int actionAttrNum=1*2;
  if(kitem->req.action <= CE_ACTION_RUN) {
    jactionArgs = env->NewObjectArray (actionAttrNum, stringclass, NULL);
    //action Attributes: action name
    jkey= JNI_NEWSTRING(_T("name"), nlstrlen(_T("name")));
    jvalue=JNI_NEWSTRING(actionTable[kitem->req.action],
                         nlstrlen(actionTable[kitem->req.action]));
    env->SetObjectArrayElement(jactionArgs,2*0, jkey);
    env->SetObjectArrayElement(jactionArgs,2*0+1, jvalue);   
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    env->SetObjectArrayElement(jargs,index++, jactionArgs);
    env->DeleteLocalRef(jactionArgs);
  } else if(kitem->req.action == 0x400000 ) {
    jactionArgs = env->NewObjectArray (actionAttrNum, stringclass, NULL);
    jkey= JNI_NEWSTRING(_T("name"), nlstrlen(_T("name")));
    jvalue=JNI_NEWSTRING(_T("EMBED"), nlstrlen(_T("EMBED")));
    env->SetObjectArrayElement(jactionArgs,2*0, jkey);
    env->SetObjectArrayElement(jactionArgs,2*0+1, jvalue);   
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    env->SetObjectArrayElement(jargs,index++, jactionArgs);
    env->DeleteLocalRef(jactionArgs);
  } else if(kitem->req.action == 0x800000) {
    jactionArgs = env->NewObjectArray (actionAttrNum, stringclass, NULL);
    jkey= JNI_NEWSTRING(_T("name"), nlstrlen(_T("name")));
    jvalue=JNI_NEWSTRING(_T("EMBED"), nlstrlen(_T("EMBED")));
    env->SetObjectArrayElement(jactionArgs,2*0, jkey);
    env->SetObjectArrayElement(jactionArgs,2*0+1, jvalue);   
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    env->SetObjectArrayElement(jargs,index++, jactionArgs);
    env->DeleteLocalRef(jactionArgs);
  } else {
    TRACE(CELOG_WARNING, _T("Action %d is invalid.\n"), kitem->req.action);
    // Delete the reference to jargs
    env->DeleteLocalRef(jargs);
    return CE_RESULT_INVALID_PARAMS;
  }

  //13: to attribute: destinytype, name, resolved name
  if(kitem->req.tofile.size > 0) {
    jobjectArray jtoArgs;
    int toAttrNum=3*2;
    jtoArgs = env->NewObjectArray (toAttrNum, stringclass, NULL);
    //toAttributes:destiny type
    jkey= JNI_NEWSTRING(_T("CE::destinytype"),nlstrlen(_T("CE::destinytype")));
    jvalue=JNI_NEWSTRING(_T("fso"), nlstrlen(_T("fso")));
    env->SetObjectArrayElement(jtoArgs,2*0, jkey);
    env->SetObjectArrayElement(jtoArgs,2*0+1, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    //toAttributes:name
    jkey= JNI_NEWSTRING(_T("CE::id"), nlstrlen(_T("CE::id")));
    jvalue=JNI_NEWSTRING(kitem->req.tofile.content,kitem->req.tofile.size);
    env->SetObjectArrayElement(jtoArgs,2*1, jkey);
    env->SetObjectArrayElement(jtoArgs,2*1+1, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    //toAttributes:resolved name
    jkey= JNI_NEWSTRING(_T("resolved_name"), nlstrlen(_T("resolved_name")));
    if(kitem->req.tofileEQ.size != 0) {
      jvalue=JNI_NEWSTRING(kitem->req.tofileEQ.content,
                           kitem->req.tofileEQ.size);
    } else
      jvalue=JNI_NEWSTRING(kitem->req.tofile.content,kitem->req.tofile.size);  
    env->SetObjectArrayElement(jtoArgs,2*2, jkey);
    env->SetObjectArrayElement(jtoArgs,2*2+1, jvalue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
    env->SetObjectArrayElement(jargs,index++, jtoArgs);
    env->DeleteLocalRef(jtoArgs);
  }

  CEResult_t ret=CallPolicyEval(g_servStub, g_serverStubClass, env, jargs);

  // Delete the reference to jargs
  env->DeleteLocalRef(jargs);

  return ret;
}
#endif
