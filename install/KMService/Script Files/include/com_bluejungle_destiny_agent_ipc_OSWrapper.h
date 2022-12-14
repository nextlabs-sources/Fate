/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_bluejungle_destiny_agent_ipc_OSWrapper */

#ifndef _Included_com_bluejungle_destiny_agent_ipc_OSWrapper
#define _Included_com_bluejungle_destiny_agent_ipc_OSWrapper
#ifdef __cplusplus
extern "C" {
#endif
/* Inaccessible static: COMP_INFO */
/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createFileMapping
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createFileMapping
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    hashChallenge
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_hashChallenge
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openFileMapping
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openFileMapping
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    mapViewOfFile
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_mapViewOfFile
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    unmapViewOfFile
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_unmapViewOfFile
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createEvent
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createEvent
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openEvent
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openEvent
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setEvent
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setEvent
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    createMutex
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_createMutex
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    openMutex
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_openMutex
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    releaseMutex
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_releaseMutex
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    waitForSingleObject
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_waitForSingleObject
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    waitForMultipleObjects
 * Signature: (I[I[Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_waitForMultipleObjects
  (JNIEnv *, jobject, jint, jintArray, jobjectArray, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readString
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readString
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readIPCResponse
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCResponse
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readIPCRequest
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readIPCRequest
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    readPolicyIPCRequest
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_readPolicyIPCRequest
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeString
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeString
  (JNIEnv *, jobject, jint, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeIPCRequest
 * Signature: (I[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeIPCRequest
  (JNIEnv *, jobject, jint, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writeIPCResponse
 * Signature: (I[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writeIPCResponse
  (JNIEnv *, jobject, jint, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    writePolicyIPCResponse
 * Signature: (I[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_writePolicyIPCResponse
  (JNIEnv *, jobject, jint, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    closeHandle
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_closeHandle
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    closeProcessToken
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_closeProcessToken
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getProcessId
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getProcessId
  (JNIEnv *, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getRDPAddress
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getRDPAddress
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    logEvent
 * Signature: (II[Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_logEvent
  (JNIEnv *, jobject, jint, jint, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setupKernelIPC
 * Signature: (I[I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setupKernelIPC
  (JNIEnv *, jobject, jint, jintArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    uninitKernelIPC
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_uninitKernelIPC
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setupSocket
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setupSocket
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    shutdownSocket
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_shutdownSocket
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getLoggedInUsers
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getLoggedInUsers
  (JNIEnv *, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileCreateTime
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileCreateTime
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileModifiedTime
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileModifiedTime
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileAccessTime
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileAccessTime
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileCustomAttributes
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileCustomAttributes
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileStandardAttributes
 * Signature: (Ljava/lang/String;ILjava/lang/Object;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileStandardAttributes
  (JNIEnv *, jobject, jstring, jint, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFileBasicAttributes
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)[J
 */
JNIEXPORT jlongArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFileBasicAttributes
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getOwnerSID
 * Signature: (Ljava/lang/String;ILjava/lang/Object;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getOwnerSID
  (JNIEnv *, jobject, jstring, jint, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getGroupSID
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getGroupSID
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getAppInfo
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getAppInfo
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getFQDN
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getFQDN
  (JNIEnv *, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getSharePhysicalPath
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getSharePhysicalPath
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDocumentsFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDocumentsFolder
  (JNIEnv *, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getMyDesktopFolder
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getMyDesktopFolder
  (JNIEnv *, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getNextKernelPolicyRequest
 * Signature: ([I)[Ljava/lang/Object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getNextKernelPolicyRequest
  (JNIEnv *, jobject, jintArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    isRemovableMedia
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_isRemovableMedia
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getUserName
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getUserName
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    setKernelPolicyResponse
 * Signature: (IJJJ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_setKernelPolicyResponse
  (JNIEnv *, jobject, jint, jlong, jlong, jlong);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    sendPolicyResponse
 * Signature: (ILjava/lang/String;J[Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_sendPolicyResponse
  (JNIEnv *, jobject, jint, jstring, jlong, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    isEmptyDirectory
 * Signature: (Ljava/lang/String;Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_isEmptyDirectory
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getContentAnalysisAttributes
 * Signature: (Ljava/lang/String;ILjava/lang/Object;Ljava/lang/String;I[Ljava/lang/String;)[I
 */
JNIEXPORT jintArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getContentAnalysisAttributes
  (JNIEnv *, jobject, jstring, jint, jobject, jstring, jint, jobjectArray);

/*
 * Class:     com_bluejungle_destiny_agent_ipc_OSWrapper
 * Method:    getNetworkDiskResources
 * Signature: (Ljava/lang/Object;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_destiny_agent_ipc_OSWrapper_getNetworkDiskResources
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
