#include <jni.h>
#include <vector>
#include "brain.h"
#include "cesdk.h"
#include "cesdk_private.h"
#include "cetype.h"
#include "celog.h"
#include "marshal.h"
#include "transport.h"
#include "PDPGeneric.h"

// Stolen from PDPEval.cpp  Should be put in some central place
#if defined (Linux) || defined (Darwin)
#define JNI_NEWSTRING(s,l)   env->NewStringUTF(s)
#endif
#if defined (WIN32) || defined (_WIN64)
#define JNI_NEWSTRING(s,l)   env->NewString((const jchar *)s,(jsize)l)
#endif

static jclass stringClass = NULL;
static jclass objectClass = NULL;
static jclass booleanClass = NULL;
static jclass integerClass = NULL;

/*
 * This code exists in marshal.cpp and a variation exists in service.cpp.  We should consolidate this, but
 * at the moment I don't have the time (see Mark Twain)
 */
static int numArguments(const nlchar *fmt)
{
    int numargs = 0;
    while (*fmt)
    {
        if (*fmt == '[') {
            fmt++;
            if (*(fmt) == 'a') {
                numargs++;
            } else {
                // Error
                return 0;
            }
        }
        else {
            numargs++;
        }
        fmt++;
    }

    return numargs;
}

typedef enum { FMT_CEINT32,
               FMT_CEBOOLEAN,
               FMT_CESTRING,
               FMT_CEATTRIBUTES,
               FMT_CEATTRIBUTES_ARRAY,
               FMT_UNKNOWN } fmtEnum;
/*
 * Split apart a format string character by character
 */
static fmtEnum getEncoding(const nlchar **fmt)
{
    fmtEnum ret = FMT_UNKNOWN;

    if (fmt != NULL)
    {
        switch(**fmt)
        {
            case 'i':
                ret = FMT_CEINT32;
                break;
            case 'b':
                ret = FMT_CEBOOLEAN;
                break;
            case 's':
                ret = FMT_CESTRING;
                break;
            case 'a':
                ret = FMT_CEATTRIBUTES;
                break;
            case '[':
                (*fmt)++;
                if (**fmt == 'a') {
                    ret = FMT_CEATTRIBUTES_ARRAY;
                }
                break;
            default:
                break;
        }

        (*fmt)++;
    }


    return ret;
}


#if defined(WIN32) || defined(_WIN64)
// Stolen from oswrapper and other places.  Yup, more consolidation is required

/** getStringFromJava
 *
 * Returns a terminated nlchar from a java string.  Remarkably,
 * the return from GetStringChars is not null terminated
 *
 * \param env - the java JNI environment
 * \param javaString - the java string
 *
 * \return a terminated nlchar representing the java string
 * \note resources allocated by this function should be freed
 * by calling releaseStringFromJava
 */
static nlchar *getStringFromJava(JNIEnv *env, jstring javaString)
{
    if (javaString == NULL)
    {
        return NULL;
    }

    const nlchar *str = (const nlchar *)env->GetStringChars(javaString, 0);
    size_t javaStrLen = env->GetStringLength(javaString);

    nlchar *ret = (nlchar *)malloc(sizeof(nlchar) * (javaStrLen + 1));

    if (ret == NULL)
    {
        return NULL;
    }

    wcsncpy_s(ret, javaStrLen + 1, str, _TRUNCATE);
    ret[javaStrLen] = L'\0';

    env->ReleaseStringChars(javaString, (const jchar *)str);

    return ret;
}

/** releaseStringFromJava
 *
 * Frees the resources allocated by getStringFromJava
 *
 * \param str - the nlchar * returned by getStringFromJava()
 */

static void releaseStringFromJava(const nlchar *str)
{
    if (str != NULL)
    {
        free((void *)str);
    }
}

#else
#error "getStringfromJava and releaseStringFromJava only defined for Windows"
#endif

/*
 * toBooleanObject
 *
 * Create a Boolean from a jboolean
 */
static jobject toBooleanObject(JNIEnv *env, jboolean torf)
{
    static jmethodID booleanConstructor = NULL;

    if (booleanConstructor == NULL)
    {
        booleanConstructor = env->GetMethodID(booleanClass, "<init>", "(Z)V");

        if (booleanConstructor == NULL)
        {
            TRACE(CELOG_ERR, _T("Cannot find boolean class constructor.\n"));
            return NULL;
        }
    }

    return env->NewObject(booleanClass, booleanConstructor, torf);
}

/*
 * fromBooleanObject
 *
 * convert an Boolean object to a jboolean
 */
static jboolean fromBooleanObject(JNIEnv *env, jobject booleanObject)
{
    static jmethodID booleanValue = NULL;

    if (booleanValue == NULL)
    {
        booleanValue = env->GetMethodID(booleanClass, "booleanValue", "()Z");

        if (booleanValue == NULL)
        {
            TRACE(CELOG_ERR, _T("Cannot find booleanValue.\n"));
            return NULL;
        }
    }

    return env->CallBooleanMethod(booleanObject, booleanValue);
}


/*
 * toIntegerObject
 *
 * Create an Integer from a jinteger
 */
static jobject toIntegerObject(JNIEnv *env, jint ival)
{
    static jmethodID integerConstructor = NULL;

    if (integerConstructor == NULL)
    {
        integerConstructor = env->GetMethodID(integerClass, "<init>", "(I)V");

        if (integerConstructor == NULL)
        {
            TRACE(CELOG_ERR, _T("Cannot find integer class constructor.\n"));
            return NULL;
        }
    }

    return env->NewObject(integerClass, integerConstructor, ival);
}

/*
 * fromIntegerObject
 *
 * convert an Integer object to a jint
 */
static jint fromIntegerObject(JNIEnv *env, jobject integerObject)
{
    static jmethodID intValue = NULL;

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

static jobjectArray toJavaAttributes(JNIEnv *env, CEAttributes *attrs)
{
    int count = (attrs == NULL) ? 0 : attrs->count;

    jobjectArray attrArray = env->NewObjectArray(count * 2, stringClass, NULL);

    for (int i = 0; i < count; i++)
    {
        if (attrs->attrs[i].key == NULL || attrs->attrs[i].key->buf == NULL)
        {
            env->SetObjectArrayElement(attrArray, i*2+1, NULL);
        }
        else
        {
            jobject str = JNI_NEWSTRING(attrs->attrs[i].key->buf, nlstrlen(attrs->attrs[i].key->buf));
            env->SetObjectArrayElement(attrArray, i*2, str);
            env->DeleteLocalRef(str);
        }
        
        if (attrs->attrs[i].value == NULL || attrs->attrs[i].value->buf == NULL)
        {
            env->SetObjectArrayElement(attrArray, i*2+1, NULL);
        }
        else
        {
            jobject str = JNI_NEWSTRING(attrs->attrs[i].value->buf, nlstrlen(attrs->attrs[i].value->buf));
            env->SetObjectArrayElement(attrArray, i*2+1, str);
            env->DeleteLocalRef(str);
        }
        
    }

    return attrArray;
}

static CEAttributes *stringArrayToAttributes(JNIEnv *env, jobjectArray jattrs)
{
    CEAttributes *attributes = new CEAttributes;

    if (jattrs == NULL)
    {
        attributes->count = 0;
    }
    else
    {
       attributes->count = env->GetArrayLength(jattrs)/2;
    }

    attributes->attrs = new (std::nothrow) CEAttribute[attributes->count];
	if (attributes->attrs != NULL)
	{
	    for (int i = 0; i < attributes->count; i++)
	    {
	        nlchar *key = getStringFromJava(env, (jstring)env->GetObjectArrayElement(jattrs, i*2));
	        nlchar *value = getStringFromJava(env, (jstring)env->GetObjectArrayElement(jattrs, i*2+1));

	        attributes->attrs[i].key = CEM_AllocateString(key);
	        attributes->attrs[i].value = CEM_AllocateString(value);

	        releaseStringFromJava(key);
	        releaseStringFromJava(value);
	    }
	}

    return attributes;
}

/*
 * convertRequestToJava
 *
 * Convert the contents of the request to java objects.  The first element of args is
 * the format string, which describes the format of the rest of the objects
 */
static jobjectArray convertRequestToJava(JNIEnv *env, std::vector<void *> &args)
{
    const nlchar *fmt = ((CEString)args[0])->buf;
    if (fmt == NULL) {
        return NULL;
    }

    jobjectArray jarray = env->NewObjectArray(numArguments(fmt), objectClass, NULL);

    int jindex = 0;
    // First thing in args is the format string.  Skip it
    for (unsigned int i = 1; i < args.size(); i++)
    {
        jobject obj = NULL;
        switch (getEncoding(&fmt))
        {
            case FMT_CEINT32:
                TRACE(CELOG_DEBUG, _T("Converting integer\n"));
                obj = toIntegerObject(env, *(CEint32 *)args[i]);
                break;
            case FMT_CEBOOLEAN:
                TRACE(CELOG_DEBUG, _T("Converting boolean\n"));
                obj = toBooleanObject(env, (jboolean)*(CEBoolean *)args[i]);
                break;
            case FMT_CESTRING:
                {
                    TRACE(CELOG_DEBUG, _T("Converting string\n"));
                    CEString str = (CEString)args[i];
                    if (str == NULL) {
                        obj = NULL;
                    } else {
                        obj = JNI_NEWSTRING(str->buf, nlstrlen(str->buf));
                    }
                }
                break;
            case FMT_CEATTRIBUTES:
                TRACE(CELOG_DEBUG, _T("Converting attributes\n"));
                obj = toJavaAttributes(env, (CEAttributes *)args[i]);
                break;
            case FMT_CEATTRIBUTES_ARRAY:
                {
                    TRACE(CELOG_DEBUG, _T("Converting attributes array\n"));
                    CEAttributes_Array *attrArray = (CEAttributes_Array *)args[i];
                    
                    obj = env->NewObjectArray(attrArray->count, objectClass, NULL);

                    for (int j = 0; j < attrArray->count; j++)
                    {
                        jobject a = toJavaAttributes(env, &attrArray->attrs_array[j]);
                        env->SetObjectArrayElement((jobjectArray)obj, j, a);
                        env->DeleteLocalRef(a);
                    }
                }
                break;
        }

        env->SetObjectArrayElement(jarray, jindex++, obj);

        if (obj != NULL)
        {
            env->DeleteLocalRef(obj);
        }
    }
    return jarray;
}

/*
 * Convert a java response (consisting of a format string and a bunch
 * of java objects) into a C++ response vector, appropriate for
 * marshalling.
 */
static CEResult_t convertJavaObjectsToResponse(JNIEnv *env, jstring jReqId, jobjectArray &response, nlchar **fmt, std::vector<void *> &outputArgs)
{
    *fmt = NULL;

    if (response == NULL) {
        TRACE(CELOG_ERR, _T("Null response\n"));
        return CE_RESULT_INVALID_PARAMS;
    }

    jobject serviceResult = env->GetObjectArrayElement(response, 0);

    if (serviceResult == NULL) {
        TRACE(CELOG_ERR, _T("serviceResult was null\n"), serviceResult);
        return CE_RESULT_INVALID_PARAMS;
    }

    CEResult_t res = (CEResult_t)fromIntegerObject(env, serviceResult);

    if (res == CE_RESULT_SUCCESS) {
        const nlchar *origFmt = getStringFromJava(env, (jstring)env->GetObjectArrayElement(response, 1));
        const nlchar *baseFmt = origFmt;

        // We have to add the reqId to the response.  This involves modifying the format string and adding it to the outputArgs
        *fmt = new nlchar[nlstrlen(baseFmt) + 2];
        nlstrcpy_s(*fmt, nlstrlen(baseFmt) + 2, L"s");
        nlstrcat_s(*fmt, nlstrlen(baseFmt) + 2, baseFmt);

        nlchar *reqId = getStringFromJava(env, jReqId);
        CEString ceReqId = CEM_AllocateString(reqId);
        releaseStringFromJava(reqId);
        outputArgs.push_back(ceReqId);

        unsigned int responseLength = env->GetArrayLength(response);
        
        for (unsigned int i = 2; i < responseLength; i++)
        {
            void *datum = NULL;
            jobject obj = env->GetObjectArrayElement(response, i);
            switch(getEncoding(&baseFmt))
            {
                case FMT_CEINT32:
                    {
                        CEint32 asInt = fromIntegerObject(env, obj);
                        datum = (void *)new CEint32(asInt);
                    }
                    break;
                case FMT_CEBOOLEAN:
                    {
                        CEBoolean asBool = (CEBoolean)fromBooleanObject(env, obj);
                        datum = (void *)new CEBoolean(asBool);
                    }
                    break;
                case FMT_CESTRING:
                    {
                        nlchar *str = getStringFromJava(env, (jstring)obj);
                        CEString cestr = CEM_AllocateString(str);
                        releaseStringFromJava(str);
                        datum = (void *)cestr;
                    }
                    break;
                case FMT_CEATTRIBUTES:
                    {
                        CEAttributes *attrs = stringArrayToAttributes(env, (jobjectArray)obj);
                        datum = attrs;
                    }
                    break;
                case FMT_CEATTRIBUTES_ARRAY:
                    {
                        CEAttributes_Array *aattrs = new CEAttributes_Array;
                        datum = aattrs;
                        int len = env->GetArrayLength((jobjectArray)obj);
                        aattrs->attrs_array = new (std::nothrow) CEAttributes[len];
						if (aattrs->attrs_array != NULL)
						{
	                        for (int j = 0; j < len; j++)
	                        {
	                            jobjectArray jattrs = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, j);
	                            CEAttributes *attrs = stringArrayToAttributes(env, jattrs);
	                            aattrs->attrs_array[j].attrs = attrs->attrs;
	                            aattrs->attrs_array[j].count = attrs->count;
	                            delete attrs;
	                        }
						}
                    }
                    break;
            }
            outputArgs.push_back(datum);
        }
        releaseStringFromJava(origFmt);
    }

    return res;
}

/* ============================FreeResponseVector=========================*
 *                                                                        *
 * Free the memory allocated by convertJavaObjectsToResponse()            *
 * Code was copied from marshal.cpp Marshal_FreeGeneric()                 *
 * Parameters:                                                            *
 * argv (input/output): the the vector of pointers to allocated memory.   *
 *                                                                        *
 * Return:                                                                *
 *   It will return CE_RESULT_GENERAL_FAILED if the function fails;       *
 *   otherwise, it will CE_RESULT_SUCCESS.                                *
 *                                                                        * 
 * Note:                                                                  *
 *   It is caller's responsibility to call function "FreeResponseVector"  *
 *   to free the memory allocated by convertJavaObjectsToResponse()       *
 *                                                                        *
 *========================================================================*/
CEResult_t FreeResponseVector(const nlchar *fmt, vector<void *> &argv)
{
    try
    {
        fmtEnum enc;
        int index = 0;

        while ((enc = getEncoding(&fmt)) != FMT_UNKNOWN)
        {
            if (enc == FMT_CEINT32) {
                delete (CEint32 *)argv[index];
            } else if (enc == FMT_CEBOOLEAN) {
                delete (CEBoolean *)argv[index];
            } else if (enc == FMT_CESTRING) {
                CEM_FreeString((CEString)argv[index]);
            } else if (enc == FMT_CEATTRIBUTES) {
                CEAttributes *attrs = (CEAttributes *)argv[index];
                if (attrs) {
                    for (int i = 0; i < attrs->count; i++) {
                        CEM_FreeString(attrs->attrs[i].key);
                        CEM_FreeString(attrs->attrs[i].value);
                    }
                }
                delete [] attrs->attrs;
                delete attrs;
            } else if (enc == FMT_CEATTRIBUTES_ARRAY) {
                CEAttributes_Array *arr = (CEAttributes_Array *)argv[index];
                for (int i = 0; i < arr->count; i++)
                {
                    CEAttributes *attrs = &arr->attrs_array[i];
                    for (int j = 0; j < attrs->count; j++) {
                        CEM_FreeString(attrs->attrs[j].key);
                        CEM_FreeString(attrs->attrs[j].value);
                    }
                    delete [] attrs;
                }
                delete [] arr->attrs_array;
                delete arr;
            }
            argv[index++] = NULL;
        }

        return CE_RESULT_SUCCESS;
    } catch (exception &) {
        return CE_RESULT_GENERAL_FAILED;
    } catch (...) {
        return CE_RESULT_GENERAL_FAILED;
    }
}

/*
 * Return a minimal "error" chunk to the client
 */

static void ErrorReply(nlsocket socket, CEString reqId, CEResult_t resultCode)
{
    TRACE(CELOG_ERR, _T("Returning error from generic handler\n"));

    vector<void*> responseVec;
    responseVec.push_back(reqId);
    responseVec.push_back(new CEint32(resultCode));

    size_t errorResponseLen = 0;
    char *errorResponse = Marshal_PackReplyGeneric(L"si",
                                                   resultCode,
                                                   responseVec,
                                                   errorResponseLen);
    
    if (errorResponse != NULL)
    {
        // This had better work
        TRANSPORT_Sendn(socket, errorResponseLen, errorResponse);
        Marshal_PackFree(errorResponse);
    }
    else
    {
        TRACE(CELOG_ERR, _T("Attempted to marshal error response failed.  No recovery possible\n"));
    }
}

static void ErrorReply(nlsocket socket, nlchar *reqId, CEResult_t resultCode)
{
    CEString ceReqId = CEM_AllocateString(reqId);
    ErrorReply(socket, ceReqId, resultCode);
    CEM_FreeString(ceReqId);
}

static CEResult_t CallGenericEval(JNIEnv *env,
                                  jobject cmStub,
                                  jclass cmStubClass,
                                  nlsocket responseSocket,
                                  jobjectArray &jargs)
{
    jmethodID evalMethod = env->GetMethodID(cmStubClass,
                                            "evaluateGenericFunctionCall",
                                            "(J[Ljava/lang/Object;)V");

    if (evalMethod == NULL)
    {
        TRACE(CELOG_ERR, _T("Unable to retrieve eval method."));
        return CE_RESULT_GENERAL_FAILED;
    } else {
    }

    env->CallVoidMethod (cmStub, evalMethod, (jlong)responseSocket, jargs);
    env->DeleteLocalRef(jargs);

    return CE_RESULT_SUCCESS;
}

CEResult_t PDP_GenericCallInit(JavaVM *PDP_jvm)
{
    JNIEnv *env = NULL;
    jint res = PDP_jvm->AttachCurrentThread((void**)&env, NULL);
    if (res < 0)
    {
        return CE_RESULT_GENERAL_FAILED;
    }

    if (objectClass == NULL) {
	jclass localClass = env->FindClass("java/lang/Object");
	if (localClass != NULL) {
	    objectClass = (jclass)env->NewGlobalRef(localClass);
	    env->DeleteLocalRef(localClass);
	}
    }
    if (stringClass == NULL) {
	jclass localClass = env->FindClass("java/lang/String");
	if (localClass != NULL) {
	    stringClass = (jclass)env->NewGlobalRef(localClass);
	    env->DeleteLocalRef(localClass);
	}
    }
    if (integerClass == NULL) {
	jclass localClass = env->FindClass("java/lang/Integer");
	if (localClass != NULL) {
	    integerClass = (jclass)env->NewGlobalRef(localClass);
	    env->DeleteLocalRef(localClass);
	}
    }
    if (booleanClass == NULL) {
	jclass localClass = env->FindClass("java/lang/Boolean");
	if (localClass != NULL) {
	    booleanClass = (jclass)env->NewGlobalRef(localClass);
	    env->DeleteLocalRef(localClass);
	}
    }
    if (objectClass == NULL || stringClass == NULL || integerClass == NULL || booleanClass == NULL) {
	TRACE(CELOG_ERR, _T("Cannot create basic JNI types.\n"));
        return CE_RESULT_GENERAL_FAILED;
    }

    // set up constructors for boolean and integer
    jobject obj;
    if ((obj = toBooleanObject(env, TRUE)) == NULL)
    {
        TRACE(CELOG_ERR, _T("Cannot initialize boolean object creation.\n"));
        return CE_RESULT_GENERAL_FAILED;
    }
    else
    {
        env->DeleteLocalRef(obj);
    }

    if ((obj = toIntegerObject(env, 0)) == NULL)
    {
        TRACE(CELOG_ERR, _T("Cannot initialize integer object creation.\n"));
        return CE_RESULT_GENERAL_FAILED;
    }
    else
    {
        env->DeleteLocalRef(obj);
    }	

    return CE_RESULT_SUCCESS;
}

CEResult_t PDP_GenericCall(JavaVM *PDP_jvm,
                           jobject cmStub,
                           jclass cmStubClass,
                           nlsocket responseSocket,
                           std::vector<void *> &inputArgs)
{
    if (inputArgs.size() < 2 || inputArgs[1] == NULL)
    {
        // Trouble.  We can't even response to the caller to report the trouble, because
        // we don't have a reqId (inputArgs[1]).
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString reqId = (CEString)inputArgs[1];

    JNIEnv *env = NULL;
    jint res = PDP_jvm->AttachCurrentThread((void**)&env, NULL);
    if (res < 0)
    {
        TRACE(CELOG_ERR, _T("Cannot attach JNI to disposer thread.\n"));
        ErrorReply(responseSocket, reqId, CE_RESULT_GENERAL_FAILED);
        return CE_RESULT_GENERAL_FAILED;
    }

    jobjectArray jargsIn = convertRequestToJava(env, inputArgs);

    if (jargsIn != NULL)
    {
        CEResult_t javaCallResult = CallGenericEval(env, cmStub, cmStubClass, responseSocket, jargsIn);
        PDP_jvm->DetachCurrentThread();
        return javaCallResult;
    }
    else
    {
        TRACE(CELOG_ERR, _T("Error converting arguments to java objects.\n"));
        ErrorReply(responseSocket, reqId, CE_RESULT_GENERAL_FAILED);
        PDP_jvm->DetachCurrentThread();
        return CE_RESULT_GENERAL_FAILED;
    }
}

CEResult_t PDP_GenericResponse(JNIEnv *env,
                               nlsocket jhandle,
                               jstring jReqId,
                               jobjectArray response)
{
    if (jReqId == NULL)
    {
        return CE_RESULT_INVALID_PARAMS;
    }
        
    nlchar *fmt = NULL;
    std::vector<void *> responseVec;
        
    CEResult_t res = convertJavaObjectsToResponse(env, jReqId, response, &fmt, responseVec);
        
    if (res != CE_RESULT_SUCCESS)
    {
        nlchar * reqId = getStringFromJava(env, jReqId);
        ErrorReply(jhandle, reqId, res);
        releaseStringFromJava(reqId);
        return res;
    }
        
    size_t marshaledResponseLen;
    char * marshaledResponse = Marshal_PackReplyGeneric(fmt,
                                                        res,
                                                        responseVec,
                                                        marshaledResponseLen);
        
    if (marshaledResponse == NULL)
    {
        nlchar * reqId = getStringFromJava(env, jReqId);
        ErrorReply(jhandle, reqId, CE_RESULT_GENERAL_FAILED);
        releaseStringFromJava(reqId);
        return CE_RESULT_GENERAL_FAILED;
    }
        
    CEResult_t transportResponse = TRANSPORT_Sendn(jhandle, marshaledResponseLen, marshaledResponse);

    Marshal_PackFree(marshaledResponse);
    FreeResponseVector(fmt, responseVec);
    releaseStringFromJava(fmt);
    return transportResponse; 
}


