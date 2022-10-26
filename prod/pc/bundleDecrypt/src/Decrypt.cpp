// Decrypt.cpp : Defines the entry point for the Decrypt console application.
//

#include "stdafx.h"
#include <jni.h>
#include <windows.h>

// #define EXCELSIOR_JET
#define MAINCLASS "com/bluejungle/destiny/agent/tools/DecryptBundle"

#ifndef EXCELSIOR_JET
#define CLASSPATH "-Djava.class.path=jlib\\agent-tools.jar;jlib\\client-pf.jar;jlib\\common-framework.jar;jlib\\common-pf.jar;jlib\\jargs.jar;jlib\\agent-controlmanager.jar;jlib\\castor-0.9.5.4.jar;jlib\\management-types.jar;jlib\\axis.jar;jlib\\jaxrpc.jar;jlib\\xercesImpl.jar;jlib\\common-domain-types.jar;jlib\\common-framework-types.jar;jlib\\commons-logging.jar;jlib\\commons-cli.jar"
#endif

typedef jint  (JNICALL *CreateJavaVM)(JavaVM **, void **, void *);

const _TCHAR* AGENT_HOME_OPTION = _T("-e");

/*
Find the agent home parameter in the arguments.  If it doesn't exist, guess based on our current location
*/
const _TCHAR* getAgentHome(int argc, TCHAR* argv[], bool *needToFreeBuf) {
    const _TCHAR* agentHome = NULL;

    *needToFreeBuf = false;

    for (int i=0; i<argc-1; i++) {
        if (_tcscmp(argv[i], AGENT_HOME_OPTION) == 0) {
            agentHome = argv[i+1];
            break;
        }
    }

    if (agentHome == NULL) 
    {
        _TCHAR *agentHomeBuf, *tmpPtr;

        agentHomeBuf = (_TCHAR *) malloc(MAX_PATH * sizeof(TCHAR));
        if (agentHomeBuf == NULL)
        {
          return NULL;
        }
        *needToFreeBuf = true;

        GetModuleFileName(NULL, agentHomeBuf, MAX_PATH);

        tmpPtr = _tcsrchr(agentHomeBuf, '\\');
        if (tmpPtr != NULL)
        {
          *tmpPtr = _T('\0');

          tmpPtr = _tcsrchr(agentHomeBuf, '\\');
          if (tmpPtr != NULL)
          {
            *tmpPtr = _T('\0');
          }
        }

        return agentHomeBuf;
    }

    return agentHome;
}

/*
FIX ME - Refactor into multiple functions
*/
int _tmain(int argc, _TCHAR* argv[])
{
    JavaVM *jvm;
    JavaVMOption options[1];
    JavaVMInitArgs vm_args;
    JNIEnv *jniEnv;
    jint jvmCreationResult;
    jclass mainClass;
    jmethodID mainMethodId;
    jobjectArray applicationArgs;
    jstring javaString;
    const _TCHAR* agentHome;
    bool needToFreeAgentHomeBuf;
    _TCHAR* jreLocation;

#ifndef EXCELSIOR_JET
    options[0].optionString = CLASSPATH;
#endif

    vm_args.version = JNI_VERSION_1_2;
    vm_args.options = options;
#ifdef EXCELSIOR_JET
    vm_args.nOptions = 0;
#else
    vm_args.nOptions = 1;
#endif
    vm_args.ignoreUnrecognized = TRUE;

    agentHome = getAgentHome(argc, argv, &needToFreeAgentHomeBuf);
    if (agentHome == NULL)
    {
        printf("%ls", _T("Out of Memory!"));
        return FALSE;
    }

    // Change directory to agent home so our jvm classpath is correct
    if (_tchdir(agentHome) != 0) 
    {
        printf("The agent home directory, %ls, could not be found.  Please check the value of the -e option.", agentHome);        
        return FALSE;
    }


    // size of agentHome + relatiave jre location + \n
    size_t agentHomeLength = _tcslen(agentHome);
    jreLocation = (TCHAR *)malloc((agentHomeLength + 28) * sizeof(_TCHAR));
    if (jreLocation == NULL)
    {
        printf("%ls", _T("Out of Memory!"));
        return FALSE;
    }
    _tcsncpy_s(jreLocation, agentHomeLength + 28, agentHome, _TRUNCATE);    
    if (needToFreeAgentHomeBuf)
    {
      free((_TCHAR*) agentHome);
    }

#if defined(_WIN64)
    _tcsncat_s(jreLocation, agentHomeLength + 28, _T("\\jre\\bin\\server\\jvm.dll"), _TRUNCATE);
#else
    _tcsncat_s(jreLocation, agentHomeLength + 28, _T("\\jre\\bin\\client\\jvm.dll"), _TRUNCATE);
#endif
    
    HMODULE hJniDll = ::LoadLibrary (jreLocation);
    if (hJniDll == NULL)
    {
        hJniDll = ::LoadLibraryA ("jvm.dll");
    }

    if (hJniDll == NULL)
    {
        printf("%ls", _T("jvm.dll could not be loaded. Make sure the -e option is set correctly or jvm.dll is in your path."));
        return FALSE;
    }

    CreateJavaVM pfnCreateJavaVM = (CreateJavaVM) ::GetProcAddress (hJniDll, "JNI_CreateJavaVM");
    jvmCreationResult = (pfnCreateJavaVM) (&jvm, (void **)&jniEnv, &vm_args);

    if (jvmCreationResult < 0) 
    {
        printf("%ls", _T("Cannot create Java VM."));
        return -1;
    }

    // Get the main class
    mainClass = jniEnv->FindClass(MAINCLASS);
    if (mainClass == 0) 
    {
        printf("%ls", _T("Cannot find main class."));
        return FALSE;
    }

    // Get the method ID for the class's main(String[]) function. 
    mainMethodId = jniEnv->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    if (mainMethodId == 0) 
    {
        printf("%ls", _T("Cannot find main method."));
        return FALSE;
    }

    applicationArgs = jniEnv->NewObjectArray(argc-1, jniEnv->FindClass("java/lang/String"), NULL);
    if (applicationArgs == 0) 
    {
        printf("%ls", _T("Out of Memory!"));
        return FALSE;
    }

    for(int i=1; i<argc; i++)
    {
        javaString = jniEnv->NewString((const jchar *)argv[i], (jsize)_tcslen(argv[i]));
        if (javaString == 0) 
        {
            printf("%ls", _T("Out of Memory!"));
            return FALSE;
        }
        jniEnv->SetObjectArrayElement(applicationArgs, i-1, javaString); 
    }

    jniEnv->CallStaticVoidMethod(mainClass, mainMethodId, applicationArgs);
    if (jniEnv->ExceptionCheck()) 
    {
        jniEnv->ExceptionDescribe();
    }

    free(jreLocation);

	return TRUE;
}

