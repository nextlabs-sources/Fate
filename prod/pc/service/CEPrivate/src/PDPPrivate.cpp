#include <jni.h>
#include <vector>
#include "JavaConstants.h"
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "PDPConn.h"


using namespace std;
//define NewString func for JNI
#if defined (Linux) || defined (Darwin)
#define JNI_NEWSTRING(s,l)   env->NewStringUTF(s)
#endif
#if defined (WIN32) || defined (_WIN64)
#define JNI_NEWSTRING(s,l)   env->NewString((jchar *)s,(jsize)l)
#endif

/*==========================================================================*
 * Exported functions implemented in this file.                             *
 *==========================================================================*/
CEResult_t PDP_CEP_StopPDP(JavaVM *PDP_jvm,
			   jobject cmStub,
			   jclass cmStubClass,
			   vector<void *> &inputArgs,
			   vector<void *> &outArgs)
{
  CEResult_t   ret = CE_RESULT_SUCCESS;
  //CEP_StopPDP function signature
  //    const nlchar *i14[3]={_T(NAME_CEString), //reqID
  //			_T(NAME_CEint32), //public: session id
  //			_T(NAME_CEString)};  //public: password
  CEString reqID = (CEString)inputArgs[0];
  //CEHandle handle = *((CEHandle*)inputArgs[1]); //which is a local pointer
  CEString passwd = (CEString)inputArgs[2];

  //Verify the password
  //attach env to this method of the current thread 
  JNIEnv *env;
  jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
  if(res<0) {
    TRACE(0, _T("Cannot attach JNI to disposer thread.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //Get the verify password method
  jmethodID verifyPasswdMethod = env->GetMethodID (cmStubClass,
						   STOP_AGENT_VERIFY,
						   "(Ljava/lang/String;)Z");
  if (verifyPasswdMethod == NULL) {
    TRACE(0,_T("Cannot retrieve the verify password method.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

  //invoke the verify password method 
  jstring passwd_jstr = JNI_NEWSTRING(passwd->buf,nlstrlen(passwd->buf));
  jboolean bCorrect=env->CallBooleanMethod (cmStub, 
					    verifyPasswdMethod, 
					    passwd_jstr);
  env->DeleteLocalRef(passwd_jstr);
  if(bCorrect == false) {
    TRACE(0,_T("Failed to stop the PDP: the password is not valid.\n"));    
    ret=CE_RESULT_PERMISSION_DENIED;
  }

  //construct the reply 
  outArgs.push_back(reqID);

  //This is problematic. It can cause access violation in some
  //situation. Since PDPMan is going to stop, so we don't mind
  //this small memory leak.
  //Free the handle of this connection
  //if(ret == CE_RESULT_SUCCESS)
  //PDP_CECONN_FreeHandle(handle);

  return ret;
}

