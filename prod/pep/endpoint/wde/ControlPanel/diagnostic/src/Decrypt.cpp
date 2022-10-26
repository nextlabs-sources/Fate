// Decrypt.cpp : Defines the entry point for the Decrypt console application.
//

#include "stdafx.h"
#include <jni.h>
#include <windows.h>
#include "utilities.h"
#include "decrypt.h"

#include "strsafe.h"
#include "include/enhancement.h"
#include "src/enhancement_i.c"


// #define EXCELSIOR_JET
#define MAINCLASS "com/bluejungle/destiny/agent/tools/DecryptBundle"

#ifndef EXCELSIOR_JET
#define CLASSPATH "-Djava.class.path=jlib\\agent-tools.jar;jlib\\client-pf.jar;jlib\\common-framework.jar;jlib\\common-pf.jar;jlib\\jargs.jar;jlib\\agent-controlmanager.jar;jlib\\castor-0.9.5.4.jar;jlib\\management-types.jar;jlib\\axis.jar;jlib\\jaxrpc.jar;jlib\\xercesImpl.jar;jlib\\common-domain-types.jar;jlib\\common-framework-types.jar;jlib\\commons-logging.jar;jlib\\commons-cli.jar"
#endif

typedef jint  (JNICALL *CreateJavaVM)(JavaVM **, void **, void *);




BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

	}


	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;

}

BOOL IsWin7()
{
	DWORD dwMajor, dwMinor;
	return ( GetOSInfo(dwMajor, dwMinor) && dwMajor >= 6 )? TRUE: FALSE;
}


HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, _Out_ void ** ppv)
{
	HRESULT hr;
	if(IsWin7())
	{
		BIND_OPTS3 bo;
		WCHAR  wszCLSID[50];
		WCHAR  wszMonikerName[300];

		StringFromGUID2(rclsid, wszCLSID,   
			sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
		hr = StringCchPrintf(wszMonikerName,  
			sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
		if (FAILED(hr))
			return hr;
		memset(&bo, 0, sizeof(bo));
		bo.cbStruct = sizeof(bo);
		bo.hwnd = hwnd;
		bo.dwClassContext  = CLSCTX_LOCAL_SERVER;

		hr = CoGetObject(wszMonikerName, &bo, riid, ppv);
	}
	else 
	{
		hr = CoCreateInstance(rclsid, NULL, CLSCTX_LOCAL_SERVER, riid, ppv);
	}

	if(FAILED(hr))
	{
		g_log.Log(CELOG_DEBUG, L"Create instance for \"enhancement\" failed, err: %x", hr);
	}

	return hr;
}


/*
Find the agent home parameter in the arguments.  If it doesn't exist, guess based on our current location
*/
static bool getAgentHome(wstring& homePath) 
{
	edp_manager::CCommonUtilities::GetPCInstallPath(homePath);

	//	pc installer path is like c:\program files\nextlabs\policy controller\
	//	we don't need the last \\, remove it 
	homePath = homePath.substr(0, homePath.length() - 1);

	return true;
}

/*
FIX ME - Refactor into multiple functions
*/
BOOL CDecryptBundle::Decrypt(const wstring& strPassword)
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

	wstring edpMgrDir;
	edp_manager::CCommonUtilities::GetComponentInstallPath(edpMgrDir);
	edpMgrDir += wstring(L"bin");

	BOOL ret = FALSE;

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

	wstring strHome;
    getAgentHome(strHome);

	wstring strJreLocation;

    // Change directory to agent home so our jvm classpath is correct
    if (_tchdir(strHome.c_str()) != 0) 
    {
        g_log.Log(CELOG_DEBUG, L"The agent home directory, %ls, could not be found.\n", strHome.c_str());
		ret = FALSE;
        goto FUN_EXIT;
    }

    // size of agentHome + relatiave jre location + \n
	strJreLocation = strHome + wstring(L"\\jre\\bin\\server\\jvm.dll");    

    HMODULE hJniDll = ::LoadLibrary (strJreLocation.c_str());
    if (hJniDll == NULL)
    {
        hJniDll = ::LoadLibraryA ("jvm.dll");
    }

    if (hJniDll == NULL)
    {
		wprintf(L"jreLocation is %s\n", strJreLocation.c_str());
        g_log.Log(CELOG_DEBUG, L"%ls", _T("jvm.dll could not be loaded. Make sure the -e option is set correctly or jvm.dll is in your path."));
		ret = FALSE;
		goto FUN_EXIT;
    }

    CreateJavaVM pfnCreateJavaVM = (CreateJavaVM) ::GetProcAddress (hJniDll, "JNI_CreateJavaVM");
    jvmCreationResult = (pfnCreateJavaVM) (&jvm, (void **)&jniEnv, &vm_args);

    if (jvmCreationResult < 0) 
    {
        g_log.Log(CELOG_DEBUG, L"%ls", _T("Cannot create Java VM."));
		ret = FALSE;
		goto FUN_EXIT;
    }

    // Get the main class
    mainClass = jniEnv->FindClass(MAINCLASS);
    if (mainClass == 0) 
    {
        g_log.Log(CELOG_DEBUG, L"%ls", _T("Cannot find main class."));
		ret = FALSE;
		goto FUN_EXIT;
    }

	g_log.Log(CELOG_DEBUG, L"%ls", _T("found main class.\n"));

    // Get the method ID for the class's main(String[]) function. 
    mainMethodId = jniEnv->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    if (mainMethodId == 0) 
    {
        g_log.Log(CELOG_DEBUG, L"%ls", _T("Cannot find main method."));
		ret = FALSE;
		goto FUN_EXIT;
    }

	g_log.Log(CELOG_DEBUG, L"%ls", _T("found main method.\n"));

	//	create two string, "-p" "password"
    applicationArgs = jniEnv->NewObjectArray(2, jniEnv->FindClass("java/lang/String"), NULL);
    if (applicationArgs == 0) 
    {
        g_log.Log(CELOG_DEBUG, L"%ls", _T("Out of Memory!"));
		ret = FALSE;
		goto FUN_EXIT;
    }

	//	insert "-p"
	javaString = jniEnv->NewString((const jchar *)L"-p", (jsize)(_tcslen(L"-p")));
	if (javaString == 0) 
	{
		g_log.Log(CELOG_DEBUG, L"%ls", _T("Out of Memory!"));
		ret = FALSE;
		goto FUN_EXIT;
	}
	jniEnv->SetObjectArrayElement(applicationArgs, 0, javaString); 

	//	insert "password"
	//	insert "-p"
	javaString = jniEnv->NewString((const jchar *)strPassword.c_str(), (jsize)strPassword.length());
	if (javaString == 0) 
	{
		g_log.Log(CELOG_DEBUG, L"%ls", _T("Out of Memory!"));
		ret = FALSE;
		goto FUN_EXIT;
	}
	jniEnv->SetObjectArrayElement(applicationArgs, 1, javaString); 

	g_log.Log(CELOG_DEBUG, L"before CallStaticVoidMethod\n");

    jniEnv->CallStaticVoidMethod(mainClass, mainMethodId, applicationArgs);
    if (jniEnv->ExceptionCheck()) 
    {
		g_log.Log(CELOG_DEBUG, L"ExceptionDescribe\n");
        jniEnv->ExceptionDescribe();
    }

	g_log.Log(CELOG_DEBUG, L"after CallStaticVoidMethod\n");


FUN_EXIT:
    if (_tchdir(edpMgrDir.c_str()) != 0) 
	{
		g_log.Log(CELOG_DEBUG, L"_tchdir(edpMgrDir.c_str() failed after decrypt bundle\n");
	}

	return TRUE;
}

BOOL CDecryptBundle::DecryptViaTool(const wstring& strPassword)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	si.wShowWindow = SW_HIDE;
	si.dwFlags =  STARTF_USESHOWWINDOW;

	wstring cmd;
	edp_manager::CCommonUtilities::GetPCInstallPath(cmd);
	cmd += wstring(L"bin\\decrypt.exe") + wstring(L" -p ") + strPassword;

	g_log.Log(CELOG_DEBUG, L"command is %s in DecryptViaTool\n", cmd.c_str());

	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		(LPWSTR)cmd.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		g_log.Log(CELOG_DEBUG, L"CreateProcess failed (%d) in DecryptViaTool\n", GetLastError() );
		return FALSE;
	}

	// Wait until child process exits.
	g_log.Log(CELOG_DEBUG, L"before WaitForSingleObject in DecryptViaTool\n");
	WaitForSingleObject( pi.hProcess, INFINITE );
	g_log.Log(CELOG_DEBUG, L"after WaitForSingleObject in DecryptViaTool\n");

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	return TRUE;
}

BOOL CDecryptBundle::DecryptWithEnh(const wstring& strPassword)
{
	HRESULT hr;

	CComPtr<INLEDPManager> s_pIEDPMgr = NULL;
	if (!s_pIEDPMgr)
	{
		hr = CoCreateInstanceAsAdmin(GetForegroundWindow(), CLSID_NLEDPManager, IID_INLEDPManager, (void**)&s_pIEDPMgr);
	}

	if(s_pIEDPMgr)
	{
		BSTR   pwd=::SysAllocString(strPassword.c_str()); 
		hr = s_pIEDPMgr->Decrypt(pwd);
		::SysFreeString(pwd);
	}
	else
	{ 
		g_log.Log(CELOG_DEBUG, L"CoCreateInstanceAsAdmin fail\n");
		return FALSE;
	}

	return TRUE;
}
