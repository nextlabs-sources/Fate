#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in 3rd party's header file, just ignore it here

// Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#include "madCHook_helper.h"
#pragma warning(pop)

#include <string>
using namespace std;

#include <winsock2.h>
#include <windows.h>
#include <mapix.h>
#include <tlhelp32.h>
#include <ShlObj.h>
#include <Wingdi.h>
#pragma warning(push)
#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>  
#include "SvrAgent.h"

using namespace boost;
#pragma warning(pop)

typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 

//Pipe handle of "SendNow Online"
static const int BUFSIZE = 512;

static boost::shared_mutex g_mAttachMail;
static string g_ActivePath; 
static boost::unordered_map<string, string> g_MapSelectedFile;
static const char* NOPDF = "no_pdf";

HANDLE hHandleAttachMailThread = NULL;

bool bUnloaded = false;

static bool GetPathfromPDDoc(PDDoc pDoc, string& outputFile)
{
    bool ret = false;
    char* outPath = NULL;
    if (NULL != pDoc)
    {
        ASFile file = PDDocGetFile(pDoc);
        if (NULL != file)
        {
            ASFileSys sys = ASFileGetFileSys(file);
            ASPathName pathName = ASFileAcquirePathName(file);
            if (NULL != sys && NULL != pathName)
            {
                outPath = ASFileSysDisplayStringFromPath(sys, pathName);
                ASFileSysReleasePath(sys, pathName);
                if (NULL != outPath)
                {
                    outputFile = string(outPath);
                    ret = true;
                }
                else
                {
                    // In portfolia, ASFileGetURL maybe crash.
                    try
                    {
                        outPath = ASFileGetURL(file);
                    }
                    catch (...)
                    {
                    }

                    if (NULL != outPath)
                    {
                        outputFile = string(outPath);
                        ret = true;
                    }
                }
            }
        }
    }
 
    if (NULL != outPath)
    {
        ASfree(outPath);
        outPath = NULL;
    }
    else
    {
        OutputDebugStringW(L"GetPathfromPDDoc: get path failed.\n");
    }

    return ret;
}

static bool GetCurrentPDFPath(string& outputFile)
{
    bool ret = false;
    AVDoc activeAVDoc = AVAppGetActiveDoc();
    if (NULL != activeAVDoc)
    {
        PDDoc activePDDoc = AVDocGetPDDoc(activeAVDoc);
        ret = GetPathfromPDDoc(activePDDoc, outputFile); 
    }
    else
    {
        OutputDebugStringW(L"GetCurrentPDFPath: get active doc failed.\n");
    }

    return ret;
}

// return 0 means it's owner, no parent
static DWORD GetParentProcessID(DWORD dwProcessID)
{
	DWORD dwParentID = 0;
	HANDLE hProcessSnap = NULL;
	//HANDLE hProcess = NULL; not used anymore
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);    // Must clean up the
		//   snapshot object!
		return FALSE;
	}

	do
	{
		if (dwProcessID == pe32.th32ProcessID)
		{
			dwParentID = pe32.th32ParentProcessID;
		}
	} while(Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return dwParentID;
}

// check if this process id is same process as the process name
static bool IsTheSameProcess(const DWORD& dwProcessID,const wchar_t* szProcessName)
{
	bool bSame=false;
	HANDLE hProcessSnap = NULL;
	//HANDLE hProcess = NULL;not used anymore
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(bSame);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);    // Must clean up the
		//   snapshot object!
		return(bSame);
	}

	do
	{
		if (dwProcessID == pe32.th32ProcessID)
		{
			if (_wcsicmp(szProcessName,pe32.szExeFile) == 0)
			{
				bSame = true;
				break;
			}
		}
	} while(Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bSame;
}

DWORD WINAPI Handle_AttachMail(LPVOID lpParameter)
{
	BOOL   fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	DWORD dwProcID = GetCurrentProcessId();

	DWORD dwParentID = GetParentProcessID(dwProcID);
	wstring strPipeName;
	if (dwParentID > 0 && (IsTheSameProcess(dwParentID, L"acrobat.exe") || IsTheSameProcess(dwParentID, L"acrord32.exe")))
	{
		strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\CEAdobe_%d") % dwParentID);
	}
	else
	{
		strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\CEAdobe_%d") % dwProcID);
	}

	for (;;)
	{
		hPipe = CreateNamedPipe( 
			strPipeName.c_str(),             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE) 
		{
			return 0;
		}

		if (bUnloaded)
		{
			CloseHandle(hPipe);
			return 0;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		if (bUnloaded)
		{
			CloseHandle(hPipe);
			return 0;
		}

		if (fConnected) 
		{//client connected
			HANDLE hHeap      = GetProcessHeap();
            if (hHeap == NULL)  continue;
			char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE);

			if (pchRequest)
			{
  				DWORD cbBytesRead = 0;
				BOOL fSuccess = FALSE;

				fSuccess = ReadFile( 
					hPipe,        // handle to pipe 
					pchRequest,    // buffer to receive data 
					BUFSIZE, // size of buffer 
					&cbBytesRead, // number of bytes read 
					NULL);        // not overlapped I/O 

				if (fSuccess && cbBytesRead > 0)
				{
  					string command(pchRequest, cbBytesRead);
					if (_stricmp(command.c_str(), "1") == 0)
					{//do "attach email"
                     	string currentPdf;
						{
							boost_share_lock readerLock(g_mAttachMail);
							currentPdf = g_ActivePath;
                            if (currentPdf.empty())
                            {
                                currentPdf = NOPDF;
                            }
                        }  

                        // save the file path when connect to outlook.
                        {
                            std::vector<FilePair>	vecFiles;
                            string strSendFile = g_MapSelectedFile[currentPdf];
                            if (strSendFile.empty())
                            {
                                strSendFile = currentPdf;
                            }
                            WCHAR strPath[BUFSIZE] = { 0 };
                            wstring strName;
                            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strSendFile.c_str(), strSendFile.length(), strPath, strSendFile.length());
                            WCHAR* pwPos = NULL; 
                            pwPos = StrRStrIW(strPath, NULL, L"\\");
                            if (NULL == pwPos)
                            {
                                pwPos = StrRStrIW(strPath, NULL, L"/");
                            }
                            if (NULL != pwPos)
                            {
                                strName = pwPos + 1;
                            }
                            else
                            {
                                strName = strPath;
                            }
                           
                            vecFiles.push_back(FilePair(strName, strPath));
                            // put the path to OE server.
                            bool bServer = CTransferInfo::put_FileInfo(L"", 1, vecFiles);
                            g_MapSelectedFile[currentPdf] = "";
                        }   
					}
				}
				HeapFree(hHeap, 0, pchRequest);
			}
			DisconnectNamedPipe(hPipe); 
		}
		CloseHandle(hPipe); 
	}
}

typedef AVDoc (* _AVDocOpenFromPDDocWithParams)(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams params);
_AVDocOpenFromPDDocWithParams next_AVDocOpenFromPDDocWithParams=NULL;
AVDoc myAVDocOpenFromPDDocWithParams(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams params)
{
    string path;
    GetPathfromPDDoc(pdDoc,path);
    if(!path.empty())
    {
        boost_unique_lock writeLock(g_mAttachMail);
        g_ActivePath = path;       
    }

    return next_AVDocOpenFromPDDocWithParams(pdDoc, tempTitle, params);
}

ASCallback gcbAVAppOpenDialog;
ACCB1 ASBool ACCB2 MyAVAppOpenDialog(
                                     AVOpenSaveDialogParams dialogParams,			
                                     ASFileSys* outputFileSys,			
                                     ASPathName** outASPathNames,		
                                     AVArraySize* outNumASPathNames,		
                                     AVFilterIndex	*ioChosenFilterIndex)
{
    if (CALL_REPLACED_PROC(gAcroViewHFT, AVAppOpenDialogSEL, gcbAVAppOpenDialog)
        (dialogParams, outputFileSys, outASPathNames, 
        outNumASPathNames, ioChosenFilterIndex))
    {
        string currentPdf;
        GetCurrentPDFPath(currentPdf);      
        if (dialogParams->windowTitle)
        {
            const char* title=ASTextGetEncoded(dialogParams->windowTitle,NULL);
            if (title)
            {
                char* pPath = ASFileSysDisplayStringFromPath(*outputFileSys, **outASPathNames);
                string strOutPath;
                if (pPath != NULL)
                {
                    strOutPath = string(pPath);

                    if(!currentPdf.empty())
                    {
                        boost_unique_lock writeLock(g_mAttachMail);
                        g_MapSelectedFile[currentPdf] = strOutPath;
                    }
                    else
                    {
                        boost_unique_lock writeLock(g_mAttachMail);
                        g_MapSelectedFile[NOPDF] = strOutPath;
                    }
                    ASfree(pPath);
                }
            }
        }
        return true;
    }

    return false;
}
    

ACCB1 ASBool ACCB2 PluginImportReplaceAndRegister(void)
{  
    CollectHooks();
    HookCode((PVOID)(*((AVDocOpenFromPDDocWithParamsSELPROTO)(gAcroViewHFT[AVDocOpenFromPDDocWithParamsSEL]))), (PVOID)myAVDocOpenFromPDDocWithParams, (PVOID*)&next_AVDocOpenFromPDDocWithParams);	
    FlushHooks();  

    gcbAVAppOpenDialog = (void*)ASCallbackCreateReplacement(AVAppOpenDialogSEL, 
        MyAVAppOpenDialog);
    REPLACE(gAcroViewHFT, AVAppOpenDialogSEL, gcbAVAppOpenDialog);

    return true;
}

void myAVAppFrontDocDidChange(AVDoc doc, void* clientData)
{
	string strCurFile;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc, strCurFile);
    if(!strCurFile.empty())
	{
		boost_unique_lock writeLock(g_mAttachMail);
		g_ActivePath = strCurFile;
	}
}

void init_notification()
{
	AVAppRegisterNotification(AVAppFrontDocDidChangeNSEL, gExtensionID, ASCallbackCreateNotification(AVAppFrontDocDidChange, (void *)myAVAppFrontDocDidChange), NULL);
}
/* PluginInit
** ------------------------------------------------------
**/
/** 
	The main initialization routine.
	
	@return true to continue loading the plug-in, 
	false to cause plug-in loading to stop.
*/
ACCB1 ASBool ACCB2 PluginInit(void)
{
	init_notification();

	hHandleAttachMailThread = CreateThread(NULL, 0, Handle_AttachMail, NULL, 0, NULL);

	return true;
}

/* PluginUnload
** ------------------------------------------------------
**/
/** 
	The unload routine.
	Called when your plug-in is being unloaded when the application quits.
	Use this routine to release any system resources you may have
	allocated.

	Returning false will cause an alert to display that unloading failed.
	@return true to indicate the plug-in unloaded.
*/
ACCB1 ASBool ACCB2 PluginUnload(void)
{
	bUnloaded = true;

	DWORD dwProcID = GetCurrentProcessId();

	DWORD dwParentID = GetParentProcessID(dwProcID);
	wstring strPipeName;
	if (dwParentID > 0 && (IsTheSameProcess(dwParentID, L"acrobat.exe") || IsTheSameProcess(dwParentID, L"acrord32.exe")))
	{
		strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\CEAdobe_%d") % dwParentID);
	}
	else
	{
		strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\CEAdobe_%d") % dwProcID);
	}


	HANDLE hPipe = CreateFileW(strPipeName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE != hPipe)
	{
		CloseHandle(hPipe);
	}

	if (hHandleAttachMailThread != NULL)
	{
		WaitForSingleObject(hHandleAttachMailThread, INFINITE);
		CloseHandle(hHandleAttachMailThread);
	}

	return true;
}

/* GetExtensionName
** ------------------------------------------------------
*/
/**
	Returns the unique ASAtom associated with your plug-in.
	@return the plug-in's name as an ASAtom.
*/
ASAtom GetExtensionName()
{
	return ASAtomFromString("ADBE:Starter");	/* Change to your extension's name */
}

/** PIHandshake
	function provides the initial interface between your plug-in and the application.
	This function provides the callback functions to the application that allow it to 
	register the plug-in with the application environment.

	Required Plug-in handshaking routine: <b>Do not change its name!</b>
	
	@param handshakeVersion the version this plug-in works with. There are two versions possible, the plug-in version 
	and the application version. The application calls the main entry point for this plug-in with its version.
	The main entry point will call this function with the version that is earliest. 
	@param handshakeData OUT the data structure used to provide the primary entry points for the plug-in. These
	entry points are used in registering the plug-in with the application and allowing the plug-in to register for 
	other plug-in services and offer its own.
	@return true to indicate success, false otherwise (the plug-in will not load).
*/
ACCB1 ASBool ACCB2 PIHandshake(Uns32 handshakeVersion, void *handshakeData)
{
	if (handshakeVersion == HANDSHAKE_V0200) {

		/* Cast handshakeData to the appropriate type */
		PIHandshakeData_V0200 *hsData = (PIHandshakeData_V0200 *)handshakeData;

		/* Set the name we want to go by */
		hsData->extensionName = GetExtensionName();

        /*
		** If you import plug-in HFTs, replace functionality, and/or want to register for notifications before
		** the user has a chance to do anything, do so in here.
		*/
		hsData->importReplaceAndRegisterCallback = (void*)ASCallbackCreateProto(PIImportReplaceAndRegisterProcType,
																		 &PluginImportReplaceAndRegister);


		/* Perform your plug-in's initialization in here */
		hsData->initCallback = (void*)ASCallbackCreateProto(PIInitProcType, &PluginInit);

		/* Perform any memory freeing or state saving on "quit" in here */
		hsData->unloadCallback = (void*)ASCallbackCreateProto(PIUnloadProcType, &PluginUnload);

		/* All done */
		return true;

	} /* Each time the handshake version changes, add a new "else if" branch */

	/*
	** If we reach here, then we were passed a handshake version number we don't know about.
	** This shouldn't ever happen since our main() routine chose the version number.
	*/
	return false;
}
