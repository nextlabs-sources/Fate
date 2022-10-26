// NetworkUtils.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <tchar.h>
#include <lm.h>
#include "com_bluejungle_framework_utils_NetworkUtils.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

/*
 * Class:     com_bluejungle_framework_utils_NetworkUtils
 * Method:    getServerList
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_framework_utils_NetworkUtils_getServerList
  (JNIEnv * env, jclass obj)
{
    return NULL;
    //::NetServerEnum (NULL, 100, bufptr, prefmaxlen, entriesread, totalentries, servertype, domain, resume);

}

/*
 * Class:     com_bluejungle_framework_utils_NetworkUtils
 * Method:    getSharedFolderList
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_bluejungle_framework_utils_NetworkUtils_getSharedFolderList
  (JNIEnv * env, jclass obj, jstring server)
{
    SHARE_INFO_1* bufPtr = NULL;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    NET_API_STATUS err = 0;
    jobjectArray ret;
    const TCHAR *serverName = (const TCHAR*)env->GetStringChars(server, 0);
    TCHAR szServer [MAX_PATH];
    _tcsncpy_s (szServer, MAX_PATH, serverName, _TRUNCATE);
    env->ReleaseStringChars (server, (const jchar*)serverName);

    err = ::NetShareEnum(szServer, 1, (LPBYTE*) &bufPtr, MAX_PREFERRED_LENGTH, &entriesRead, &totalEntries, &resumeHandle);

    //do we need to worry about the case where totalEntries > MAX_PREFERRED_LENGTH
    if (err == NERR_Success)
    {
        DWORD sharedFolderCount = 0;
        for (DWORD i = 0; i < entriesRead; i++)
        {
            if (bufPtr[i].shi1_type == STYPE_DISKTREE)
            {
                sharedFolderCount++;
            }
        }

        ret= (jobjectArray)env->NewObjectArray(sharedFolderCount,
            env->FindClass("java/lang/String"),
            env->NewString((const jchar*)_T(""), 0));
    
        DWORD index = 0;
        for (DWORD i = 0; i < entriesRead; i++)
        {
            if (bufPtr[i].shi1_type == STYPE_DISKTREE)
            {
                env->SetObjectArrayElement(ret, index++, env->NewString((const jchar*)bufPtr[i].shi1_netname, _tcslen (bufPtr[i].shi1_netname)));
            }
        }

        ::NetApiBufferFree (bufPtr);
    } else 
    {
        ret= (jobjectArray)env->NewObjectArray(0,
            env->FindClass("java/lang/String"),
            env->NewString((const jchar*)_T(""), 0));
    }

    return (ret);
}
