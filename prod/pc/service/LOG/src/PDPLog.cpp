/*================================PDPLog.cpp================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author :                                                                 *
 * Date   : 03/26/2008                                                      *
 * Note   : Implemente SDK API CELOG_XXX on PDP (a.k.a policy controller)   *
 *          side.                                                           *
 *==========================================================================*/
#include <jni.h>
#include <vector>
#include "JavaConstants.h"
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "celog.h"
#include "PDPLog.h"

using namespace std;
//define NewString func for JNI
#if defined (Linux) || defined (Darwin)
#define JNI_NEWSTRING(s,l)   env->NewStringUTF(s)
#endif
#if defined (WIN32) || defined (_WIN64)
#define JNI_NEWSTRING(s,l)   env->NewString((jchar *)s,(jsize)l)
#endif

namespace {
inline CEResult_t CheckLogAssistantInput(CEString logIdentifier, 
					 CEString assistantName, 
					 CEString assistantOptions, 
					 CEString assistantDescription,
					 CEString assistantUserActions)
{
  if(logIdentifier && logIdentifier->buf) {
    TRACE(CELOG_DEBUG,
	  _T("LogAssistantData: logId: %s\n"), logIdentifier->buf);
  } else {
    TRACE(CELOG_ERR, 
	  _T("logAssistantData: logId: NULL\n"));
    return CE_RESULT_INVALID_PARAMS;
  }

  if(assistantName && assistantName->buf) {
    TRACE(CELOG_DEBUG, 
	  _T("LogAssistantData: assistantName: %s\n"), assistantName->buf);
  } else {
    TRACE(CELOG_ERR, 
	  _T("logAssistantData: assistantName: NULL\n"));
    return CE_RESULT_INVALID_PARAMS;
  }

  if(assistantOptions && assistantOptions->buf) {
    TRACE(CELOG_DEBUG, _T("LogAssistantData: assistantOptions: %s\n"), 
	  assistantOptions->buf);
  } else {
    TRACE(CELOG_INFO, 
	  _T("logAssistantData: assistantOptions: NULL\n"));
  } 
  
  if(assistantDescription && assistantDescription->buf) {
    TRACE(CELOG_DEBUG, _T("LogAssistantData: assistantDescription: %s\n"), 
	  assistantDescription->buf);
  } else {
    TRACE(CELOG_INFO, 
	  _T("logAssistantData: assistantDescription: NULL\n"));
  } 
  
  if(assistantUserActions && assistantUserActions->buf) {
    TRACE(CELOG_DEBUG, _T("LogAssistantData: assistantUserActions: %s\n"), 
	  assistantUserActions->buf);
  }else {
    TRACE(CELOG_ERR, 
	  _T("logAssistantData: assistantUserActions: NULL\n"));
  } 
  return CE_RESULT_SUCCESS;
}
}

/*==========================================================================*
 * Exported functions implemented in this file.                             *
 *==========================================================================*/
CEResult_t PDP_CELOG_LogDecision(JavaVM *PDP_jvm,
				 jobject cmStub,
				 jclass cmStubClass,
				 vector<void *> &inputArgs,
				 vector<void *> &outArgs)
{
  CEResult_t   ret = CE_RESULT_SUCCESS;
  //CELOG_LogDecision function signature
  //const nlchar *i16[5]={_T(NAME_CEString), //reqID
                        //_T(NAME_CEHandle), //public: session id
                        //_T(NAME_CEString), //public: cookie
                        //_T(NAME_CEint32), //public: userResponse (Deny/Allow)
                        //_T(NAME_CEAttributes)}; //public: optional attrs
  CEString reqID = (CEString)inputArgs[0];
  //CEHandle handle = *((CEHandle*)inputArgs[1]); //which is a local pointer
  CEString cookie = (CEString)inputArgs[2];
  CEResponse_t userResponse = *((CEResponse_t *)inputArgs[3]);
  CEAttributes *optAttributes=(CEAttributes *)inputArgs[4];

  //construct the reply first since nothing will return from Java call
  outArgs.push_back(reqID);

  //Call Java function to log the decision on server side
  //attach env to this method of the current thread 
  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(0, _T("Cannot attach JNI to disposer thread.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //Get the logging decision method
  jmethodID logDecisionMethod = env->GetMethodID (cmStubClass,
						  "logDecision",
						  "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V");
  if (logDecisionMethod == NULL) {
    TRACE(0,_T("Cannot retrieve the java method 'logDecision'.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //invoke the logging decision method 
  //Construct function arguments
  jstring jCookieStr;
  jstring jUserResponseStr;
  jobjectArray jOptAttrs;
  jstring jkey, jvalue;
  jclass stringClass=env->FindClass("java/lang/String");
  if(stringClass == NULL) {
    TRACE(0, _T("Cannot find 'java/lang/String' class\n"));
    return CE_RESULT_GENERAL_FAILED; 
  }
  //Construct cookie string
  jCookieStr = JNI_NEWSTRING(cookie->buf,nlstrlen(cookie->buf));
  TRACE(0, _T("LogDecision: 0 cookie: %s\n"), cookie->buf);
  //Construct user decision string
  if(userResponse == CEAllow) {
    jUserResponseStr = JNI_NEWSTRING(_T("ALLOW"), 5);
    TRACE(0, _T("LogDecision: 1 decision: ALLOW\n"));
  } else {
    jUserResponseStr = JNI_NEWSTRING(_T("DENY"),4);
    TRACE(0, _T("LogDecision: 1 decision: DENY\n"));
  }
  //Construct optional attributes
  if(optAttributes) {
    TRACE(0, _T("LogDecision: 2 optAttributes: %d\n"), optAttributes->count);
    jOptAttrs=env->NewObjectArray ((optAttributes->count)*2,
				   stringClass , 
				   NULL);
    for(int i = 0;i<optAttributes->count;i++) {
      //for each key/value pair
      TRACE(0, _T("LogDecision: 2 optAttributes[%d]: %s\n"), 
	    2*i, (optAttributes->attrs[i].key)->buf);
      jkey = JNI_NEWSTRING((optAttributes->attrs[i].key)->buf,
			   nlstrlen((optAttributes->attrs[i].key)->buf));
      TRACE(0, _T("LogDecision: 2 optAttributes[%d]: %s\n"), 
	    2*i+1, (optAttributes->attrs[i].value)->buf);
      jvalue = JNI_NEWSTRING((optAttributes->attrs[i].value)->buf,
			     nlstrlen((optAttributes->attrs[i].value)->buf));
      env->SetObjectArrayElement(jOptAttrs,2*i, jkey);
      env->SetObjectArrayElement(jOptAttrs,2*i+1, jvalue);
      env->DeleteLocalRef(jkey);
      env->DeleteLocalRef(jvalue);
    }
  } else {
    TRACE(0, _T("LogDecision: 2 optAttributes: null\n"));
    jOptAttrs=env->NewObjectArray (0,
				   stringClass , 
				   NULL);
  }
  
  //Call the java LogDecision method
  env->CallVoidMethod (cmStub, logDecisionMethod, jCookieStr,
		       jUserResponseStr, jOptAttrs);

  //Clean off
  env->DeleteLocalRef(jCookieStr);  
  env->DeleteLocalRef(jUserResponseStr);  
  env->DeleteLocalRef(jOptAttrs);
  return ret;
}

CEResult_t PDP_CELOG_LogAssistantData(JavaVM *PDP_jvm,
				      jobject cmStub,
				      jclass cmStubClass,
				      vector<void *> &inputArgs,
				      vector<void *> &outArgs)
{
  CEResult_t   ret = CE_RESULT_SUCCESS;
  //SDK API: CELOGGING_LogAssistantData 
  //const nlchar *i17[8]={_T(NAME_CEString), //reqID
  //		_T(NAME_CEHandle), //public: session id
  //		_T(NAME_CEString), //public: logIdentifier
  //		_T(NAME_CEString), //public: assistantName 
  //		_T(NAME_CEString), //public: assistantOptions
  //		_T(NAME_CEString), //public: assistantDescription
  //		_T(NAME_CEString), //public: assistantUserActions
  //		_T(NAME_CEAttributes)}; //public: optional attrs
  CEString reqID = (CEString)inputArgs[0];
  //CEHandle handle = *((CEHandle*)inputArgs[1]); //which is a local pointer
  CEString logIdentifier = (CEString)inputArgs[2];
  CEString assistantName = (CEString)inputArgs[3];
  CEString assistantOptions= (CEString)inputArgs[4];
  CEString assistantDescription = (CEString)inputArgs[5];
  CEString assistantUserActions = (CEString)inputArgs[6];
  CEAttributes *optAttributes=(CEAttributes *)inputArgs[7];

  //construct the reply first since nothing will return from Java call
  outArgs.push_back(reqID);

  //Check Inputs
  ret=CheckLogAssistantInput(logIdentifier, assistantName, 
			 assistantOptions, assistantDescription,
			 assistantUserActions);
  if(ret !=CE_RESULT_SUCCESS)
    return ret;

  //Call Java function server side
  //attach env to this method of the current thread 
  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(CELOG_ERR, _T("Cannot attach JNI to disposer thread.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //Get the logging decision method
  jmethodID logAssistantDataMethod = env->GetMethodID (cmStubClass,
						  "logAssistantData",
						  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V");
  if (logAssistantDataMethod == NULL) {
    TRACE(CELOG_ERR,
	  _T("Cannot retrieve the java method 'logAssistantData'.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //invoke the logging decision method 
  //Construct function arguments
  jstring jLogIdStr=NULL;
  jstring jAsNameStr=NULL;
  jstring jAsOptStr=NULL;
  jstring jAsDesStr=NULL;
  jstring jAsUActStr=NULL;
  jobjectArray jOptAttrs;
  jstring jkey, jvalue;
  jclass stringClass=env->FindClass("java/lang/String");
  if(stringClass == NULL) {
    TRACE(CELOG_ERR, _T("Cannot find 'java/lang/String' class\n"));
    return CE_RESULT_GENERAL_FAILED; 
  }

  //Construct logIdentifier string
  jLogIdStr = JNI_NEWSTRING(logIdentifier->buf,nlstrlen(logIdentifier->buf));
  if(jLogIdStr == NULL) {
    TRACE(CELOG_ERR, 
	  _T("JNI_NEWSTRING failed for parameter logIdentifier\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //Construct assistantName string
  jAsNameStr = JNI_NEWSTRING(assistantName->buf,
			     nlstrlen(assistantName->buf));
  if(jAsNameStr == NULL) {
    TRACE(CELOG_ERR, 
	  _T("JNI_NEWSTRING failed for parameter assistantName\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  if(assistantOptions && assistantOptions->buf) {
    jAsOptStr = JNI_NEWSTRING(assistantOptions->buf,
				nlstrlen(assistantOptions->buf));
  }else
    jAsOptStr = JNI_NEWSTRING(_T(""), 0);
  if(jAsOptStr == NULL) {
    TRACE(CELOG_ERR, 
	  _T("JNI_NEWSTRING failed for parameter assistantOptions\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
    
  
  //Construct assistantDescription string
  if(assistantDescription && assistantDescription->buf)
    jAsDesStr = JNI_NEWSTRING(assistantDescription->buf,
			      nlstrlen(assistantDescription->buf));
  else 
    jAsDesStr = JNI_NEWSTRING(_T(""), 0);    
  if(jAsDesStr==NULL)  {
    TRACE(CELOG_ERR, 
	  _T("JNI_NEWSTRING failed for parameter assistantDescription\n"));
    return CE_RESULT_GENERAL_FAILED;
  }  

  //Construct assistantUserActions string
  if(assistantUserActions && assistantUserActions->buf)
    jAsUActStr = JNI_NEWSTRING(assistantUserActions->buf,
			       nlstrlen(assistantUserActions->buf));
  else
    jAsUActStr = JNI_NEWSTRING(_T(""), 0);    
  if(jAsUActStr==NULL) {
    TRACE(CELOG_ERR, 
	  _T("JNI_NEWSTRING failed for parameter assistantUserAction\n"));
    return CE_RESULT_GENERAL_FAILED;
  }  

  //Construct optional attributes
  if(optAttributes && optAttributes->count>0) {
    TRACE(CELOG_DEBUG, 
	  _T("LogAssistantData: optAttributes: %d\n"), optAttributes->count);
    jOptAttrs=env->NewObjectArray ((optAttributes->count)*2,
				   stringClass , 
				   NULL);
    if(jOptAttrs==NULL) {
      TRACE(CELOG_ERR, 
	    _T("JNI_NEWObjectArray failed for parameter optAttributes\n"));
      return CE_RESULT_GENERAL_FAILED;
    }  
    for(int i = 0;i<optAttributes->count;i++) {
      if(optAttributes->attrs[i].key && 
	 (optAttributes->attrs[i].key)->buf &&
	 optAttributes->attrs[i].value &&
	 (optAttributes->attrs[i].value)->buf) {
	//for each key/value pair
	TRACE(CELOG_DEBUG, _T("LogAssistantData: optAttributes[%d]: %s "), 
	      2*i, (optAttributes->attrs[i].key)->buf);
	jkey = JNI_NEWSTRING((optAttributes->attrs[i].key)->buf,
			     nlstrlen((optAttributes->attrs[i].key)->buf));
	TRACE(CELOG_DEBUG, _T("%s\n"), 
	      2*i+1, (optAttributes->attrs[i].value)->buf);
	jvalue = JNI_NEWSTRING((optAttributes->attrs[i].value)->buf,
			       nlstrlen((optAttributes->attrs[i].value)->buf));
	env->SetObjectArrayElement(jOptAttrs,2*i, jkey);
	env->SetObjectArrayElement(jOptAttrs,2*i+1, jvalue);
	env->DeleteLocalRef(jkey);
	env->DeleteLocalRef(jvalue);
      }
    }
  } else {
    TRACE(0, _T("LogAssistantData: optAttributes: null\n"));
    jOptAttrs=env->NewObjectArray (0,
				   stringClass , 
				   NULL);
  }
  
  //Call the java LogDecision method
  env->CallVoidMethod (cmStub, 
		       logAssistantDataMethod, 
		       jLogIdStr,
		       jAsNameStr,
		       jAsOptStr,
		       jAsDesStr,
		       jAsUActStr,
		       jOptAttrs);

  //Clean off
  if(jLogIdStr)
    env->DeleteLocalRef(jLogIdStr);
  if(jAsNameStr)
    env->DeleteLocalRef(jAsNameStr);
  if(jAsOptStr)
    env->DeleteLocalRef(jAsOptStr);
  if(jAsDesStr)
    env->DeleteLocalRef(jAsDesStr);
  if(jAsUActStr)
    env->DeleteLocalRef(jAsUActStr);
  if(jOptAttrs)
    env->DeleteLocalRef(jOptAttrs);
  return ret;
}
