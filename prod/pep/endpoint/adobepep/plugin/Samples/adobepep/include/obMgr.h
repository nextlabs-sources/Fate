#pragma once

#include "eframework\platform\cesdk_obligations.hpp"
#include "celog.h"

#pragma warning(push)
#pragma warning(disable:4819 4996 4995)
#include "PAMngr.h"
#include <boost/algorithm/string.hpp>
#pragma warning(pop)

typedef void (*PReleaseTagBuf)(const wchar_t* szBuf);
static const char* g_szReleaseBufName = "ReleaseTagBuf";
static const wchar_t* g_szOB_EncryptionName=L"SE_ENCRYPTION";
static const wchar_t* g_szOB_AutomaticFileTaggingName=L"AUTOMATIC_FILE_TAGGING";
static const wchar_t* g_szOB_InteractiveFileTaggingName=L"INTERACTIVE_FILE_TAGGING";
static const wchar_t* g_szOB_RichUserMessage=L"RICH_USER_MESSAGE";

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_OBMGR_H

extern BOOL bReaderXProtectedMode;
bool Connect(HANDLE& h);
int Send(HANDLE hPipe, const unsigned char *data, int len);

std::string MyWideCharToMultipleByte(const std::wstring & strValue);
std::wstring GetEnforceBinDir();

extern CRITICAL_SECTION g_showbubbleCriticalSection;
extern HWND g_hBubbleWnd;

typedef struct  
{
    ULONG ulSize;
    WCHAR methodName [64];
    WCHAR params [7][256];
}NOTIFY_INFO;

typedef void (__stdcall* type_notify2)(NOTIFY_INFO* pInfo, int nDuration, HWND hCallingWnd, void* pReserved, const WCHAR* pHyperLink);

extern type_notify2 notify2;

class CObMgr
{
public:
	static bool CheckIfExistEncryptOb(_In_ nextlabs::Obligations& obs)
	{
		bool bRet = false;
		const std::list<nextlabs::Obligation> listOb = obs.GetObligations();
		for (std::list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
		{
			if(0 == _wcsicmp(g_szOB_EncryptionName, (*itr).name.c_str()))
			{
				CELOG_LOG(CELOG_DEBUG, L"has encryption obligation\n");
				return true;
			}
		}
		return bRet;
	}
	static bool CheckIfExistAutomaticFileTaggingOb(_In_ nextlabs::Obligations& obs)
	{
		bool bRet = false;
		const std::list<nextlabs::Obligation> listOb = obs.GetObligations();
		for (std::list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
		{
			if(0 == _wcsicmp(g_szOB_AutomaticFileTaggingName, (*itr).name.c_str()))
			{
				CELOG_LOG(CELOG_DEBUG, L"has automatic file tagging obligation\n");
				return true;
			}
		}
		return bRet;
	}
	static bool CheckIfExistInteractiveFileTaggingOb(_In_ nextlabs::Obligations& obs)
	{
		bool bRet = false;
		const std::list<nextlabs::Obligation> listOb = obs.GetObligations();
		for (std::list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
		{
			if(0 == _wcsicmp(g_szOB_InteractiveFileTaggingName, (*itr).name.c_str()))
			{
				CELOG_LOG(CELOG_DEBUG, L"has interactive file tagging obligation\n");
				return true;
			}
		}
		return bRet;
	}
	static int GetObTags(_In_  nextlabs::Obligations &obs, 
		_Out_ vector<pair<wstring,wstring> > &srcTag,
		_Out_ vector<pair<wstring,wstring> > &dstTag, 
		_In_ const wchar_t *szSrc, const wchar_t *szDst,
		bool bDisplayDest = false)
	{
		PA_Mngr::CPAMngr pam(NULL);
		wstring strtempdst = szSrc;
		if(szDst != NULL)	
			strtempdst = szDst;

		//Display destination name
		if (bDisplayDest && NULL != szDst)
		{
			pam.SetObligations(szDst,strtempdst.c_str(),obs);
		}
		else
		{
			pam.SetObligations(szSrc,strtempdst.c_str(),obs);
		}
		
		static HMODULE hMod = NULL;
		static PReleaseTagBuf fnReleaseBuf=NULL;
		if(hMod == NULL)
		{
			wchar_t wzDllPath[512]={0};
			DWORD dwDllLen = ::GetDllDirectory(512,wzDllPath);
			if(dwDllLen < 1)
			{
				CELOG_LOG(CELOG_DEBUG, L"GetDllDirecotry fail\n");
			}

			extern std::wstring GetCommonComponentsDir();
			wstring strTaggingModule = GetCommonComponentsDir();
			::SetDllDirectoryW(strTaggingModule.c_str());
#ifdef _M_IX86
			strTaggingModule += L"\\pa_filetagging32.dll";
#else
			strTaggingModule += L"\\pa_filetagging.dll";
#endif
			hMod = LoadLibrary(strTaggingModule.c_str());
			if(hMod == NULL)
			{
				CELOG_LOG(CELOG_DEBUG, L"load lib pa_filetagging fail, %s\n", strTaggingModule.c_str());
			}

			if(dwDllLen > 0)		
				SetDllDirectoryW(wzDllPath);
			if(fnReleaseBuf == NULL && hMod!=NULL)	
				fnReleaseBuf = (PReleaseTagBuf)GetProcAddress(hMod,g_szReleaseBufName);
		}
		if(hMod == NULL || fnReleaseBuf == NULL)	
		{
			CELOG_LOG(CELOG_DEBUG, L"fail to load lib or get release buf function address\n");
			return -1;
		}

		// get obligation tag pairs (source,destination).
		PABase::ACTION theAction = PABase::AT_COPY;
		if(szDst == NULL)	
			theAction = PABase::AT_MOVE;	//this is ugly code, i am not original author of it, i don't know how pabase and pamngr is written, officepep team say this will work, i just copy code from officepep branch

		HWND hWnd = NULL;
		wchar_t* psrcTag=NULL,*pdstTag=NULL;
		DWORD srcLen=0,dstLen=0;
		int nRet = pam.DoFileTagging2(hMod,NULL,&psrcTag,srcLen,&pdstTag,dstLen,theAction,TRUE,L"OK",hWnd);

		if (0 != nRet)
		{
			if (NULL != psrcTag)
			{
				fnReleaseBuf(psrcTag);
				psrcTag = NULL;
			}

			if (NULL != pdstTag)
			{
				fnReleaseBuf(pdstTag);
				pdstTag = NULL;
			}

			return -1;
		}

		if(psrcTag != NULL)
		{
			GetVecStringPairFromBuf(psrcTag,srcLen,srcTag);
			fnReleaseBuf(psrcTag);
			psrcTag = NULL;
		}
		if(pdstTag != NULL)
		{
			GetVecStringPairFromBuf(pdstTag,dstLen,dstTag);
			fnReleaseBuf(pdstTag);
			pdstTag = NULL;
		}
		return nRet;
	}

    struct BubbleStruct
    {
        const wchar_t* pHyperLink;
        const wchar_t* pAction;
    };

    static void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction)
    {
        if (bReaderXProtectedMode)
        {
            HANDLE hPipe = INVALID_HANDLE_VALUE;
            if (Connect(hPipe) && hPipe != INVALID_HANDLE_VALUE)
            {
                string strpHyperLink=MyWideCharToMultipleByte(pHyperLink);
                string strpAction=MyWideCharToMultipleByte(pAction);

                char* buf = new char[strpHyperLink.length() + 6 + sizeof(int) + 1 + strpAction.length()]();
                
                unsigned int len=strpHyperLink.length() + 5 + sizeof(int) + 1 + strpAction.length();
                memcpy(buf, &len, 4);
                memcpy(buf + 4, "3", 1);
                *(int*)(buf + 5) = Timeout;
                memcpy(buf + 5 + sizeof(int), strpHyperLink.c_str(), strpHyperLink.length());
                memcpy(buf + 5 + sizeof(int) + strpHyperLink.length() + 1, strpAction.c_str(), strpAction.length());

                Send(hPipe, (const unsigned char*)buf, len);
                delete []buf;

                CloseHandle(hPipe);		
            }
        }
        else
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
    }

    static DWORD WINAPI BubbleThreadProc(LPVOID lpParameter)
    {
         std::wstring wde_bin_path=GetEnforceBinDir();
         if (wde_bin_path.empty())
         {
             SetEvent((HANDLE)lpParameter);
             return 0;
         }
 
 
 #ifdef _WIN64
         wde_bin_path+=L"\\notification.dll";
 #else
         wde_bin_path+=L"\\notification32.dll";
 #endif

        HMODULE hmodule = LoadLibraryW(wde_bin_path.c_str());

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
        wcex.hInstance		= NULL;
        wcex.hIcon			= NULL;
        wcex.hCursor		= NULL;
        wcex.hbrBackground	= NULL;
        wcex.lpszMenuName	= NULL;
        wcex.lpszClassName	= L"BubbleClass";
        wcex.hIconSm		= NULL;

        RegisterClassExW(&wcex);

        g_hBubbleWnd = CreateWindowW(L"BubbleClass", NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);;

        SetEvent((HANDLE)lpParameter);

        MSG msg = { 0 };

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return 0;
    }

    static LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

private:
	static bool GetVecStringPairFromBuf(_In_ const wchar_t* szBuf,_In_ const DWORD& dwLen,
		vector<pair<wstring,wstring>>& vecTagPair)
	{
		if(szBuf == NULL || szBuf[0] == NULL || dwLen < 1)	
			return false;
		vector<wstring> vecNxlTags;
		boost::algorithm::split(vecNxlTags,szBuf,boost::algorithm::is_any_of(L"\n"));
		if(vecNxlTags.size() %2  == 0)	
		{
			for(size_t i=0;i<vecNxlTags.size();)
			{
				vecTagPair.push_back(pair<wstring,wstring>(vecNxlTags[i],vecNxlTags[i+1]));
				i += 2;
			}
		}
		return true;
	}
};

