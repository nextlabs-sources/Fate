#include "StdAfx.h"
#include <stdlib.h>
#include "javaclassloader.h"

typedef jint  (JNICALL *CreateJavaVM)(JavaVM **, void **, void *);
#define MAX_OPTIONS 20

//---------------------------------------------------------------------------
// UnicodeToAnsi
// 
// Tranlsates Unicode to Ansi strings
//---------------------------------------------------------------------------
static BOOL UnicodeToAnsi(
	LPCWSTR pszwUniString, 
	LPSTR  pszAnsiBuff,
	DWORD  dwAnsiBuffSize
	)
{
	int  iRet ;
    iRet = ::WideCharToMultiByte(
		CP_ACP,
		0,
		pszwUniString,
		-1,
		pszAnsiBuff,
		dwAnsiBuffSize,
		NULL,
		NULL
		);
	return (0 != iRet);
}

JavaClassLoader::JavaClassLoader(void)
{
}

JavaClassLoader::~JavaClassLoader(void)
{
}

BOOL JavaClassLoader::InitializeJvm (LPCTSTR pszwJRELocation, LPCTSTR pszwJavaArgs)
{

    jint res;
    JavaVMInitArgs vm_args;
    JavaVMOption options[MAX_OPTIONS];
    char jreLocation [MAX_PATH];
    DWORD argCount = 0;
    char* pArg = NULL;
    
    size_t bufSize = _tcslen (pszwJavaArgs) * 2 + 1;
    char* pszArgs = (char*) malloc (bufSize);
    char* pNextToken = NULL;

    UnicodeToAnsi (pszwJRELocation, jreLocation, MAX_PATH);
    UnicodeToAnsi (pszwJavaArgs, pszArgs, static_cast<DWORD>(bufSize));

    UINT i = 0;

    while ((pArg = strtok_s (i == 0? pszArgs: NULL, " ",&pNextToken)) != NULL)
    {
        options[i++].optionString = pArg;
        argCount++;
    }

    vm_args.version = JNI_VERSION_1_2;
    vm_args.options = options;
    vm_args.nOptions = argCount;
    vm_args.ignoreUnrecognized = TRUE;

    strncat_s (jreLocation, MAX_PATH,"\\jvm.dll", _TRUNCATE);
    HMODULE hJniDll = ::LoadLibraryA (jreLocation);
    if (hJniDll == NULL)
    {
        hJniDll = ::LoadLibraryA ("jvm.dll");
    }
    if (hJniDll == NULL)
    {
        return FALSE;
    }

    CreateJavaVM pfnCreateJavaVM = (CreateJavaVM) ::GetProcAddress (hJniDll, "JNI_CreateJavaVM");
    res = (pfnCreateJavaVM) (&vm, (void **)&env, &vm_args);
    return TRUE;
}


jclass JavaClassLoader::getClass (LPCSTR pszName)
{
    return env->FindClass(pszName);;
}

void JavaClassLoader::callStaticMainMethod (jclass classObj, LPTSTR* ppszArgs, int numArgs)
{
    jobjectArray args;
        
    // Get the method ID for the class's main(String[]) function. 
    jmethodID mid = env->GetStaticMethodID(classObj, "main", "([Ljava/lang/String;)V");

    // If there are arguments, create an ObjectArray sized to contain the
    // argument list, and then scan the list, inserting each argument into
    // the ObjectArray.
    // Otherwise, create an empty array. This is needed to avoid
    // creating an overloaded main that takes no arguments in the Java
    // app, and then getting a different method ID to the no-argument
    // main() method in this invoker code.
    args = env->NewObjectArray(numArgs,
        env->FindClass("java/lang/String"), NULL);

    for(int i=0; i<numArgs; i++)
    {
        jstring jstr = env->NewString((const jchar*)ppszArgs[i], static_cast<jsize>(_tcslen (ppszArgs[i])));
        env->SetObjectArrayElement(args, i, jstr); 
    }

    env->CallStaticVoidMethod(classObj, mid, args);

}

