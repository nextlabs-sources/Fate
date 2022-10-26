#pragma once

#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <jni.h>

static JavaVM *vm;
static JNIEnv *env;

#ifdef JAVACLASSLOADER_EXPORTS
    #define JAVACLASSLOADEREXPORT __declspec( dllexport)
#else
    #define JAVACLASSLOADEREXPORT __declspec( dllimport)
#endif // _AFXDLL

class JAVACLASSLOADEREXPORT JavaClassLoader
{
public:
    JavaClassLoader(void);
    virtual ~JavaClassLoader(void);

    static BOOL InitializeJvm (LPCTSTR pszJRELocation, LPCTSTR pszwJavaArgs);

    static jclass getClass (LPCSTR pszName);

    static void callStaticMainMethod (jclass classObj, LPTSTR* ppszArgs, int numArgs);
};
