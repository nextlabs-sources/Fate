/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 1998-2006 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 ---------------------------------------------------------------------

 StarterInit.cpp

 - Skeleton .cpp file for a plug-in. It implements the basic
   handshaking methods required for all plug-ins.

*********************************************************************/

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
#include <Wininet.h>
#include <mapix.h>
#include <tlhelp32.h>
#include <ShlObj.h>
#include "WinAPIFile.h"
#include "Action.h"
#include "policy.h"
#include "nltag.h"
#include "Encrypt.h"
#include "SaveAsObligation.h"
#include "DoEncryptObligation.h"
#include "PDDocInsertPages.h"
#include "Classify.h"
#include "Send.h"
#include "Edit.h"
#include "celog.h"
#include "nlconfig.hpp"
#include "HookDlg.h"
#include <Wingdi.h>
#include "eframework/auto_disable/auto_disable.hpp"
#pragma warning(push)
#pragma warning(disable:4819 4996 4995)
#include "PAMngr.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "AdobeXI.h"
using namespace boost;
#pragma warning(pop)

#include "ReaderToolsWndProcess.h"
#include "GetMsgHook.h"
#include "contentstorage.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_SRC_STARTERINIT_CPP

#pragma warning(push)
#pragma warning(disable: 6386 6031 6328 6258 6309 6387 6334 4267)  
#include <boost/asio.hpp>
#pragma warning(pop)

using namespace boost::asio;

#include <boost/lexical_cast.hpp>

const char* SCEServerIP = "127.0.0.1";

const USHORT SCEServerBasedPort = 20000;

const WCHAR* NamedPipePrefix = L"\\\\.\\pipe\\";
const WCHAR* NamedPipeFlag = L"AdobeScreenCapture";
const char* charNamedPipeFlag = "AdobeScreenCapture";
const int NamedPipeBufferSize = 512;

const char* RegisterHead = "register:";
const char* RegisterType = "type=namedpipe";
const char* RegisterName = "name=";
const char* RegisterPID = "pid=";
const char* RegisterSeparator = ";";
const int RegisterLength = 64;

const char* RegisterSuccessful = "register=successful";
const int RegisterResultLength = 32;

const char* UnregisterHead = "unregister:";
const int UnregisterLength = 64;

const char* UnregisterSuccessful = "unregister=successful";
const char* UnregisterFailed = "unregister=failed";
const int UnregisterResultLength = 32;

const char* QueryAllow = "query=allow";
const char* QueryDeny = "query=deny;displaytext=";
const int QueryResultLength = 512;

const char* ScreenCaptureAction = "SCREEN_CAPTURE";

boost::shared_mutex g_mAllOpenedFileAndTag;
static map<string, vector<pair<wstring, wstring>>> g_AllOpenedFileAndTag;

HANDLE hServerThread = NULL;
HANDLE hHandleAttachMailThread = NULL;

bool bUnloaded = false;

class CActiveDoc
{
public:
	string m_strPath;
	vector<pair<wstring, wstring>> m_tags;
};

const WCHAR SaveAsInExploreTagName[] = L"AdobePEPSrcOfTheTempFile";

AVTVersionNumPart MajorVersion = 0;
AVTVersionNumPart MinorVersion = 0;

BOOL bReaderXProtectedMode = FALSE;
BOOL bInIExplore = FALSE;
BOOL g_BeSavingAs = FALSE;

boost::shared_mutex g_mbeClosedFile;
string gbeClosedFile;

boost::shared_mutex g_mbeAttachment;
string gbeAttachment;

boost::shared_mutex g_mbeAcrobatcomFile;
string gbeAcrobatcomFile;

boost::shared_mutex g_mbeClosedASFile;
ASFile gbeClosedASFile = NULL;

boost::shared_mutex g_mtheLastClosedASFile;
ASFile gtheLastClosedASFile = NULL;

std::wstring g_wstrAutoSavePath;

std::string g_strRecordPathForAcrobatXIProtectedView;

boost::shared_mutex g_mFilesSEStatus;
boost::unordered_map<string, BOOL> gFilesSEStatus;

boost::shared_mutex g_mNeedDeletedAttachmentFile;
string gNeedDeletedAttachmentFile;

char TempLowPath[MAX_PATH] = { 0 };
int TempLowPathLength = 0;

//Pipe handle of "SendNow Online"
HANDLE hPipeOfSendNowOnline = NULL;
const int SENDNOWBUFSIZE = 512;

static boost::shared_mutex g_mAttachMail;
static CActiveDoc g_ActivePath;

nextlabs::recursion_control hook_control;  

boost::unordered_map<string, vector<string>> gMapSelectedFile;

CRITICAL_SECTION g_showbubbleCriticalSection;
HWND g_hBubbleWnd = NULL;

type_notify2 notify2 = NULL;

BOOL bReaderX=FALSE;

#define ADOBE_PDF_FILTER "Adobe PDF Files"
#define WARNING_CLOSEFILE L"This action requires security controls be applied. \nThe saved copy will be closed. Would you like to continue? "



/** 
	Starter is a plug-in template that provides a minimal 
	implementation for a plug-in. Developers may use this plug-in a 
	basis for their plug-ins.
*/

/*-------------------------------------------------------
	Core Handshake Callbacks
-------------------------------------------------------*/

/* PluginExportHFTs
** ------------------------------------------------------
**/



// return 0 means it's owner, no parent
BOOL GetParentProcessID(DWORD dwProcessID, DWORD& dwParentID)
{
	BOOL bret = FALSE;
	HANDLE hProcessSnap = NULL;
	//HANDLE hProcess = NULL; not used anymore
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		CELOG_LOG(CELOG_DEBUG, L"CreateToolhelp32Snapshot (of processes) failed.......\n");
		return( FALSE );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	
	// Retrieve information about the first process,
	// and exit if unsuccessful
	int count = 0;
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return( FALSE );
	}

	do
	{
		count++;
		if(dwProcessID == pe32.th32ProcessID )
		{
			dwParentID = pe32.th32ParentProcessID;
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	//for win10, only can enum system process and self process. so we using a const count 4 to define have an error.
	if (count >= 4)
	{
		bret = TRUE;
	}
	
	CloseHandle( hProcessSnap );
	return bret;
}

// check if this process id is same process as the process name
static bool IsTheSameProcess(const DWORD& dwProcessID,const wchar_t* szProcessName)
{
	bool bSame=false;
	HANDLE hProcessSnap = NULL;
	//HANDLE hProcess = NULL;not used anymore
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		CELOG_LOG(CELOG_DEBUG, L"CreateToolhelp32Snapshot (of processes) failed.......\n");
		return ( bSame );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return ( bSame );
	}

	do
	{
		if(dwProcessID == pe32.th32ProcessID )
		{
			if(_wcsicmp(szProcessName,pe32.szExeFile) == 0)
			{
				bSame = true;
				break;
			}
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return bSame;
}

bool Connect(HANDLE& h)
{
	bool bRet = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE; 

	DWORD dwProcID = GetCurrentProcessId();

	//DWORD dwParentID = GetParentProcessID(dwProcID);
	wstring strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\AdobeParent_%d") % dwProcID);

	CELOG_LOG(CELOG_DEBUG, L"try to connect pipe: %s\n", strPipeName.c_str());

	for(int i = 0; i < 1; i++) 
	{ 
		hPipe = CreateFile( 
			strPipeName.c_str(),   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE) 
		{
			h = hPipe;
			bRet = true;

			CELOG_LOG(CELOG_DEBUG, L"connected pipe: %s\n", strPipeName.c_str());

			break; 
		}
		else
		{
			if (ERROR_PIPE_BUSY == GetLastError())
			{
				i = -1;
				Sleep(200);
			}
		}
	//	Sleep(1000);
	} 

	return bRet;
}

int Send(HANDLE hPipe, const unsigned char *data, int len)
{
	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	BOOL fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if ( ! fSuccess) 
	{
		printf("SetNamedPipeHandleState failed\n");
		CELOG_LOGA(CELOG_DEBUG, "SetNamedPipeHandleState failed\n");
		return GetLastError();
	}

	int totalWrite = 0;


	while ( len > 0 )
	{
		DWORD cbWritten = 0;
		fSuccess = WriteFile( 
			hPipe,                  // pipe handle 
			data + totalWrite,             // message 
			len <= SENDNOWBUFSIZE? len: SENDNOWBUFSIZE,              // message length 
			&cbWritten,             // bytes written 
			NULL);  

		if ( ! fSuccess) 
		{
			DWORD dwErr = GetLastError();
			printf("WriteFile failed, error: %d\n", dwErr);
			CELOG_LOGA(CELOG_DEBUG, "WriteFile failed, error\n");
			return dwErr;
		}
		len -= cbWritten;
		totalWrite += cbWritten;
	}

	FlushFileBuffers(hPipe);
	return 0;
}

void RemoveOpenedFile(_In_ const string& fileName) 
{
	string strFileName(fileName);
	boost::replace_all(strFileName, L"/", L"\\");

	boost_unique_lock writeLock(g_mAllOpenedFileAndTag);
	g_AllOpenedFileAndTag.erase(strFileName);
}

void AddOpenedFile(_In_ const string& fileName) 
{
	if (!fileName.empty())
	{
		vector<pair<wstring, wstring>> tags;

		CTag::GetInstance()->read_tag(MyMultipleByteToWideChar(fileName), tags, NULL);
		string strFileName (fileName);
		boost::replace_all(strFileName, L"/", L"\\");

		boost_unique_lock writeLock(g_mAllOpenedFileAndTag);
		g_AllOpenedFileAndTag[strFileName] = tags;
	}
}

_Check_return_ bool QueryScreenCapture(_Out_ std::string& DisplayText)
{
	boost_share_lock readerLock(g_mAllOpenedFileAndTag);

	for (map<string, vector<pair<wstring, wstring>>>::iterator it = g_AllOpenedFileAndTag.begin(); it != g_AllOpenedFileAndTag.end(); ++it)
	{
		bool bdeny = false;
		nextlabs::Obligations obs;

		CPolicy::GetInstance()->query(ScreenCaptureAction, it->first, "", bdeny, obs, &(it->second));	

		if (bdeny)
		{
			return false;
		}
	}

	return true;
}

void RegisterPEPCLient()
{
	DWORD SessionID = 0;
	DWORD ProcessID = GetCurrentProcessId();

	ProcessIdToSessionId(ProcessID, &SessionID);

	USHORT ServerPort = SCEServerBasedPort + static_cast<USHORT>(SessionID);

	std::string RegisterStr = RegisterHead; 
	RegisterStr += RegisterType; 
	RegisterStr += RegisterSeparator; 
	RegisterStr += RegisterName; 
	RegisterStr += charNamedPipeFlag + boost::lexical_cast<string>(ProcessID);;
	RegisterStr += RegisterSeparator; 
	RegisterStr += RegisterPID; 
	RegisterStr += boost::lexical_cast<std::string>(ProcessID); 
	RegisterStr.resize(RegisterLength);

	while(true)
	{
		if (bUnloaded)
		{
			return;
		}

		try
		{
			io_service ios;

			ip::tcp::socket sock(ios);
			ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), ServerPort);

			sock.connect(ep);

			write(sock, buffer(RegisterStr));

			std::vector<char> str(RegisterResultLength);

			read(sock, buffer(str));
			
			if (boost::algorithm::iequals(&str[0], RegisterSuccessful))
			{
				break;
			}
		}
		catch (...)
		{
			CELOG_LOG(CELOG_DEBUG, L"exception in RegisterPEPCLient\n");	
		}
	}
}

void UnregisterPEPCLient()
{
	DWORD SessionID = 0;

	ProcessIdToSessionId(GetCurrentProcessId(), &SessionID);

	USHORT ServerPort = SCEServerBasedPort + static_cast<USHORT>(SessionID);

	DWORD ProcessID = GetCurrentProcessId();

	std::string UnregisterStr = UnregisterHead; 
	UnregisterStr += RegisterPID; 
	UnregisterStr += boost::lexical_cast<std::string>(ProcessID); 
	UnregisterStr.resize(UnregisterLength);

	try
	{
		io_service ios;

		ip::tcp::socket sock(ios);
		ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), ServerPort);

		sock.connect(ep);

		write(sock, buffer(UnregisterStr));

		std::vector<char> str(UnregisterResultLength);

		read(sock, buffer(str));
	}
	catch (...)
	{
		CELOG_LOG(CELOG_DEBUG, L"exception in UnregisterPEPCLient\n");	
	}
}

DWORD WINAPI ServerThread(LPVOID lpParameter)
{
	std::wstring PipeName = NamedPipePrefix;
	PipeName += NamedPipeFlag + boost::lexical_cast<std::wstring>(GetCurrentProcessId());;

	HANDLE hPipe = CreateNamedPipeW ( PipeName.c_str ( ), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, NamedPipeBufferSize, NamedPipeBufferSize, 0, NULL );

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		CELOG_LOG(CELOG_DEBUG, L"CreateNamedPipe fail, error:%d\n", GetLastError());	
		return 0;
	}

	RegisterPEPCLient();

	while(true)
	{
		if (bUnloaded)
		{
			CloseHandle(hPipe);
			return 0;
		}

		if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED)
		{
			CELOG_LOG(CELOG_DEBUG, L"ConnectNamedPipe fail, error:%d\n", GetLastError());	
			continue;
		}

		if (bUnloaded)
		{
			CloseHandle(hPipe);
			return 0;
		}

		string DisplayText;

		string QueryResult;

		if (QueryScreenCapture(DisplayText))
		{
			QueryResult = QueryAllow;
		}
		else
		{
			QueryResult = QueryDeny + DisplayText;
		}

		QueryResult.resize(QueryResultLength);
		QueryResult[QueryResultLength - 1] = '\0';

		DWORD WriteNumber = 0;

		if (!WriteFile(hPipe, QueryResult.c_str(), QueryResultLength, &WriteNumber, NULL) || WriteNumber != QueryResultLength)
		{
			CELOG_LOG(CELOG_DEBUG, L"In TalkWithSCE::StartServerThread, WriteFile error:%d\n", GetLastError());	
		}

		if (!FlushFileBuffers(hPipe))
		{
			CELOG_LOG(CELOG_DEBUG, L"FlushFileBuffers fail, error:%d\n", GetLastError());		
		}

		if (!DisconnectNamedPipe(hPipe))
		{
			CELOG_LOG(CELOG_DEBUG, L"DisconnectNamedPipe fail, error:%d\n", GetLastError());	
		}
	}
}

/*
This is a thread function.
This thread is waiting for other process to connect, and then
this thread can do query and send the result to the client.
*/
DWORD WINAPI Handle_AttachMail(LPVOID lpParameter)
{
	BOOL   fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	

	DWORD dwProcID = GetCurrentProcessId();
	std::wstring strPipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\Adobepep_%d") % dwProcID);

	for (;;)
	{
		hPipe = CreateNamedPipe( 
			strPipeName.c_str(),             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			SENDNOWBUFSIZE,                  // output buffer size 
			SENDNOWBUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE) 
		{
			CELOG_LOG(CELOG_DEBUG, L"Failed to create named pipe, name: %s, last error: %d\n", strPipeName.c_str(), GetLastError());
			return 0;
		}

		CELOG_LOG(CELOG_DEBUG, L"create named pipe successfully. name: %s\n", strPipeName.c_str());

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

				if(fSuccess && cbBytesRead > 0)
				{
					string command(pchRequest, cbBytesRead);
					CELOG_LOGA(CELOG_DEBUG, "received: %s\n", command.c_str());

					if (_stricmp(command.c_str(), "1") == 0)
					{//do "attach email" query
						string current_pdf;
						vector<pair<wstring, wstring>> tags;

						{
							boost_share_lock readerLock(g_mAttachMail);

							current_pdf = g_ActivePath.m_strPath;
							

							CELOG_LOGA(CELOG_DEBUG, "Try to do Attach mail evaluation current path, %s\n", current_pdf.c_str());

							vector<string> vPath = gMapSelectedFile[current_pdf];
							if (vPath.size() > 0)
							{
								current_pdf = vPath[0];
								CELOG_LOGA(CELOG_DEBUG, "Handle_AttachMail, cached path: %s\n", current_pdf.c_str());
							}

							if (_stricmp(current_pdf.c_str(), g_ActivePath.m_strPath.c_str()) == 0)
							{
								tags = g_ActivePath.m_tags;
								CELOG_LOGA(CELOG_DEBUG, "attach mail, path: %s, tag number: %d\n", current_pdf.c_str(), tags.size());
							}
						}

						
					
						bool bdeny=false;
						CPolicy* ins=CPolicy::GetInstance();
						nextlabs::Obligations obs;
						ins->query("SEND", current_pdf, "", bdeny, obs, &tags);
						

						CELOG_LOGA(CELOG_DEBUG, "Eval path: %s, result DENY: %d\n", current_pdf.c_str(), bdeny);

						//send the result back to the client
						string buf = boost::str(boost::format("%d") % bdeny);
						
						Send(hPipe, (const unsigned char*)buf.c_str(), buf.length());
					}
					else if(_stricmp(command.c_str(), "2") == 0)
					{
						//父进程有setclipboarddata事件
						string current_pdf;
						vector<pair<wstring, wstring>> tags;

						{
							boost_share_lock readerLock(g_mAttachMail);

							current_pdf = g_ActivePath.m_strPath;


							CELOG_LOGA(CELOG_DEBUG, "Try to do PASTE evaluation current path, %s\n", current_pdf.c_str());

							vector<string> vPath = gMapSelectedFile[current_pdf];
							if (vPath.size() > 0)
							{
								current_pdf = vPath[0];
								CELOG_LOGA(CELOG_DEBUG, "Handle_AttachMail, cached path: %s\n", current_pdf.c_str());
							}

							if (_stricmp(current_pdf.c_str(), g_ActivePath.m_strPath.c_str()) == 0)
							{
								tags = g_ActivePath.m_tags;
							}
						}



						bool bdeny=false;
						CPolicy* ins=CPolicy::GetInstance();
						nextlabs::Obligations obs;
						ins->query("PASTE", current_pdf, "", bdeny, obs, &tags);


						CELOG_LOGA(CELOG_DEBUG, "Eval path: %s, result DENY: %d\n", current_pdf.c_str(), bdeny);

						//send the result back to the client
						string buf = boost::str(boost::format("%d") % bdeny);

						Send(hPipe, (const unsigned char*)buf.c_str(), buf.length());
					}
                    else if(_stricmp(command.c_str(), "3") == 0)
                    {
                        bool bdeny = false;
                        bdeny = AdobeXI::CAdobeXITool::IsGreyOnlineWndOnSaveAsDlg();
                        string buf = boost::str(boost::format("%d") % bdeny);
                        Send(hPipe, (const unsigned char*)buf.c_str(), buf.length());
                    }
					else if(_stricmp(command.c_str(), "4") == 0)
					{
						string current_pdf;
						{
							boost_share_lock readerLock(g_mAttachMail);
							current_pdf = g_ActivePath.m_strPath;
						}

						vector<pair<wstring, wstring>> tags;
						bool bdeny=false;
						CPolicy* ins=CPolicy::GetInstance();
						nextlabs::Obligations obs;
						ins->query("SEND", current_pdf, "", bdeny, obs, &tags);
						string buf = boost::str(boost::format("%d") % bdeny);
						Send(hPipe, (const unsigned char*)buf.c_str(), buf.length());
					}
				}

				HeapFree(hHeap, 0, pchRequest);
			}

			DisconnectNamedPipe(hPipe); 
			
		}

		CloseHandle(hPipe); 
	}
}



/**
** Create and register the HFT's.
**
** @return true to continue loading plug-in,
** false to cause plug-in loading to stop.
*/
struct Print_OL
{
	COverLay Overlay;
	bool bIsDoOverlay;
	string strPrtOLFilePath;
	PDDoc OLDoc;
};

static Print_OL g_Print_OL;

ACCB1 ASBool ACCB2 PluginExportHFTs(void)
{
	return true;
}


#include "commonlib_helper.h"
static void GS_CopyContent_Path_Tags(_Inout_ wstring& strSrcPath,_Inout_ vector<pair<wstring,wstring>>& file_tags,_In_ bool bGet)
{
	// load nlcommonlib32.dll, for getting interfaces of nl_CacheData/nl_GetData/nl_FreeMem.
	static HMODULE hnlStorage = NULL;
	static nextlabs::nl_CacheData fnCacheData = NULL;
	static nextlabs::nl_GetData fnGetData = NULL;
	static nextlabs::nl_FreeMem fnFreeMem = NULL;

	if(hnlStorage == NULL)
	{
		std::wstring strCommonPath = GetCommonComponentsDir() ;
#ifdef _WIN64
		wstring strLib = strCommonPath + L"\\nlcommonlib.dll";
#else
		wstring strLib = strCommonPath + L"\\nlcommonlib32.dll";
#endif
		hnlStorage = LoadLibraryW(strLib.c_str());
		
		// get interface should be used later, they are nl_CacheData/nl_GetData/nl_FreeMem,
		// and you can see the definition of them in head file commonlib_helper.h.
		if (hnlStorage)
		{
			fnCacheData = (nextlabs::nl_CacheData)GetProcAddress(hnlStorage, "nl_CacheData");
			fnGetData = (nextlabs::nl_GetData)GetProcAddress(hnlStorage, "nl_GetData");
			fnFreeMem = (nextlabs::nl_FreeMem)GetProcAddress(hnlStorage, "nl_FreeMem");
		}
	}
	assert(hnlStorage != NULL && fnCacheData != NULL &&
		fnGetData != NULL && fnFreeMem != NULL);
	if(hnlStorage == NULL || fnCacheData == NULL ||
		fnGetData == NULL || fnFreeMem == NULL)	
	{
		CELOG_LOG(CELOG_DEBUG, L"fail to load functions from context manager\n");
		return ;
	}

	// the keyvalue and data you would cache/put
	const wchar_t* NL_CP_PathKey = L"NL_PC_PathKey";
	const wchar_t* NL_CP_PathName = L"NL_PC_FilePath";

	const wchar_t* NL_CP_Tag_Key = L"NL_PC_TagKey";
	const wchar_t* NL_CP_Tag_KeyName = L"NL_PC_FileTag";


	nextlabs::cache_key _key_path;
	nextlabs::cache_key _key_tags;
	
	
	int err = ERR_UNKNOWN;

	_key_path.set_value(NL_CP_PathKey, (const unsigned char*)NL_CP_PathName, (UINT)wcslen(NL_CP_PathName)*sizeof(wchar_t));
	_key_tags.set_value(NL_CP_Tag_Key, (const unsigned char*)NL_CP_Tag_KeyName, (UINT)wcslen(NL_CP_Tag_KeyName)*sizeof(wchar_t));


	if(bGet)
	{
		unsigned char* buf = NULL;
		unsigned int len = 0;

		//现在取source file path
		err = fnGetData(&_key_path, false, &buf, &len);
		if(err == ERR_SUCCESS && buf != NULL)
		{
			strSrcPath = (wchar_t*)buf;
			fnFreeMem(buf);
		}
		else
		{
			goto err;
		}

		//现在是要取tags
		err=fnGetData(&_key_tags, false, &buf, &len);

		if(err == ERR_SUCCESS && buf != NULL)
		{
			CELOG_LOG(CELOG_DEBUG, L"we got source file tags\n");

			//取到了tags，但是现在取到的tags是一连串字符，是tag名字加0x01加tag的值，加0x01加tag名字.....例如itar0x01yes0x01classification0x01public
			//另外这串字符串是wchar_t类型的
			wstring tags_wstring((wchar_t*)buf);
			wstring::size_type pos=tags_wstring.find((wchar_t)0x01);
			while(wstring::npos!=pos)
			{
				//找到了0x01，看看还有没有Ox01，如果没有的话就是最后一个tag了
				bool bLastTag=false;

				wstring::size_type pos_next=tags_wstring.find((wchar_t)0x01,pos+1);
				if (wstring::npos==pos_next)
				{
					//最后一个tag
					bLastTag=true;
				}

				//找tag的名字
				wstring tagname=tags_wstring.substr(0,pos);

				if (true==bLastTag)
				{
					//在这是最后一个tag的情况下，找tag的值
					wstring tagvalue=tags_wstring.substr(pos+1,tags_wstring.length()-pos-1);
					file_tags.push_back(pair<wstring,wstring>(tagname,tagvalue));

					break;
				}
				else
				{
					//在这不是最后一个tag的情况下，找tag的值
					wstring tagvalue=tags_wstring.substr(pos+1,pos_next-pos-1);
					file_tags.push_back(pair<wstring,wstring>(tagname,tagvalue));


					//更新tags_wstring，寻找下一个0x01
					tags_wstring=tags_wstring.substr(pos_next+1,tags_wstring.length()-pos_next-1);
					pos=tags_wstring.find((wchar_t)0x01);
				}
			}

			fnFreeMem(buf);
		}
	}
	else
	{
		//存储source file path
		err = fnCacheData(&_key_path, (const unsigned char*)strSrcPath.c_str(), (UINT)(strSrcPath.length()+1)*sizeof(wchar_t));

		if (err!=ERR_SUCCESS)
		{
			goto err;
		}

		//现在组装tag成为string
		wstring tags_wstring;
		for (DWORD i=0;i<file_tags.size();i++)
		{
			tags_wstring+=file_tags[i].first;
			tags_wstring+=(wchar_t)0x01;
			tags_wstring+=file_tags[i].second;

			if (file_tags.size()>(i+1))
			{
				//还不是最后一个tag,最后一个tag不用这样
				tags_wstring+=(wchar_t)0x01;
			}
		}
		if (tags_wstring.length())
		{
			//现在存储source fie tags
			CELOG_LOG(CELOG_DEBUG, L"try to store source file tags\n");
			err = fnCacheData(&_key_tags, (const unsigned char*)tags_wstring.c_str(), (UINT)(tags_wstring.length()+1)*sizeof(wchar_t));
		}		
	}

err:
	if(err != ERR_SUCCESS)
	{
		CELOG_LOG(CELOG_DEBUG, L"%s copy paste path %s failed,error is [%d].\n", (bGet?L"get":L"set"), 
			(bGet?L"":strSrcPath.c_str()), err);
	}
}



//////////////////////////////////////////////////////////////////////////
typedef struct {
	LPCSTR	dllName;
	LPCSTR	funcName;
	PVOID	newFunc;
	PVOID	*oldFunc;
} HookEntry;
void ConvertURLCharacterW(std::wstring& strUrl)
{
	/*
	*@Add for change '%5c%5b'->'\['->'[',  '%5c%5d'->'\]'->']' for bug 9339
	*/
	std::transform(strUrl.begin(), strUrl.end(), strUrl.begin(), towlower);
	boost::replace_all(strUrl,L"%25",L"%");	//'%28'-> '%'
	boost::replace_all(strUrl,L"%20",L" ");	//'%20' -> ' '
	boost::replace_all(strUrl,L"%21",L"!");	//'%21' -> '!'
	boost::replace_all(strUrl,L"%23",L"#");	//'%23' -> '#'
	boost::replace_all(strUrl,L"%24",L"$");	//'%24' -> '$'
	boost::replace_all(strUrl,L"%2a",L"*");	// '%2a  -> '*'
	boost::replace_all(strUrl,L"%2e",L".");	// '%2e  -> '.'
	boost::replace_all(strUrl,L"%2f",L"/");	// '%2f' -> '/'
	boost::replace_all(strUrl,L"%5c",L"\\");// '%5c' -> '\'
	boost::replace_all(strUrl,L"%5f",L"_");	// '%5f' -> '_'
	boost::replace_all(strUrl,L"+",L" ");		// '+'   -> ' '
	boost::replace_all(strUrl,L"%2d",L"-");	//'%2d' -> '-'
	boost::replace_all(strUrl,L"%26",L"-");	//'%26' -> '&'
	boost::replace_all(strUrl,L"%3a",L":");	//'%3a' -> ':'
	boost::replace_all(strUrl,L"%3d",L"=");	//'%3d' -> '='
	boost::replace_all(strUrl,L"%3f",L"?");	//'%3f' -> '?'
	boost::replace_all(strUrl,L"%40",L"@");	//'%40' -> '@'
	boost::replace_all(strUrl,L"%28",L"(");	//'%28'-> '('
	boost::replace_all(strUrl,L"%29",L")");	//'%29'-> ')'
	boost::replace_all(strUrl,L"%5b",L"[");	//'%5b'-> '['
	boost::replace_all(strUrl,L"%5d",L"]");	//'%5d'-> ']'
	boost::replace_all(strUrl,L"%5e",L"^");	//'%5e'-> '^'
	boost::replace_all(strUrl,L"%7b",L"{");	//'%7b'-> '{'
	boost::replace_all(strUrl,L"%7d",L"}");	//'%7d'-> '}'
	boost::replace_all(strUrl,L"%7e",L"~");	//'%7d'-> '~'

}

void ConvertURLCharacterA(std::string& strUrl)
{
	/*
	*@Add for change '%5c%5b'->'\['->'[',  '%5c%5d'->'\]'->']' for bug 9339
	*/
	std::transform(strUrl.begin(), strUrl.end(), strUrl.begin(), towlower);
	boost::replace_all(strUrl,"%25","%");	//'%28'-> '%'
	boost::replace_all(strUrl,"%20"," ");	//'%20' -> ' '
	boost::replace_all(strUrl,"%21","!");	//'%21' -> '!'
	boost::replace_all(strUrl,"%23","#");	//'%23' -> '#'
	boost::replace_all(strUrl,"%24","$");	//'%24' -> '$'
	boost::replace_all(strUrl,"%2a","*");	// '%2a  -> '*'
	boost::replace_all(strUrl,"%2e",".");	// '%2e  -> '.'
	boost::replace_all(strUrl,"%2f","/");	// '%2f' -> '/'
	boost::replace_all(strUrl,"%5c","\\");  // '%5c' -> '\'
	boost::replace_all(strUrl,"%5f","_");	// '%5f' -> '_'
	boost::replace_all(strUrl,"+"," ");		// '+'   -> ' '
	boost::replace_all(strUrl,"%2d","-");	//'%2d' -> '-'
	boost::replace_all(strUrl,"%26","-");	//'%26' -> '&'
	boost::replace_all(strUrl,"%3a",":");	//'%3a' -> ':'
	boost::replace_all(strUrl,"%3d","=");	//'%3d' -> '='
	boost::replace_all(strUrl,"%3f","?");	//'%3f' -> '?'
	boost::replace_all(strUrl,"%40","@");	//'%40' -> '@'
	boost::replace_all(strUrl,"%28","(");	//'%28'-> '('
	boost::replace_all(strUrl,"%29",")");	//'%29'-> ')'
	boost::replace_all(strUrl,"%5b","[");	//'%5b'-> '['
	boost::replace_all(strUrl,"%5d","]");	//'%5d'-> ']'
	boost::replace_all(strUrl,"%5e","^");	//'%5e'-> '^'
	boost::replace_all(strUrl,"%7b","{");	//'%7b'-> '{'
	boost::replace_all(strUrl,"%7d","}");	//'%7d'-> '}'
	boost::replace_all(strUrl,"%7e","~");	//'%7d'-> '~'

}

int (WINAPI *NextStartDocW)(HDC hdc, CONST DOCINFOW* lpdi);
int (WINAPI *NextEndPage) (HDC hdc);
int (WINAPI *NextEndDoc)(HDC hdc);



BOOL IsDCPostscript( HDC hDC )
{
	int		nEscapeCode;
	TCHAR	szTechnology[MAX_PATH] = TEXT("");

	// If it supports POSTSCRIPT_PASSTHROUGH, it must be PS.
	nEscapeCode = POSTSCRIPT_PASSTHROUGH;
	if( ExtEscape( hDC, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&nEscapeCode, 0, NULL ) > 0 )
		return TRUE;

	// If it doesn't support GETTECHNOLOGY, we won't be able to tell.
	nEscapeCode = GETTECHNOLOGY;
	if( ExtEscape( hDC, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&nEscapeCode, 0, NULL ) <= 0 )
		return FALSE;

	// Get the technology string and check to see if the word "postscript" is in it.
	if( ExtEscape( hDC, GETTECHNOLOGY, 0, NULL, MAX_PATH, (LPSTR)szTechnology ) <= 0 )
		return FALSE;
	_wcsupr_s( szTechnology );
	if( wcsstr( szTechnology, L"POSTSCRIPT" ) == NULL )
		return FALSE;

	// The word "postscript" was not found and it didn't support 
	//   POSTSCRIPT_PASSTHROUGH, so it's not a PS printer.
	return FALSE;
}




bool CheckPostScript(bool bIsExistOL,HDC hdc)
{
	if(bIsExistOL)
	{
		if(IsDCPostscript( hdc ))
		{
			::MessageBox(NULL,L"The printer you have selected does not support printing PDF document with security overlay. Please select a non-PostScript printer and try again.",L"Nextlabs Product",MB_OK);
			return false;
		}
	}
	return true;
}


int WINAPI BJStartDocW(
							  HDC hdc,              // handle to DC
							  CONST DOCINFOW* lpdi   // contains file names
							  )
{

	if ((NULL == lpdi) || (NULL == lpdi->lpszDocName)) {
		return NextStartDocW(hdc, lpdi);
	}

	
	// do eva

	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();

	nextlabs::Obligations obs;

	wstring strPrintPath = MyMultipleByteToWideChar(g_Print_OL.strPrtOLFilePath);

	if(strPrintPath.find(L"https://") != wstring::npos ||
		strPrintPath.find(L"http://") != wstring::npos)
	{
		ConvertURLCharacterW(strPrintPath);
	}

	CELOG_LOG(CELOG_DEBUG, L"BJStartDocW: %s \n",  strPrintPath.c_str());
	ins_policy->QueryObl("PRINT",strPrintPath,bdeny,obs);
	if (bdeny==true)
	{
		::SetLastError (ERROR_ACCESS_DENIED);
		return 0;
	}

	g_Print_OL.bIsDoOverlay = g_Print_OL.Overlay.SetOverlayData(obs,strPrintPath);

	bool bRet = CheckPostScript(g_Print_OL.bIsDoOverlay,hdc);
	if (!bRet)
	{
		::SetLastError (ERROR_ACCESS_DENIED);
		return 0;
	}

	if (g_Print_OL.bIsDoOverlay)
	{
		g_Print_OL.Overlay.SetHDC(hdc);
		g_Print_OL.Overlay.StartGDIPlus();
	}
	return NextStartDocW(hdc, lpdi);
}



int WINAPI BJEndPage(HDC hdc)
{
	if(g_Print_OL.bIsDoOverlay && g_Print_OL.Overlay.IsSameHDC(hdc) )
	{
		// do print overlay
		g_Print_OL.Overlay.DoPrintOverlay();
	}
	return NextEndPage(hdc);
}


int WINAPI BJEndDoc(HDC hdc)
{
	if (g_Print_OL.bIsDoOverlay)
	{
		g_Print_OL.Overlay.releaseHDC(hdc);
		g_Print_OL.Overlay.ReleaseOverlayData();
		g_Print_OL.Overlay.ShutDownGDIPlus() ;
	}
	return NextEndDoc(hdc);
}

string RecvData(HANDLE hPipe)
{
	HANDLE hHeap      = GetProcessHeap();
	char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE);

	if (pchRequest == NULL)
		return "";

	//Try to get the data from client
	std::string ret;
	unsigned int nTotalLen = 0;
	for(;;)
	{
		DWORD cbBytesRead = 0;
		BOOL fSuccess = FALSE;

		fSuccess = ReadFile( 
			hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE, // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 


		if ( !fSuccess && GetLastError() != ERROR_MORE_DATA )
		{   
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				CELOG_LOGA(CELOG_DEBUG, "InstanceThread: client disconnected.\n");
			}
			else
			{
				CELOG_LOGA(CELOG_DEBUG,"InstanceThread ReadFile failed, GLE=%d.\n", GetLastError());
			}
			break;
		}

		ret.append(pchRequest, cbBytesRead);

		if (nTotalLen == 0 && ret.length() >= 4)
		{
			unsigned int nDataLen = 0;
			memcpy(&nDataLen, ret.c_str(), 4);

			//get the length of byte stream
			nTotalLen = nDataLen;
		}
		if(nTotalLen > 0 && ret.length() == nTotalLen)
			break;
	}

	HeapFree(hHeap, 0, pchRequest);

	return ret;
}

HANDLE (WINAPI *NextCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,  DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE WINAPI try_BJCreateFileA( LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	if( hook_control.is_disabled() == true)
	{
		return NextCreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
			dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	string file(lpFileName);

	HANDLE res =NextCreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes, dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);

	if ( INVALID_HANDLE_VALUE != res )
	{
		if ( NULL == hPipeOfSendNowOnline && lpFileName != NULL)
		{
			//Determine this is the pipe of "SendNow Online"
			if ( 0 == _strnicmp ( "\\\\.\\pipe\\", lpFileName, strlen ( "\\\\.\\pipe\\" ) ) && (string::npos != file.rfind ( "\\Adobe\\Acrobat\\11.0\\Acrobat\\Synchronizer\\Commands" ) || string::npos != file.rfind ( "\\Adobe\\Acrobat\\11.0\\Synchronizer\\Commands" )) )
			{
				hPipeOfSendNowOnline = res;
			}
		}
	}

	return res;
}

HANDLE (WINAPI *NextCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,  DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE WINAPI try_BJCreateFileW( LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	if( hook_control.is_disabled() == true)
	{
		return NextCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
			dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	if ( NULL != lpFileName && !g_wstrAutoSavePath.empty() )
	{
		//Encrypt all tmp files in "AutoSave" Path in any case
		if ( boost::algorithm::istarts_with ( lpFileName, g_wstrAutoSavePath ) && boost::algorithm::iends_with( lpFileName, L".tmp" ) )
		{
			std::wstring wstrFileName(lpFileName);
			
			// send data

			if (bReaderXProtectedMode)
			{
				//
				CELOG_LOG(CELOG_DEBUG, L"try to connect in create file\n");


				HANDLE hPipe = INVALID_HANDLE_VALUE;
				if (Connect(hPipe) && hPipe != INVALID_HANDLE_VALUE)
				{
					CELOG_LOG(CELOG_DEBUG, L"try to send data to pipe in create file\n");

					string strFile=MyWideCharToMultipleByte(wstrFileName);
					char* buf = new char[strFile.length() + 6];
					memset(buf,0,strFile.length() + 6);

					if(buf != NULL)
					{
						unsigned int len=strFile.length() + 5;
						memcpy(buf, &len, 4);
						memcpy(buf + 4, "2", 1);
						memcpy(buf + 5, strFile.c_str(), strFile.length());

						Send(hPipe, (const unsigned char*)buf, len);
						delete []buf;
					}
					else
					{
						CELOG_LOG(CELOG_DEBUG, L"new buf fail!!!\n");
					}

					string recvdata =RecvData(hPipe);

					CELOG_LOG(CELOG_DEBUG, L"after recvdata in create file, length: %d\n", recvdata.length());
					

					CloseHandle(hPipe);		
				}
				else
				{
					CELOG_LOG(CELOG_DEBUG, L"connect error [%d]\n",GetLastError());
				}
			}
			else
			{
				CEncrypt::Encrypt(wstrFileName, TRUE);
			}
		}
	}

	//在往portfolio拖拽文件的时候，被拖拽的源文件没有在asfilesysopenfile里调用，奇怪，所以只能用windows的createfilew了
	//这里要修改，上面一行说的奇怪的地方，其真正原因是，虽然没在asfilesysopenfile里调用，但是在asfilesysopenfile64里调用。
	//所以，要修改createfile，不用createfile，而全部用asfilesysopenfile和openfile64.因为createfile会有很多不相干的文件，但asfilesysopenfile和openfile64就会少很多。
	//但是这个改动可以放在后面做。
	wstring file(lpFileName);
	HANDLE res =NextCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
		dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	if (res!=INVALID_HANDLE_VALUE)
	{
		//OutputDebugStringA("in try_BJCreateFileW\n");
		//OutputDebugStringW(lpFileName);
		CWinAPIFile* ins = CWinAPIFile::GetInstance();
		string strFile(file.begin(), file.end());
		ins->AddFileCreated(strFile);
	}

	if ( 10 == MajorVersion )  //Only consider Adobe Acrobat/Reader X
	{
		if ( INVALID_HANDLE_VALUE != res )
		{
			if ( NULL == hPipeOfSendNowOnline && lpFileName != NULL)
			{
				//Determine this is the pipe of "SendNow Online"
				if ( 0 == _wcsnicmp ( L"\\\\.\\pipe\\", lpFileName, wcslen ( L"\\\\.\\pipe\\" ) ) && wstring::npos != file.rfind ( L"\\Adobe\\Acrobat\\10.0\\Synchronizer\\Commands" ) )
				{
					hPipeOfSendNowOnline = res;
				}
			}
		}
	}

	return res;
}

BOOL (WINAPI* NextWriteFile) ( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped ) = NULL;
BOOL WINAPI try_BJWriteFile ( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	//the handle is the pipe?
	if ( hFile == hPipeOfSendNowOnline && NULL != lpBuffer && nNumberOfBytesToWrite > 500 )
	{
		std::string charBuffer ( (char*)lpBuffer, nNumberOfBytesToWrite );
		
		//Determine whether it is the command that contains file path
		if ( 0 == strncmp ( charBuffer.c_str(), "AddResourceOperation?id", strlen("AddResourceOperation?id") ) )
		{
			std::string::size_type ithandle = charBuffer.find("&req_handle=");
			std::string::size_type itheader = charBuffer.rfind("&req_header=");
			
			bool bNeedHandle = true;
			
			if (11 == MajorVersion)
			{
				std::string::size_type iurl = charBuffer.find("&req_URL=https%3A%2F%2Fsecure.echosign.com%2Fservices%2F");	
				if (std::string::npos == iurl)
				{
					bNeedHandle = false;
				}
			}

			if (bNeedHandle)
			{
				if ( std::string::npos != ithandle && std::string::npos != itheader && ithandle < itheader )
				{
					std::string FilePath = charBuffer.substr ( ithandle + 12, itheader - ithandle - 12 );    // 12 == strlen("&req_handle=");

					ConvertURLCharacterA ( FilePath );

					bool bdeny = false;
					CPolicy* ins = CPolicy::GetInstance ( );
					ins->queryLocalSourceAndDoTagObligation ( FilePath.c_str(), bdeny, "SEND" );

					if ( bdeny )
					{
						char* Buffer = new char[nNumberOfBytesToWrite];

						memcpy ( Buffer, lpBuffer, nNumberOfBytesToWrite);
						Buffer[--itheader] = '<';   //Set the last character of the file path to be < character, < character is not valid character of file path

						BOOL bRet = NextWriteFile(hFile, Buffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped );

						delete[] Buffer;

						return bRet;					
					}
				}
			}
		}
	}

	return NextWriteFile ( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped );
}



LONG (WINAPI* next_RegSetValueExA) ( HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData ) = NULL;
LONG WINAPI myRegSetValueExA ( HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData )
{
	if ( 0 == _stricmp ( lpValueName, "iTrustedMode" ) )
	{
		return ERROR_SUCCESS;
	}

	return next_RegSetValueExA ( hKey, lpValueName, Reserved, dwType, lpData, cbData );
}
typedef HANDLE (WINAPI* _GetClipboardData)(_In_  UINT uFormat);
_GetClipboardData next_GetClipboardData =NULL;
HANDLE WINAPI myGetClipboardData(_In_  UINT uFormat)
{
	CELOG_LOG(CELOG_DEBUG, L"in myGetClipboardData\n");

	//找到当前的pdf的信息，包括路径和tag，作为PASTE的dest信息
	//然后从context manager中去取PASTE的源信息
	string destfile;
	getCurrentPDFPath(destfile);
	wstring wDestFile(destfile.begin(),destfile.end());

	wstring wSrcFile;
	vector<pair<wstring,wstring>> srcTags;
	GS_CopyContent_Path_Tags(wSrcFile,srcTags,true);

	if (wSrcFile.length())
	{
		CELOG_LOG(CELOG_DEBUG, L"source file path:%s\n", wSrcFile.c_str());

		//读dest的tag
		vector<pair<wstring,wstring>> destTags;
		CTag* ins_tag=CTag::GetInstance();
		PDDoc pddoc;
		getCurrentPDDoc(pddoc);
		ins_tag->read_tag(wDestFile,destTags,pddoc);

		//做paste的evaluation
		CPolicy* ins_policy=CPolicy::GetInstance();

		string SrcFile(wSrcFile.begin(),wSrcFile.end());
		bool bdeny=false;
		nextlabs::Obligations obs;
		ins_policy->query("PASTE",SrcFile,destfile,bdeny,obs,&srcTags,NULL,&destTags);

		if (true==bdeny)
		{
			//原来我们是通过直接返回NULL来deny，但这样存在一个bug，就是如果剪切板里的内容被allow取出过来一次，以后再deny，也没作用--应该是因为之前已经被取出来过了，adobe自己有缓存。
			//所以我不直接返回，而是先empty剪切板，然后调用真实的next_GetClipboardData。目前的测试来看，这种方法很OK。
			CELOG_LOG(CELOG_DEBUG, L"deny copy content\n");
			EmptyClipboard();
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"allow copy content\n");
		}
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"fail to get source file path\n");
	}
	

	return next_GetClipboardData(uFormat);
}
typedef HANDLE (WINAPI* _SetClipboardData)(
	_In_      UINT uFormat,
	_In_opt_  HANDLE hMem
	);
_SetClipboardData next_SetClipboardData=NULL;
HANDLE WINAPI mySetClipboardData(
								 _In_      UINT uFormat,
								 _In_opt_  HANDLE hMem
								 )
{
	CELOG_LOG(CELOG_DEBUG, L"in mySetClipboardData\n");

	//找到当前的pdf的信息，包括路径和tag
	//然后设置到context manager中去，作为PASTE的源信息
	string file;
	getCurrentPDFPath(file);
	wstring wFile(file.begin(),file.end());

	vector<pair<wstring,wstring>> srcTags;

	CTag* ins_tags=CTag::GetInstance();
	PDDoc pddoc;
	getCurrentPDDoc(pddoc);
	ins_tags->read_tag(wFile,srcTags,pddoc);
	
	//做paste的evaluation
	CPolicy* ins_policy=CPolicy::GetInstance();
	bool bdeny=false;
	string dstFile;
	nextlabs::Obligations obs;
	ins_policy->query("PASTE",file,dstFile,bdeny,obs,&srcTags);

	if (true==bdeny)
	{
		CELOG_LOG(CELOG_DEBUG, L"deny copy content\n");
		SetLastError(5);
		return NULL;
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"allow copy content\n");
	}

	return next_SetClipboardData(uFormat,hMem);

	/************************************************************************/
	/*no need do evaluation on PASTE, only do evaluation on COPY in release 6.1
	/*so, no need for this code.
	/************************************************************************/
	//GS_CopyContent_Path_Tags(wFile,srcTags,false);
}
typedef BOOL (WINAPI* _EnableWindow)(
						 _In_  HWND hWnd,
						 _In_  BOOL bEnable
						 );
_EnableWindow next_EnableWindow=NULL;
BOOL WINAPI myEnableWindow(
						 _In_  HWND hWnd,
						 _In_  BOOL bEnable
						 )
{
	char text[256]={0};
	GetWindowTextA(hWnd,text,256);
	if (0!=text[0] && TRUE==bEnable)
	{
		//判断这是不是用户想要修改custom property
		if ((string)text==(string)"&Add" || (string)text==(string)"Delete" || (string)text==(string)"&Change")
		{
			CELOG_LOGA(CELOG_DEBUG, "in myEnableWindow, %s\n", text);

			CClassify* ins_classify=CClassify::GetInstance();
			if (ins_classify->IsDocPropDlgShowed())
			{
				//i am sure user want to delete/change/add custom properties. because the document properties dialog is now showed. so deny the enable window message if enforcement is deny
				CPolicy* ins_policy=CPolicy::GetInstance();
				string file;
				getCurrentPDFPath(file);
				bool bdeny=false;
				ins_policy->queryLocalSourceAndDoTagObligation(file.c_str(),bdeny,"CLASSIFY");
				if (true==bdeny)
				{
					return next_EnableWindow(hWnd,FALSE);
				}
			}
		}
		//判断这是不是用户想要Send and Collaborate
		else if ((string)text==(string)"Send")
		{
			CSend* ins_send=CSend::GetInstance();
			if(ins_send->IsSendCollaborateLiveShowed())
			{
				//当前已经有文件已经被选好了，要通过send and collaborate live发送到adobe.com
				//这个文件可以是当前打开的文件，也可以是通过OPEN对话框选择的，不过后者已经在OPEN对话框的时候做过evaluation了，
				//所以现在要对当前打开的文件做SEND的evaluation，如果是deny，就不能enable这个SEND的button
				bool bdeny=false;
				CPolicy* ins=CPolicy::GetInstance();

				string file;
				getCurrentPDFPath(file);

				if (file.length())
				{
					ins->queryLocalSourceAndDoTagObligation(file.c_str(),bdeny,"SEND");
					if (bdeny)
					{
						CELOG_LOG(CELOG_DEBUG, L"deny Send and Collaborate Live to Acrobat.com!!!!\n");
						return next_EnableWindow(hWnd,FALSE);
					}
				}
			}
		}
	}

	return next_EnableWindow(hWnd,bEnable);
}
typedef BOOL (WINAPI* _ShowWindow)(
					   _In_  HWND hWnd,
					   _In_  int nCmdShow
					   );
_ShowWindow next_ShowWindow=NULL;
BOOL WINAPI myShowWindow(
					   _In_  HWND hWnd,
					   _In_  int nCmdShow
					   )
{
	char text[256]={0};
	GetWindowTextA(hWnd,text,256);

	if (0!=text[0] && (string)"Document Properties"==(string)text)
	{
		CELOG_LOGA(CELOG_DEBUG, "in myShowWindow, %s\n", text);
		CClassify* ins_classify=CClassify::GetInstance();
		bool bShow= false;
		if (SW_SHOW==nCmdShow)
		{
			bShow=true;
		}
		ins_classify->ShowDocPropDlg(bShow);
	}
	else if (0!=text[0] && (string)"Send and Collaborate Live"==(string)text)
	{
		CELOG_LOGA(CELOG_DEBUG, "in myShowWindow, %s\n", text);
		CSend* ins_send=CSend::GetInstance();
		bool bShow= false;
		if (SW_SHOW==nCmdShow)
		{
			bShow=true;
		}
		ins_send->ShowSendAndCollaborateLive(bShow);
	}
	else if (0!=text[0] && ((string)"Getting Started"==(string)text || (string)"Send for Shared Review"==(string)text))
	{
		if (SW_SHOW==nCmdShow)
		{
			CELOG_LOGA(CELOG_DEBUG, "in myShowWindow, %s\n", text);

			string file;
			getCurrentPDFPath(file);
			if (!file.length())
			{
				MessageBoxA(NULL,"Please open a file before use review feature","Nextlabs DLP Warning",MB_OK);

				CELOG_LOG(CELOG_DEBUG, L"Post Cancel Message\n");

				PostMessageW(hWnd,WM_COMMAND,2,0);
			}
			else
			{
				CPolicy* ins_policy=CPolicy::GetInstance();
				bool bdeny=false;
				ins_policy->queryLocalSourceAndDoTagObligation(file.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					CELOG_LOG(CELOG_DEBUG, L"Post Cancel Message\n");
					PostMessageW(hWnd,WM_COMMAND,2,0);
				}
			}
		}
	}
	else if(0 != text[0] && (0 == strcmp(HD_STR_SPLIT_DOCUMENT_11, text) || 0 == strcmp(HD_STR_DISTRIBUTE_FORM, text) || 0 == strcmp(HD_STR_EXTRACT_PAGES, text)))
	{
		OnSpecificShowWindow(hWnd,text,nCmdShow);
	}

	return next_ShowWindow(hWnd,nCmdShow);
}

typedef BOOL
(WINAPI*
_SetWindowTextW)(
			   _In_ HWND hWnd,
			   _In_opt_ LPCWSTR lpString);
_SetWindowTextW next_SetWindowTextW=NULL;
BOOL
WINAPI
mySetWindowTextW(
			   _In_ HWND hWnd,
			   _In_opt_ LPCWSTR lpString)
{
	wstring wtext;
	if (lpString)
	{
		wtext=(wstring)lpString;
	}
	BOOL res=next_SetWindowTextW(hWnd,lpString);

	//看看这是不是在做"Upload Documents to Acrobat.com"
	if ((wstring)L"Upload"==wtext)
	{
		//parent' parent' parent' window text is "Upload Documents to Acrobat.com"
		HWND hparent=NULL;
		if (0==(hparent=GetParent(hWnd)))
		{
			goto _exit;
		}
		if (0==(hparent=GetParent(hparent)))
		{
			goto _exit;
		}
		if (0==(hparent=GetParent(hparent)))
		{
			goto _exit;
		}

		wchar_t parent_text[256]={0};
		GetWindowTextW(hparent,parent_text,255);

		if (0==parent_text[0])
		{
			goto _exit;
		}

		if((wstring)L"Upload Documents to Acrobat.com"==(wstring)parent_text)
		{
			if (IsWindowEnabled(hWnd))
			{
				//这是用户在做reader 9(acrobat 9)->file->Collaborate->
				//我们要做的是，检查reader当前打开的文件能不能被SEND，如果不能被SEND，我们就要DISABLE这个叫做"Upload"的button
				string current_pdf;
				getCurrentPDFPath(current_pdf);

				bool bdeny=false;
				CPolicy* ins=CPolicy::GetInstance();
				ins->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					CELOG_LOG(CELOG_DEBUG, L"deny upload to Acrobat.com, currently opened file!!!!\n");
					next_EnableWindow(hWnd,FALSE);
				}
			}		
		}
	}
	//看看这是不是在做"Share Documents on Acrobat.com"
	if ((wstring)L"Next >"==wtext)
	{
		//parent' parent' parent' window text is "Share Documents on Acrobat.com"
		HWND hparent=NULL;
		if (0==(hparent=GetParent(hWnd)))
		{
			goto _exit;
		}
		if (0==(hparent=GetParent(hparent)))
		{
			goto _exit;
		}
		if (0==(hparent=GetParent(hparent)))
		{
			goto _exit;
		}

		wchar_t parent_text[256]={0};
		GetWindowTextW(hparent,parent_text,255);

		if (0==parent_text[0])
		{
			goto _exit;
		}

		if((wstring)L"Share Documents on Acrobat.com"==(wstring)parent_text)
		{
			if (IsWindowEnabled(hWnd))
			{
				//这是用户在做reader 9(acrobat 9)->file->Collaborate->
				//我们要做的是，检查reader当前打开的文件能不能被SEND，如果不能被SEND，我们就要DISABLE这个叫做"Upload"的button
				string current_pdf;
				getCurrentPDFPath(current_pdf);

				bool bdeny=false;
				CPolicy* ins=CPolicy::GetInstance();
				ins->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					CELOG_LOG(CELOG_DEBUG, L"deny upload to Acrobat.com, currently opened file!!!!\n");
					next_EnableWindow(hWnd,FALSE);
				}
			}		
		}
	}

	if ((wstring)L"Continue"==wtext)
	{
		HWND hparent=NULL;
		if (0==(hparent=GetParent(hWnd)))
		{
			goto _exit;
		}
		if (0==(hparent=GetParent(hparent)))
		{
			goto _exit;
		}

		wchar_t parent_text[256]={0};
		GetWindowTextW(hparent,parent_text,255);

		if (0==parent_text[0])
		{
			goto _exit;
		}

		if((wstring)L"Save to Adobe Document Cloud"==(wstring)parent_text)
		{
			if (IsWindowEnabled(hWnd))
			{
				string current_pdf;
				getCurrentPDFPath(current_pdf);

				bool bdeny=false;
				CPolicy* ins=CPolicy::GetInstance();
				ins->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					next_EnableWindow(hWnd,FALSE);
				}
			}	
		}
	}

_exit:
	return res;
}

boost::shared_mutex g_Mutex_SEPdfWillBeRenamed;
wstring gSEPdfWillBeRenamed;//this is a SE pdf will be renamed

bool NeedEncrypt(const wstring& strExistingFile, const wstring& strNewFile)
{
	bool bRet = false;
	if (true==boost::algorithm::iends_with(strExistingFile,L".pdf") )
	{
		if(emIsEncrytFile== CEncrypt::Encrypt(strExistingFile, false,true))
		{
			boost_unique_lock writeLock(g_Mutex_SEPdfWillBeRenamed);
			gSEPdfWillBeRenamed = strExistingFile;
			CELOG_LOG(CELOG_DEBUG, L"we set gSEPdfWillBeRenamed, path: %s\n", strExistingFile.c_str());
		}
	}
	else if(true==boost::algorithm::iends_with(strNewFile,L".pdf"))
	{
		boost_share_lock readerLock(g_Mutex_SEPdfWillBeRenamed);
		if(strNewFile == gSEPdfWillBeRenamed)
		{
			CELOG_LOG(CELOG_DEBUG, L"encrypt file %s\n",strNewFile.c_str());

			bRet = true;
		}
	}

	return bRet;
}

typedef BOOL
(WINAPI*
_MoveFileExW)(
			_In_     LPCWSTR lpExistingFileName,
			_In_opt_ LPCWSTR lpNewFileName,
			_In_     DWORD    dwFlags
			);
_MoveFileExW next_MoveFileExW=NULL;
BOOL
WINAPI
myMoveFileExW(
			_In_     LPCWSTR lpExistingFileName,
			_In_opt_ LPCWSTR lpNewFileName,
			_In_     DWORD    dwFlags
			)
{
	if( hook_control.is_disabled() == true || NULL==lpExistingFileName || NULL==lpNewFileName)
	{
		return next_MoveFileExW(			/*_In_     LPCWSTR */lpExistingFileName,
			/*_In_opt_ LPCWSTR */lpNewFileName,
			/*_In_     DWORD    */dwFlags);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	CELOG_LOG(CELOG_DEBUG, L"MoveFileEx, src: %s, dest: %s\n", lpExistingFileName, lpNewFileName);

	bool bNeedEncrypt = NeedEncrypt(wstring(lpExistingFileName), wstring(lpNewFileName));

	BOOL ret = next_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	DWORD dwLastError = GetLastError();

	if (bNeedEncrypt)
	{
		CEncrypt::Encrypt(wstring(lpNewFileName),false);
	}

	SetLastError(dwLastError);
	return ret;
}

typedef BOOL (WINAPI* MoveFileWType)(
									 LPCWSTR lpExistingFileName,
									 LPCWSTR lpNewFileName);
MoveFileWType next_MoveFileW = NULL;
BOOL WINAPI myMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	//only handle acrobat 9
	if(MajorVersion != 9 ||  hook_control.is_disabled() == true || NULL==lpExistingFileName || NULL==lpNewFileName)
	{
		return next_MoveFileW(lpExistingFileName, lpNewFileName);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	CELOG_LOG(CELOG_DEBUG, L"MoveFileW, src: %s, dest: %s\n", lpExistingFileName, lpNewFileName);

	bool bNeedEncrypt = NeedEncrypt(wstring(lpExistingFileName), wstring(lpNewFileName));

	BOOL ret = next_MoveFileW(lpExistingFileName, lpNewFileName);
	DWORD dwLastError = GetLastError();

	if (bNeedEncrypt)
	{
		CEncrypt::Encrypt(wstring(lpNewFileName),false);
	}

	SetLastError(dwLastError);
	return ret;
}

typedef HRESULT (WINAPI *DoDragDropType)(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect);
DoDragDropType next_DoDragDrop = NULL;
HRESULT WINAPI myDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect)
{
	if(hook_control.is_disabled())
	{
		return next_DoDragDrop(pDataObj, pDropSource, dwOkEffects, pdwEffect);	
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (isContentData(pDataObj))
	{
		string current_pdf;
		getCurrentPDFPath(current_pdf);

		CContextStorage storage = CContextStorage();
		storage.StoreDragDropContentFileInfo(current_pdf);
	}

	return next_DoDragDrop(pDataObj, pDropSource, dwOkEffects, pdwEffect);	
}

LPMAPILOGONEX next_MAPILogonEx = NULL;
HRESULT WINAPI MyMAPILogonEx ( ULONG_PTR ulUIParam, LPTSTR lpszProfileName, LPTSTR lpszPassword, ULONG ulFlags, LPMAPISESSION* lppSession )\
{
	bool bDeny = false;

	CELOG_LOG(CELOG_DEBUG, L"in MyMAPILogonEx in AdobePEP, bInIExplore:%d\n",bInIExplore);

	if (bInIExplore)
	{

		string current_pdf;
		vector<pair<wstring, wstring>> tags;

		{
			boost_share_lock readerLock(g_mAttachMail);

			current_pdf = g_ActivePath.m_strPath;


			CELOG_LOGA(CELOG_DEBUG, "Try to do Attach mail evaluation current path, %s\n", current_pdf.c_str());

			vector<string> vPath = gMapSelectedFile[current_pdf];
			if (vPath.size() > 0)
			{
				current_pdf = vPath[0];
				CELOG_LOGA(CELOG_DEBUG, "Handle_AttachMail, cached path: %s\n", current_pdf.c_str());
			}

			if (_stricmp(current_pdf.c_str(), g_ActivePath.m_strPath.c_str()) == 0)
			{
				tags = g_ActivePath.m_tags;
				CELOG_LOGA(CELOG_DEBUG, "attach mail, path: %s, tag number: %d\n", current_pdf.c_str(), tags.size());
			}
		}

		CPolicy* ins=CPolicy::GetInstance();
		nextlabs::Obligations obs;
		ins->query("SEND", current_pdf, "", bDeny, obs, &tags);


		CELOG_LOGA(CELOG_DEBUG, "Eval path: %s, result DENY: %d\n", current_pdf.c_str(), bDeny);



		if (bDeny)
		{
			return MAPI_E_USER_CANCEL;
		}
	}
	return next_MAPILogonEx(ulUIParam, lpszProfileName, lpszPassword, ulFlags, lppSession);
}
HWND (WINAPI *next_CreateWindowExW)(
				__in DWORD dwExStyle,
				__in_opt LPCWSTR lpClassName,
				__in_opt LPCWSTR lpWindowName,
				__in DWORD dwStyle,
				__in int X,
				__in int Y,
				__in int nWidth,
				__in int nHeight,
				__in_opt HWND hWndParent,
				__in_opt HMENU hMenu,
				__in_opt HINSTANCE hInstance,
				__in_opt LPVOID lpParam)=NULL;

HWND WINAPI myCreateWindowExW(
				__in DWORD dwExStyle,
				__in_opt LPCWSTR lpClassName,
				__in_opt LPCWSTR lpWindowName,
				__in DWORD dwStyle,
				__in int X,
				__in int Y,
				__in int nWidth,
				__in int nHeight,
				__in_opt HWND hWndParent,
				__in_opt HMENU hMenu,
				__in_opt HINSTANCE hInstance,
				__in_opt LPVOID lpParam)
{
	HWND hWndRet = next_CreateWindowExW(dwExStyle,lpClassName,lpWindowName,	dwStyle,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
	
    if( hWndRet && (int)lpClassName>0x0000ffff && (int)lpWindowName>0x0000ffff &&
		_wcsicmp(lpClassName,L"ShockwaveFlashLibrary")==0 &&
        _wcsicmp(lpWindowName,L"AVFlashView")==0 )
	{
		//install getmessage hook
		if (AdobeXI::theGetMsgHook.GetHookHandle() == NULL )
		{
		   AdobeXI::theGetMsgHook.InstallGetMsgHook();
		}
		
		//
		AdobeXI::theReaderToolsWndProc.AddToolsWnd(hWndRet);
	
	}

	if (hWndRet != NULL)
	{
		wchar_t strWinTxt[MAX_PATH]={0};
		GetWindowText(hWndRet,strWinTxt,MAX_PATH);	
		if(_wcsicmp(strWinTxt,L"AVPageView")==0)
		{
				COverLay ol;
				ol.SetViewHwnd(hWndRet);

#if ACRO_SDK_LEVEL==0x000A0000	
			HWND hOverlayDependView = GetParent(GetParent(GetParent(GetParent(GetParent(hWndRet)))));			
			if(hOverlayDependView != NULL)
			{
				HWND hAVFormEditFormView = NULL;

				if (9 == MajorVersion)
				{
					hAVFormEditFormView = GetParent(hOverlayDependView);
				}
				else
				{
					hAVFormEditFormView = GetParent(GetParent(hOverlayDependView));
				}

				if (hAVFormEditFormView != NULL)
				{
					GetWindowText(hAVFormEditFormView,strWinTxt,MAX_PATH);
					if(_wcsicmp(strWinTxt,L"AVFormEditFormView")==0)
					{
						HWND hFrameWnd = GetParent(GetParent(hAVFormEditFormView));
						if (hFrameWnd != NULL)
						{
							COverLayData &Data = COverLayData::GetInstance();
							wstring strFilePath = Data.GetFilePathFromFrameWnd(hFrameWnd);
							if (!strFilePath.empty())
							{
								bool bdeny=false;
								CPolicy* ins_policy=CPolicy::GetInstance();					
								nextlabs::Obligations obs;
								ins_policy->QueryObl("OPEN",strFilePath,bdeny,obs);
								ol.DoViewOL(obs,strFilePath);

							}
						}
					}
				}
			}
#endif
				
			
		}
	}
	return hWndRet;
}

HINTERNET (WINAPI *next_HttpOpenRequestA)(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);

HINTERNET WINAPI myHttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{
	if (lpszVerb != NULL)
	{
		if (boost::algorithm::equals(lpszVerb, "PUT"))
		{
			if (lpszObjectName != NULL && boost::algorithm::starts_with( lpszObjectName, "/api/uux/assets/"))
			{
				string current_pdf;
				{
					boost_share_lock readerLock(g_mAttachMail);
					current_pdf = g_ActivePath.m_strPath;
				}

				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();
				ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					return NULL;
				}
			}
		}
		else if (boost::algorithm::equals(lpszVerb, "POST"))
		{
			if (lpszObjectName != NULL && boost::algorithm::starts_with( lpszObjectName, "/api/uux/assets"))
			{
				string current_pdf;
				{
					boost_share_lock readerLock(g_mAttachMail);
					current_pdf = g_ActivePath.m_strPath;
				}

				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();
				ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"SEND");
				if (bdeny)
				{
					return NULL;
				}
			}
		}
	}

	return next_HttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
}

HookEntry HookTable[] =
{	
	{ "advapi32.dll",	"RegSetValueExA",		myRegSetValueExA,		  (PVOID*)&next_RegSetValueExA },
	{ "Kernel32.DLL",	"CreateFileW",			try_BJCreateFileW,		  (PVOID*)&NextCreateFileW },
	{ "user32.DLL",		"SetClipboardData",		mySetClipboardData,		  (PVOID*)&next_SetClipboardData },

	{ "Gdi32.DLL",       	"StartDocW",			BJStartDocW,	       	  (PVOID*)&NextStartDocW},
	{ "Gdi32.DLL",       	"EndPage",			BJEndPage,	       	          (PVOID*)&NextEndPage},
	{ "Gdi32.DLL",       	"EndDoc",			BJEndDoc,	       	          (PVOID*)&NextEndDoc},
	/************************************************************************/
	/* NO NEED to do evaluation in PASTE in release 6.1						*/
	/************************************************************************/
	//{ "user32.DLL",		"GetClipboardData",		myGetClipboardData,		  (PVOID*)&next_GetClipboardData },
	{ "user32.DLL",		"EnableWindow",			myEnableWindow,			  (PVOID*)&next_EnableWindow },
	{ "user32.DLL",		"ShowWindow",			myShowWindow,			  (PVOID*)&next_ShowWindow },
	{ "user32.DLL",		"SetWindowTextW",			mySetWindowTextW,			  (PVOID*)&next_SetWindowTextW },

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
	{ "MAPI32.DLL",	"MAPILogonEx",			MyMAPILogonEx,		  (PVOID*)&next_MAPILogonEx},
	{ "Kernel32.DLL",	"MoveFileW",			myMoveFileW,		  (PVOID*)&next_MoveFileW},
#else
	
#endif

	{ "ole32.DLL",	"DoDragDrop",			myDoDragDrop,		  (PVOID*)&next_DoDragDrop},
	{ "Kernel32.DLL",	"MoveFileExW",			myMoveFileExW,		  (PVOID*)&next_MoveFileExW},
	{ "user32.DLL",		"CreateWindowExW",		myCreateWindowExW,	  (PVOID*)&next_CreateWindowExW },
	{ "Kernel32.DLL",	"WriteFile",			try_BJWriteFile,	  (PVOID*)&NextWriteFile },
};
void winapihook()
{
	int i;
	int nHooks = sizeof(HookTable) / sizeof(HookEntry); 
	for (i = 0; i < nHooks; i++)
	{
		HookAPI (HookTable[i].dllName, HookTable[i].funcName, HookTable[i].newFunc, HookTable[i].oldFunc, 0);
	}

	if ( 11 == MajorVersion )
	{
		HookAPI ( "Kernel32.DLL", "CreateFileA", try_BJCreateFileA, (PVOID*)&NextCreateFileA, 0 );

#if ACRO_SDK_LEVEL==0x000A0000	
		HookAPI ( "Wininet.DLL", "HttpOpenRequestA", myHttpOpenRequestA, (PVOID*)&next_HttpOpenRequestA, 0 );
#endif
	}

}

//////////////////////////////////////////////////////////////////////////
typedef ACEX1 PDFileAttachment (ACEX2* _PDFileAttachmentNewFromFile)(CosDoc parentDoc, ASFile sourceFile, const ASAtom* filterNames, const ASArraySize numFilters, CosObj filterParams, ASProgressMonitor monitor, ASConstText monitorText, void* monitorData);
_PDFileAttachmentNewFromFile next_PDFileAttachmentNewFromFile=NULL;
ACEX1 PDFileAttachment ACEX2 myPDFileAttachmentNewFromFile(CosDoc parentDoc, ASFile sourceFile, const ASAtom* filterNames, const ASArraySize numFilters, CosObj filterParams, ASProgressMonitor monitor, ASConstText monitorText, void* monitorData)
{
	//往pdf里添加attachment，这个函数不会被调用
	CELOG_LOG(CELOG_DEBUG, L"in myPDFileAttachmentNewFromFile\n");
	PDFileAttachment res = next_PDFileAttachmentNewFromFile(/*CosDoc*/ parentDoc, /*ASFile */sourceFile, /*const ASAtom* */filterNames, /*const ASArraySize */numFilters, /*CosObj */filterParams, /*ASProgressMonitor */monitor, /*ASConstText */monitorText, /*void* */monitorData);
	return res;
}
typedef ACEX1 PDFileAttachment (ACEX2* _PDFileAttachmentFromCosObj)(CosObj cosAttachment);
_PDFileAttachmentFromCosObj next_PDFileAttachmentFromCosObj=NULL;
ACEX1 PDFileAttachment ACEX2 myPDFileAttachmentFromCosObj(CosObj cosAttachment)
{
	//如果打开一个有attachment的pdf，这个函数会被调用
	//OutputDebugStringA("in myPDFileAttachmentFromCosObj\n");
	PDFileAttachment res = next_PDFileAttachmentFromCosObj(cosAttachment);
	ASText text = PDFileAttachmentGetFileName(res);
	const char* char_text = ASTextGetEncoded(text, NULL);
	if (char_text)
	{
		//可以得到attachment的名字，就是pdf里显示的名字，没有路径
		/*OutputDebugStringA("attachment name:\n");
		OutputDebugStringA(char_text);*/
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"can't get attachment name\n");
	}
	return res;
}
typedef ACEX1 void (ACEX2* _PDFileAttachmentUpdateFromFile)(PDFileAttachment attachment, ASFile sourceFile, ASProgressMonitor monitor, ASConstText monitorText, void* monitorData);
_PDFileAttachmentUpdateFromFile next_PDFileAttachmentUpdateFromFile=NULL;
ACEX1 void ACEX2 myPDFileAttachmentUpdateFromFile(PDFileAttachment attachment, ASFile sourceFile, ASProgressMonitor monitor, ASConstText monitorText, void* monitorData)
{
	//往pdf添加attachment的时候，会被调用，但是是临时路径里的文件，不是真正的源文件，如 file:///C|/Users/bsong/AppData/Local/Temp/A9RD67F.tmp/Microsoft.VC90.MFC.manifest
	//真正的源文件是C:\Users\bsong\Desktop\x86\Microsoft.VC90.MFC.manifest
	//根据process monitor发现，adobe会readfile源文件，然后writefile临时文件。为简单起见，我们就hook createfile，根据文件名，把源文件和临时文件联系起来。
	//最终把源文件和添加attachment事件联系起来

	CELOG_LOG(CELOG_DEBUG, L"in myPDFileAttachmentUpdateFromFile\n");
	
	//输出attachment的名字	
	get_attachment_name(attachment);

	//输出attachment文件的路径，这是一个临时文件
	ASFileSys sys = ASFileGetFileSys(sourceFile);
	ASPathName pathname = ASFileAcquirePathName(sourceFile);
	char* char_sourcefile=NULL;
	if (sys && pathname)
	{
		char_sourcefile = ASFileSysDisplayStringFromPath(sys, pathname);
		ASFileSysReleasePath(sys, pathname);
		if (char_sourcefile)
		{
			//可以得到文件的路径
			CELOG_LOGA(CELOG_DEBUG, "char_sourcefile: %s\n", char_sourcefile);
			//根据临时文件找到原文件
			CWinAPIFile* ins = CWinAPIFile::GetInstance();
			string origin_file;
			if (ins->GetOriginFileByTempFile(char_sourcefile, origin_file))
			{
				//找到了attachment原文件的全路径，可以做evaluation了
				CELOG_LOG(CELOG_DEBUG, L"this is add attachment or add contents to portfolio\n");

				//这是add attachment或者是往portfolio里添加文件，
				//现在我们只做CONVERT的evaluation
				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();
				ins_policy->queryLocalSourceAndDoTagObligation(origin_file.c_str(),bdeny,"CONVERT");
				if (bdeny)
				{
					//不调用真正的直接返回
					CELOG_LOG(CELOG_DEBUG, L"deny!!!!\n");
					ASfree(char_sourcefile);
					return;
				}
				else
				{
					CELOG_LOG(CELOG_DEBUG, L"allow!!!!\n");
				}
				////取得当前的pdf路径
				//string current_pdf;
				//getCurrentPDFPath(current_pdf);
			}

			ASfree(char_sourcefile);
		}
	}
	if (!char_sourcefile)
	{
		CELOG_LOG(CELOG_DEBUG, L"can't get char_sourcefile\n");
	}

	next_PDFileAttachmentUpdateFromFile(/*PDFileAttachment */attachment, /*ASFile */sourceFile, /*ASProgressMonitor */monitor, /*ASConstText */monitorText, /*void* */monitorData);
	return;
}
typedef ACEX1 ASBool (ACEX2* _ASFileAttachmentGetPDFileAttachment)(ASFileSys fileSys, ASPathName pathName, PDFileAttachment* attachment);
_ASFileAttachmentGetPDFileAttachment next_ASFileAttachmentGetPDFileAttachment=NULL;
ACEX1 ASBool ACEX2 myASFileAttachmentGetPDFileAttachment(ASFileSys fileSys, ASPathName pathName, PDFileAttachment* attachment)
{
	//添加attachment时候没有被调用过
	CELOG_LOG(CELOG_DEBUG, L"in myASFileAttachmentGetPDFileAttachment\n");
	ASBool res = next_ASFileAttachmentGetPDFileAttachment(/*ASFileSys */fileSys, /*ASPathName */pathName, /*PDFileAttachment* */attachment);
	return res;
}
typedef ACEX1 ASErrorCode (ACEX2* _ASFileSysOpenFile)(ASFileSys fileSys, ASPathName pathName, ASFileMode mode, ASFile* fP);
_ASFileSysOpenFile next_ASFileSysOpenFile = NULL;
ACEX1 ASErrorCode ACEX2 myASFileSysOpenFile(ASFileSys fileSys, ASPathName pathName, ASFileMode mode, ASFile* fP)
{
	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();

	BOOL bSEFile = FALSE;

	string strPath = GetPath(fileSys, pathName);

	if( !boost::algorithm::iends_with(strPath,".pdf") )
		return next_ASFileSysOpenFile(fileSys, pathName, mode, fP);

	if (!strPath.empty())
	{
		CELOG_LOGA(CELOG_DEBUG, "in myASFileSysOpenFile:%s\n", strPath.c_str());

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
		if( 11 == MajorVersion && AdobeXI::CAdobeXITool::IsiProtectedViewValueGreaterThanZero(MajorVersion, true))
		{
			if (!boost::algorithm::equals(g_strRecordPathForAcrobatXIProtectedView, strPath))
			{
				g_strRecordPathForAcrobatXIProtectedView = strPath;

				MessageBoxA(NULL,"There was an error opening this document in protect view.","Adobe Acrobat",MB_OK|MB_ICONWARNING);
			}

			return -1;
		}
#else
		if( 11 == MajorVersion && AdobeXI::CAdobeXITool::IsiProtectedViewValueGreaterThanZero(MajorVersion))
		{
			MessageBoxA(NULL,"There was an error opening this document in protect view.","Adobe Reader",MB_OK|MB_ICONWARNING);
			return -1;
		}
#endif

		//看看是不是auto save file
		//SE this auto save file if the pdf file is SE file
		if (IsAutoSaveFile(strPath))
		{
			wstring wpath = MyMultipleByteToWideChar(strPath);
			if (emIsEncrytFile==CEncrypt::Encrypt(wpath,false,true))
			{
				CEncrypt::Encrypt(wpath,true);
			}
		}
		//做OPEN的evaluation，这是最基本的。
		//我们要在OPEN文件之前对其打tag-如果有打tag的obligation的话。这是因为一旦文件被OPEN了之后就只能在adobe释放掉handle后才能对其打tag，而这会引发一些问题，
		//所以我们要在open之前做evaluation，而这时肯定没有PDDoc，所以不能用PDDocGetInfo，而要用tag library。
		if (true==IsURLPath(strPath))
		{
			ins_policy->queryHttpSource(strPath.c_str(),NULL,bdeny,"OPEN",true);
		}
		else
		{
			boost_share_lock readLock(g_mbeAttachment);
			if( 0 != _stricmp(gbeAttachment.c_str(), strPath.c_str()))
			{
				ins_policy->queryLocalSourceAndDoTagObligation(strPath.c_str(),bdeny,"OPEN");
			}
		}
		if (bdeny==true)
		{
			return -1;
		}

		//这是检查COPY, e.g. extract page OR split document are all COPY actions,
		CPDDocInsertPages* ins_pddoc_insertpage=CPDDocInsertPages::GetInstance();
		ins_pddoc_insertpage->execute_asopenfile(strPath,bdeny);
		if(true==bdeny)
		{
			//被deny了
			return -1;
		}

		//Query SE status, if deny, the file is SE
		if ( bReaderXProtectedMode )
		{
			ins_policy->queryLocalSourceAndDoTagObligation(strPath.c_str(),bdeny,"DECRYPT");
			if ( bdeny )
			{
				bSEFile = TRUE;
			}	
		}
	}

	ASErrorCode  res = next_ASFileSysOpenFile(fileSys, pathName, mode, fP);
	
	if ( bReaderXProtectedMode )
	{
		if ( 0 == res && !strPath.empty() )	//Open successfully, and path isn't null, save SE status
		{	
			std::string filepath = strPath;
			std::transform ( filepath.begin(), filepath.end(), filepath.begin(), towlower );
			
			boost_unique_lock writeLock(g_mFilesSEStatus);
			gFilesSEStatus[filepath] = bSEFile;

			CELOG_LOGA(CELOG_DEBUG, "Protected mode, se file: %s, %d\n", filepath.c_str(), bSEFile);
		}
	}

	//For bug 18425
	if ( bInIExplore && 0 == res && !strPath.empty() )
	{
		if ( 0 == _strnicmp ( TempLowPath, strPath.c_str(), TempLowPathLength ) && boost::algorithm::iends_with ( strPath.c_str(), ".tmp" ) )
		{
			string current_pdf;
			getCurrentPDFPath(current_pdf);

			wstring wstrPath = MyMultipleByteToWideChar(strPath);
			wstring wcurrent_pdf = MyMultipleByteToWideChar(current_pdf);

			vector<pair<wstring,wstring>> Tags;
			Tags.push_back ( pair<wstring,wstring> ( SaveAsInExploreTagName, wcurrent_pdf ) );

			CTag* ins_tag=CTag::GetInstance();
			ins_tag->add_tag_using_resattrmgr(wstrPath,Tags, TRUE);
		}
	}

	return res;
}
typedef ACEX1 ASTArraySize (ACEX2* _ASFileWrite)(ASFile aFile, const char* p, ASTArraySize count);
_ASFileWrite next_ASFileWrite=NULL;
ACEX1 ASTArraySize ACEX2 myASFileWrite(ASFile aFile, const char* p, ASTArraySize count)
{
	ASTArraySize res=next_ASFileWrite(/*ASFile */aFile, /*const char* */p, /*ASTArraySize */count);
	if (res)
	{
        const char* file = NULL;
        // In portfolia, ASFileGetURL maybe crash.
        try
        {
		    file = ASFileGetURL(aFile);
        }
        catch (...)
        {
        }

		if (file)
		{
			//OutputDebugStringA("in myASFileWrite\n");
			//OutputDebugStringA(file);
		}

	}
	return res;
}
typedef ACEX1 ASBool (ACEX2* _AVAppBeginSave)(AVOpenSaveDialogParams dialogParams, ASFileSys* outFileSys, ASPathName* outASPathNameToWrite, ASInt16* outChosenFilterIndex, ASPathName* outASPathNameChosen, AVAppFileSaveHandle* outFileSaveHandle);
_AVAppBeginSave next_AVAppBeginSave = NULL;
ACEX1 ASBool ACEX2 myAVAppBeginSave(AVOpenSaveDialogParams dialogParams, ASFileSys* outFileSys, ASPathName* outASPathNameToWrite, ASInt16* outChosenFilterIndex, ASPathName* outASPathNameChosen, AVAppFileSaveHandle* outFileSaveHandle)
{
	//只有在export portfolio的内容到别的文件的时候才会被调用
	//一般的save as不会被调用
	//但是当export文件夹的时候，beginsave和endsave不会被调用
	//sharepoint上的文件save as到本地的时候会被调用
	//reader 9 也会走到这里！！
	CELOG_LOG(CELOG_DEBUG, L"in myAVAppBeginSave\n");

	string current_pdf;
	getCurrentPDFPath(current_pdf);

	ASBool res = next_AVAppBeginSave(dialogParams, outFileSys,  outASPathNameToWrite, outChosenFilterIndex, outASPathNameChosen, outFileSaveHandle);
	
	const char* char_test = ASTextGetEncoded(dialogParams->windowTitle, NULL);
	if (char_test != NULL && boost::algorithm::equals(char_test, "Extract File"))
	{
		return res;
	}

	if(res)
	{
		string strDestPath = GetPath(*outFileSys, *outASPathNameToWrite);

		if (!strDestPath.empty())
		{
			CELOG_LOGA(CELOG_DEBUG, "output file path is: %s\n", strDestPath.c_str());

			if(true==boost::algorithm::iends_with(strDestPath,".pdf"))
			{
				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();

				vector<pair<wstring,wstring>> dest_tags;
				bool bDestEncrypted=false;
				PDDoc pddoc=NULL;
				getCurrentPDDoc(pddoc);
				ins_policy->QueryCopy_Get_Obligation_Inheritance(current_pdf.c_str(),strDestPath.c_str(),bdeny,dest_tags,bDestEncrypted,pddoc);

				if (bdeny)
				{
					return false;
				}

				//看看要不要对目标文件加密，如果要，并且目标文件不在本地，因为SE不可能支持对不在本地的文件加密，所以，我们要block这个COPY
				if (bDestEncrypted)
				{
					//看看目标文件的地址
					//目标文件不在本地，因为SE不可能支持对不在本地的文件加密，所以，我们要block这个COPY
					string dest(strDestPath);
					std::wstring wdest=MyMultipleByteToWideChar(dest);
					if (bReaderXProtectedMode || false==IsLocalPath(wdest.c_str()))
					{
						//这个不是本地文件,BLOCK
						CELOG_LOG(CELOG_DEBUG, L"myAVAppBeginSave, block save as, since adobepep can't do encryption for this case. (protected mode or dest is remote path)\n");
						AVAlertNote("This action is not permitted. Please use Windows explorer to perform the action.");
						
						return false;
					}

					DWORD dwUnused=0;
					CDoEncryObligation::DoMarkEncrypt(dwUnused,wdest);
				}

#if ACRO_SDK_LEVEL==0x000A0000	
#else//reader
				//Reader 9 doesn't need to close the file, since Reader 9 won't open the newly saved file.
				if (9 != MajorVersion && !dest_tags.empty() && MessageBoxW(GetForegroundWindow(), WARNING_CLOSEFILE, L"Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
				{
					
					return false;
				}

#endif
				//然后，设置要做tag的标志，并且保存tag的值
				CDoTag_SaveAs* ins_dotag_saveas=CDoTag_SaveAs::GetInstance();
				ins_dotag_saveas->setFlagAndObligationTags(dest_tags);
				ins_dotag_saveas->SetFilePath(strDestPath);
				ins_dotag_saveas->SetType("endsave");

				
			}
			else//the case "select content->right click->export selection as
			{
				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();
				ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"CONVERT");
				if (bdeny)
				{
					CELOG_LOGA(CELOG_DEBUG, "explort selection as, path: %s\n", current_pdf.c_str());
					return false;
				}
			}
			
		}
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"user cancel in myAVAppBeginSave\n");
	}


	return res;
}
typedef ACEX1 ASTArraySize (ACEX2* _ASFileRead)(ASFile aFile, char* p, ASTArraySize count);
_ASFileRead next_ASFileRead=NULL;
ACEX1 ASTArraySize ACEX2 myASFileRead(ASFile aFile, char* p, ASTArraySize count)
{
	ASTArraySize res=next_ASFileRead(/*ASFile */aFile, /*char* */p, /*ASTArraySize */count);
	
	return res;
}
typedef ACEX1 ASBool (ACEX2* _AVAppEndSave)(AVAppFileSaveHandle inFileSaveHandle, ASPathName inASPathNameWritten);
_AVAppEndSave next_AVAppEndSave=NULL;
ACEX1 ASBool ACEX2 myAVAppEndSave(AVAppFileSaveHandle inFileSaveHandle, ASPathName inASPathNameWritten)
{
	//在export portfolio的内容到别的文件的时候会被调用，调用到这里的时候，可以打tag了，当然也可以在这里做encryption
	//但是当export文件夹的时候，beginsave和endsave不会被调用
	CELOG_LOG(CELOG_DEBUG, L"in myAVAppEndSave\n");
	
//sync custom properties to gold tag, here only handle Reader 9
#if ACRO_SDK_LEVEL==0x000A0000	
#else//reader
	if (9 == MajorVersion )
	{
		string src;
		getCurrentPDFPath(src);
		wstring wsrc = MyMultipleByteToWideChar(src);

		//the file name of save as
		string dest = CDoTag_SaveAs::GetInstance()->GetFilePath();
		wstring wdest = MyMultipleByteToWideChar(dest);

		TAGS tags;
		CTag::GetInstance()->get_cachednativetag(wsrc, tags);
		CELOG_LOG(CELOG_DEBUG, L"get cached native tags in myAVAppEndSave (for Reader 9), path: %s, tag count: %d\n", wsrc.c_str(), tags.size());

		if (!tags.empty() && emIsEncrytFile==CEncrypt::Encrypt(wdest,false,true))
		{
			CELOG_LOGA(CELOG_DEBUG, "(Reader 9)sync custom properties to gold tags for local-se file, src: %s, dest: %s\n", src.c_str(), dest.c_str());
			CTag::GetInstance()->add_tag_using_resattrmgr(wdest, tags);
		}
	}
#endif


	CDoTag_SaveAs* ins_dotag_saveas=CDoTag_SaveAs::GetInstance();
	if (ins_dotag_saveas->getFlag())
	{
		ins_dotag_saveas->execute_avappendsave();
	}

	return next_AVAppEndSave(/*AVAppFileSaveHandle */inFileSaveHandle, /*ASPathName */inASPathNameWritten);
}
typedef ACEX1 void (ACEX2* _PDFileAttachmentSaveToFile)(PDFileAttachment attachment, ASFile destFile);
_PDFileAttachmentSaveToFile next_PDFileAttachmentSaveToFile=NULL;
ACEX1 void ACEX2 myPDFileAttachmentSaveToFile(PDFileAttachment attachment, ASFile destFile)
{
	//export portfolio里的文件，文件夹都会走到这里，所以在这里做evaluation，虽然export文件的时候beginsave也会对做copy的evaluation，
	//但是我可以对evaluation做缓存，这样就不会做两次evaluation了，也就不会有两次obligation了
	CELOG_LOG(CELOG_DEBUG, L"in myPDFileAttachmentSaveToFile\n");

	//输出attachment的名字	
	get_attachment_name(attachment);

	ASFileSys sys = ASFileGetFileSys(destFile);
	ASPathName pathname = ASFileAcquirePathName(destFile);
	char* destFile_path = ASFileSysDisplayStringFromPath(sys, pathname);
	ASFileSysReleasePath(sys, pathname);
	if (destFile_path)
	{
		//根据经验总结，如果是临时路径，那么不是用户想要export，所以这里要判断路径
		char buffer[1024];
		GetTempPathA(1024, buffer);
		if (!strstr(destFile_path, buffer))
		{
			//这不是临时路径
			string currentpdf;
			getCurrentPDFPath(currentpdf);

			CELOG_LOGA(CELOG_DEBUG, "dest File path is: %s\n", destFile_path);

			CELOG_LOG(CELOG_DEBUG, L"this is save attachment to local or something like case\n");

			bool bdeny=false;
			CPolicy* ins=CPolicy::GetInstance();
			nextlabs::Obligations obs;
			ins->query("COPY",currentpdf,destFile_path,bdeny,obs);

			if (bdeny)
			{
				{
					boost_unique_lock writeLock(g_mNeedDeletedAttachmentFile);
					gNeedDeletedAttachmentFile = destFile_path;
				}

				ASfree(destFile_path);

				return;
			}
		}
		ASfree(destFile_path);
	}

	//如果要deny，直接返回，而不调用下面的代码，是可以deny的
	return next_PDFileAttachmentSaveToFile(/*PDFileAttachment */attachment, /*ASFile */destFile);
}

typedef AVConversionStatus (* _AVConversionConvertStreamToPDF)(AVConversionFlags flags, const char* mimeType, ASStm stream, ASCab metaData, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertStreamToPDF next_AVConversionConvertStreamToPDF=NULL;
AVConversionStatus myAVConversionConvertStreamToPDF(AVConversionFlags flags, const char* mimeType, ASStm stream, ASCab metaData, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertStreamToPDF\n");
	return next_AVConversionConvertStreamToPDF(/*AVConversionFlags */flags, /*const char* */mimeType, /*ASStm */stream, /*ASCab */metaData, /*PDDoc* */outPDDoc, /*AVStatusMonitorProcs */statusMonitor);
}

typedef AVConversionStatus (* _AVConversionConvertStreamToPDFWithHandler)(AVConversionToPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, ASStm stream, ASCab metaData, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertStreamToPDFWithHandler next_AVConversionConvertStreamToPDFWithHandler=NULL;
AVConversionStatus myAVConversionConvertStreamToPDFWithHandler(AVConversionToPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, ASStm stream, ASCab metaData, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertStreamToPDFWithHandler\n");
	return next_AVConversionConvertStreamToPDFWithHandler(/*AVConversionToPDFHandler */inHandler, /*ASCab */inSettings, /*AVConversionFlags */flags, /*ASStm */stream, /*ASCab */metaData, /*PDDoc* */outPDDoc, /*AVStatusMonitorProcs */statusMonitor);
}
typedef AVConversionStatus (* _AVConversionConvertToPDF)(AVConversionFlags flags, ASPathName inPath, ASFileSys inFileSys, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertToPDF next_AVConversionConvertToPDF=NULL;
AVConversionStatus myAVConversionConvertToPDF(AVConversionFlags flags, ASPathName inPath, ASFileSys inFileSys, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor)
{
	//观察到，这个函数会嵌套调用AVConversionConvertToPDFWithHandler函数，所以到目前的观察为止，没有发现这个函数的利用价值
	CELOG_LOG(CELOG_DEBUG, L"in AVConversionConvertToPDF\n");


	AVConversionStatus res=next_AVConversionConvertToPDF(/*AVConversionFlags*/ flags, /*ASPathName */inPath, /*ASFileSys */inFileSys, /*PDDoc* */outPDDoc, /*AVStatusMonitorProcs */statusMonitor);

	CELOG_LOG(CELOG_DEBUG, L"out of next_AVConversionConvertToPDF\n");

	string outfile;
	GetPathfromPDDoc(*outPDDoc, outfile);

	return res;
}
typedef AVConversionStatus (* _AVConversionConvertToPDFWithHandler)(AVConversionToPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, ASPathName inPath, ASFileSys inFileSys, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertToPDFWithHandler next_AVConversionConvertToPDFWithHandler=NULL;
AVConversionStatus myAVConversionConvertToPDFWithHandler(AVConversionToPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, ASPathName inPath, ASFileSys inFileSys, PDDoc* outPDDoc, AVStatusMonitorProcs statusMonitor)
{
	//这个函数会被调用--当从别的类型的文件转换成pdf时
	//比如，merge多个文件以创建新的pdf
	//但是从pdf创建新的pdf不会调用这个函数
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertToPDFWithHandler\n");
	
	char* in_path = ASFileSysDisplayStringFromPath(inFileSys, inPath);
	if (in_path)
	{
		//this is source file
		CELOG_LOGA(CELOG_DEBUG, "this is input path: %s\n", in_path);

		//这是在想要把这个文件convert成pdf文件，
		//我们的PRD要求现在很简单，看看允许不允许convert，deny的话就不执行下去了
		bool bdeny=false;
		CPolicy* ins_policy=CPolicy::GetInstance();
		ins_policy->queryLocalSourceAndDoTagObligation(in_path,bdeny,"CONVERT");
		ASfree(in_path);

		if (bdeny)
		{
			CELOG_LOG(CELOG_DEBUG, L"deny convert!!!!\n");
			return kAVConversionFailed;
		}

		
	}


	AVConversionStatus res = next_AVConversionConvertToPDFWithHandler(/*AVConversionToPDFHandler */inHandler, /*ASCab */inSettings, /*AVConversionFlags */flags, /*ASPathName */inPath, /*ASFileSys */inFileSys, /*PDDoc* */outPDDoc, /*AVStatusMonitorProcs */statusMonitor);

	CELOG_LOG(CELOG_DEBUG, L"out of next_AVConversionConvertToPDFWithHandler\n");

	string outfile;
	GetPathfromPDDoc(*outPDDoc, outfile);

	return res;
}


typedef ASBool (* _AVDocDoSaveAsWithParams)(AVDoc doc, AVDocSaveParams params);
_AVDocDoSaveAsWithParams next_AVDocDoSaveAsWithParams=NULL;
ASBool myAVDocDoSaveAsWithParams(AVDoc doc, AVDocSaveParams params)
{
	//以pdf创建pdf的时候，这个函数不会被调用
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocDoSaveAsWithParams\n");

	ASBool res = next_AVDocDoSaveAsWithParams(doc,params);

	CELOG_LOG(CELOG_DEBUG, L"out myAVDocDoSaveAsWithParams\n");

	return res;
}
typedef ACEX1 void (ACEX2* _CosDocSaveToFile)(CosDoc cosDoc, ASFile asFile, CosDocSaveFlags saveFlags, CosDocSaveParams saveParams);
_CosDocSaveToFile next_CosDocSaveToFile=NULL;
ACEX1 void ACEX2 myCosDocSaveToFile(CosDoc cosDoc, ASFile asFile, CosDocSaveFlags saveFlags, CosDocSaveParams saveParams)
{
	//到目前没发现这个函数被调用过
	CELOG_LOG(CELOG_DEBUG, L"in myCosDocSaveToFile\n");
	next_CosDocSaveToFile(/*CosDoc*/ cosDoc, /*ASFile*/ asFile, /*CosDocSaveFlags*/ saveFlags, /*CosDocSaveParams*/ saveParams);
}
typedef ACEX1 void (ACEX2* _CosDocSaveWithParams)(CosDoc cosDoc, ASFile asFile, CosDocSaveFlags saveFlags, CosDocSaveParams saveParams);
_CosDocSaveWithParams next_CosDocSaveWithParams=NULL;
ACEX1 void ACEX2 myCosDocSaveWithParams(CosDoc cosDoc, ASFile asFile, CosDocSaveFlags saveFlags, CosDocSaveParams saveParams)
{
	//到目前没发现这个函数被调用过
	CELOG_LOG(CELOG_DEBUG, L"in myCosDocSaveWithParams\n");
	next_CosDocSaveWithParams(/*CosDoc*/ cosDoc, /*ASFile*/ asFile, /*CosDocSaveFlags*/ saveFlags, /*CosDocSaveParams*/ saveParams);
}
typedef AVConversionStatus (* _AVConversionConvertFromPDFWithHandler)(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, PDDoc inPDDoc, ASPathName outPath, ASFileSys outFileSys, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertFromPDFWithHandler next_AVConversionConvertFromPDFWithHandler=NULL;
AVConversionStatus myAVConversionConvertFromPDFWithHandler(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, PDDoc inPDDoc, ASPathName outPath, ASFileSys outFileSys, AVStatusMonitorProcs statusMonitor)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertFromPDFWithHandler\n");
	return next_AVConversionConvertFromPDFWithHandler(/*AVConversionFromPDFHandler*/ inHandler, /*ASCab*/ inSettings, /*AVConversionFlags*/ flags, /*PDDoc*/ inPDDoc, /*ASPathName*/ outPath, /*ASFileSys*/ outFileSys, /*AVStatusMonitorProcs*/ statusMonitor);
}
typedef AVConversionStatus (* _AVConversionConvertStreamFromPDFWithHandler)(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, PDDoc inPDDoc, ASStm stream, ASCab metaData, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertStreamFromPDFWithHandler next_AVConversionConvertStreamFromPDFWithHandler=NULL;
AVConversionStatus myAVConversionConvertStreamFromPDFWithHandler(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, PDDoc inPDDoc, ASStm stream, ASCab metaData, AVStatusMonitorProcs statusMonitor)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertStreamFromPDFWithHandler\n");
	return next_AVConversionConvertStreamFromPDFWithHandler(/*AVConversionFromPDFHandler*/ inHandler, /*ASCab */inSettings, /*AVConversionFlags */flags, /*PDDoc */inPDDoc, /*ASStm */stream, /*ASCab */metaData, /*AVStatusMonitorProcs */statusMonitor);
}

typedef AVConversionStatus (* _AVConversionConvertStreamFromStructNodeWithHandler)(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, AVStructNode inStructNode, ASStm stream, ASCab metaData, AVStatusMonitorProcs statusMonitor);
_AVConversionConvertStreamFromStructNodeWithHandler next_AVConversionConvertStreamFromStructNodeWithHandler=NULL;
AVConversionStatus myAVConversionConvertStreamFromStructNodeWithHandler(AVConversionFromPDFHandler inHandler, ASCab inSettings, AVConversionFlags flags, AVStructNode inStructNode, ASStm stream, ASCab metaData, AVStatusMonitorProcs statusMonitor)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVConversionConvertStreamFromStructNodeWithHandler\n");
	return next_AVConversionConvertStreamFromStructNodeWithHandler(/*AVConversionFromPDFHandler */inHandler, /*ASCab */inSettings, /*AVConversionFlags */flags, /*AVStructNode */inStructNode, /*ASStm */stream, /*ASCab */metaData, /*AVStatusMonitorProcs */statusMonitor);
}
typedef ACEX1 ASErrorCode (ACEX2* _ASFileSysOpenFile64)(ASFileSys fileSys, ASPathName pathName, ASFileMode mode, ASFile* fP);
_ASFileSysOpenFile64 next_ASFileSysOpenFile64=NULL;
ACEX1 ASErrorCode ACEX2 myASFileSysOpenFile64(ASFileSys fileSys, ASPathName pathName, ASFileMode mode, ASFile* fP)
{
	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();


	string strPath = GetPath(fileSys, pathName);

	if (!strPath.empty())
	{
		CELOG_LOGA(CELOG_DEBUG, "in myASFileSysOpenFile64:%s\n", strPath.c_str());
		
		//做OPEN的evaluation，这是最基本的。
		//我们要在OPEN文件之前对其打tag-如果有打tag的obligation的话。这是因为一旦文件被OPEN了之后就只能在adobe释放掉handle后才能对其打tag，而这会引发一些问题，
		//所以我们要在open之前做evaluation，而这时肯定没有PDDoc，所以不能用PDDocGetInfo，而要用tag library。
		if (true==IsURLPath(strPath))
		{
			ins_policy->queryHttpSource(strPath.c_str(),NULL,bdeny,"OPEN",true);
		}
		else
		{
			boost_unique_lock writeLock(g_mbeAttachment);
			if( 0 != _stricmp(gbeAttachment.c_str(), strPath.c_str()))
			{
				ins_policy->queryLocalSourceAndDoTagObligation(strPath.c_str(),bdeny,"OPEN");
			}
			else
			{
				gbeAttachment.clear();
			}
		}

		
		if (bdeny==true)
		{
			return -1;
		}
	}
	
	ASErrorCode  res = next_ASFileSysOpenFile64(/*ASFileSys */fileSys, /*ASPathName */pathName, /*ASFileMode */mode, /*ASFile* */fP);

	//For bug 18425
	if ( bInIExplore && 0 == res && !strPath.empty() )
	{
		if ( 0 == _strnicmp ( TempLowPath, strPath.c_str(), TempLowPathLength ) && boost::algorithm::iends_with ( strPath.c_str(), ".tmp" ) )
		{
			string current_pdf;
			getCurrentPDFPath(current_pdf);

			wstring wstrPath = MyMultipleByteToWideChar(strPath);
			wstring wcurrent_pdf = MyMultipleByteToWideChar(current_pdf);

			vector<pair<wstring,wstring>> Tags;
			Tags.push_back ( pair<wstring,wstring> ( SaveAsInExploreTagName, wcurrent_pdf ) );

			CTag* ins_tag=CTag::GetInstance();
			ins_tag->add_tag_using_resattrmgr(wstrPath,Tags, TRUE);
		}
	}

	return res;
}
typedef ACEX1 void (ACEX2* _AVWindowShow)(AVWindow win);
_AVWindowShow next_AVWindowShow=NULL;
ACEX1 void ACEX2 myAVWindowShow(AVWindow win)
{
	//OutputDebugStringA("in myAVWindowShow\n");

	
	if (win)
	{
		ASText text=ASTextNew();
		AVWindowGetTitle(win,text);

		if (text)
		{
			const char* title=ASTextGetEncoded(text,NULL);
			if (title)
			{
				CELOG_LOGA(CELOG_DEBUG, "in myAVWindowShow title is: %s\n", title);
			}
			else
			{
				//OutputDebugStringA("fail to get title\n");
			}
		}
	}
	next_AVWindowShow(win);
}
typedef ACEX1 ASBool (ACEX2* _AVWindowDoModal)(AVWindow window);
_AVWindowDoModal next_AVWindowDoModal=NULL;
ACEX1 ASBool ACEX2 myAVWindowDoModal(AVWindow window)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVWindowDoModal\n");
	return next_AVWindowDoModal(window);
}
typedef ACEX1 AVCommandStatus (ACEX2* _AVCommandExecute)(AVCommand cmd);
_AVCommandExecute next_AVCommandExecute=NULL;
ACEX1 AVCommandStatus ACEX2 myAVCommandExecute(AVCommand cmd)
{
//	OutputDebugStringA("in myAVCommandExecute\n");
	return next_AVCommandExecute(cmd);
}
typedef ACEX1 AVCommand (ACEX2* _AVCommandNew)(ASAtom name);
_AVCommandNew next_AVCommandNew=NULL;
ACEX1 AVCommand ACEX2 myAVCommandNew(ASAtom name)
{
//	OutputDebugStringA("in myAVCommandNew\n");
	const char* ch_name=ASAtomGetString(name);
	if (ch_name)
	{
		CELOG_LOGA(CELOG_DEBUG, "myAVCommandNew: %s\n", ch_name);
	}
	return next_AVCommandNew(name);
}
typedef ACEX1 AVCommandStatus (ACEX2* _AVCommandWork)(AVCommand cmd);
_AVCommandWork next_AVCommandWork=NULL;
ACEX1 AVCommandStatus ACEX2 myAVCommandWork(AVCommand cmd)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVCommandWork\n");


	CAction* ins = CAction::GetInstance();

	ASAtom name=AVCommandGetName(cmd);
	const char* ch_name=ASAtomGetString(name);
	if (ch_name)
	{
		CELOG_LOGA(CELOG_DEBUG, "myAVCommandWork: %s\n", ch_name);

		if (strcmp(ch_name,"InsertPages") == 0)
		{
			//用户想要insert pages，这是我们处理的第一步
			ins->SetAction(INSERT_PAGES);
		}
	}

	AVCommandStatus res=next_AVCommandWork(cmd);
	CELOG_LOG(CELOG_DEBUG, L"out next_AVCommandWork\n");

	//完成了，重置action
	ins->reset();

	return res;
}
typedef ASBool (* _AVDocDoSaveAs)(AVDoc doc);
_AVDocDoSaveAs next_AVDocDoSaveAs =NULL;
ASBool myAVDocDoSaveAs(AVDoc doc)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocDoSaveAs\n");

	g_BeSavingAs = TRUE;

	ASBool res=next_AVDocDoSaveAs(doc);

	g_BeSavingAs = FALSE;

	CELOG_LOG(CELOG_DEBUG, L"out next_AVDocDoSaveAs\n");

	return res;
}
typedef ACEX1 PDPage (ACEX2* _PDDocAcquirePage)(PDDoc doc, ASInt32 pageNum);
_PDDocAcquirePage next_PDDocAcquirePage=NULL;
ACEX1 PDPage ACEX2 myPDDocAcquirePage(PDDoc doc, ASInt32 pageNum)
{
	//OutputDebugStringA("in myPDDocAcquirePage\n");
	return next_PDDocAcquirePage(doc,pageNum);
}
typedef ACEX1 void (ACEX2* _PDDocCopyToFile)(PDDoc pdDoc, PDDocCopyParams params);
_PDDocCopyToFile next_PDDocCopyToFile=NULL;
ACEX1 void ACEX2 myPDDocCopyToFile(PDDoc pdDoc, PDDocCopyParams params)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocCopyToFile\n");
	return next_PDDocCopyToFile(pdDoc,params);
}
typedef PDDoc (* _PDDocCreate)();
_PDDocCreate next_PDDocCreate=NULL;
PDDoc myPDDocCreate()
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocCreate\n");
	PDDoc res= next_PDDocCreate();
	CELOG_LOG(CELOG_DEBUG, L"out next_PDDocCreate\n");

	string outfile;
	GetPathfromPDDoc(res, outfile);
	return res;
}
typedef PDPage (* _PDDocCreatePage)(PDDoc doc, ASInt32 afterPageNum, ASFixedRect mediaBox);
_PDDocCreatePage next_PDDocCreatePage=NULL;
PDPage myPDDocCreatePage(PDDoc doc, ASInt32 afterPageNum, ASFixedRect mediaBox)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocCreatePage\n");
	PDPage page= next_PDDocCreatePage(/*PDDoc*/ doc, /*ASInt32*/ afterPageNum, /*ASFixedRect*/ mediaBox);
	CELOG_LOG(CELOG_DEBUG, L"out next_PDDocCreatePage\n");

	string outfile;
	GetPathfromPDDoc(doc, outfile);

	return page;
}
typedef void (* _PDDocInsertPages)(PDDoc doc, ASInt32 mergeAfterThisPage, PDDoc doc2, ASInt32 startPage, ASInt32 numPages, ASUns16 insertFlags, ProgressMonitor progMon, void* progMonClientData, CancelProc cancelProc, void* cancelProcClientData);
_PDDocInsertPages next_PDDocInsertPages=NULL;
void myPDDocInsertPages(PDDoc doc, ASInt32 mergeAfterThisPage, PDDoc doc2, ASInt32 startPage, ASInt32 numPages, ASUns16 insertFlags, ProgressMonitor progMon, void* progMonClientData, CancelProc cancelProc, void* cancelProcClientData)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocInsertPages\n");

	//这是我们处理的第二步，找到被insert的文件和host文件
	CELOG_LOG(CELOG_DEBUG, L"host file is:\n");
	string file;
	GetPathfromPDDoc(doc, file);
	CELOG_LOG(CELOG_DEBUG, L"source file is:\n");
	string file2;
	GetPathfromPDDoc(doc2,file2);

	//新的代码，2012/7/17
	//新的PRD非常简单，
	//这里就是要把file2 CONVERT
	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();
	ins_policy->queryLocalSourceAndDoTagObligation(file2.c_str(),bdeny,"CONVERT",doc2);
	if (bdeny)
	{
		CELOG_LOG(CELOG_DEBUG, L"deny convert!!!!\n");
		return;
	}
	else
	{
		//2012/7/17
		//新的PRD非常简单，这里就是要把file2 CONVERT，这里允许CONVERT,我们直接返回，不做下面的检查了，下面的是老的PRD，是为了COPY用的
		next_PDDocInsertPages(/*PDDoc */doc, /*ASInt32 */mergeAfterThisPage, /*PDDoc */doc2, /*ASInt32 */startPage, /*ASInt32 */numPages, /*ASUns16 */insertFlags, /*ProgressMonitor */progMon, /*void* */progMonClientData, /*CancelProc */cancelProc, /*void* */cancelProcClientData);

		CELOG_LOG(CELOG_DEBUG, L"out next_PDDocInsertPages\n");

		return;
	}


}
typedef ASErrorCode (* _ASFileClose)(ASFile aFile);
_ASFileClose next_ASFileClose=NULL;
ASErrorCode real_asfileclose(ASFile handle)
{
	ASErrorCode res = 0;

	__try
	{
		res = next_ASFileClose(handle);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}

	return res;
}
ASFileSys myASFileGetFileSys(ASFile handle, bool& bException)
{
	ASFileSys sys = NULL;
	__try
	{
		sys = ASFileGetFileSys(handle);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bException = true;
	}
	return sys;
}

ASErrorCode myASFileClose(ASFile aFile)
{
	//Every time save the ASFile in global variable
	{
		boost_unique_lock writeLock(g_mtheLastClosedASFile);
		gtheLastClosedASFile = aFile;
	}

	string filepath;
	ASFileSys sys = NULL;
	ASPathName pathname = NULL;
	if (aFile)
	{
		bool bException = false;
		sys = myASFileGetFileSys(aFile, bException);

		if (!bException)
		{
			pathname = ASFileAcquirePathName(aFile);
			if (sys&&pathname)
			{
				char* out_path = ASFileSysDisplayStringFromPath(sys, pathname);
				if (out_path)
				{
					filepath =(string)out_path;

					ASfree(out_path);
				}
			}
		}
	}
	wstring wfilepath = MyMultipleByteToWideChar(filepath);
	
	CELOG_LOGA(CELOG_DEBUG, "myASFileClose: %s\n", filepath.c_str());
	RemoveOpenedFile(filepath);

	//try to get the tags of the current file. if the current file is local SE, we need to sync the custom properies to gold tag.
	TAGS tags;
	CTag::GetInstance()->get_cachednativetag(wfilepath, tags);
	
	CELOG_LOG(CELOG_DEBUG, L"Get native tags in cache, path: %s, tag count: %d\n", wfilepath.c_str(), tags.size());
	

	//先把handle close掉，在这个后面，我们才能打tag
	ASErrorCode res= real_asfileclose(aFile);
	if (sys&&pathname)
	{
		ASFileSysReleasePath(sys, pathname);
	}

	//sync the custom properties to gold tag for local SE files.
	if (!tags.empty() && emIsEncrytFile==CEncrypt::Encrypt(wfilepath,false,true))
	{
		CELOG_LOG(CELOG_DEBUG, L"(Reader X OR acorbat )Sync custom properties to gold tag for local-se files, path: %s, tag number: %d\n", wfilepath.c_str(), tags.size());


		CTag::GetInstance()->add_tag_using_resattrmgr(wfilepath, tags);
	}

	CPDDocInsertPages* ins_insertpage=CPDDocInsertPages::GetInstance();
	ins_insertpage->execute_asfileclose(filepath);

	//看看EDIT action是不是需要做啥
	CEdit* ins_edit=CEdit::GetInstance();
	ins_edit->execute_asfileclose(filepath);


	//如果这个文件是要被打tag的文件，那就要告诉想要知道的人，这个文件已经被adobe close了
	CDoTag_SaveAs* ins_saveas=CDoTag_SaveAs::GetInstance();
	if(ins_saveas->getFlag())
	{
		if (ins_saveas->GetFilePath()==filepath)
		{
#if ACRO_SDK_LEVEL==0x000A0000	
#else
			if ( 10 == MajorVersion || 11 == MajorVersion)
			{
				CELOG_LOGA(CELOG_DEBUG, "do tag in myASFileClose for: %s\n", filepath.c_str());
				ins_saveas->DoTag_TagLib_2(filepath);

				ins_saveas->reset();
			}
#endif
			ins_saveas->set_closed_flag();
		}
	}

	CTag::GetInstance()->remove_cachedtag(wfilepath);
	CTag::GetInstance()->remove_cachednativetag(wfilepath);
	
	{
		boost_unique_lock writeLock(g_mNeedDeletedAttachmentFile);
		if (!gNeedDeletedAttachmentFile.empty())
		{
			if (gNeedDeletedAttachmentFile == filepath)
			{
				DeleteFileA(filepath.c_str());
				gNeedDeletedAttachmentFile.clear();
			}
		}
	}

	return res;
}


typedef AVDoc (* _AVDocOpenFromPDDocWithParams)(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams params);
_AVDocOpenFromPDDocWithParams next_AVDocOpenFromPDDocWithParams=NULL;
AVDoc myAVDocOpenFromPDDocWithParams(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams params)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocOpenFromPDDocWithParams\n");

	string path;
	GetPathfromPDDoc(pdDoc,path);

	if (path.length())
	{
		CPDDocInsertPages* ins_pddocinsertpages=CPDDocInsertPages::GetInstance();
		ins_pddocinsertpages->execute_AVDocOpenFromPDDocWithParams(path);
	}

	CPolicy* ins_policy=CPolicy::GetInstance();
	bool bdeny=false;
	ins_policy->queryHttpSource(path.c_str(),pdDoc,bdeny,"OPEN",false,true);



	CELOG_LOGA(CELOG_DEBUG, "myAVDocOpenFromPDDocWithParams: %s \n", path.c_str());

	if (!path.empty())
	{
		boost_unique_lock writeLock(g_mAttachMail);
		g_ActivePath.m_strPath = path;
		g_ActivePath.m_tags.clear();
		CTag::GetInstance()->read_tag(MyMultipleByteToWideChar(path), g_ActivePath.m_tags, pdDoc);
	}

	if (true==bdeny)
	{
		return NULL;
	}

	AddOpenedFile(path);

	return next_AVDocOpenFromPDDocWithParams(pdDoc, tempTitle, params);
}
typedef ASInt32 (* _PDDocGetFlags)(PDDoc doc);
_PDDocGetFlags next_PDDocGetFlags=NULL;
ASInt32 myPDDocGetFlags(PDDoc doc)
{
	//为什么不在PDDocSetFlags里面做EDIT呢？因为发现并不是所有的修改，比如删除一个comment，的时候都会调用PDDocSetFlags

	ASInt32 flags=next_PDDocGetFlags(doc);

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
#else
	if (PDDocNeedsSave==(PDDocNeedsSave&flags)&&PDDocIsModified==(PDDocIsModified&flags))
	{
		string logs=str(boost::format("in myPDDocGetFlags, flags %x\n") % flags);
	//	OutputDebugStringA(logs.c_str());


		//this is EDIT
		string file;
		GetPathfromPDDoc(doc, file);

		if (!file.length())
		{
			
			CELOG_LOGA(CELOG_DEBUG, "we fail to get file path from GetPathfromPDDoc\n");
		}
		else
		{	
			CEdit* ins_edit=CEdit::GetInstance();

			CPolicy* ins_policy=CPolicy::GetInstance();
			bool bdeny=false;
			vector<pair<wstring,wstring>> w_obligation_tags;
			bool bDest_Encrypt=false;
			nextlabs::Obligations obs;


			//如果这个文件有obs，说明这个文件已经做过了query，并且，还没有打过tag，也就是文件还没有关闭，
			//因为文件关闭的时候，会把这个文件的信息从CEdit里面移除掉
			if (false==ins_edit->get_obligation(file) && !ins_edit->isinvalid_obligaiton(file))
			{
				//找不到obs，说明这个文件没有做过query，或者做过query，但是之前已经关闭过了，这是第二次打开
				//所以我们可以查策略
				ins_policy->queryEdit_GetObligation(file.c_str(),bdeny,doc,w_obligation_tags,bDest_Encrypt,obs,true);
			}
			else
			{
				//找到了obs，说明这个文件已经query过了，并且还没有关闭过，
				//所以我们啥也不需要做，直接返回
				return flags;
			}
			
			if (bReaderXProtectedMode)
			{
				string filepath = file;
				std::transform ( filepath.begin(), filepath.end(), filepath.begin(), towlower );

				BOOL bfind=FALSE;
				{
					boost_share_lock readerLock(g_mFilesSEStatus);
					bfind=gFilesSEStatus[filepath];
				}

				if (bfind)
				{
				//	AVAlertNote("This action is not permitted. Please edit SE file in non protected mode.");
				
				CELOG_LOGA(CELOG_DEBUG, "always deny edit SE file if it is in reader X and protected mode\n");
				//OutputDebugStringA("always deny edit SE file if it is in reader X and protected mode\n");
				bdeny=true;
				}
			}


			if (true==bdeny)
			{
				//denied, don't return PDDocNeedsSave and PDDocIsModified flag
				ASInt32 myflags=(flags&(~PDDocNeedsSave));
				myflags=myflags&(~PDDocIsModified);
				logs=str(boost::format("myPDDocGetFlags block EDIT, return flags: %x, and we clear flag\n") % myflags);


				//clear PDDocNeedsSave and PDDocIsModified flag
				PDDocClearFlags(doc, PDDocNeedsSave|PDDocIsModified);

				return myflags;
			}

			//do obligation work
			ins_edit->do_tag_obligation(doc,file,w_obligation_tags);
			ins_edit->save_obligaiton(file,obs);
			if (bDest_Encrypt)
			{
				CELOG_LOGA(CELOG_DEBUG, "has SE obligation\n");
				ins_edit->do_se_obligation(file);
			}
		}
	}
#endif
	return flags;
}
typedef AVDoc (* _AVDocOpenFromPDDoc)(PDDoc doc, const ASText tempTitle);
_AVDocOpenFromPDDoc next_AVDocOpenFromPDDoc=NULL;
AVDoc myAVDocOpenFromPDDoc(PDDoc doc, const ASText tempTitle)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocOpenFromPDDoc\n");

	return next_AVDocOpenFromPDDoc(doc,tempTitle);
}
typedef AVDoc (* _AVDocOpenFromPDDocWithParamString)(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams p, const char* s);
_AVDocOpenFromPDDocWithParamString next_AVDocOpenFromPDDocWithParamString=NULL;
AVDoc myAVDocOpenFromPDDocWithParamString(PDDoc pdDoc, const ASText tempTitle, AVDocOpenParams p, const char* s)
{
	CELOG_LOG(CELOG_DEBUG, L"in next_AVDocOpenFromPDDocWithParamString\n");

	return next_AVDocOpenFromPDDocWithParamString(pdDoc,tempTitle,p,s);
}
typedef AVDoc (* _AVDocOpenFromFileWithParams)(ASPathName pathName, ASFileSys fileSys, const ASText tempTitle, AVDocOpenParams params);
_AVDocOpenFromFileWithParams next_AVDocOpenFromFileWithParams=NULL;
AVDoc myAVDocOpenFromFileWithParams(ASPathName pathName, ASFileSys fileSys, const ASText tempTitle, AVDocOpenParams params)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocOpenFromFileWithParams\n");

	AVDoc avdoc= next_AVDocOpenFromFileWithParams(/*ASPathName */pathName, /*ASFileSys */fileSys, /*const ASText */tempTitle, /*AVDocOpenParams */params);


	CELOG_LOG(CELOG_DEBUG, L"after next_AVDocOpenFromFileWithParams\n");


	PDDoc doc=AVDocGetPDDoc(avdoc);
	if (doc)
	{
		CPolicy* ins_policy=CPolicy::GetInstance();
		bool bdeny=false;

		string path;
		GetPathfromPDDoc(doc,path);

		if (path.length())
		{
			//For Acrobat.com Files
		#if ACRO_SDK_LEVEL==0x000A0000	//only available for acrobat
			if(IsAcrobatcom(path))
		#else
			if(IsAcrobatcom(path) || (17 == MajorVersion && boost::algorithm::iends_with(path, ".pdf") && !boost::algorithm::icontains(path, L"\\")))
		#endif
			{
				boost_unique_lock writeLock(g_mbeAcrobatcomFile);
				if( 0 == _stricmp(gbeAcrobatcomFile.c_str(),path.c_str()) )
				{
					AVDocClose(avdoc, false);
					gbeAcrobatcomFile.clear();
					return NULL;
				}
			}

			ins_policy->queryHttpSource(path.c_str(),doc,bdeny,"OPEN",false,true);

			if (true==bdeny)
			{
				CELOG_LOG(CELOG_DEBUG, L"call AVDocClose to close the denied file\n");

				AVDocClose(avdoc, false);
				return NULL;
			}
			
			AddOpenedFile(path);
		}
	}

	return avdoc;
}
void myhook()
{
	CELOG_LOG(CELOG_DEBUG, L"in myhook\n");
	CollectHooks();

	winapihook();
	HookCode((PVOID)(*(AVMenuAddMenuItem)), (PVOID)AdobeXI::CAdobeXITool::myAVMenuAddMenuItem, (PVOID*)&AdobeXI::CAdobeXITool::next_AVMenuAddMenuItem);

	if (11 == MajorVersion)
	{
		char* pAddress = AdobeXI::CAdobeXITool::CheckEmailFuncAddress();
		if (pAddress != NULL)
		{
			if (HookCode(pAddress, AdobeXI::CAdobeXITool::mySendMail, (PVOID*)&AdobeXI::CAdobeXITool::next_SendMail))
			{
				OutputDebugStringW(L"++++++++++++++++ get the right address and hook success.\n");
			}
		}

	}
	else if (17 == MajorVersion)
	{
		wchar_t szModule[512] = { 0 };
		GetModuleFileName(NULL, szModule, 512);

		wchar_t* pPos = wcsrchr(szModule, L'\\');
		if (pPos != NULL)
		{
			pPos[1] = L'\0';
			wcscat_s(szModule, L"plug_ins\\SendMail.api");

			HMODULE hLib = LoadLibraryW(szModule);
			if (hLib != NULL)
			{
#if ACRO_SDK_LEVEL==0x000A0000	//for acrobat
				char* pAddress = (char*)hLib + 0x5FC1B;
#else
				BYTE theBytes[12] = {0x55, 0x8b, 0xec, 0xff, 0x75, 0x0c, 0x6a, 0x00, 0xff, 0x75, 0x08, 0xe8};

				BYTE* start = (BYTE*)hLib;
				BYTE* pAddress = std::search(start, (BYTE*)hLib + 0x100000, theBytes, theBytes + 12);
#endif
				HookCode(pAddress, AdobeXI::CAdobeXITool::mySendMail, (PVOID*)&AdobeXI::CAdobeXITool::next_SendMail);
			}
		}
	}

	HookCode((PVOID)(*((PDFileAttachmentNewFromFileSELPROTO)(gPDModelHFT[PDFileAttachmentNewFromFileSEL]))), (PVOID)myPDFileAttachmentNewFromFile, (PVOID*)&next_PDFileAttachmentNewFromFile);
	HookCode((PVOID)(*((PDFileAttachmentFromCosObjSELPROTO)(gPDModelHFT[PDFileAttachmentFromCosObjSEL]))), (PVOID)myPDFileAttachmentFromCosObj, (PVOID*)&next_PDFileAttachmentFromCosObj);
	HookCode((PVOID)(*((PDFileAttachmentUpdateFromFileSELPROTO)(gPDModelHFT[PDFileAttachmentUpdateFromFileSEL]))), (PVOID)myPDFileAttachmentUpdateFromFile, (PVOID*)&next_PDFileAttachmentUpdateFromFile);
	HookCode((PVOID)(*((ASFileAttachmentGetPDFileAttachmentSELPROTO)(gPDModelHFT[ASFileAttachmentGetPDFileAttachmentSEL]))), (PVOID)myASFileAttachmentGetPDFileAttachment, (PVOID*)&next_ASFileAttachmentGetPDFileAttachment);
	HookCode((PVOID)(*((ASFileSysOpenFileSELPROTO)(gAcroSupportHFT[ASFileSysOpenFileSEL]))), (PVOID)myASFileSysOpenFile, (PVOID*)&next_ASFileSysOpenFile);	
	HookCode((PVOID)(*((ASFileWriteSELPROTO)(gAcroSupportHFT[ASFileWriteSEL]))), (PVOID)myASFileWrite, (PVOID*)&next_ASFileWrite);
	HookCode((PVOID)(*((AVAppBeginSaveSELPROTO)(gAcroViewHFT[AVAppBeginSaveSEL]))), (PVOID)myAVAppBeginSave, (PVOID*)&next_AVAppBeginSave);
	HookCode((PVOID)(*((ASFileReadSELPROTO)(gAcroSupportHFT[ASFileReadSEL]))), (PVOID)myASFileRead, (PVOID*)&next_ASFileRead);
	HookCode((PVOID)(*((AVAppEndSaveSELPROTO)(gAcroViewHFT[AVAppEndSaveSEL]))), (PVOID)myAVAppEndSave, (PVOID*)&next_AVAppEndSave);
	HookCode((PVOID)(*((PDFileAttachmentSaveToFileSELPROTO)(gPDModelHFT[PDFileAttachmentSaveToFileSEL]))), (PVOID)myPDFileAttachmentSaveToFile, (PVOID*)&next_PDFileAttachmentSaveToFile);
	HookCode((PVOID)(*((ASFileSysOpenFile64SELPROTO)(gAcroSupportHFT[ASFileSysOpenFile64SEL]))), (PVOID)myASFileSysOpenFile64, (PVOID*)&next_ASFileSysOpenFile64);	
	HookCode((PVOID)(*((AVWindowShowSELPROTO)(gAcroViewHFT[AVWindowShowSEL]))), (PVOID)myAVWindowShow, (PVOID*)&next_AVWindowShow);	
	HookCode((PVOID)(*((AVWindowDoModalSELPROTO)(gAcroViewHFT[AVWindowDoModalSEL]))), (PVOID)myAVWindowDoModal, (PVOID*)&next_AVWindowDoModal);	
	HookCode((PVOID)(*((AVCommandExecuteSELPROTO)(gAcroViewHFT[AVCommandExecuteSEL]))), (PVOID)myAVCommandExecute, (PVOID*)&next_AVCommandExecute);	
	HookCode((PVOID)(*((AVCommandNewSELPROTO)(gAcroViewHFT[AVCommandNewSEL]))), (PVOID)myAVCommandNew, (PVOID*)&next_AVCommandNew);	
	HookCode((PVOID)(*((AVCommandWorkSELPROTO)(gAcroViewHFT[AVCommandWorkSEL]))), (PVOID)myAVCommandWork, (PVOID*)&next_AVCommandWork);	
	HookCode((PVOID)(*((PDDocAcquirePageSELPROTO)(gPDModelHFT[PDDocAcquirePageSEL]))), (PVOID)myPDDocAcquirePage, (PVOID*)&next_PDDocAcquirePage);	
	HookCode((PVOID)(*((PDDocCopyToFileSELPROTO)(gPDModelHFT[PDDocCopyToFileSEL]))), (PVOID)myPDDocCopyToFile, (PVOID*)&next_PDDocCopyToFile);	
	HookCode((PVOID)(*((ASFileCloseSELPROTO)(gAcroSupportHFT[ASFileCloseSEL]))), (PVOID)myASFileClose, (PVOID*)&next_ASFileClose);	
	HookCode((PVOID)(*((AVDocOpenFromPDDocWithParamsSELPROTO)(gAcroViewHFT[AVDocOpenFromPDDocWithParamsSEL]))), (PVOID)myAVDocOpenFromPDDocWithParams, (PVOID*)&next_AVDocOpenFromPDDocWithParams);	
	HookCode((PVOID)(*((PDDocGetFlagsSELPROTO)(gPDModelHFT[PDDocGetFlagsSEL]))), (PVOID)myPDDocGetFlags, (PVOID*)&next_PDDocGetFlags);	
	HookCode((PVOID)(*((AVDocOpenFromPDDocSELPROTO)(gAcroViewHFT[AVDocOpenFromPDDocSEL]))), (PVOID)myAVDocOpenFromPDDoc, (PVOID*)&next_AVDocOpenFromPDDoc);	
	HookCode((PVOID)(*((AVDocOpenFromPDDocWithParamStringSELPROTO)(gAcroViewHFT[AVDocOpenFromPDDocWithParamStringSEL]))), (PVOID)myAVDocOpenFromPDDocWithParamString, (PVOID*)&next_AVDocOpenFromPDDocWithParamString);	
	HookCode((PVOID)(*((AVDocOpenFromFileWithParamsSELPROTO)(gAcroViewHFT[AVDocOpenFromFileWithParamsSEL]))), (PVOID)myAVDocOpenFromFileWithParams, (PVOID*)&next_AVDocOpenFromFileWithParams);	

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
	CELOG_LOGA(CELOG_DEBUG, "hook only for acrobat\n");
	HookCode((PVOID)(*((AVConversionConvertStreamToPDFSELPROTO)(gAcroViewHFT[AVConversionConvertStreamToPDFSEL]))), (PVOID)myAVConversionConvertStreamToPDF, (PVOID*)&next_AVConversionConvertStreamToPDF);
	HookCode((PVOID)(*((AVConversionConvertStreamToPDFWithHandlerSELPROTO)(gAcroViewHFT[AVConversionConvertStreamToPDFWithHandlerSEL]))), (PVOID)myAVConversionConvertStreamToPDFWithHandler, (PVOID*)&next_AVConversionConvertStreamToPDFWithHandler);
	HookCode((PVOID)(*((AVConversionConvertToPDFSELPROTO)(gAcroViewHFT[AVConversionConvertToPDFSEL]))), (PVOID)myAVConversionConvertToPDF, (PVOID*)&next_AVConversionConvertToPDF);
	HookCode((PVOID)(*((AVConversionConvertToPDFWithHandlerSELPROTO)(gAcroViewHFT[AVConversionConvertToPDFWithHandlerSEL]))), (PVOID)myAVConversionConvertToPDFWithHandler, (PVOID*)&next_AVConversionConvertToPDFWithHandler);
	HookCode((PVOID)(*((AVDocDoSaveAsWithParamsSELPROTO)(gAcroViewHFT[AVDocDoSaveAsWithParamsSEL]))), (PVOID)myAVDocDoSaveAsWithParams, (PVOID*)&next_AVDocDoSaveAsWithParams);
	HookCode((PVOID)(*((CosDocSaveToFileSELPROTO)(gCosHFT[CosDocSaveToFileSEL]))), (PVOID)myCosDocSaveToFile, (PVOID*)&next_CosDocSaveToFile);
	HookCode((PVOID)(*((CosDocSaveWithParamsSELPROTO)(gCosHFT[CosDocSaveWithParamsSEL]))), (PVOID)myCosDocSaveWithParams, (PVOID*)&next_CosDocSaveWithParams);
	HookCode((PVOID)(*((AVConversionConvertFromPDFWithHandlerSELPROTO)(gAcroViewHFT[AVConversionConvertFromPDFWithHandlerSEL]))), (PVOID)myAVConversionConvertFromPDFWithHandler, (PVOID*)&next_AVConversionConvertFromPDFWithHandler);
	HookCode((PVOID)(*((AVConversionConvertStreamFromPDFWithHandlerSELPROTO)(gAcroViewHFT[AVConversionConvertStreamFromPDFWithHandlerSEL]))), (PVOID)myAVConversionConvertStreamFromPDFWithHandler, (PVOID*)&next_AVConversionConvertStreamFromPDFWithHandler);
	HookCode((PVOID)(*((AVConversionConvertStreamFromStructNodeWithHandlerSELPROTO)(gAcroViewHFT[AVConversionConvertStreamFromStructNodeWithHandlerSEL]))), (PVOID)myAVConversionConvertStreamFromStructNodeWithHandler, (PVOID*)&next_AVConversionConvertStreamFromStructNodeWithHandler);
	HookCode((PVOID)(*((AVDocDoSaveAsSELPROTO)(gAcroViewHFT[AVDocDoSaveAsSEL]))), (PVOID)myAVDocDoSaveAs, (PVOID*)&next_AVDocDoSaveAs);	
	HookCode((PVOID)(*((PDDocCreateSELPROTO)(gPDModelHFT[PDDocCreateSEL]))), (PVOID)myPDDocCreate, (PVOID*)&next_PDDocCreate);	
	HookCode((PVOID)(*((PDDocCreatePageSELPROTO)(gPDModelHFT[PDDocCreatePageSEL]))), (PVOID)myPDDocCreatePage, (PVOID*)&next_PDDocCreatePage);	
	HookCode((PVOID)(*((PDDocInsertPagesSELPROTO)(gPDModelHFT[PDDocInsertPagesSEL]))), (PVOID)myPDDocInsertPages, (PVOID*)&next_PDDocInsertPages);	
#endif
	

	FlushHooks();
	CELOG_LOG(CELOG_DEBUG, L"out myhook\n");
}

//////////////////////////////////////////////////////////////////////////
ASCallback gcbAVAppSaveDialog;
ACCB1 ASBool ACCB2 MyAVAppSaveDialog(
									 AVOpenSaveDialogParams dialogParams,			
									 ASFileSys		*outFileSys,			
									 ASPathName		*outASPathName,			
									 AVFilterIndex	*ioChosenFilterIndex)
{

	CELOG_LOG(CELOG_DEBUG, L"in MyAVAppSaveDialog\n");
    //	get current pdf file path
    string current_pdf;
    getCurrentPDFPath(current_pdf);

    AdobeXI::CAdobeXITool::SetCurrentSaveAsPath(current_pdf);
	if(CALL_REPLACED_PROC(gAcroViewHFT, AVAppSaveDialogSEL, gcbAVAppSaveDialog)
		(dialogParams, outFileSys, outASPathName, ioChosenFilterIndex)) 
	{
		string action="CONVERT";
		int choosed_filter=ioChosenFilterIndex? *ioChosenFilterIndex:-1;

		CELOG_LOG(CELOG_DEBUG, L"numFileFilters: %d, ioChosenFilterIndex: %p,%d\n",dialogParams->numFileFilters,ioChosenFilterIndex,ioChosenFilterIndex? *ioChosenFilterIndex:-1);
		for (DWORD i_filter=0;i_filter<dialogParams->numFileFilters;i_filter++)
		{
			ASText des=dialogParams->fileFilters[i_filter]->filterDescription;
			const char* char_text = ASTextGetEncoded(des, NULL);
			if (!char_text)
			{
				CELOG_LOG(CELOG_DEBUG, L"can't get description for this filter\n");
			}
			else
			{
				CELOG_LOGA(CELOG_DEBUG, "filter description: %s, ADOBE_PDF_FILTER: %s\n",char_text, ADOBE_PDF_FILTER);

				if ((choosed_filter>=0) && ((DWORD)choosed_filter==i_filter))
				{
					CELOG_LOGA(CELOG_DEBUG, "above is the selected filter, char_text: %s\n", char_text);

#if ACRO_SDK_LEVEL==0x000A0000//acrobat,we need to filter "save as certified document". for this case, the "filter" is also "Adobe PDF Files"
					if (0 == _stricmp(ADOBE_PDF_FILTER,char_text))
					{
						if (dialogParams->numFileFilters > 1)
						{
							action="COPY";
							CELOG_LOG(CELOG_DEBUG, L"this is COPY\n");
						}
						else
						{
							if (boost::algorithm::starts_with(current_pdf, TempLowPath) && boost::algorithm::iends_with(current_pdf, ".pdf"))
							{
								string FileName = current_pdf.substr(current_pdf.rfind("\\"));
								if (boost::algorithm::istarts_with(FileName, "\\Portfolio"))
								{
									action="COPY";
									CELOG_LOG(CELOG_DEBUG, L"this is COPY\n");
								}
							}
						}
					}
#else
					if (0 == _stricmp(ADOBE_PDF_FILTER,char_text))
					{
						action="COPY";
						CELOG_LOG(CELOG_DEBUG, L"this is COPY\n");
					}
#endif
				}
			}
		}


		PDDoc doc;
		getCurrentPDDoc(doc);

		//	get target file path

		if (outASPathName && outFileSys)
		{
			string strOutPath = GetPath(*outFileSys, *outASPathName);

			if (!strOutPath.empty())
			{
				CELOG_LOGA(CELOG_DEBUG, "output_path is:%s\n", strOutPath.c_str());

				bool bdeny=false;
				CPolicy* ins_policy=CPolicy::GetInstance();

				//this is not to save as pdf, we just do CONVERT on it
				if ("CONVERT"==action)
				{
					if (true==IsURLPath(current_pdf.c_str()))
					{
						ins_policy->queryHttpSource(current_pdf.c_str(),doc,bdeny,"CONVERT");
					}
					else
					{
						ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"CONVERT");
					}
					
					if (true==bdeny)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
				
				//this is to save as pdf, treat it as COPY
				vector<pair<wstring,wstring>> w_obligation_tags_dest;
				bool bDest_Encrypt=false;
				
				//If current file is SE, deny save sa
				if ( bReaderXProtectedMode )
				{
					string filepath = current_pdf;
					std::transform ( filepath.begin(), filepath.end(), filepath.begin(), towlower );
				
					BOOL bfind=FALSE;
					{
						boost_share_lock readerLock(g_mFilesSEStatus);
						bfind=gFilesSEStatus[filepath];
					}

					if ( bfind )
					{
						CELOG_LOGA(CELOG_DEBUG, "need to call message box\n");
						MessageBoxW ( GetForegroundWindow(), L"This action is not permitted. Please use Windows explorer to perform the action.", L"Warning", MB_ICONWARNING | MB_OK );
						return false;
					}
				}

				CELOG_LOGA(CELOG_DEBUG, "before QueryCopy_Get_Obligation_Inheritance\n");

				//查询策略，并且拿到tag obligation和encryption obligation的信息
				//tag obligation现在不能做，因为现在文件还没有真正生成，不可能打tag
				//encryption obligation现在可以做，因为可以在生成之前mark就行了

			#if ACRO_SDK_LEVEL==0x000A0000	//for Acrobat
				if (boost::algorithm::istarts_with(current_pdf, TempLowPath) && boost::algorithm::iends_with(current_pdf, ".tmp"))
				{
					//Should display destination name
					ins_policy->QueryCopy_Get_Obligation_Inheritance(current_pdf.c_str(),strOutPath.c_str(),bdeny,w_obligation_tags_dest,bDest_Encrypt,NULL,true);
				}
				else	
			#endif
				{
					ins_policy->QueryCopy_Get_Obligation_Inheritance(current_pdf.c_str(),strOutPath.c_str(),bdeny,w_obligation_tags_dest,bDest_Encrypt);

					CEdit* ins_edit=CEdit::GetInstance();
					ins_edit->invalid_obligaiton(current_pdf);
				}
				
				//看看要不要对目标文件加密，如果要，并且目标文件不在本地，因为SE不可能支持对不在本地的文件加密，所以，我们要block这个COPY
				if (bDest_Encrypt)
				{
					//看看目标文件的地址
					string dest(strOutPath.c_str());
					wstring wdest = MyMultipleByteToWideChar(dest);
					if (bReaderXProtectedMode || false==IsLocalPath(wdest.c_str()))
					{
						//这个不是本地文件,BLOCK
						CELOG_LOG(CELOG_DEBUG, L"to AVAlertNote\n");
						AVAlertNote("This action is not permitted. Please use Windows explorer to perform the action.");
						return false;
					}
				}


				if (bdeny)
				{
					//不调用真正的，直接返回
					return false;
				}
				else
				{
					string strDest(strOutPath.c_str());
					wstring wstrDest = MyMultipleByteToWideChar(strDest);


					//看看这个source pdf有多少页
					DWORD dwPageNum=PDDocGetNumPages(doc);

					//下面处理encryption
 					if (bDest_Encrypt)
					{
						#if ACRO_SDK_LEVEL==0x000A0000	//Acrobat
							if ( 0 == _stricmp ( strOutPath.c_str(), current_pdf.c_str() ) )	//If save as file is same as current opened file, encrypt temp file. 
							{
								wstring wstrEncryptDirectory = wstrDest;
								wstrEncryptDirectory.replace ( wstrEncryptDirectory.rfind('\\'), std::wstring::npos, L"\\*" );
								CEncrypt::Encrypt(wstrEncryptDirectory, TRUE);
							}
							else
							{
								CDoEncryObligation::DoMarkEncrypt(dwPageNum,wstrDest);
							}										
						#else
							if ( 9 == MajorVersion )
							{
								CDoEncryObligation::DoMarkEncrypt(dwPageNum,wstrDest);
							}
							else
							{
								if ( 0 == _stricmp ( strOutPath.c_str(), current_pdf.c_str() ) )	//If save as file is same as current opened file, encrypt temp file. 
								{
									wstring wstrEncryptDirectory = wstrDest;
									wstrEncryptDirectory.replace ( wstrEncryptDirectory.rfind('\\'), std::wstring::npos, L"\\*" );
									CEncrypt::Encrypt(wstrEncryptDirectory, TRUE);
								}
								else
								{
									CDoEncryObligation::DoMarkEncrypt(dwPageNum,wstrDest);
								}
							}
						#endif
 					}


					//下面处理tag obligation
					if (w_obligation_tags_dest.size()!=0)
					{
						bool bNeedWarningBox = false;
#if ACRO_SDK_LEVEL==0x000A0000//acrobat,
						if(bDest_Encrypt)
							bNeedWarningBox = true;
#else
						bNeedWarningBox = true;
#endif
						CELOG_LOG(CELOG_DEBUG, L"Need pop up warning box,%s, %d\n", wstrDest.c_str(), bNeedWarningBox);

						if (bNeedWarningBox && MessageBoxW(GetForegroundWindow(), WARNING_CLOSEFILE, L"Warning", MB_ICONWARNING | MB_YESNO) == IDNO)
						{
							return false;
						}

						CDoTag_SaveAs* ins_dotag_saveas=CDoTag_SaveAs::GetInstance();

						ins_dotag_saveas->setFlagAndObligationTags(w_obligation_tags_dest);
						ins_dotag_saveas->SetFilePath(strOutPath.c_str());

						//设置文件是否是SE文件，SE文件打tag的方式不同
						if (bDest_Encrypt)
						{
							ins_dotag_saveas->SetSEFlag();
						}

						//寻找文件后缀
						string strOutput(strOutPath.c_str());
						string::size_type pos=strOutput.rfind('.');
						if (string::npos==pos)
						{
							RemoveOpenedFile(current_pdf);
							AddOpenedFile(strOutPath);

							//被save的文件没有后缀，这是不合乎预期逻辑的，所以直接返回
							CELOG_LOG(CELOG_DEBUG, L"dest file has no postfix when save as\n");
							return true;
						}

						//找到文件的后缀
						string postfix=strOutput.substr(pos,strOutput.length()-pos);//postfix是比如.txt的值，要包含'.'号

	
						//设置文件后缀
						ins_dotag_saveas->SetType(postfix);

						//执行execute
						ins_dotag_saveas->execute_avappsavedialog(dwPageNum);
					}
				}

				RemoveOpenedFile(current_pdf);
				AddOpenedFile(strOutPath);
			}
		}

		return true;
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"cancel in MyAVAppSaveDialog\n");
		return false;
	}
}
ASCallback gcbAVAppOpenDialog;
ACCB1 ASBool ACCB2 MyAVAppOpenDialog(
									 AVOpenSaveDialogParams dialogParams,			
									 ASFileSys* outFileSys,			
									 ASPathName** outASPathNames,		
									 AVArraySize* outNumASPathNames,		
									 AVFilterIndex	*ioChosenFilterIndex)
{

	CELOG_LOG(CELOG_DEBUG, L"in MyAVAppOpenDialog\n");
	if( CALL_REPLACED_PROC(gAcroViewHFT, AVAppOpenDialogSEL, gcbAVAppOpenDialog)
		(dialogParams, outFileSys, outASPathNames, 
		outNumASPathNames, ioChosenFilterIndex))
	{
		//取得当前的pdf路径
		string current_pdf;
		getCurrentPDFPath(current_pdf);

		//判断对话框的标题
		if (dialogParams->windowTitle)
		{
			const char* title=ASTextGetEncoded(dialogParams->windowTitle,NULL);
			if (title)
			{
				CELOG_LOGA(CELOG_DEBUG, "title is: %s\n", title);


				//取得被选中的文件的路径
				char* pPath = ASFileSysDisplayStringFromPath(*outFileSys, **outASPathNames);
				string strOutPath;
				if (pPath != NULL)
				{
					strOutPath = string(pPath);
					ASfree(pPath);
				}

				if (strcmp("Select File With New Pages",title)==0)
				{
					//这是用户在做reader 9,document-replace pages
					//做convert的evaluation
					bool bdeny=false;
					CPolicy* ins=CPolicy::GetInstance();
					ins->queryLocalSourceAndDoTagObligation(strOutPath.c_str(),bdeny,"CONVERT");
					if (bdeny)
					{
						CELOG_LOG(CELOG_DEBUG, L"deny replace pages!!!!\n");
						return false;
					}
				}
				else if (strcmp("Select files to upload to Acrobat.com",title)==0 ||
					strcmp("Select files to share on Acrobat.com",title)==0)
				{
					//这是用户在做reader 9(acrobat 9)->file->Collaborate->
					//我们要做send的evaluation，对用户选择的文件做send的evaluation
					bool bdeny=false;
					CPolicy* ins=CPolicy::GetInstance();
					ins->queryLocalSourceAndDoTagObligation(strOutPath.c_str(),bdeny,"SEND");
					if (bdeny)
					{
						CELOG_LOG(CELOG_DEBUG, L"deny Select files to upload to Acrobat.com!!!!\n");
						return false;
					}
				}
				else if ( 0 == strcmp ( "Select File", title ) )
				{
				#if ACRO_SDK_LEVEL!=0x000A0000
					if ( 10 == MajorVersion )  //Only consider Adobe Reader X
					{
						if ( NULL != dialogParams->fileFilters && NULL != *( dialogParams->fileFilters ) && (*(dialogParams->fileFilters))->filterDescription )
						{
							const char* char_text = ASTextGetEncoded ( (*(dialogParams->fileFilters))->filterDescription, NULL );

							if ( 0 == strcmp ( char_text, "All supported files" ) )   //This dialog belongs to Create PDF Files
							{
								bool bdeny = false;
								CPolicy* ins = CPolicy::GetInstance ( );
								ins->queryLocalSourceAndDoTagObligation ( strOutPath.c_str(), bdeny, "SEND" );
								if ( bdeny )
								{
									return false;
								}
							}
						}
					}
				#endif
					
					if ( NULL == dialogParams->fileFilters )
					{
						CELOG_LOGA(CELOG_DEBUG, "Select a file for \"email\", selected path: %s, current opened file: %s\n", strOutPath.c_str(), current_pdf.c_str());

						vector<string> vPath;
						vPath.push_back(strOutPath.c_str());

						boost_unique_lock writeLock(g_mAttachMail);
						gMapSelectedFile[current_pdf] = vPath;
					}

				}
				else if ( 0 == _stricmp("add files", title))
				{
					bool bdeny=false;
					CPolicy* ins=CPolicy::GetInstance();
					ins->queryLocalSourceAndDoTagObligation(strOutPath.c_str(),bdeny,"CONVERT");
					if (bdeny)
					{
						CELOG_LOGA(CELOG_DEBUG, "Reduce file size, %s\n", strOutPath.c_str());
						return false;
					}
				}
				else if ( 0 == _stricmp("Add Attachment",title))
				{		
					bool bdeny=false;
					CPolicy* ins=CPolicy::GetInstance();
					ins->queryLocalSourceAndDoTagObligation(strOutPath.c_str(),bdeny,"CONVERT");
					if (bdeny)
					{
						return false;
					}
					else
					{
						boost_unique_lock writeLock(g_mbeAttachment);
						gbeAttachment = strOutPath;
					}
				}
				else if ( 0 == _stricmp("Select File Containing Form Data", title))
				{
					bool bdeny=false;
					CPolicy* ins=CPolicy::GetInstance();
					ins->queryLocalSourceAndDoTagObligation(strOutPath.c_str(),bdeny,"CONVERT");
					if (bdeny)
					{
						return false;
					}
				}
			}
			else
			{
				CELOG_LOG(CELOG_DEBUG, L"fail to get title\n");
			}
		}
		else
		{
			//这是说明，没有title，可能是默认的title，也就是"Open"
			//取得被选中的文件的路径
			char* out_path = ASFileSysDisplayStringFromPath(*outFileSys, **outASPathNames);
			

			CSend* ins_send=CSend::GetInstance();
			if(out_path && ins_send->IsSendCollaborateLiveShowed())
			{
				//现在这个被选中的文件，是要send到adobe.com的文件
				bool bdeny=false;
				CPolicy* ins=CPolicy::GetInstance();
				ins->queryLocalSourceAndDoTagObligation(out_path,bdeny,"SEND");

				ASfree(out_path);

				if (bdeny)
				{
					CELOG_LOG(CELOG_DEBUG, L"deny Send and Collaborate Live to Acrobat.com!!!!\n");
					return false;
				}
			}
		}
		//判断用户选择了几个文件

		if (outNumASPathNames)
		{
			CELOG_LOG(CELOG_DEBUG, L"user select %d files\n", *outNumASPathNames);
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"MyAVAppOpenDialog, outNumASPathNames is NULL\n");
		}
		

		return true;
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"cancel in MyAVAppOpenDialog\n");
		return false;
	}
}

void SendOLInfo(CPolicy* ins_policy,string current_pdf,bool &bdeny)
{
	if(!bReaderXProtectedMode)
		return;
	//start cache overlay info
	// this deal with right click and ie open pdf to do print action
	nextlabs::Obligations obs;
	if(ins_policy == NULL)
	{
		return;
	}
	wstring strFilePath = MyMultipleByteToWideChar(current_pdf);
	ins_policy->QueryObl("PRINT",strFilePath,bdeny,obs);
	g_Print_OL.bIsDoOverlay = false;
	if (bdeny==false)
	{
		g_Print_OL.bIsDoOverlay = g_Print_OL.Overlay.SetOverlayData(obs,strFilePath);
	}
	//end cache overlay info

	//start construct data	
	wstring wstrData = g_Print_OL.Overlay.ConstructOLData(bdeny,g_Print_OL.bIsDoOverlay);

	string strData = MyWideCharToMultipleByte(wstrData);

	unsigned int l_Data_Len = strData.length();
	//end construct data

	// send data
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (Connect(hPipe) && hPipe != INVALID_HANDLE_VALUE)
	{
		CELOG_LOG(CELOG_DEBUG, L"try to send data to pipe\n");

		char* buf = new char[l_Data_Len + 6];
		memset(buf,0,l_Data_Len + 6);

		if(buf != NULL)
		{

			memcpy(buf, &l_Data_Len, 4);
			memcpy(buf + 4, "1", 1);
			memcpy(buf + 5, strData.c_str(), l_Data_Len);

			Send(hPipe, (const unsigned char*)buf, l_Data_Len + 6);
			delete []buf;
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"new buf fail!!!\n");
		}

		CloseHandle(hPipe);
	}
	// end send
}

ASCallback gcbAVDocDoPrint;
ACCB1 void ACCB2 myAVDocDoPrint(AVDoc doc)
{

	CELOG_LOG(CELOG_DEBUG, L"in myAVDocDoPrint\n");
	string current_pdf;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc,current_pdf);

	g_Print_OL.strPrtOLFilePath = current_pdf;
	

	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();
	SendOLInfo(ins_policy,current_pdf,bdeny);
	if (true==IsURLPath(current_pdf))
	{
		//读取share point上的文件的tag，然后查询策略
		CELOG_LOG(CELOG_DEBUG, L"this is to print http file\n");
		ins_policy->queryHttpSource(current_pdf.c_str(),pdDoc,bdeny,"PRINT");
	}
	else
	{
		ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"PRINT",pdDoc);
	}

	if (bdeny==true)
	{
		return;
	}

	//allow,call really function
	CALL_REPLACED_PROC(gAcroViewHFT, AVDocDoPrintSEL, gcbAVDocDoPrint)(doc);
}
ASCallback gcbAVDocPrintPages;
void myAVDocPrintPages(AVDoc doc, AVPageIndex firstPage, AVPageIndex lastPage, ASInt32 psLevel, ASBool binaryOK, ASBool shrinkToFit)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocPrintPages\n");

	string current_pdf;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc,current_pdf);

	g_Print_OL.strPrtOLFilePath = current_pdf;

	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();
	if (true==IsURLPath(current_pdf))
	{
		//read the tags of sharepoint file, then do query
		CELOG_LOG(CELOG_DEBUG, L"this is to print http file\n");
		ins_policy->queryHttpSource(current_pdf.c_str(),pdDoc,bdeny,"PRINT");
	}
	else
	{
		ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"PRINT",pdDoc);
	}

	if (bdeny==false)
	{
		CALL_REPLACED_PROC(gAcroViewHFT, AVDocPrintPagesSEL, gcbAVDocPrintPages)(/*AVDoc*/ doc, /*AVPageIndex*/ firstPage, /*AVPageIndex*/ lastPage, /*ASInt32*/ psLevel, /*ASBool*/ binaryOK, /*ASBool*/ shrinkToFit);
	}
	
	CELOG_LOG(CELOG_DEBUG, L"out real AVDocPrintPages\n");
}
ASCallback gcbAVDocPrintPagesWithParams;
void myAVDocPrintPagesWithParams(AVDoc doc, AVDocPrintParams params)
{
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocPrintPagesWithParams\n");
	
	string current_pdf;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc,current_pdf);

	g_Print_OL.strPrtOLFilePath = current_pdf;
	
	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();

	SendOLInfo(ins_policy,current_pdf,bdeny);

	if (true==IsURLPath(current_pdf))
	{
		//读取share point上的文件的tag，然后查询策略
		CELOG_LOG(CELOG_DEBUG, L"this is to print http file\n");

		ins_policy->queryHttpSource(current_pdf.c_str(),pdDoc,bdeny,"PRINT");
	}
	else
	{
		ins_policy->queryLocalSourceAndDoTagObligation(current_pdf.c_str(),bdeny,"PRINT",pdDoc);
	}

	if (bdeny==false)
	{
		CALL_REPLACED_PROC(gAcroViewHFT, AVDocPrintPagesWithParamsSEL, gcbAVDocPrintPagesWithParams)(/*AVDoc*/ doc, params);
	}
	
	CELOG_LOG(CELOG_DEBUG, L"out real AVDocPrintPagesWithParams\n");
}

#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
ASCallback gcbPDDocSave;
void myPDDocSave(PDDoc doc, PDSaveFlags saveFlags, ASPathName newPath, ASFileSys fileSys, ProgressMonitor progMon, void* progMonClientData)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocSave\n");
	CALL_REPLACED_PROC(gPDModelHFT, PDDocSaveSEL, gcbPDDocSave)(/*PDDoc*/ doc, /*PDSaveFlags */saveFlags, /*ASPathName */newPath, /*ASFileSys */fileSys, /*ProgressMonitor */progMon, /*void* */progMonClientData);
	CELOG_LOG(CELOG_DEBUG, L"out real myPDDocSave\n");
}

bool isFilterSaveAsBehavior(PDDocSaveParams inParams)
{
	if (inParams->saveFlags == (PDSaveCollectGarbage | PDSaveBinaryOK | PDSaveLinearized | PDSaveFull) || inParams->saveFlags == (PDSaveForceIncremental | PDSaveBinaryOK))
	{
		string newPath = GetPath(inParams->fileSys, inParams->newPath);
		if (!newPath.empty())
		{
			return true;
		}
	}

	return false;
}

ASCallback gcbPDDocSaveWithParams;
void myPDDocSaveWithParams(PDDoc doc, PDDocSaveParams inParams)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocSaveWithParams\n");

	ASInt32 flags = PDDocGetFlags(doc);
	if(PDDocIsModified==(flags&PDDocIsModified) && !g_BeSavingAs && !isFilterSaveAsBehavior(inParams))
	{
		//the file to be save is modified,
		CELOG_LOGA(CELOG_DEBUG, "\nbelow file is modified, now user try to save/save as it, this is EDIT\n");

		string file;
		GetPathfromPDDoc(doc, file);

		if (!file.length())
		{
			CELOG_LOGA(CELOG_DEBUG, "we fail to get file path from GetPathfromPDDoc\n");
		}
		else
		{	
			if(IsAutoSaveFile(file))
			{
				CELOG_LOGA(CELOG_DEBUG, "ignore this auto save temp file: %s\n", file.c_str());
				CALL_REPLACED_PROC(gPDModelHFT, PDDocSaveWithParamsSEL, gcbPDDocSaveWithParams)(doc,inParams);
				return;
			}

			CPolicy* ins_policy=CPolicy::GetInstance();
			bool bdeny=false;
			vector<pair<wstring,wstring>> w_obligation_tags;
			bool bDest_Encrypt=false;
			nextlabs::Obligations obs;

			//如果这个文件有obs，说明这个文件已经做过了query，不然不会有obs，并且，文件还没有关闭，
			//因为文件关闭的时候，会把这个文件的信息从CEdit里面移除掉
			CEdit* ins_edit=CEdit::GetInstance();
			if (false==ins_edit->get_obligation(file))
			{
				//找不到obs，说明这个文件没有做过query，或者做过query，但是之前已经关闭过了，这是第二次打开
				//所以我们可以查策略
				ins_policy->queryEdit_GetObligation(file.c_str(),bdeny,doc,w_obligation_tags,bDest_Encrypt,obs,false);
			}
			else
			{
				//找到了obs，说明这个文件已经query过了，并且还没有关闭过，
				//所以我们啥也不需要做，直接返回
				CALL_REPLACED_PROC(gPDModelHFT, PDDocSaveWithParamsSEL, gcbPDDocSaveWithParams)(doc,inParams);
				return;
			}


			if (true==bdeny)
			{
				return;
			}

			//do obligation work
			ins_edit->do_tag_obligation(doc,file,w_obligation_tags);

			//force Acrobat to read tags, then the latest native tags can be cached.
			TAGS forcetags;
			CTag::GetInstance()->read_tag(MyMultipleByteToWideChar(file), forcetags, doc);

			//存下obs，表示已经query过了，这个存的会在close的时候移除掉
			ins_edit->save_obligaiton(file,obs);
			
			if (bDest_Encrypt)
			{
				
				CELOG_LOGA(CELOG_DEBUG, "has SE obligation\n");
				ins_edit->do_se_obligation(file);
			}
				
			wstring wstrFile = MyMultipleByteToWideChar ( file );
			if ( emIsEncrytFile == CEncrypt::Encrypt ( wstrFile, FALSE, TRUE ) )
			{
				HANDLE hFile = NextCreateFileW ( wstrFile.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				if ( INVALID_HANDLE_VALUE != hFile )
				{
					BOOL bNeedEncryptTempFile = FALSE;

					char FileHeader[9] = { 0 };
					DWORD dwReadSize = 0;

					if ( ReadFile ( hFile, FileHeader, 8, &dwReadSize, NULL ) && 8 == dwReadSize )
					{
						if ( 0 == _stricmp ( FileHeader, "%PDF-1.2" ) || 0 == _stricmp ( FileHeader, "%PDF-1.1" ) )
						{
							bNeedEncryptTempFile = TRUE;	//Current file is 1.2 or 1.1 and SE
						}
					}	
					CloseHandle ( hFile );

					if ( bNeedEncryptTempFile )
					{
						wstrFile.replace ( wstrFile.rfind('\\'), std::wstring::npos, L"\\*" );
						CEncrypt::Encrypt(wstrFile, TRUE);
					}
				}
			}
		}
	}
	CALL_REPLACED_PROC(gPDModelHFT, PDDocSaveWithParamsSEL, gcbPDDocSaveWithParams)(doc,inParams);

	//try to do tagging for the dest file.
	CDoEncryObligation::DoPostEncrypt();

	CDoTag_SaveAs* doTagAfterSaveAs_ins=CDoTag_SaveAs::GetInstance();

	string doc_path;
	GetPathfromPDDoc(doc,doc_path);//get current open file

	CELOG_LOGA(CELOG_DEBUG, "myPDDocSaveWithParams: %s\n", doc_path.c_str());

	if (doTagAfterSaveAs_ins->getFlag())
	{
		//only handle the case "save as PDF extension"
		doTagAfterSaveAs_ins->execute_pddocdidsave(doc);
	}

	CELOG_LOG(CELOG_DEBUG, L"out real myPDDocSaveWithParams\n");
}
#endif


void myreplace()
{
	gcbAVAppSaveDialog = (void*)ASCallbackCreateReplacement(AVAppSaveDialogSEL, 
		MyAVAppSaveDialog);
	REPLACE(gAcroViewHFT, AVAppSaveDialogSEL, gcbAVAppSaveDialog);

	gcbAVAppOpenDialog = (void*)ASCallbackCreateReplacement(AVAppOpenDialogSEL, 
		MyAVAppOpenDialog);
	REPLACE(gAcroViewHFT, AVAppOpenDialogSEL, gcbAVAppOpenDialog);

	gcbAVDocDoPrint = (void*)ASCallbackCreateReplacement(AVDocDoPrintSEL, 
		myAVDocDoPrint);
	REPLACE(gAcroViewHFT, AVDocDoPrintSEL, gcbAVDocDoPrint);

	//尝试找explorer context menu print on pdf的解决方案
	gcbAVDocPrintPages = (void*)ASCallbackCreateReplacement(AVDocPrintPagesSEL, 
		myAVDocPrintPages);
	REPLACE(gAcroViewHFT, AVDocPrintPagesSEL, gcbAVDocPrintPages);
	gcbAVDocPrintPagesWithParams = (void*)ASCallbackCreateReplacement(AVDocPrintPagesWithParamsSEL, 
		myAVDocPrintPagesWithParams);
	REPLACE(gAcroViewHFT, AVDocPrintPagesWithParamsSEL, gcbAVDocPrintPagesWithParams);


#if ACRO_SDK_LEVEL==0x000A0000	//hooks only for acrobat
	gcbPDDocSave = (void*)ASCallbackCreateReplacement(PDDocSaveSEL, 
		myPDDocSave);
	REPLACE(gPDModelHFT, PDDocSaveSEL, gcbPDDocSave);

	gcbPDDocSaveWithParams = (void*)ASCallbackCreateReplacement(PDDocSaveWithParamsSEL, 
		myPDDocSaveWithParams);
	REPLACE(gPDModelHFT, PDDocSaveWithParamsSEL, gcbPDDocSaveWithParams);
#endif
}
/* PluginImportReplaceAndRegister
** ------------------------------------------------------
** */
/** 
	The application calls this function to allow it to
	<ul>
	<li> Import plug-in supplied HFTs.
	<li> Replace functions in the HFTs you're using (where allowed).
	<li> Register to receive notification events.
	</ul>

	@return true to continue loading plug-in,
	false to cause plug-in loading to stop.
*/

ACCB1 ASBool ACCB2 PluginImportReplaceAndRegister(void)
{
	AVAppGetVersion ( &MajorVersion, &MinorVersion );

	WCHAR AutoSavePath[MAX_PATH] = { 0 };

	if ( SUCCEEDED ( SHGetFolderPathW ( NULL, CSIDL_APPDATA, NULL, 0, AutoSavePath ) ) ) 
	{ 
		g_wstrAutoSavePath = AutoSavePath;

		if ( 9 == MajorVersion )
		{
			g_wstrAutoSavePath.append(L"\\Adobe\\Acrobat\\9.0\\AutoSave\\");
		}
		else if ( 10 == MajorVersion )
		{
			g_wstrAutoSavePath.append(L"\\Adobe\\Acrobat\\10.0\\AutoSave\\");
		}
		else
		{
			g_wstrAutoSavePath.clear();
		}
	}

	#if ACRO_SDK_LEVEL==0x000A0000	//only available for acrobat
	#else
        if (10 == MajorVersion || 11 == MajorVersion || 17 == MajorVersion) //Is Adobe Reader X
		{
			DWORD dwParentID = 0;
			BOOL bret = GetParentProcessID (GetCurrentProcessId(),dwParentID);

			if ( dwParentID > 0 && IsTheSameProcess ( dwParentID, L"AcroRd32.exe" ) )
			{
				bReaderXProtectedMode = TRUE;
			}

			if (bret == FALSE)
			{
				bReaderXProtectedMode = TRUE;
			}
		}

		if (10==MajorVersion)
		{
			bReaderX=TRUE;
		}
	#endif

	//我在ASCallbackCreateReplacement的前面调用madchook
	myhook();

	//我在madchook的后面调用replacement
	myreplace();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	notification
ACCB1 void ACCB2 myAVDocWillOpenFromPDDoc(PDDoc pdDoc, void* clientData)
{
	//打开网上的pdf，会走到这个地方来
	//根据测试，adobe x本地的pdf也会走到这里
	CELOG_LOG(CELOG_DEBUG, L"in myAVDocWillOpenFromPDDoc\n");

#if ACRO_SDK_LEVEL==0x000A0000	//this function is only available for acrobat
	ASText text=PDDocGetXAPMetadata(pdDoc);

	//ASTextGetEncoded: Returns a NULL-terminated string in the given encoding. 
	//The memory to which this string points is owned by the ASText object and may not be valid after additional operations are performed on the object.
	const char* char_text = ASTextGetEncoded(text, NULL);
	
	if (char_text)
	{
		CELOG_LOGA(CELOG_DEBUG, "xap metadata: %s\n", char_text);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"can't get xap metadata\n");
	}

	ASTextDestroy(text); 
#endif

	CELOG_LOG(CELOG_DEBUG, L"out myAVDocWillOpenFromPDDoc\n");
}
ACCB1 void ACCB2 myPDDocDidSave(PDDoc doc, ASInt32 err, void* clientData)
{
	CELOG_LOG(CELOG_DEBUG, L"in myPDDocDidSave\n");

	
	CELOG_LOG(CELOG_DEBUG, L"out myPDDocDidSave\n");
}
void myAVDocDidOpen(AVDoc doc, ASInt32 error, void* clientData)
{
	
	string doc_path;
	PDDoc pddoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pddoc,doc_path);

	CELOG_LOGA(CELOG_DEBUG, "in myAVDocDidOpen: %s\n",doc_path.c_str());

	bool bdeny=false;
	CPolicy* ins_policy=CPolicy::GetInstance();
	
	wstring strFilePath(doc_path.begin(),doc_path.end());
	nextlabs::Obligations obs;

	if (IsURLPath(doc_path) == false&&boost::algorithm::icontains(strFilePath,L"/"))
	{
		ConvertURLCharacterW(strFilePath);
		boost::replace_all(strFilePath,L"/",L"\\");	
	}

	ins_policy->QueryObl("OPEN",strFilePath,bdeny,obs);

	if(bdeny)
	{
	#if ACRO_SDK_LEVEL==0x000A0000	//only available for acrobat
		if(IsAcrobatcom(doc_path))
	#else
		if(IsAcrobatcom(doc_path) || (17 == MajorVersion && boost::algorithm::iends_with(doc_path, ".pdf") && !boost::algorithm::icontains(doc_path, L"\\")))
	#endif
		{
			boost_unique_lock writeLock(g_mbeAcrobatcomFile);
			gbeAcrobatcomFile = doc_path;
		}
	}

	if (11 == MajorVersion)
	{
		AdobeXI::CAdobeXITool::QueryFileOnlineRight(doc_path, pddoc);
	}
	
	COverLay DoOverlay;

	bool bRet = DoOverlay.IsExistViewOL(obs);
	if (bRet)
	{
		ConvertURLCharacterW(strFilePath);
		DoOverlay.DoViewOL(obs,strFilePath);
		CMenuItem* ins_menuitem=CMenuItem::GetInstance();
		ins_menuitem->GetNewWindowMeunItem();

		if (ins_menuitem->m_menuitem_new_window != NULL)
		{
			AVMenuItemRemove(ins_menuitem->m_menuitem_new_window);
		}

		ins_menuitem->GetAutoScrollMeunItem();

		if (ins_menuitem->m_menuitem_auto_scroll != NULL)
		{
			AVMenuItemRemove(ins_menuitem->m_menuitem_auto_scroll);
		}

		ins_menuitem->GetSplitMeunItem();

		if (ins_menuitem->m_menuitem_split != NULL)
		{
			AVMenuItemRemove(ins_menuitem->m_menuitem_split);
		}

		ins_menuitem->GetSpreadSplitMeunItem();

		if (ins_menuitem->m_menuitem_spread_split != NULL)
		{
			AVMenuItemRemove(ins_menuitem->m_menuitem_spread_split);
		}

		if (ins_menuitem->m_menuitem_Compare_Documents != NULL)
		{
			AVMenuItemRemove(ins_menuitem->m_menuitem_Compare_Documents);
		}
	}

	CELOG_LOG(CELOG_DEBUG, L"out myAVDocDidOpen\n");


	
}

void myAVDocDidClose(AVDoc doc, void* clientData)
{
	if (doc == NULL) return;
	
	//Get the ClosedFile from global variable that "WillClose" event put in
	ASFile ClosedFile = NULL;
	{
		boost_share_lock readerLock(g_mbeClosedASFile);
		ClosedFile = gbeClosedASFile;
	}

	if ( NULL != ClosedFile )
	{
		//Get the LastClosedFile from global variable and flush to NULL
		ASFile LastClosedFile = NULL;
		{
			boost_unique_lock writeLock(g_mtheLastClosedASFile);
			LastClosedFile = gtheLastClosedASFile;
			gtheLastClosedASFile = NULL;
		}	

		//If equivalent, the ClosedFile has been closed by ASFileClose
		if ( LastClosedFile != ClosedFile )
		{
			CELOG_LOGA(CELOG_DEBUG, "myAVDocDidClose: ASFile has not been closed by ASFileClose\n");	

			string strCurFile;
			{
				boost_share_lock readerLock(g_mbeClosedFile);
				strCurFile = gbeClosedFile;
			}

			boost::algorithm::replace_all(strCurFile, "/", "\\");

			if (boost::algorithm::iends_with(strCurFile, ".pdf") && !boost::algorithm::istarts_with(strCurFile, TempLowPath))
			{
				//use Acrobat/Reader to open a 1.3 pdf file, do save as, then close the file. Acrobat won't call ASFileClose, here, we will try to close it.
				//in fact, there are other cases which Acrobat/Reader won't call ASFileClose.
				if (!(9 == MajorVersion&&boost::algorithm::istarts_with(strCurFile,"http:")))
				{
					ASFileClose(ClosedFile);
				}
			}
		}
	}
}

void myAVDocWillClose(AVDoc doc, void* clientData)
{
	if (doc == NULL) return;

	string strCurFile;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc, strCurFile);

	CELOG_LOGA(CELOG_DEBUG, "myAVDocWillClose: %s\n", strCurFile.c_str());

	ASFile file = PDDocGetFile(pdDoc);

	//In the "WillClose" event, get the ClosedFile and ClosedASFile,save in global variable for using of "DidClose" event
	{
		boost_unique_lock writeLock(g_mbeClosedFile);
		gbeClosedFile = strCurFile;
	}
	{
		boost_unique_lock writeLock(g_mbeClosedASFile);
		gbeClosedASFile = file;
	}
}

void myAVAppFrontDocDidChange(AVDoc doc, void* clientData)
{
	string strCurFile;
	PDDoc pdDoc=AVDocGetPDDoc(doc);
	GetPathfromPDDoc(pdDoc, strCurFile);

	CELOG_LOGA(CELOG_DEBUG, "myAVAppFrontDocDidChange: %s \n", strCurFile.c_str());

	if (!strCurFile.empty())
	{
		boost_unique_lock writeLock(g_mAttachMail);
		g_ActivePath.m_strPath = strCurFile;
		g_ActivePath.m_tags.clear();
		CTag::GetInstance()->read_tag(MyMultipleByteToWideChar(strCurFile), g_ActivePath.m_tags, pdDoc);
	}
#if ACRO_SDK_LEVEL==0x000A0000//acrobat,
#else//only Adobe Reader X will come here, Adobe Reader 9 won't come here.
	//try to do tagging for the dest file.


	CDoEncryObligation::DoPostEncrypt();

	CDoTag_SaveAs* doTagAfterSaveAs_ins=CDoTag_SaveAs::GetInstance();

	if (doTagAfterSaveAs_ins->getFlag())
	{
		//only handle the case "save as PDF extension"
		doTagAfterSaveAs_ins->execute_pddocdidsave(pdDoc);
	}
#endif
}

void init_notification()
{
	AVAppRegisterNotification(AVDocWillOpenFromPDDocNSEL, gExtensionID, ASCallbackCreateNotification( AVDocWillOpenFromPDDoc, (void *)myAVDocWillOpenFromPDDoc), NULL);
	AVAppRegisterNotification(PDDocDidSaveNSEL, gExtensionID, ASCallbackCreateNotification( PDDocDidSave, (void *)myPDDocDidSave), NULL);
	AVAppRegisterNotification(AVDocDidOpenNSEL, gExtensionID, ASCallbackCreateNotification( AVDocDidOpen, (void *)myAVDocDidOpen), NULL);
	AVAppRegisterNotification(AVAppFrontDocDidChangeNSEL, gExtensionID, ASCallbackCreateNotification( AVAppFrontDocDidChange, (void *)myAVAppFrontDocDidChange), NULL);

	if ( !bInIExplore )
	{
		AVAppRegisterNotification(AVDocWillCloseNSEL, gExtensionID, ASCallbackCreateNotification( AVDocWillClose, (void *)myAVDocWillClose), NULL);
		AVAppRegisterNotification(AVDocDidCloseNSEL, gExtensionID, ASCallbackCreateNotification( AVDocDidClose, (void *)myAVDocDidClose), NULL);
	}
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
	
	if ( IsTheSameProcess ( GetCurrentProcessId(), L"iexplore.exe" ) )
	{
		bInIExplore = TRUE;
	}

	GetTempPathA ( MAX_PATH, TempLowPath );
	TempLowPathLength = strlen ( TempLowPath );

    InitializeCriticalSection(&g_showbubbleCriticalSection);

	init_notification();

	std::wstring wde_bin_path=GetEnforceBinDir();
	if (wde_bin_path.length())
	{
		wde_bin_path+=L"\\basepep32.dll";

		LoadLibraryW(wde_bin_path.c_str());
	}

	hHandleAttachMailThread = CreateThread(NULL, 0, Handle_AttachMail, NULL, 0, NULL);

	hServerThread = CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);

	return true;
}

void CloseThreads()
{
	std::wstring PipeName = NamedPipePrefix;
	PipeName += NamedPipeFlag + boost::lexical_cast<std::wstring>(GetCurrentProcessId());;
	
	HANDLE hPipe = CreateFileW(PipeName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE != hPipe)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
	
	DWORD dwProcID = GetCurrentProcessId();
	PipeName = boost::str(boost::wformat(L"\\\\.\\pipe\\Adobepep_%d") % dwProcID);

	hPipe = CreateFileW(PipeName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE != hPipe)
	{
		CloseHandle(hPipe);
	}
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

	SendMessage(g_hBubbleWnd, WM_DESTROY, NULL, NULL);

	UnregisterPEPCLient();

	CloseThreads();

	AdobeXI::CAdobeXITool::DelAllFileRightOnline();
    DeleteCriticalSection(&g_showbubbleCriticalSection);

	if (hServerThread != NULL)
	{
		WaitForSingleObject(hServerThread, INFINITE);
		CloseHandle(hServerThread);
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
	return ASAtomFromString("NLAdobePEP");	/* Change to your extension's name */
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

		CELOG_LOG(CELOG_DEBUG, L"PIHandshake\n");

		/* Cast handshakeData to the appropriate type */
		PIHandshakeData_V0200 *hsData = (PIHandshakeData_V0200 *)handshakeData;

		/* Set the name we want to go by */
		hsData->extensionName = GetExtensionName();

		/* If you export your own HFT, do so in here */
		hsData->exportHFTsCallback = (void*)ASCallbackCreateProto(PIExportHFTsProcType, &PluginExportHFTs);

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
