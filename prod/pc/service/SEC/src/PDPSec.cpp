/*
 * Created on Apr 22, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

#include <jni.h>
#include <vector>
#include "JavaConstants.h"
#include "brain.h"
#include "PDPSec.h"
#include "cesdk.h"
#include "cetype.h"
#include "celog.h"

//define NewString func for JNI
#if defined (Linux) || defined (Darwin)
#define JNI_NEWSTRING(s,l)   env->NewStringUTF(s)
#endif
#if defined (WIN32) || defined (_WIN64)
#define JNI_NEWSTRING(s,l)   env->NewString((jchar *)s,(jsize)l)
#endif

namespace {
    CEResult_t CheckMakeTrustedInput(CEString password)
    {
        if (password != NULL && password->buf != NULL) {
            TRACE(CELOG_DEBUG,
                  _T("MakeTrusted: password: %s\n"), password->buf);
        } else {
            TRACE(CELOG_DEBUG, _T("MakeTrusted: password: NULL"));
            return CE_RESULT_INVALID_PARAMS;
        }

        return CE_RESULT_SUCCESS;
    }
}

CEResult_t PDP_CESEC_MakeTrusted(JavaVM *PDP_jvm,
                                 jobject cmStub,
                                 jclass cmStubClass,
                                 std::vector<void *>& inputArgs,
                                 std::vector<void *>& outputArgs)
{
    CEResult_t ret = CE_RESULT_SUCCESS;

    CEString reqID = (CEString)inputArgs[0];
    CEint32 processID = *(CEint32 *)inputArgs[1];
    CEString password = (CEString)inputArgs[2];

    outputArgs.push_back(reqID);

    JNIEnv *env;
    jint res = PDP_jvm->AttachCurrentThread((void**)&env,NULL);
    if(res<0) {
        TRACE(0, _T("Cannot attach JNI to disposer thread.\n"));
        return CE_RESULT_GENERAL_FAILED;
    }
    
    jmethodID makeTrustedMethod = env->GetMethodID(cmStubClass,
                                                   "makeTrusted",
                                                   "(ILjava/lang/String;)Z");

    if (makeTrustedMethod == NULL) {
        TRACE(CELOG_ERR, _T("Unable to retrieve makeTrusted method"));
        return CE_RESULT_GENERAL_FAILED;
    }

    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == NULL) {
        TRACE(0, _T("Cannot find 'java/lang/String' class\n"));
        return CE_RESULT_GENERAL_FAILED; 
    }

    jstring jPassword = JNI_NEWSTRING(password->buf, nlstrlen(password->buf));

    jboolean result = env->CallBooleanMethod(cmStub, makeTrustedMethod, processID, jPassword);

    if (!result) {
        ret = CE_RESULT_PERMISSION_DENIED;
    }

    env->DeleteLocalRef(jPassword);
    env->DeleteLocalRef(stringClass);

    return ret;
}
