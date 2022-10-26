#include <vector>
#include <jni.h>
#include "brain.h"
#include "nlstrings.h"
#include "nlthread.h"
#include "cetype.h"
#include "CEsdk.h"
#include "celog.h"
#include <hash_set>

#if defined (Linux) || defined (Darwin)
#ifdef nlsprintf
#undef nlsprintf
#define nlsprintf snprintf
#endif
#endif

using namespace std;

namespace {
enum {PDP_CECONN_STRING_LENGTH_MAX=1024};
}

class ActiveHandles {
private:
    nlthread_mutex_t m;
    stdext::hash_set <CEHandle> hset; 

public:
    ActiveHandles() {
        nlthread_mutex_init(&m);
    }

    ~ActiveHandles() {
        nlthread_mutex_destroy(&m);
    }
    
    /**
     * Adds the handle to the list of active handles
     */
    void add(CEHandle h) {
        nlthread_mutex_lock(&m);
        hset.insert(h);
        nlthread_mutex_unlock(&m);
    }

    /**
     * Removes the handle fom the list of active handles
     * \return true if the handle was in the list, false if not
     */
    bool remove(CEHandle h) {
        nlthread_mutex_lock(&m);
        bool found = false;
        stdext::hash_set<CEHandle>::iterator p = hset.find(h);
        if (p != hset.end()) {
            found = true;
            hset.erase(p);
        }
        nlthread_mutex_unlock(&m);
        return found;
    }
};

ActiveHandles active;

/** conn_event
 *
 *  \brief Inform the Policy Controller that a connection has occurred.  An
 *         array is constructed for calling into the JVM and the format is
 *         as follows:
 *             <jstring>        Plug-in type.
 *             <jobjectArray>   Key array.
 *             <jobjectArray>   Value array to match key array.
 *
 *         The size of the attribute arrays must match.
 *
 *  \param PDP_jvm (in) Java VM instance.
 *  \param type (in)    Plug-in type.
 *
 *  \return CE_RESULT_SUCCESS when the type may connect.
 *          CE_RESULT_NOT_SUPPORTED when the type may not connect.  This
 *          may occur when the plug-in type has been disabled.
 */


static CEResult_t conn_event( JavaVM *PDP_jvm ,
			      const nlchar* type )
{
  CEResult_t ret = CE_RESULT_SUCCESS;

  if( type != NULL )
  {
    /* Confirm the type is supported by the Policy Controller */

    JNIEnv *env;
    jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);

    if( res < 0 )
    {
      return CE_RESULT_GENERAL_FAILED;
    }

    jclass stringclass = env->FindClass("java/lang/String");

    if( stringclass == NULL )
    {
      return CE_RESULT_GENERAL_FAILED;
    }

    jobjectArray jargs;
    jargs = env->NewObjectArray(1,stringclass,NULL);

    if( jargs == NULL )
    {
      return CE_RESULT_GENERAL_FAILED;
    }

    /* Plug-in type is the only fixed parameter.  It identifies a policy
       structure for validation purposes.
    */
    nlchar wtype[512];
    int wtype_len;
    wtype_len = nlsprintf(wtype,512, _T("%s"),type);

    jstring jstr = env->NewString((const jchar*)wtype, wtype_len);
    int index = 0;
    env->SetObjectArrayElement(jargs, index++, jstr);
    env->DeleteLocalRef(jstr);

    /* There is an optional parameter of an two arrays:
          (1) Array of keys
	  (2) Array of values to keys in (1)

       Here they are constructed but empty.
    */

    jobjectArray jkeys = env->NewObjectArray(0,stringclass,NULL);
    jobjectArray jvals = env->NewObjectArray(0,stringclass,NULL);

    env->SetObjectArrayElement(jargs,index++, jkeys);
    env->SetObjectArrayElement(jargs,index++, jvals);

    env->DeleteLocalRef(jkeys);
    env->DeleteLocalRef(jvals);

    /* call into JVM */

    // Delete the reference to jargs
    env->DeleteLocalRef(jargs);
  }

  return ret;
}/* conn_event */

/*==========================================================================*
 * Exported APIS                                                            *
 *==========================================================================*/
CEResult_t PDP_CECONN_Initialize (nlsocket sockid,
				  JavaVM *PDP_jvm,
				  jobject PDP_servStub,
				  unsigned long long *outerScopeCEHandleWrapper,
				  std::vector<void *> &argv,
				  std::vector<void *> &replyArgs)
{
  CEString reqID = ((CEString)argv[0]);
  CEString type = ((CEString)argv[1]); 
  CEString appName = ((CEString)argv[2]); 
  CEString binaryPath = ((CEString)argv[3]);
  CEString userName = ((CEString)argv[4]);
  CEString userID = ((CEString )argv[5]);
  CEint32 ipAddress = *((CEint32 *)argv[6]);
  CEResult_t ret = CE_RESULT_SUCCESS;

  //1. malloc a area for the CEHandle and fill it with incoming data
  CEHandle handle= new struct _CEHandle();
  *outerScopeCEHandleWrapper = (unsigned long long)handle;

  //length in CEString is sting length, not buffer length!!!!
  if(type) {
    handle->type = new nlchar[type->length+1];  
    nlstrncpy_s(handle->type, type->length+1, type->buf, _TRUNCATE);
  } else
    handle->type = NULL;

  //length in CEString is sting length, not buffer length!!!!
  if(appName) {
    handle->appName = new nlchar[appName->length+1];  
    nlstrncpy_s(handle->appName, appName->length+1, appName->buf, _TRUNCATE);
  } else
    handle->appName = NULL;

  if(binaryPath) {
    //Binary Path can be empty
    handle->binaryPath = new nlchar[binaryPath->length+1];
    nlstrncpy_s(handle->binaryPath, binaryPath->length+1, binaryPath->buf, _TRUNCATE);
  } else
    handle->binaryPath = NULL;

  if(userName) {
    //userName Path can be empty
    handle->userName = new nlchar[userName->length+1];
    nlstrncpy_s(handle->userName, userName->length+1, userName->buf, _TRUNCATE);
  } else 
    handle->userName = NULL;    

  if(userID) {
    handle->userID = new nlchar[userID->length+1];
    nlstrncpy_s(handle->userID, userID->length+1, userID->buf, _TRUNCATE);
  } else
    handle->userID = NULL;

  handle->serverSfd = sockid;
  if(argv[6]) 
    ipAddress = *((CEint32 *)argv[6]);
  else
    ipAddress = 0;
  handle->hostIPAddress = ipAddress;
  TRACE(CELOG_INFO,
	_T("PDP_CECONN_Initialize: type(%s) appName(%s) binaryPath(%s) userName(%s) userID(%s) socket(%d) hostIPAddress(%d)\n"), 
	(handle->type)?handle->type:_T("NULL"),
	(handle->appName)?handle->appName:_T("NULL"),
	(handle->binaryPath)?handle->binaryPath:_T("NULL"),
	(handle->userName)?handle->userName:_T("NULL"),
	(handle->userID)?handle->userID:_T("NULL"),
	handle->serverSfd, handle->hostIPAddress);

  //2. construct the reply and send it back
  replyArgs.push_back(reqID);

  // We have to push a pointer here, hence the need to get a pointer from outside this
  // function.  Yes, this is very broken.
  replyArgs.push_back(outerScopeCEHandleWrapper);

  /* Plug-in project is not used in release 3.1 so avoid a useless
     notification.
  */
#if 0
  /* Notify the Policy Controller that a connection has occurred.  The Policy
     Controller will also determine if this type is supported.
  */
  CEResult_t ce_ret;

  ce_ret = conn_event(PDP_jvm,handle->type);

  if( ce_ret != CE_RESULT_SUCCESS )
  {
    ret = ce_ret;
  }
#endif

  active.add(handle);

  return ret;
}

void PDP_CECONN_FreeHandle(CEHandle handle)
{
  if (!active.remove(handle)) {
      return;
  }

  if(handle->type) {
    delete [] handle->type;
    handle->type=NULL;
  } 

  if(handle->appName) {
    delete [] handle->appName;
    handle->appName=NULL;
  } 

  if(handle->binaryPath) {
    delete [] handle->binaryPath;
    handle->binaryPath=NULL;
  }

  if(handle->userName) {
    delete [] handle->userName;
    handle->userName=NULL;
  }

  if(handle->userID) {
    delete [] handle->userID;
    handle->userID=NULL;
  }

  delete handle;
}

CEResult_t PDP_CECONN_Close (std::vector<void *> &inputArgs,
			     std::vector<void *> &replyArgs)
{
  CEString reqID = (CEString)inputArgs[0];
  CEHandle handle = (CEHandle)*((unsigned long long*)inputArgs[1]);
  CEResult_t   ret = CE_RESULT_SUCCESS;

  //Construct reply
  replyArgs.push_back(reqID);
  
  //The connection is closed by client side
  //TRANSPORT_Close(handle->serverSfd);
  
  //free the handle of this connection
  PDP_CECONN_FreeHandle(handle);

  //Return
  return ret;
}

