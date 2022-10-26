// All sources, binaries and HTML pages (C) copyright 2004 
// by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#include "com_bluejungle_destiny_agent_controlmanager_PDPJni.h"
#include "com_bluejungle_destiny_agent_ipc_OSWrapper.h"
#include "jni.h"
#include "brain.h"
#include "nlthread.h"
#include "nlthreadpool.h"
#include "transport.h"
#if defined (WIN32) || defined (_WIN64)
#include "pdpgeneric.h"
#endif
#include "marshal.h"
#define  NL_KIF_USER
#if defined (WIN32) || defined (_WIN64)
#include "cekif.h"
#include "cekif_user.h"
#endif
#include "celog.h"


#if defined(WIN32) || defined(_WIN64)
#define NEW_STRING(env, buf, size)            env->NewString((const jchar*)buf, size)
#define GET_STRING_CHARS(env, jbuf)           (const nlchar *)env->GetStringChars(jbuf, 0)
#define RELEASE_STRING_CHARS(env, jbuf, buf)  env->ReleaseStringChars((jstring)jbuf, (const jchar *)buf)
#else
#define NEW_STRING(env, buf, size)            env->NewStringUTF(buf)
#define GET_STRING_CHARS(env, jbuf)           (const nlchar *)env->GetStringUTFChars(jbuf, 0)
#define RELEASE_STRING_CHARS(env, jbuf, buf)  env->ReleaseStringUTFChars((jstring)jbuf, (const char *)buf)
#endif

namespace {
  //some constants
  enum {PDPMan_MAX_BUF_LENGTH=1024};

  void *GetProcessToken(JNIEnv * env, jobject processToken)
  {
    void *tokenValue=NULL;
    if(processToken) {
      jclass longClass = env -> FindClass("java/lang/Long"); 
      jmethodID longValueMID = env -> GetMethodID (longClass, 
                                                   "longValue", 
                                                   "()J");
      tokenValue = (void *)env->CallLongMethod(processToken, longValueMID);
    } else {
      tokenValue=NL_OpenProcessToken();
    }
    return tokenValue;
  }
}

JNIEXPORT jstring JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getAppInfo (JNIEnv * env, 
                                                            jobject obj, 
                                                            jstring fileName) 
{
  jstring jstr;
  nlchar buf[1024];
  
  const nlchar *str = GET_STRING_CHARS(env, fileName);

  if(NL_GetFingerprint(str, buf, 1024)) {
    jstr = NEW_STRING(env, buf, (jsize)nlstrlen(buf));
  } else {
    jstr = NEW_STRING(env, _T(""), 0);
  }
  RELEASE_STRING_CHARS(env, fileName, str);

  return jstr;
}
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getUserName
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getUserName(JNIEnv * env, jobject obj, jstring userSid)
{

  const nlchar *stringSid = GET_STRING_CHARS(env, userSid);

  jstring rv; 
  nlchar *userName=NULL;
  if(NL_GetUserName(stringSid, &userName) && userName != NULL) {
    rv = NEW_STRING(env, userName, nlstrlen(userName));
    NL_GetUserName_Free(userName);
  } else {
    rv = NEW_STRING(env, _T(""), 0);
  }
  
  RELEASE_STRING_CHARS(env, userSid, stringSid);
  return rv;
}
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    logEvent
 * Signature: (II[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_logEvent(JNIEnv * env, jobject obj, jint type, jint eventId, jobjectArray paramArray)
{
  vector<nlchar *> ppStrArray;
  int size = env->GetArrayLength (paramArray);
  if (size > 0) {
    try {
      for (int i=0; i < size; i++) {
        jobject paramObj = env->GetObjectArrayElement(paramArray, i);
        const nlchar *pParam=(const nlchar *)env->GetStringChars((jstring)paramObj, 0);
        ppStrArray.push_back(new nlchar[nlstrlen(pParam) + 1]);
        nlstrcpy_s(ppStrArray[i], nlstrlen(pParam) + 1, pParam);
        env->ReleaseStringChars ((jstring) paramObj, (const jchar*)pParam);
      }
      NL_LogEvent(type, eventId, size, &ppStrArray[0]);
      for (int i=0; i < ppStrArray.size(); i++) {
        delete[] (ppStrArray[i]);
      }
    } catch (...) {
      for (int i=0; i < ppStrArray.size(); i++) {
        delete[] (ppStrArray[i]);
      }
    }
  } else {
    NL_LogEvent(type, eventId, 0, NULL);
  }
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    isRemovableMedia
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_isRemovableMedia(JNIEnv * env, jobject obj, jstring jFileName)
{
  const nlchar *lpFileName = (const nlchar *)env->GetStringChars(jFileName, 0);
  bool result=NL_IsRemovableMedia(lpFileName);
  env->ReleaseStringChars (jFileName, (const jchar*)lpFileName);

  return result;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDocumentsFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDocumentsFolder(JNIEnv *env , jobject obj)
{
  nlchar *fBuf= new nlchar[256];
  int size=256;
  NL_GetMyDocumentsFolder(fBuf, &size);
  jstring rv = NEW_STRING(env, fBuf, size);
  delete[] fBuf;
  return rv;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDesktopFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDesktopFolder (JNIEnv *env , jobject obj)
{
  nlchar *fBuf= new nlchar[256];
  int size=256;
  NL_GetMyDesktopFolder(fBuf, &size);
  jstring rv = NEW_STRING(env, fBuf, size);
  delete[] fBuf;
  return rv;
}

JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFQDN(JNIEnv * env, jobject obj)
{
  nlchar *fqdnBuf= new nlchar[256];
  int size=256;
  jstring rv;

  if(NL_GetFQDN(fqdnBuf, &size)) {
    rv = NEW_STRING(env, fqdnBuf, size);
  } else {
    rv = NEW_STRING(env, _T(""), 0);
  }

  delete[] fqdnBuf;
  return rv;
}


static jint fromIntegerObject(JNIEnv *env, jobject integerObject)
{
  static jmethodID intValue = NULL;
  
  static jclass integerClass = NULL;

  if (integerClass == NULL) {
    jclass localClass = env->FindClass("java/lang/Integer");
    if (localClass != NULL) {
      integerClass = (jclass)env->NewGlobalRef(localClass);
      env->DeleteLocalRef(localClass);
    }
  }

  if (intValue == NULL)
  {
    intValue = env->GetMethodID(integerClass, "intValue", "()I");
    
    if (intValue == NULL)
    {
      TRACE(CELOG_ERR, _T("Cannot find intValue.\n"));
      return NULL;
    }
  }
  
  return env->CallIntMethod(integerObject, intValue);
}

JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_controlmanager_PDPJni_SendPolicyMultiResponse(JNIEnv *env, jobject jObj, jlong jhandle,
                                                                                                           jstring requestID, jobjectArray responses)
{
  jboolean ret = false;

  const nlchar *reqIDwstr = GET_STRING_CHARS(env, requestID);
  TRACE(4, _T("\nControl manager oswrapper SendPolicyMultiResponse: reqID=%s\n"),reqIDwstr);

  CEString reqID = CEM_AllocateString(reqIDwstr);
  RELEASE_STRING_CHARS(env, requestID, reqIDwstr);

  nlstring fmt = _T("si");  // request id, number of responses
  int numResponses = env->GetArrayLength(responses);

  for (int i = 0; i < numResponses; ++i)
  {
    fmt += _T("ia");  // allow/deny and the obligations
  }

  vector<void *> args;

  args.push_back(reqID);
  args.push_back(new CEint32(numResponses));

  for (int i = 0; i < numResponses; ++i)
  {
    jobjectArray response = (jobjectArray) env->GetObjectArrayElement(responses, i);

    args.push_back(new CEint32(fromIntegerObject(env, env->GetObjectArrayElement(response, 0))));

    jobjectArray jattrs = (jobjectArray) env->GetObjectArrayElement(response, 1);

    CEAttributes *attrs = new CEAttributes();

    attrs->count = (jattrs == NULL) ? 0 : env->GetArrayLength(jattrs)/2;

    attrs->attrs = attrs->count == 0 ? NULL : new CEAttribute[attrs->count];

    for (int j = 0; j < attrs->count; ++j)
    {
      jstring jkey = (jstring)env->GetObjectArrayElement(jattrs, j*2);
      const nlchar *key = GET_STRING_CHARS(env, jkey);

      jstring jvalue = (jstring)env->GetObjectArrayElement(jattrs, j*2+1);
      const nlchar *value = GET_STRING_CHARS(env, jvalue);

      attrs->attrs[j].key = CEM_AllocateString(key);
      attrs->attrs[j].value = CEM_AllocateString(value);

      RELEASE_STRING_CHARS(env, jkey, key);
      RELEASE_STRING_CHARS(env, jvalue, value);
    }

    args.push_back(attrs);
  }

  size_t replyLen;

  char *reply = Marshal_PackFuncAndFormatReply(_T("CEEVALUATE_CheckResourcesEx"),
                                               fmt.c_str(),
                                               CE_RESULT_SUCCESS,
                                               args,
                                               replyLen);

  if (reply != NULL)
  {
    ret = (TRANSPORT_Sendn((nlsocket)jhandle, replyLen, reply) == CE_RESULT_SUCCESS);

    Marshal_PackFree(reply);
  }

  // Free the allocated data
  CEM_FreeString((CEString)args[0]);
  delete (CEint32 *)args[1];

  for (int i = 0; i < numResponses; ++i)
  {
    delete (CEint32 *)args[i*2 + 2];
    
    CEAttributes *attrs = (CEAttributes *)args[i*2 + 3];

    if (attrs != NULL)
    {
      for (int j = 0; j < attrs->count; ++j)
      {
        CEM_FreeString(attrs->attrs[j].key);
        CEM_FreeString(attrs->attrs[j].value);
      }
    }
    delete [] attrs->attrs;
    delete attrs;
  }
  

  return ret;
}


JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_controlmanager_PDPJni_SendPolicyResponse(JNIEnv* env, jobject jObj, jlong jhandle, 
                                                                                                      jstring requestID, jlong allow, jobjectArray attrs)
{
  jboolean ret = true;
  CEAttributes enforcementAttrs;
  CEEnforcement_t efmt;
#if defined (WIN32) || defined (_WIN64)
  NL_KIF_POLICY_RESPONSE kifReply={0,0};
#endif
  size_t replyLen;
  bool bToKIF=false;

  //Get the request ID
  const nlchar* reqIDwstr = GET_STRING_CHARS(env, requestID);

  CEString reqID = CEM_AllocateString(reqIDwstr);

  //debug
  TRACE(4,_T("Control manager oswrapper sendPolicyResponse: reqID=%s\n"),
        reqIDwstr);
  if(nlstrstr(reqIDwstr, _T("KIF"))) {
    bToKIF=true;
#if defined (WIN32) || defined (_WIN64)
    kifReply.index=_ttoi(&reqIDwstr[3]);
#endif
  }
  RELEASE_STRING_CHARS(env, requestID, reqIDwstr);

  //Fetch the returned enforcement
  //1. Deny/Allow
  //debug
  TRACE(1,_T("In sendPolicyResponse: allow =%d\n"),allow);
#if defined (WIN32) || defined (_WIN64)
  if(bToKIF)
    kifReply.allow=(nlint)allow;
  else
#endif
    efmt.result = (CEResponse_t)allow;

  //2. Obligation to UIF
  if(!bToKIF) {
    int strnum = env->GetArrayLength(attrs);
    int attrnum = strnum/2;
    CEAttribute *enforcementattr=NULL;
    if(attrnum > 0) {
      enforcementattr = new CEAttribute[attrnum];
      for(int i=0;i<attrnum;i++) {
        jstring keybase = (jstring) env->GetObjectArrayElement(attrs,2*i);
        const nlchar *keywstr = GET_STRING_CHARS(env, keybase);

        jstring valuebase = (jstring) env->GetObjectArrayElement(attrs,2*i+1);
        const nlchar *valuewstr = GET_STRING_CHARS(env, valuebase);

        //debug
        TRACE(0,_T("in sendPolicyResponse: key for attribute %d=%s\n"),
              i,keywstr);
        TRACE(0,_T("in sendPolicyResponse: value for attribute %d=%s\n"),
              i,valuewstr);

        CEString cekey = CEM_AllocateString(keywstr);
        CEString cevalue = CEM_AllocateString(valuewstr);
        enforcementattr[i].key =  cekey;
        enforcementattr[i].value = cevalue;

        RELEASE_STRING_CHARS (env, keybase, keywstr);
        RELEASE_STRING_CHARS (env, valuebase, valuewstr);
      }
    }
    enforcementAttrs.count = attrnum;
    enforcementAttrs.attrs = enforcementattr;
    efmt.obligation = &enforcementAttrs;

    //send reply for CEEVALUATE_CheckResource
    //  const nlchar *o8[2]={_T(NAME_CEString), //reqID 
    //  _T(NAME_CEEnforcement_t)}; //public: enforcement
    vector<void*> argv;
    argv.push_back(reqID);
    argv.push_back(&efmt);
    char *reply=Marshal_PackFuncReply(_T("CEEVALUATE_CheckMetadata"),
                                      CE_RESULT_SUCCESS,
                                      argv, replyLen);
    if(reply) {
      //Send out the reply
      if(TRANSPORT_Sendn((nlsocket)jhandle, replyLen, reply) != CE_RESULT_SUCCESS) {
        TRACE(0,_T("TRANSPORT_Sendn failed\n"));
        ret = false;
      }
      Marshal_PackFree(reply); 
    } else {
      TRACE(0, _T("Marshal_PackFuncReply failed\n"));
      ret = false;
    }

    //Free allocated memory
    CEM_FreeString(reqID);  
    if(attrnum > 0) {
      for(int i=0;i<attrnum;i++) {
        CEM_FreeString(enforcementattr[i].key);
        CEM_FreeString(enforcementattr[i].value);
      }
      delete[] enforcementattr;
    }
  } else {//send policy response via KIF
#if defined (WIN32) || defined (_WIN64)
    TRACE(1,
          _T("In sendPolicyResponse to KIF: kif-Index=%d reqID=%d,allow=%d\n"),
          jhandle, kifReply.index, kifReply.allow);
    if(NL_KIF_SendKernelResponse((nlsocket)jhandle, kifReply) != CE_RESULT_SUCCESS) {
      TRACE(0, _T("NL_KIF_SendKernelResponse failed\n"));
      ret=false;
    }
#endif
  }
  return ret;
}

JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_controlmanager_PDPJni_SendServiceResponse(JNIEnv* env, jobject jObj, jlong jhandle, jstring reqId, jobjectArray response)
{
#if defined (WIN32) || defined (_WIN64)
  CEResult_t res = PDP_GenericResponse(env, (nlsocket)jhandle, reqId, response);

  return (res == CE_RESULT_SUCCESS);
#else
  return CE_RESULT_SUCCESS;
#endif
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileModifiedTime
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jlong JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileModifiedTime
(JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{
  jlong ret = -1;
  if (fileName) {
    void *tokenValue=GetProcessToken(env, processToken);

    const nlchar *str = GET_STRING_CHARS(env, fileName);
    ret = NL_GetFileModifiedTime(str, tokenValue);
    RELEASE_STRING_CHARS(env, fileName, str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getOwnerSID
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getOwnerSID(JNIEnv * env, jobject obj, jstring fileName, jint agentType, jobject processToken) 
{
  if (!fileName) {
    return NEW_STRING(env, _T(""), 0);
  }

  const nlchar *str = GET_STRING_CHARS(env, fileName);
  void *processTokenHandle=GetProcessToken(env, processToken);
  nlchar sidBuf[PDPMan_MAX_BUF_LENGTH];

  if(NL_GetOwnerSID(str, agentType, processTokenHandle, 
                    sidBuf, PDPMan_MAX_BUF_LENGTH) == false) {

    RELEASE_STRING_CHARS(env, fileName, str);
    return NEW_STRING(env, _T(""), 0);
  }
  RELEASE_STRING_CHARS(env, fileName, str);

  return NEW_STRING(env, sidBuf, nlstrlen(sidBuf));
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileAccessTime
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jlong JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileAccessTime(JNIEnv * env,
                                                                  jobject obj, jstring fileName, jobject processToken)
{
  jlong ret = -1;
  if (fileName) {
    void *tokenValue=GetProcessToken(env, processToken);

    const nlchar *str = GET_STRING_CHARS(env, fileName);
    ret = NL_GetFileAccessTime(str, tokenValue);
    RELEASE_STRING_CHARS(env, fileName, str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileCreateTime
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileCreateTime
(JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{
  jlong ret = -1;
  if (fileName) {
    void *tokenValue=GetProcessToken(env, processToken);
    const nlchar *str = GET_STRING_CHARS(env, fileName);
    ret = NL_GetFileCreateTime(str, tokenValue);
    RELEASE_STRING_CHARS(env, fileName, str);
  }
  return ret;
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getGroupSID
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getGroupSID(JNIEnv * env, jobject obj, jstring fileName, jobject processToken)
{
  if (!fileName) {
    return NEW_STRING(env, _T(""), 0);
  }

  nlchar sidBuf[PDPMan_MAX_BUF_LENGTH];

  const nlchar *str = GET_STRING_CHARS(env, fileName);
  
  if (NL_GetGroupSID(str,sidBuf, PDPMan_MAX_BUF_LENGTH) == false) {
    env -> ReleaseStringChars ((jstring) fileName, (const jchar*)str);
    return NEW_STRING(env, _T(""), 0);
  }

  RELEASE_STRING_CHARS(env, fileName, str);

  return NEW_STRING(env, sidBuf, nlstrlen(sidBuf));
}

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getRdpAddress
 * Signature: 
 * 
 */
JNIEXPORT jlong JNICALL 
Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getRDPAddress (JNIEnv * env, jobject obj, jint processId)
{
  return NL_GetRdpSessionAddress(processId);
}
