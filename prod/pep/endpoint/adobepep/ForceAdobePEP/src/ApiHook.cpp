#include "../include/ApiHook.h"
#include <string>
#include <process.h>
#include <Wininet.h>
#include <tlhelp32.h>
#include "OverLay.h"
nextlabs::recursion_control hook_control;

#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 
#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_FORCEADOBEPEP_SRC_APIHOOK_CPP


SHORT (WINAPI* next_GetAsyncKeyState) ( int vKey ) = NULL;
LPMAPILOGONEX next_MAPILogonEx = NULL;
HANDLE (WINAPI *next_CreateFileW) ( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = NULL;

LPMAPILOGONEX next_MAPILogonExForOtherProcess = NULL;

using namespace std;
const int BUFSIZE = 512 * 10;


#pragma warning(push)
#pragma warning(disable:4819 4996 4995)
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#pragma warning(pop)


#include "Encrypt.h"

const wchar_t* CONTROL_CLASS_NAME = L"ToolbarWindow32";
const wchar_t* CONTROL_CAPTION = L"Save to Online Account";


struct Print_OL
{
	COverLay Overlay;
	bool bIsDoOverlay;
	string strPrtOLFilePath;
	string strOLInfo;
};


#ifndef _M_X64
static Print_OL g_Print_OL;
#endif

extern HMODULE hMod;

extern bool g_bIsReader;
extern int g_iReaderVersion;

extern DWORD g_ParentProcess;

CRITICAL_SECTION g_showbubbleCriticalSection;
HWND g_hBubbleWnd = NULL;

type_notify2 notify2 = NULL;

void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction);
DWORD WINAPI BubbleThreadProc(LPVOID lpParameter);
LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool GetAttachMailEvaluation();

struct BubbleStruct
{
    const wchar_t* pHyperLink;
    const wchar_t* pAction;
};

static DWORD dwChildProcessID = GetCurrentProcessId();

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: strValue=%hs \n", strValue.c_str());

	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
	{
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: nLen=%d, dError=%lu, pBuf=%ls \n", nLen,dError,print_long_string(pBuf) );
		return L"";
	}

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: nLen=%d, dError=%lu, pBuf=%ls \n", nLen,dError,print_long_string(pBuf) );

	return strResult;
}


bool Connect(HANDLE& h,DWORD dwReconectCnt=10)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: h=%p \n", h);

	bool bRet = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE; 

	if (dwChildProcessID == 0)
	{
		return false;
	}
	wchar_t szPipeName[1024] = {0};
	_snwprintf_s(szPipeName, 1024, _TRUNCATE, L"\\\\.\\pipe\\Adobepep_%d", dwChildProcessID);

	CELOG_LOG(CELOG_DEBUG, L"Try to connect to pipe\n");
	for(DWORD i = 0; i < dwReconectCnt; i++) 
	{ 
		hPipe = CreateFile( 
			szPipeName,   // pipe name 
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
			CELOG_LOG(CELOG_DEBUG,L"%ls \n", szPipeName);

			break; 
		}	
		CELOG_LOG(CELOG_DEBUG, L"fail to connect to pipe \n");
	} 
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bRet=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", bRet?L"TRUE":L"FALSE",hPipe,dwChildProcessID,print_long_string(szPipeName) );

	return bRet;
}

bool ConnectForOtherProcess(HANDLE& h,DWORD dwReconectCnt=10)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: h=%p \n", h);

	bool bRet = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE; 

	if (g_ParentProcess == 0)
	{
		return false;
	}
	wchar_t szPipeName[1024] = {0};
	_snwprintf_s(szPipeName, 1024, _TRUNCATE, L"\\\\.\\pipe\\ReaderXIn64bit_%d", g_ParentProcess);

	CELOG_LOG(CELOG_DEBUG, L"Try to connect to pipe\n");
	for(DWORD i = 0; i < dwReconectCnt; i++) 
	{ 
		hPipe = CreateFile( 
			szPipeName,   // pipe name 
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
			CELOG_LOG(CELOG_DEBUG,L"%ls \n", szPipeName);

			break; 
		}	
		CELOG_LOG(CELOG_DEBUG, L"fail to connect to pipe \n");
	} 
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bRet=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", bRet?L"TRUE":L"FALSE",hPipe,dwChildProcessID,print_long_string(szPipeName) );

	return bRet;
}

int Send(HANDLE hPipe, const unsigned char *data, int len)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: hPipe=%p, data=0X%p, len=%d \n", hPipe,data,len);

	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	BOOL fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if ( ! fSuccess) 
	{
		printf("SetNamedPipeHandleState failed\n");
		CELOG_LOG(CELOG_DUMP, L"Local variables are: dwMode=%lu, fSuccess=%ls \n", dwMode,fSuccess?L"TRUE":L"FALSE" );
		return GetLastError();
	}

	int totalWrite = 0;


	while ( len > 0 )
	{
		DWORD cbWritten = 0;
		fSuccess = WriteFile( 
			hPipe,                  // pipe handle 
			data + totalWrite,             // message 
			len <= BUFSIZE? len: BUFSIZE,              // message length 
			&cbWritten,             // bytes written 
			NULL);  

		if ( ! fSuccess) 
		{
			DWORD dwErr = GetLastError();
			printf("WriteFile failed, error: %d\n", dwErr);
			CELOG_LOG(CELOG_DUMP, L"Local variables are: dwMode=%lu, fSuccess=%ls, totalWrite=%d \n", dwMode,fSuccess?L"TRUE":L"FALSE",totalWrite );
			return dwErr;
		}
		len -= cbWritten;
		totalWrite += cbWritten;
	}
	FlushFileBuffers(hPipe);
	CELOG_LOG(CELOG_DUMP, L"Local variables are: dwMode=%lu, fSuccess=%ls, totalWrite=%d \n", dwMode,fSuccess?L"TRUE":L"FALSE",totalWrite );

	return 0;
}

string RecvData(HANDLE hPipe)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: hPipe=%p \n", hPipe);

	HANDLE hHeap      = GetProcessHeap();
	char* pchRequest = (char*)HeapAlloc(hHeap, 0, BUFSIZE);

	if (pchRequest == NULL)
	{
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: hHeap=%p, pchRequest=%hs \n", hHeap,print_string(pchRequest) );
		return "";
	}

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


		if ( !fSuccess && GetLastError() != ERROR_MORE_DATA)
		{   
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n")); 		
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError()); 
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
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hHeap=%p, ret=%hs, nTotalLen=%u \n", hHeap,ret.c_str(),nTotalLen );

	return ret;
}


void Pipe_Service(void* pParam)
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: pParam=%p \n", pParam);

	BOOL   fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	wchar_t szPipeName[1024] = {0};
	_snwprintf_s(szPipeName, 1024, _TRUNCATE, L"\\\\.\\pipe\\AdobeParent_%d", dwChildProcessID);

	for (;;)
	{
		SetLastError(0);

		hPipe = CreateNamedPipe( 
			szPipeName,             // pipe name 
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
			CELOG_LOG(CELOG_DEBUG, L"create named pipe failed\n");
			CELOG_LOG(CELOG_DUMP, L"Local variables are: fConnected=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", fConnected?L"TRUE":L"FALSE",hPipe,dwChildProcessID,print_long_string(szPipeName) );
			return;
		}

		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			CloseHandle(hPipe); 
			return;
		}

		CELOG_LOG(CELOG_DEBUG, L"Create named pipe successfully\n");

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		if (fConnected) 
		{//client connected
			string ret = RecvData(hPipe);

			if (ret.length() > 5)
			{
				if(ret[4] == '1')
				{
					#ifndef _M_X64
						g_Print_OL.strOLInfo = ret.substr(5);
						CELOG_LOG(CELOG_DEBUG, L"%hs \n", g_Print_OL.strOLInfo.c_str());
					#endif

				}
				else if (ret[4] == '2')
				{
					//要加密临时文件
					string file=ret.substr(5);

					string temp=boost::str(boost::format("need to encrypt [%s]") % file.c_str());
					CELOG_LOG(CELOG_DEBUG,L"%hs \n", temp.c_str());

					wstring wfile = MyMultipleByteToWideChar(file);
					CEncrypt::Encrypt(wfile,true);

					char response[6];
					memset(response,0,6);
					unsigned int len=6;
					memcpy(response, &len, 4);
					memcpy(response + 4, "1", 1);
					memcpy(response + 5, "1", 1);


					Send(hPipe, (const unsigned char*)response, len);
					
				}
                else if (ret[4] == '3')
                {
                    string strHyperLink = &ret[5 + sizeof(int)];
                    wstring wstrHyperLink = MyMultipleByteToWideChar(strHyperLink);
                    
                    string strAction = ret.substr(5 + sizeof(int) + strHyperLink.length() + 1);
                    wstring wstrAction = MyMultipleByteToWideChar(strAction);

                    int TimeOut = *(int*)(ret.c_str() + 5);

                    ShowBubble(wstrHyperLink.c_str(), TimeOut, wstrAction.c_str());
                }
			}

			DisconnectNamedPipe(hPipe); 

		}
		CELOG_LOG(CELOG_DUMP, L"Local variables are: fConnected=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", fConnected?L"TRUE":L"FALSE",hPipe,dwChildProcessID,print_long_string(szPipeName) );
		CloseHandle(hPipe); 
	}

}

void Pipe_ReaderXIn64bitService(void* pParam)
{
	CELOG_LOG(CELOG_DUMP, L"Pipe_ReaderXIn64bitService Parameter variables are: pParam=%p \n", pParam);

	BOOL   fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE;

	wchar_t szPipeName[1024] = {0};
	_snwprintf_s(szPipeName, 1024, _TRUNCATE, L"\\\\.\\pipe\\ReaderXIn64bit_%d", GetCurrentProcessId());

	for (;;)
	{
		SetLastError(0);

		hPipe = CreateNamedPipe( 
			szPipeName,             // pipe name 
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
			CELOG_LOG(CELOG_DEBUG, L"Pipe_ReaderXIn64bitService create named pipe failed\n");
			CELOG_LOG(CELOG_DUMP, L"Pipe_ReaderXIn64bitService Local variables are: fConnected=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", fConnected?L"TRUE":L"FALSE",hPipe,GetCurrentProcessId(),print_long_string(szPipeName) );
			return;
		}

		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			CloseHandle(hPipe); 
			return;
		}

		CELOG_LOG(CELOG_DEBUG, L"Pipe_ReaderXIn64bitService Create named pipe successfully\n");

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
		
		if (!fConnected)
		{
			CloseHandle(hPipe); 
			continue;
		}

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
				string result(pchRequest, cbBytesRead);
				CELOG_LOG(CELOG_DEBUG, L"result=%hs \n",result.c_str());

				if (_stricmp(result.c_str(), "1") == 0)
				{
					if (GetAttachMailEvaluation())
					{
						Send(hPipe, (const unsigned char*)"1", 1);
					}
					else
					{
						Send(hPipe, (const unsigned char*)"0", 1);
					}
				}
			}

			HeapFree(hHeap, 0, pchRequest);
		}

		DisconnectNamedPipe(hPipe); 

		CELOG_LOG(CELOG_DUMP, L"Pipe_ReaderXIn64bitService Local variables are: fConnected=%ls, hPipe=%p, dwProcID=%lu, szPipeName=%ls \n", fConnected?L"TRUE":L"FALSE",hPipe,GetCurrentProcessId(),print_long_string(szPipeName) );
		CloseHandle(hPipe); 
	}

}

void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction)
{
    static bool bFirst = false;

    if (!bFirst)
    {
        ::EnterCriticalSection(&g_showbubbleCriticalSection);
        if (!bFirst)
        {
            HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

            CreateThread(NULL, 0, BubbleThreadProc, hEvent, 0, NULL);

            WaitForSingleObject(hEvent, INFINITE);
            CloseHandle(hEvent);

            bFirst = true;
        }    
        ::LeaveCriticalSection(&g_showbubbleCriticalSection);
    }

    BubbleStruct ThisBubble = {pHyperLink, pAction};

    SendMessage(g_hBubbleWnd, WM_USER + 1, (WPARAM)&ThisBubble, Timeout);
}

DWORD WINAPI BubbleThreadProc(LPVOID lpParameter)
{
    WCHAR CurrentDir[MAX_PATH] = { 0 };

    GetModuleFileNameW(hMod, CurrentDir, MAX_PATH);

    WCHAR* pCDirEnd = wcsrchr(CurrentDir, L'\\');
    if(NULL != pCDirEnd)
    {
        *pCDirEnd = NULL;
    }

#ifdef _WIN64
    wcsncat_s(CurrentDir, MAX_PATH, L"\\notification.dll", _TRUNCATE);
#else
    wcsncat_s(CurrentDir, MAX_PATH, L"\\notification32.dll", _TRUNCATE);
#endif

    HMODULE hmodule = LoadLibraryW(CurrentDir);

    if (hmodule == NULL)
    {
        SetEvent((HANDLE)lpParameter);
        return 0;
    }

    notify2 = (type_notify2)GetProcAddress(hmodule, "notify2");

    if (notify2 == NULL)
    {   
        SetEvent((HANDLE)lpParameter);
        return 0;
    }

    WNDCLASSEX wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style			= 0;
    wcex.lpfnWndProc	= BubbleWndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hMod;
    wcex.hIcon			= NULL;
    wcex.hCursor		= NULL;
    wcex.hbrBackground	= NULL;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= L"BubbleClass";
    wcex.hIconSm		= NULL;

    RegisterClassExW(&wcex);

    g_hBubbleWnd = CreateWindowW(L"BubbleClass", NULL, 0, 0, 0, 0, 0, NULL, NULL, hMod, NULL);;

    SetEvent((HANDLE)lpParameter);

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_USER + 1:
        {
            BubbleStruct* ThisBubble = (BubbleStruct*)wParam;
            NOTIFY_INFO Info = { 0 };
            wcsncpy_s(Info.methodName, 64, L"ShowNotification", _TRUNCATE);
            wcsncpy_s(Info.params[0], 256, L"Fri Mar 06 11:36:47 CST 2015", _TRUNCATE);
            wcsncpy_s(Info.params[1], 256, ThisBubble->pAction, _TRUNCATE);
            wcsncpy_s(Info.params[2], 256, L"file:///c:/fake", _TRUNCATE);
            wcsncpy_s(Info.params[3], 256, L"fake", _TRUNCATE);

            notify2(&Info, (int)lParam, 0, 0, (const WCHAR*)ThisBubble->pHyperLink);
        }

    case WM_PAINT:
        {
            PAINTSTRUCT ps = { 0 };
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

SHORT WINAPI MyGetAsyncKeyState ( int vKey )
{	
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: vKey=%d \n", vKey);

	static int iCount = 0;

	if ( iCount < 2 && VK_SHIFT == vKey )
	{
		iCount++;
		CELOG_LOG(CELOG_DUMP, L"Local variables are: iCount=%d \n", iCount );			
		return 0;
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: iCount=%d \n", iCount );

	return next_GetAsyncKeyState ( vKey );
}



int (WINAPI *NextStartDocW)(HDC hdc, CONST DOCINFOW* lpdi);
int (WINAPI *NextEndPage) (HDC hdc);
int (WINAPI *NextEndDoc)(HDC hdc);




int WINAPI BJStartDocW(
					   HDC hdc,              // handle to DC
					   CONST DOCINFOW* lpdi   // contains file names
					   )
{
	#ifndef _M_X64
	
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: hdc=%p, lpdi=%p \n", hdc,lpdi);

	if ((NULL == lpdi) || (NULL == lpdi->lpszDocName)) {
		return NextStartDocW(hdc, lpdi);
	}

	// read regedit info
	bool bDeny = false;
	bool bIsExistOL = false;

	wstring strInfo = MyMultipleByteToWideChar(g_Print_OL.strOLInfo);



	NM_VLObligation::VisualLabelingInfo VLInfo;

	g_Print_OL.Overlay.GetVLInfo(strInfo,bDeny,bIsExistOL,VLInfo);

	if(bDeny)
	{
		::SetLastError (ERROR_ACCESS_DENIED);
		CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, bIsExistOL=%ls \n", bDeny?L"TRUE":L"FALSE",bIsExistOL?L"TRUE":L"FALSE" );
		return 0;
	}

	g_Print_OL.bIsDoOverlay = bIsExistOL;

	if (g_Print_OL.bIsDoOverlay)
	{
		g_Print_OL.Overlay.SetOverLayDate(VLInfo);
		g_Print_OL.Overlay.SetHDC(hdc);
		g_Print_OL.Overlay.StartGDIPlus();
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, bIsExistOL=%ls \n", bDeny?L"TRUE":L"FALSE",bIsExistOL?L"TRUE":L"FALSE" );

	#endif

	return NextStartDocW(hdc, lpdi);
}



int WINAPI BJEndPage(HDC hdc)
{
	#ifndef _M_X64

	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: hdc=%p \n", hdc);
	
	if(g_Print_OL.bIsDoOverlay && g_Print_OL.Overlay.IsSameHDC(hdc) )
	{
		// do print overlay
		g_Print_OL.Overlay.DoPrintOverlay();
	}

	#endif

	return NextEndPage(hdc);
}


int WINAPI BJEndDoc(HDC hdc)
{
	#ifndef _M_X64

	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: hdc=%p \n", hdc);

	if (g_Print_OL.bIsDoOverlay)
	{
		g_Print_OL.Overlay.releaseHDC(hdc);
		g_Print_OL.Overlay.ReleaseOverlayData();
		g_Print_OL.Overlay.ShutDownGDIPlus() ;
	}

	#endif

	return NextEndDoc(hdc);
}

bool GetAttachMailEvaluation()
{
	bool bDeny = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (Connect(hPipe, 10) && hPipe != INVALID_HANDLE_VALUE)
	{
		Send(hPipe, (const unsigned char*)"1", 1);

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
				string result(pchRequest, cbBytesRead);
				CELOG_LOG(CELOG_DEBUG, L"result=%hs \n",result.c_str());

				if (_stricmp(result.c_str(), "1") == 0)
				{
					bDeny = true;
				}
			}

			HeapFree(hHeap, 0, pchRequest);
		}
		CloseHandle(hPipe);
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, hPipe=%p \n", bDeny?L"TRUE":L"FALSE",hPipe );	

	return bDeny;
}

HRESULT WINAPI MyMAPILogonEx ( ULONG_PTR ulUIParam, LPTSTR lpszProfileName, LPTSTR lpszPassword, ULONG ulFlags, LPMAPISESSION* lppSession )\
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: ulUIParam=%lu, lpszProfileName=%ls, lpszPassword=%ls, ulFlags=%lu, lppSession=%p \n", ulUIParam,print_long_string(lpszProfileName),print_long_string(lpszPassword),ulFlags,lppSession);

	if (GetAttachMailEvaluation())
	{
		return MAPI_E_USER_CANCEL;
	}
	return next_MAPILogonEx(ulUIParam, lpszProfileName, lpszPassword, ulFlags, lppSession);
}

HRESULT WINAPI MyMAPILogonExForOtherProcess ( ULONG_PTR ulUIParam, LPTSTR lpszProfileName, LPTSTR lpszPassword, ULONG ulFlags, LPMAPISESSION* lppSession )\
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: ulUIParam=%lu, lpszProfileName=%ls, lpszPassword=%ls, ulFlags=%lu, lppSession=%p \n", ulUIParam,print_long_string(lpszProfileName),print_long_string(lpszPassword),ulFlags,lppSession);

	bool bDeny = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (ConnectForOtherProcess(hPipe, 10) && hPipe != INVALID_HANDLE_VALUE)
	{
		Send(hPipe, (const unsigned char*)"1", 1);

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
				string result(pchRequest, cbBytesRead);
				CELOG_LOG(CELOG_DEBUG, L"result=%hs \n",result.c_str());

				if (_stricmp(result.c_str(), "1") == 0)
				{
					bDeny = true;
				}
			}

			HeapFree(hHeap, 0, pchRequest);
		}
		CloseHandle(hPipe);
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, hPipe=%p \n", bDeny?L"TRUE":L"FALSE",hPipe );	

	if (bDeny)
	{
		return MAPI_E_USER_CANCEL;
	}
	return next_MAPILogonExForOtherProcess(ulUIParam, lpszProfileName, lpszPassword, ulFlags, lppSession);
}

DWORD WINAPI FakeEvaluatonThread(LPVOID)
{
	nextlabs::cesdk_context cesdk_context_instance;

	nextlabs::comm_helper::Init_Cesdk(&cesdk_context_instance);
	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &cesdk_context_instance);

	nextlabs::eval_parms parm;
	parm.SetAction(L"OPEN");
	parm.SetSrc(L"m:\\foo.txt");
	parm.SetUserInfo(L"FakeUser", L"S-0-0-00-0000000000-0000000000-000000000-0000");
	ptr->Query(&parm);

	return 0;
}


HANDLE WINAPI MyCreateFileW ( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: lpFileName=%ls, dwDesiredAccess=%lu, dwShareMode=%lu, lpSecurityAttributes=%p, dwCreationDisposition=%lu ,dwFlagsAndAttributes=%lu, hTemplateFile=%p \n", print_long_string(lpFileName),dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile );

	if( !lpFileName || hook_control.is_disabled() == true )
	{
		return next_CreateFileW ( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring strFile(lpFileName);
	if(boost::algorithm::iends_with(strFile, L".dll") ||
		boost::algorithm::iends_with(strFile, L".exe") )
	{
		CELOG_LOG(CELOG_DUMP, L"Local variables are: strFile=%ls \n", strFile.c_str() );
		return next_CreateFileW ( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}
	
	static bool bIsFirst = false;

	if(!bIsFirst)
	{
		bIsFirst = true;

		HANDLE hThread = CreateThread(NULL, 0, FakeEvaluatonThread, NULL, CREATE_SUSPENDED, NULL);
		
		if (NULL != hThread)
		{
			SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
			ResumeThread(hThread);

			CloseHandle(hThread);
		}
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: strFile=%ls, bIsFirst=%ls \n", strFile.c_str(),bIsFirst?L"TRUE":L"FALSE" );

	return next_CreateFileW ( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
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
	CELOG_LOG(CELOG_DUMP, L"Parameter variables are: uFormat=%u  \n", uFormat);

	//CELOG_LOG(CELOG_DEBUG, );

	CELOG_LOG(CELOG_DEBUG, L"in mySetClipboardData\n");

	if(GetModuleHandleW(L"NLReaderPEP32.api"))
	{
		CELOG_LOG(CELOG_DEBUG, L"loaded adobepep, do nothing\n");
		return next_SetClipboardData(uFormat,hMem);
	}

	bool bDeny = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (Connect(hPipe,1) && hPipe != INVALID_HANDLE_VALUE)
	{
		Send(hPipe, (const unsigned char*)"2", 1);

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
				string result(pchRequest, cbBytesRead);
				//CELOG_LOG(CELOG_DEBUG, "result:");
				CELOG_LOG(CELOG_DEBUG,L"result=%hs \n", result.c_str());
				if (_stricmp(result.c_str(), "1") == 0)
				{
					bDeny = true;
				}
			}
			else
			{
				string temp=boost::str(boost::format("error %d\n") % GetLastError());
				CELOG_LOG(CELOG_DEBUG,L"%hs \n", temp.c_str());
			}

			HeapFree(hHeap, 0, pchRequest);
		}
		CloseHandle(hPipe);
	}

	if (bDeny)
	{
		CELOG_LOG(CELOG_DEBUG, L"deny in mySetClipboardData\n");
		SetLastError(5);
		CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, hPipe=%p \n", bDeny?L"TRUE":L"FALSE",hPipe );

		return NULL;
	}

	CELOG_LOG(CELOG_DUMP, L"Local variables are: bDeny=%ls, hPipe=%p \n", bDeny?L"TRUE":L"FALSE",hPipe );

	return next_SetClipboardData(uFormat,hMem);
}


typedef LONG (WINAPI* _RegEnumValueA)(
									  _In_         HKEY hKey,
									  _In_         DWORD dwIndex,
									  _Out_        LPSTR lpValueName,
									  _Inout_      LPDWORD lpcchValueName,
									  /*_Reserved_*/   LPDWORD lpReserved,
									  _Out_opt_    LPDWORD lpType,
									  _Out_opt_    LPBYTE lpData,
									  _Inout_opt_  LPDWORD lpcbData
									  );
_RegEnumValueA Next_RegEnumValueA=NULL;
LONG WINAPI myRegEnumValueA(
							_In_         HKEY hKey,
							_In_         DWORD dwIndex,
							_Out_        LPSTR lpValueName,
							_Inout_      LPDWORD lpcchValueName,
							/*_Reserved_  */ LPDWORD lpReserved,
							_Out_opt_    LPDWORD lpType,
							_Out_opt_    LPBYTE lpData,
							_Inout_opt_  LPDWORD lpcbData
						 )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hKey=%p, dwIndex=%lu \n",hKey,dwIndex );

	//OutputDebugStringA("in myRegEnumValue");

	LONG res=Next_RegEnumValueA(
		/*_In_         HKEY */hKey,
		/*_In_         DWORD */dwIndex,
		/*_Out_        LPTSTR */lpValueName,
		/*_Inout_      LPDWORD */lpcchValueName,
		/*_Reserved_   LPDWORD */lpReserved,
		/*_Out_opt_    LPDWORD */lpType,
		/*_Out_opt_    LPBYTE */lpData,
		/*_Inout_opt_  LPDWORD */lpcbData
		);

	if (lpValueName)
	{
		
		CELOG_LOG(CELOG_DEBUG, L"%hs \n",lpValueName);


		string valuename(lpValueName);
		if(string::npos != valuename.find("iTrustedMode"))
		{
			if(lpData)
			{
				DWORD* pvalue= (DWORD*)lpData;
				std::string temp = boost::str(boost::format("iTrustedMode is [%d]") % *pvalue);//Index: 2, Name: iTrustedMode, Type: REG_DWORD, Length: 4, Data: 1
				CELOG_LOG(CELOG_DEBUG, L"%hs \n",temp.c_str());

				*((DWORD*)lpData)=0;
			}
		}
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: res=%ld \n", res );

	return res;
}


BOOL CALLBACK EnumSaveAsDlgChildProc(HWND hwnd,LPARAM lParam)
{  
    if(hwnd != NULL)
    {
        WCHAR strClass[MAX_PATH] = {0};
        ::GetClassName(hwnd,strClass,MAX_PATH);  
        WCHAR strText[MAX_PATH] = {0};
        GetWindowText(hwnd,strText,MAX_PATH);  
        if (boost::algorithm::iequals(strClass,CONTROL_CLASS_NAME) && boost::algorithm::iequals(strText,CONTROL_CAPTION))
        {
            EnableWindow(hwnd,false);      
            return FALSE;
        }
    }
    return TRUE;
}


void DenySaveAsToAcrobat(HWND hSaveAsWnd)
{
    EnumChildWindows(hSaveAsWnd,EnumSaveAsDlgChildProc,NULL);
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

    WCHAR strClass[MAX_PATH] = {0};
    ::GetClassName(hWnd,strClass,MAX_PATH);  

    char text[MAX_PATH]={0};
    GetWindowTextA(hWnd,text,MAX_PATH);

    if(0!= text[0]&&(string)"Save As"==(string)text && boost::algorithm::iequals(strClass,L"#32770"))
    {

        bool bDeny = false;
        HANDLE hPipe = INVALID_HANDLE_VALUE;
        if (Connect(hPipe,1) && hPipe != INVALID_HANDLE_VALUE)
        {
            Send(hPipe, (const unsigned char*)"3", 1);

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
                    string result(pchRequest, cbBytesRead);
                    //CELOG_LOG(CELOG_DEBUG, "result:");
                    CELOG_LOG(CELOG_DEBUG,L"result=%hs \n", result.c_str());
                    if (_stricmp(result.c_str(), "1") == 0)
                    {
                        bDeny = true;
                    }
                }
                else
                {
                    string temp=boost::str(boost::format("error %d\n") % GetLastError());
                    CELOG_LOG(CELOG_DEBUG,L"%hs \n", temp.c_str());
                }

                HeapFree(hHeap, 0, pchRequest);
            }
            CloseHandle(hPipe);
        }

        if (bDeny)
        {
            CELOG_LOGA(CELOG_DEBUG, "in myShowWindow, %s\n", text);
            DenySaveAsToAcrobat(hWnd);

        }
    }
    return next_ShowWindow(hWnd,nCmdShow);
}

typedef BOOL (WINAPI* _CreateProcessAsUserW)(HANDLE hToken,
											 LPCWSTR lpApplicationName,
											 LPWSTR lpCommandLine,
											 LPSECURITY_ATTRIBUTES lpProcessAttributes,
											 LPSECURITY_ATTRIBUTES lpThreadAttributes,
											 BOOL bInheritHandles,
											 DWORD dwCreationFlags,
											 LPVOID lpEnvironment,
											 LPCWSTR lpCurrentDirectory,
											 LPSTARTUPINFOW lpStartupInfo,
											 LPPROCESS_INFORMATION lpProcessInformation);

_CreateProcessAsUserW next_CreateProcessAsUserW = NULL;

BOOL WINAPI MyCreateProcessAsUserW(HANDLE hToken,
								   LPCWSTR lpApplicationName,
								   LPWSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreationFlags,
								   LPVOID lpEnvironment,
								   LPCWSTR lpCurrentDirectory,
								   LPSTARTUPINFOW lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL ret = next_CreateProcessAsUserW(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, 
		lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

	if (ret == TRUE && lpApplicationName != NULL && boost::algorithm::iends_with(lpApplicationName, L"acrord32.exe"))
	{
		dwChildProcessID = lpProcessInformation->dwProcessId;
		::_beginthread(Pipe_Service, 0, NULL);
	}
	return ret;
}

bool GetSaveToAcrobatCOM()
{
	bool bDeny = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (Connect(hPipe,1) && hPipe != INVALID_HANDLE_VALUE)
	{
		Send(hPipe, (const unsigned char*)"4", 1);

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
				string result(pchRequest, cbBytesRead);
				//CELOG_LOG(CELOG_DEBUG, "result:");
				CELOG_LOG(CELOG_DEBUG,L"result=%hs \n", result.c_str());
				if (_stricmp(result.c_str(), "1") == 0)
				{
					bDeny = true;
				}
			}
			else
			{
				string temp=boost::str(boost::format("error %d\n") % GetLastError());
				CELOG_LOG(CELOG_DEBUG,L"%hs \n", temp.c_str());
			}

			HeapFree(hHeap, 0, pchRequest);
		}
		CloseHandle(hPipe);
	}
	
	return bDeny;
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
				if (GetSaveToAcrobatCOM())
				{
					return NULL;
				}
			}
		}
		else if (boost::algorithm::equals(lpszVerb, "POST"))
		{
			if (lpszObjectName != NULL && boost::algorithm::starts_with( lpszObjectName, "/api/uux/assets"))
			{
				if (GetSaveToAcrobatCOM())
				{
					return NULL;
				}
			}
		}
	}

	return next_HttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
}

// return 0 means it's owner, no child
static DWORD GetChildProcessID(DWORD dwProcessID)
{
	DWORD dwChildID = 0;
	
	// Take a snapshot of all processes in the system.
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return dwChildID;
	}

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return dwChildID;
	}

	do
	{
		if (dwProcessID == pe32.th32ParentProcessID)
		{
			if (_wcsicmp(L"AcroRd32.exe", pe32.szExeFile) == 0)
			{
				dwChildID = pe32.th32ProcessID;
				break;
			}
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return dwChildID;
}

BOOL GetOSbitInfo ( BOOL& bIs64bit )
{
#ifdef _M_X64
	bIs64bit = TRUE;
	return TRUE;
#else
	bIs64bit = FALSE;

	typedef BOOL (WINAPI *Typedef_IsWow64Process) ( HANDLE, PBOOL );

	Typedef_IsWow64Process Fun_IsWow64Process = NULL;
	HMODULE hMod = GetModuleHandleW ( L"kernel32" );
	if(hMod !=NULL)	Fun_IsWow64Process = (Typedef_IsWow64Process)GetProcAddress (hMod , "IsWow64Process" );

	if ( NULL != Fun_IsWow64Process )
	{
		if ( !Fun_IsWow64Process ( GetCurrentProcess ( ), &bIs64bit ) )
		{
			return FALSE;
		}

		return TRUE;
	}

	return TRUE;
#endif
}

void tryCreatePipeServer()
{
	if (g_bIsReader && 10 == g_iReaderVersion)
	{
		BOOL bIs64bit = FALSE;
		if (GetOSbitInfo(bIs64bit) && bIs64bit)
		{
			::_beginthread(Pipe_ReaderXIn64bitService, 0, NULL);
		}
	}

	DWORD childProcessID = GetChildProcessID(GetCurrentProcessId());
	if (0 == childProcessID)
	{
		return;
	}

	dwChildProcessID = childProcessID;
    ::_beginthread(Pipe_Service, 0, NULL);
}

//Hook Apis
void Hook ( )
{
	InitializeMadCHook ( );

	HookAPI ( "User32.dll", "GetAsyncKeyState", MyGetAsyncKeyState, (PVOID*)&next_GetAsyncKeyState, 0 );
	HookAPI ( "Kernel32.DLL", "CreateFileW", MyCreateFileW, (PVOID*)&next_CreateFileW, 0 );
	HookAPI ( "Gdi32.DLL", "StartDocW", BJStartDocW, (PVOID*)&NextStartDocW, 0 );
	HookAPI ( "Gdi32.DLL", "EndPage", BJEndPage, (PVOID*)&NextEndPage, 0 );
	HookAPI ( "Gdi32.DLL", "EndDoc", BJEndDoc, (PVOID*)&NextEndDoc, 0 );
	HookAPI ( "user32.DLL", "SetClipboardData", mySetClipboardData, (PVOID*)&next_SetClipboardData, 0 );
	HookAPI ( "user32.DLL", "ShowWindow", myShowWindow, (PVOID*)&next_ShowWindow, 0 );


	WCHAR ProcessPath[MAX_PATH] = { 0 };

	GetModuleFileNameW ( NULL, ProcessPath, MAX_PATH );
	
	HookAPI ( "MAPI32.dll", "MAPILogonEx", MyMAPILogonEx, (PVOID*) &next_MAPILogonEx, 0 );
	HookAPI ( "Advapi32.dll", "RegEnumValueA", myRegEnumValueA, (PVOID*) &Next_RegEnumValueA, 0 );

	HookAPI( "Advapi32.dll", "CreateProcessAsUserW", MyCreateProcessAsUserW, (PVOID*)&next_CreateProcessAsUserW, 0);
	tryCreatePipeServer();

	if (g_bIsReader && g_iReaderVersion == 11)
	{
		HookAPI ( "Wininet.DLL", "HttpOpenRequestA", myHttpOpenRequestA, (PVOID*)&next_HttpOpenRequestA, 0 );
	}

	CELOG_LOG(CELOG_DUMP, L"Local variables are: ProcessPath=%ls \n", print_long_string(ProcessPath) );

}

//Unhook Apis
void Unhook ( )
{
	if (next_GetAsyncKeyState)
	{
		UnhookAPI ( (PVOID*) &next_GetAsyncKeyState );
	}
	
	if (next_MAPILogonEx)
	{
		UnhookAPI ( (PVOID*) &next_MAPILogonEx );
	}
	
	if (next_CreateFileW)
	{
		UnhookAPI ( (PVOID*)&next_CreateFileW );
	}

	if (NextStartDocW)
	{
		UnhookAPI ( (PVOID*)&NextStartDocW );
	}
	if (NextEndPage)
	{
		UnhookAPI ( (PVOID*)&NextEndPage );
	}
	if (NextEndDoc)
	{
		UnhookAPI ( (PVOID*)&NextEndDoc );
	}

	if (next_ShowWindow)
	{
		UnhookAPI ( (PVOID*) &next_ShowWindow );
	}

	if (next_CreateProcessAsUserW)
	{
		UnhookAPI( (PVOID*)&next_CreateProcessAsUserW);
	}
	
	
	

	FinalizeMadCHook();
}

//Hook Apis
void HookForOtherProcess ( )
{
	InitializeMadCHook ( );

	HookAPI ( "MAPI32.dll", "MAPILogonEx", MyMAPILogonExForOtherProcess, (PVOID*) &next_MAPILogonExForOtherProcess, 0 );
}

//Unhook Apis
void UnhookForOtherProcess ( )
{
	if (next_MAPILogonExForOtherProcess)
	{
		UnhookAPI ( (PVOID*) &next_MAPILogonExForOtherProcess );
	}

	FinalizeMadCHook();
}