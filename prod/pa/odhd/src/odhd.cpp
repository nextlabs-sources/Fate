#include "stdafx.h"

#define ODHD_EXPORTS 1

#include "odhd.h"

#include "odhd_fact.h"

//#include "./ohdui/ohdDlg.h"

#include "resource.h"

//#include "./ohdui/HdrProgDlg.h"

#include "assistantdlg.h"

#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")


#pragma warning(disable : 4819 311 4312 4018)

#include "madCHook_helper.h"

#include <Dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

#define CONFIG_FILE_NAME	L"hdr.ini"

#define ERROR_SECTION_NAME	L"HdrErrorMessages"

//////////////////////////////////////////ntapi///////////////////////////////////////////////////
PVOID WINAPI MyGetProcAddress(HMODULE hMod, LPCSTR pczFuncName);

#define NTSIGNATURE(ptr) ((LPVOID)((BYTE *)(ptr) + ((PIMAGE_DOS_HEADER)(ptr))->e_lfanew))
#define SIZE_OF_NT_SIGNATURE (sizeof(DWORD))
#define PEFHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE))
#define OPTHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)))
#define SECHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)+sizeof(IMAGE_OPTIONAL_HEADER)))
#define RVATOVA(base,offset) ((LPVOID)((DWORD)(base)+(DWORD)(offset)))
#define VATORVA(base,offset) ((LPVOID)((DWORD)(offset)-(DWORD)(base)))
//////////////////////////////////////////ntapi///////////////////////////////////////////////////

HINSTANCE       g_hInstance   = NULL;   //this should be modified.for temperlate using.

HFONT           g_fntNormal   = NULL;

HFONT           g_fntBold     = NULL;

std::auto_ptr<MSOAppInstance> MSOAppInstance::_instance;



std::wstring g_wstrNoSupportOffice2K7 = L"";

std::wstring g_wstrNeedOfficeSP3 = L"";

std::wstring g_wstrCantOpenFile = L"";



std::wstring g_wstrConfigFilePath = L"";



/*

REMOVEFAIL: Remove Hidden data failed

NOHD:	No Hidden data found

HAVEHD:	Found hidden data

REMOVEOK:Hidden data be removed succeeded

INSPECTFAIL:Detect hidden data failed

This definition should be consistent with enum ODHDSTATUS in odhd.h

*/

//enum {REMOVEFAIL=-1,NOHD=0, HAVEHD=1, REMOVEOK=2, INSPECTFAIL=3 };



void CreateCustomFont()

{

    g_fntNormal = CreateFontW(14,

        0,

        0,

        0,

        550,

        0,

        0,

        0,

        DEFAULT_CHARSET,

        OUT_DEFAULT_PRECIS,

        CLIP_DEFAULT_PRECIS,

        DEFAULT_QUALITY, //CLEARTYPE_QUALITY,

        DEFAULT_PITCH | FF_DONTCARE,

        L"Arial"

        );

    g_fntBold = CreateFontW(14,

        0,

        0,

        0,

        650,

        0,

        0,

        0,

        DEFAULT_CHARSET,

        OUT_DEFAULT_PRECIS,

        CLIP_DEFAULT_PRECIS,

        DEFAULT_QUALITY, //CLEARTYPE_QUALITY,

        DEFAULT_PITCH | FF_DONTCARE,

        L"Arial"

        );

}

void ReleaseCustomFont()

{

    if(g_fntNormal) DeleteObject(g_fntNormal); g_fntNormal=NULL;

    if(g_fntBold) DeleteObject(g_fntBold); g_fntBold=NULL;

}



void ReadConfigFile(void)

{

	DWORD dwRet = 0;

	WCHAR wzBuffer[1024]; memset(wzBuffer, 0, sizeof(wzBuffer));

	LPCWSTR lpwzDefaultNoSupportOffice2K7 = L"Policy Assistant cannot scan attached Office 2007 document {file}. Please open the document manually and use File | Save As to save it in Office 2003 format before reattaching the document.";

	LPCWSTR lpwzDefaultNeedOfficeSP3 = L"Policy Assistant is unable to remove header footer or water marker in {file}. Please remove them manually and re-attach the file. This problem can be avoided if you have Microsoft Office Service Pack 3 installed.";

	LPCWSTR lpwzDefaultCantOpenFile = L"Policy Assistant cannot open the office document {file}, please make sure it is a valid office document.";



	dwRet = GetPrivateProfileString(ERROR_SECTION_NAME, L"NoSupportOffice2K7", lpwzDefaultNoSupportOffice2K7, wzBuffer, 1023, g_wstrConfigFilePath.c_str());

	if (dwRet <= 0)

	{

		g_wstrNoSupportOffice2K7 = lpwzDefaultNoSupportOffice2K7;

	}

	else

	{

		g_wstrNoSupportOffice2K7 = wzBuffer;

	}



	dwRet = GetPrivateProfileString(ERROR_SECTION_NAME, L"NeedOfficeSP3", lpwzDefaultNeedOfficeSP3, wzBuffer, 1023, g_wstrConfigFilePath.c_str());

	if (dwRet <= 0)

	{

		g_wstrNeedOfficeSP3 = lpwzDefaultNeedOfficeSP3;

	}

	else

	{

		g_wstrNeedOfficeSP3 = wzBuffer;

	}



	dwRet = GetPrivateProfileString(ERROR_SECTION_NAME, L"CantOpenFile", lpwzDefaultCantOpenFile, wzBuffer, 1023, g_wstrConfigFilePath.c_str());

	if (dwRet <= 0)

	{

		g_wstrCantOpenFile = lpwzDefaultCantOpenFile;

	}

	else

	{

		g_wstrCantOpenFile = wzBuffer;

	}

}



extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)

{

	g_hInstance = hInstance;

	switch(dwReason)

	{

	case DLL_PROCESS_ATTACH:

		{

			WCHAR wzConfigPath[MAX_PATH+1]; memset(wzConfigPath, 0, sizeof(wzConfigPath));  

            GetModuleFileName((HMODULE)hInstance, wzConfigPath, MAX_PATH);   

            (_tcsrchr(wzConfigPath,'\\'))[1] = 0;

			g_wstrConfigFilePath = wzConfigPath;

			g_wstrConfigFilePath += CONFIG_FILE_NAME;

			ReadConfigFile();



			CreateCustomFont();

		}

		break;

	case DLL_PROCESS_DETACH:

        ReleaseCustomFont();

		break;

	default:

		break;

	}

	return TRUE;

}

//////////////////////////////////////////////////////////////////////////

// UI part

class CHdrAssistDlg : public CAssistantDialog

{

public:

	CHdrAssistDlg(std::vector<CODHDInspector *>* pInspectors, std::wstring &strMsg, HelpUrlVector* pHelpUrls, LONG lRemoveVisible=0xFFFFFFFF):CAssistantDialog()

    {

        m_bOK        = FALSE;

		m_lRemoveVisible = lRemoveVisible;

        m_nCurAttach = 0;

        m_pInspectors= pInspectors;

		m_strTopMessage = strMsg;

        m_pHelpUrls  = pHelpUrls;

        if(1 >= (int)m_pInspectors->size()) m_bOK=TRUE;

		m_hNormalFont = CreateFontW(15,
			0,
			0,
			0,
			400,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial"
			);

    }

	~CHdrAssistDlg()
	{
		if (m_hNormalFont!=NULL)
		{
			DeleteObject(m_hNormalFont);
			m_hNormalFont = NULL;
		}
		
	}

    void ProcessOK()

    {

        if(m_bOK)

        {

			if(::IsWindow(this->m_hWnd))

            EndDialog(IDOK);

            return;

        }



        int nMax = (int)m_pInspectors->size();

        // Re-set warn information

        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), HDR_DLG_WARN);
		if (m_lRemoveVisible == FALSE)
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), L"");
			SendMessage(::GetDlgItem(m_hWnd, IDC_WARNICON), STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)NULL);
		}
		
        // Remove all data

        m_viewDlg.CleanView();

        m_hdrData.clear();

        m_nCurAttach++;

        AddHdrAttachData();
	
		if (m_lRemoveVisible == FALSE)
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), L"");
			SendMessage(::GetDlgItem(m_hWnd, IDC_WARNICON), STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)NULL);
		}
		else
		{
			UINT nIcon = IDI_HDRWARN;
			HICON hIcon = LoadIcon(g_hInstance,MAKEINTRESOURCE(nIcon));
			if (hIcon != NULL)
			{
				::SendMessageW(::GetDlgItem(m_hWnd, IDC_WARNICON),
					STM_SETIMAGE,
					(WPARAM)IMAGE_ICON,
					(LPARAM)hIcon);

				::SetWindowPos(::GetDlgItem(m_hWnd, IDC_WARNICON), HWND_TOP, m_Warn_x, m_Warn_y, SMALLICON, SMALLICON, SWP_SHOWWINDOW);
			}
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), HDR_DLG_WARN);
		}

		AddViewData();

		if(m_nCurAttach == (nMax-1))

        {

            ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"OK");

            m_bOK = TRUE;

        }

    }

    void ProcessCancel()

    {

		if(::IsWindow(this->m_hWnd))

        EndDialog(IDCANCEL);

    }

    void ProcessItemRemove(int nItem)

    {

        CODHDInspector* pODHDInspector = (*m_pInspectors)[m_nCurAttach];

        int nStatus = pODHDInspector->Remove(nItem)?REMOVEOK:REMOVEFAIL;

        std::wstring strNote;

        if(pODHDInspector->GetNote(strNote) && strNote.length()>0)

        {

            if(strNote.length()>0)

                ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), strNote.c_str());

            else

                ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), HDR_DLG_WARN);

        }

// Reset Data
#ifdef WSO2013
		int nItemView = nItem;
		if ((CODHDInspectorFactory::GetAttachType((*m_pInspectors)[m_nCurAttach]->GetFilePath()) == ODTYPE_WORD)&&
			nItem >=2)
		{
			nItem++;  
		}
		m_hdrData[nItemView].strTitle = (*m_pInspectors)[m_nCurAttach]->GetTitle(nItem).c_str();

        m_hdrData[nItemView].strBody  = (*m_pInspectors)[m_nCurAttach]->GetResult(nItem).c_str();

        m_hdrData[nItemView].nStatus  = (*m_pInspectors)[m_nCurAttach]->GetCatStatus(nItem);

       m_viewDlg.ResetViewItem(nItemView, m_hdrData[nItemView].strTitle.c_str(), m_hdrData[nItemView].strBody.c_str(), m_hdrData[nItemView].nStatus);
#else
		m_hdrData[nItem].strTitle = (*m_pInspectors)[m_nCurAttach]->GetTitle(nItem).c_str();

		m_hdrData[nItem].strBody  = (*m_pInspectors)[m_nCurAttach]->GetResult(nItem).c_str();

		m_hdrData[nItem].nStatus  = (*m_pInspectors)[m_nCurAttach]->GetCatStatus(nItem);
        m_viewDlg.ResetViewItem(nItem, m_hdrData[nItem].strTitle.c_str(), m_hdrData[nItem].strBody.c_str(), m_hdrData[nItem].nStatus);
#endif

    }

    void AddHdrAttachData()

    {

        int i=0, nMax = (int)(*m_pInspectors)[m_nCurAttach]->GetSize();   // m_pInspectors->size();

        m_strAttach = (*m_pInspectors)[m_nCurAttach]->GetFilePath();

        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_FILEPATH), m_strAttach.c_str());

		m_lRemoveVisible=FALSE;
	
		int nIn = 0;
        for (i=0; i<nMax; i++)

        {
#ifdef WSO2013
			if ((*m_pInspectors)[m_nCurAttach]->GetTitle(i).empty())
			{
				continue;
			}
#endif
            m_hdrData.push_back(CHdrData());

            m_hdrData[nIn].strTitle = (*m_pInspectors)[m_nCurAttach]->GetTitle(i).c_str();

            m_hdrData[nIn].strBody  = (*m_pInspectors)[m_nCurAttach]->GetResult(i).c_str();

            m_hdrData[nIn].nStatus  = (*m_pInspectors)[m_nCurAttach]->GetCatStatus(i);

			if (i != ODHDCAT_EXCEL_AUTOFILTER)
			{
				HDRFile* pHDRFile=(*m_pInspectors)[m_nCurAttach]->GetHDRFile();
				if(pHDRFile)
					m_hdrData[nIn].bBtnVisible = pHDRFile->m_dwRemoveButton&(0x1<<(i));
				else
					m_hdrData[nIn].bBtnVisible =TRUE;

			}
			else

				m_hdrData[nIn].bBtnVisible = FALSE;

			if(m_hdrData[nIn].bBtnVisible != 0)
				m_lRemoveVisible=TRUE;

			m_hdrData[nIn].bFiltered = (*m_pInspectors)[m_nCurAttach]->GetFilter((ODHDCATEGORY)i);

			DP((L"HDR data[%d]=%s body=%s, filter=%d", i, m_hdrData[nIn].strTitle.c_str(), m_hdrData[nIn].strBody.c_str(), m_hdrData[nIn].bFiltered));
			++nIn;
        }        

    }



protected:

    void AddViewData()

    {

        //for (std::vector<CHdrData>::iterator it=m_hdrData.begin(); it!=m_hdrData.end(); ++it)

		for (int i = 0; i < m_hdrData.size(); i++)

        {

			// Hide filtered category items.

			if (!m_hdrData[i].bFiltered)

				m_viewDlg.AddItem(m_hdrData[i], i);

        }

        if(m_pHelpUrls && m_nCurAttach<(int)m_pHelpUrls->size()) m_strHelpUrl = (*m_pHelpUrls)[m_nCurAttach];

        else m_strHelpUrl=L"";

        m_viewDlg.ResetView();

        m_viewDlg.Invalidate(TRUE);

    }

    void ReallocWindows()

    {
		if(m_hNormalFont!=NULL)
		{
			::SendMessage(::GetDlgItem(m_hWnd, IDC_NOTIFY), WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
			::SendMessage(::GetDlgItem(m_hWnd, IDC_WARNINFO), WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
			::SendMessage(::GetDlgItem(m_hWnd, IDOK), WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
			::SendMessage(::GetDlgItem(m_hWnd, IDCANCEL), WM_SETFONT, (WPARAM)m_hNormalFont, (LPARAM)TRUE);
		}
		
        RECT rcWinClient, rcUserClient;

        int  nCY = 0;



        // Set Icon

        HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 

            IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

        SetIcon(hIcon, TRUE);

        HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 

            IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

        SetIcon(hIconSmall, FALSE);



        // Set whole windows position

        SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);

        GetClientRect(&rcWinClient);

        rcUserClient.top   = rcUserClient.left = MYMARGIN;

        rcUserClient.right = rcWinClient.right - MYMARGIN;

        rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;

        nCY = rcUserClient.top;



        // Set main icon and text

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EXTMAIL), HWND_TOP, rcUserClient.left, nCY-7, BIGICON, BIGICON, SWP_SHOWWINDOW);

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_FILEPATH), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY, WIDTH(rcUserClient)-(BIGICON+MYMARGIN/2), BIGICON, SWP_SHOWWINDOW);

        ::SendMessage(::GetDlgItem(m_hWnd, IDC_FILEPATH), WM_SETFONT, (WPARAM)g_fntBold, (LPARAM)TRUE);

        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_FILEPATH), m_strAttach.c_str());

        nCY += BIGICON;



        // Set DLG Notify information

        nCY += LINESPACE;

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_NOTIFY), HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), BIGICON, SWP_SHOWWINDOW);

        //::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), HDR_DLG_INFO);

		::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_NOTIFY), m_strTopMessage.c_str());

        nCY += BIGICON;



        // Set View

        nCY += LINESPACE;

        CViewItemDlg::m_nX     = MYMARGIN;

        CViewItemDlg::m_nWidth = WIDTH(rcUserClient) - MYMARGIN -2 - ::GetSystemMetrics(SM_CXVSCROLL);

        m_viewDlg.Create(m_hWnd, 0);

        m_viewDlg.SetWindowPos(HWND_TOP, rcUserClient.left, nCY, WIDTH(rcUserClient), HEIGHT(rcUserClient)-nCY-BUTTONCY*2, SWP_SHOWWINDOW);



        // Set Warn icon

        nCY = rcUserClient.bottom-BUTTONCY-20;

		m_Warn_x = rcUserClient.left+LINESPACE;
		m_Warn_y = nCY;
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_WARNICON), HWND_TOP, rcUserClient.left+LINESPACE, nCY, SMALLICON, SMALLICON, SWP_SHOWWINDOW);

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_WARNINFO), HWND_TOP, rcUserClient.left+LINESPACE+SMALLICON+DBLINESPACE, nCY, 250, SMALLICON, SWP_SHOWWINDOW);
		if (m_lRemoveVisible == FALSE)
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WARNINFO), L"");
			SendMessage(::GetDlgItem(m_hWnd, IDC_WARNICON), STM_SETIMAGE, (WPARAM)IMAGE_ICON , (LPARAM)NULL);
		}
		
        // Set Button

        nCY = rcUserClient.bottom-BUTTONCY;

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.right-BUTTONCX*2-10, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);

        if(!m_bOK) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Next");

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.right-BUTTONCX, nCY, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);

    }



    std::wstring          m_strAttach;

    std::vector<CHdrData> m_hdrData;

    std::vector<CODHDInspector *>* m_pInspectors;

    int                   m_nCurAttach;

    std::wstring          m_strNote;



private:

    BOOL                  m_bOK;

	LONG				  m_lRemoveVisible;

    HelpUrlVector*        m_pHelpUrls;

	std::wstring		  m_strTopMessage;

	LONG				  m_Warn_x;

	LONG				  m_Warn_y;
	HFONT                 m_hNormalFont;

};



//////////////////////////////////////////////////////////////////////////

static DWORD GetSecurityLevelValue(HKEY hKeyRoot, LPCWSTR wzKeyPath, LPCWSTR wzValueName)

{

    long	lResult	    = 0;

    HKEY    hKeyAppRoot = NULL;

    HKEY    hKeySecurity= NULL;



    DWORD   dwSecurityLevel = 0xFFFFFFFF;

    DWORD   dwLevelSize     = sizeof(DWORD);

    DWORD   dwType          = REG_DWORD;



    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, wzKeyPath, 0, KEY_ALL_ACCESS, &hKeyAppRoot);

    if(ERROR_SUCCESS == lResult && NULL != hKeyAppRoot)

    {

        lResult = RegCreateKeyEx(hKeyAppRoot, wzKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeySecurity, NULL);

        if(ERROR_SUCCESS==lResult && NULL!=hKeySecurity)

        {

            if( ERROR_SUCCESS != RegQueryValueEx(hKeySecurity, wzValueName, NULL, &dwType, (LPBYTE)&dwSecurityLevel, &dwLevelSize) && REG_DWORD == dwType)

            {

                dwSecurityLevel = 0xFFFFFFFF;

            }

            RegCloseKey(hKeySecurity);

            hKeySecurity = NULL;

        }

        RegCloseKey(hKeyAppRoot);

        hKeyAppRoot = NULL;

    }



    return dwSecurityLevel;

}



static void SetKeyValue(HKEY hKeyRoot, LPCWSTR wzRootKeyPath, LPCWSTR wzKeyName, LPCWSTR wzValueName, DWORD dwValue)

{

    long	lResult	    = 0;

    HKEY    hKeyAppRoot = NULL;

    HKEY    hKeySecurity= NULL;

    DWORD   dwSetValue  = dwValue;



    lResult = RegOpenKeyEx(hKeyRoot, wzRootKeyPath, 0, KEY_ALL_ACCESS, &hKeyAppRoot);

    if(ERROR_SUCCESS==lResult)

    {

        lResult = RegCreateKeyEx(hKeyAppRoot, wzKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeySecurity, NULL);

        if(ERROR_SUCCESS==lResult && NULL!=hKeySecurity)

        {

            RegSetValueEx(hKeySecurity, wzValueName, 0, REG_DWORD, (LPBYTE)&dwSetValue, sizeof(DWORD));

            RegCloseKey(hKeySecurity);

            hKeySecurity = NULL;

        }

        RegCloseKey(hKeyAppRoot);

        hKeyAppRoot = NULL;

    }

}

#ifndef WSO2K3

#include <TlHelp32.h>

BOOL CODHDUtilities::UpPrivilege(HANDLE hprocess,LPCTSTR lpname) //up the process' privilege to "debug"

{

    HANDLE hToken;

    TOKEN_PRIVILEGES Privileges;

    LUID luid;



    static BOOL bAdjusted = FALSE;

    if(bAdjusted)

        return TRUE;



    OpenProcessToken(hprocess,TOKEN_ADJUST_PRIVILEGES,&hToken);

    Privileges.PrivilegeCount=1;

    LookupPrivilegeValue(NULL,lpname,&luid);

    Privileges.Privileges[0].Luid=luid;

    Privileges.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

    if(AdjustTokenPrivileges(hToken,FALSE,&Privileges,NULL,NULL,NULL)!=0)

    {

        bAdjusted = TRUE;

        return TRUE;

    }



    return FALSE;

}

BOOL CODHDUtilities::GetDestProcessId(LPCWSTR wzProcName, DWORD* pdwProcId)

{

    BOOL bFind = FALSE;

    if(pdwProcId) *pdwProcId = 0;



    DP((L"\n"));

    HANDLE hCurrentProc = ::GetCurrentProcess();

    DWORD  dwCurrentProcId = ::GetCurrentProcessId();

    DP((L"Current process handle  = 0x%X\n", hCurrentProc));

    DP((L"Current process ID      = 0x%X\n", dwCurrentProcId));



    if(NULL != hCurrentProc)

    {

        const WCHAR privilege_name[]=SE_DEBUG_NAME;

        UpPrivilege(hCurrentProc,privilege_name); 

    }



    HANDLE tlhelp=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

    if(INVALID_HANDLE_VALUE == tlhelp)

    {

        DP((L"[GetDestProcessId] Fail to create snapshot\n"));

        return FALSE;

    }



    PROCESSENTRY32 pe32;    memset(&pe32, 0, sizeof(PROCESSENTRY32));

    pe32.dwSize=sizeof(PROCESSENTRY32);



    if(Process32First(tlhelp,&pe32))

    {

        do

        {

            WCHAR* pwzFileName = wcsrchr(pe32.szExeFile, L'\\');

            if(pwzFileName)

                pwzFileName++;

            else

                pwzFileName = pe32.szExeFile;

            if(0 == _wcsicmp(pwzFileName, wzProcName))

            {

                bFind = TRUE;



                DP((L"--------------\nFind:\n"));

                DP((L"        Name = %s\n", pe32.szExeFile));

                DP((L"        Parent proc ID  = 0x%X\n", pe32.th32ParentProcessID));

                DP((L"        process ID      = 0x%X\n", pe32.th32ProcessID));



                HANDLE hProcParent = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ParentProcessID);

                if(NULL != hProcParent)

                {

                    WCHAR wzParentImageName[MAX_PATH+1]; memset(wzParentImageName, 0, sizeof(wzParentImageName));

                    GetProcessImageFileName(hProcParent, wzParentImageName, MAX_PATH);

                    CloseHandle(hProcParent);

                    if(0 != wzParentImageName[0])

                    {

                        WCHAR* pwzParentFileName = wcsrchr(wzParentImageName, L'\\');

                        if(pwzParentFileName)

                            pwzParentFileName++;

                        else

                            pwzParentFileName = wzParentImageName;



                        if(0 == _wcsicmp(pwzParentFileName, L"svchost.exe"))

                        {

                            // We get it

                            if(pdwProcId) *pdwProcId = pe32.th32ProcessID;

                            DP((L"--- we get it (0x%X)!! ---\n", *pdwProcId));

                        }

                    }

                }

            }

        } while(Process32Next(tlhelp,&pe32));

    }



    DP((L"\n"));

    CloseHandle(tlhelp);

    return bFind;

}

//
// Fix bug 7681 -- the new code enumerate LoadLibrary address in remote process.
// By Gavin Ye
//
//extern PVOID WINAPI MyGetProcAddress(HMODULE hMod, LPCSTR pczFuncName);

// Get module base address in remote process
static DWORD GetRemoteModuleBase(LPCWSTR wzModuleName, HANDLE hProcess)
{
    HANDLE hSnap = INVALID_HANDLE_VALUE;
    DWORD  dwBaseAddress = 0;
    MODULEENTRY32W   module32;

    DP((L"\n---->GetRemoteModuleBase\n"));
    DP((L"Get %s Module Base\n", (NULL==hProcess)?L"Local":L"Remote"));

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, (NULL==hProcess)?0:GetProcessId(hProcess));
    if(INVALID_HANDLE_VALUE == hSnap)
    {
        DP((L"ERROR:: fail to get remote module snap shot. err code = %d", GetLastError()));
        goto _exit;
    }
    DP((L"Handle of snapshot = 0x%p\n", hSnap));

    memset(&module32, 0, sizeof(MODULEENTRY32W));
    module32.dwSize = sizeof(MODULEENTRY32W);
    if(!Module32FirstW(hSnap, &module32))
    {
        DP((L"ERROR:: fail to get first module"));
        goto _exit;
    }
    //wprintf(L"Succeed get first module\n");

    do
    {
        //wprintf(L"Module %s\n", module32.szModule);
        if(0 == _wcsicmp(wzModuleName, module32.szModule))
        {
            DP((L"NOTE:: find module %s\n", module32.szModule));
            DP((L"       path = %s\n", module32.szExePath));
            DP((L"       hModule     = 0x%X\n", (DWORD)module32.hModule));
            DP((L"       BaseAddress = 0x%X\n", (DWORD)module32.modBaseAddr));
            DP((L"       BaseSize    = 0x%X\n", (DWORD)module32.modBaseSize));
            dwBaseAddress = (DWORD)module32.modBaseAddr;
            break;
        }
        memset(&module32, 0, sizeof(MODULEENTRY32W));
        module32.dwSize = sizeof(MODULEENTRY32W);
    }while(Module32NextW(hSnap, &module32));

_exit:
    DP((L"\n<----GetRemoteModuleBase\n"));
    if(INVALID_HANDLE_VALUE!=hSnap) CloseHandle(hSnap);
    hSnap = INVALID_HANDLE_VALUE;
    return dwBaseAddress;
}

static DWORD GetRemoteLoadLibraryAddress(HANDLE hProcess)
{
    DWORD   dwLocalBase   = 0;
    DWORD   dwRemoteBase  = 0;
    DWORD	dwRemoteFunc= NULL;
    DWORD	dwLocalFunc = NULL;

    dwLocalBase = GetRemoteModuleBase(L"kernel32.dll", NULL);
    dwLocalFunc = (DWORD)MyGetProcAddress((HMODULE)dwLocalBase, "LoadLibraryW");
    if(NULL == dwLocalFunc)
    {
        DP((L"ERROR:: fail to get local LoadLibraryW address. err code = %d\n", GetLastError()));
        goto _exit;
    }

    dwRemoteBase = GetRemoteModuleBase(L"kernel32.dll", hProcess);
    if(0 != dwRemoteBase)
    {
        // Now we get the function address in remote process
        dwRemoteFunc = dwRemoteBase + (dwLocalFunc - dwLocalBase);
    }

    DP((L"Local Mod Base:   %p\n", dwLocalBase));
    DP((L"Remote Mod Base:  %p\n", dwRemoteBase));
    DP((L"Local Func Base:  %p\n", dwLocalFunc));
    DP((L"Remote Func Base: %p\n", dwRemoteFunc));

_exit:
    return dwRemoteFunc;
}

static BOOL InjectDll(HANDLE hProcess, LPCWSTR wzDll)
{
    BOOL	bRet      = FALSE;
    DWORD	dwMemSize = 0;
    LPVOID	lpBuf     = 0;

    dwMemSize = (DWORD)wcslen(wzDll)*sizeof(WCHAR) + sizeof(WCHAR);
    lpBuf = VirtualAllocEx(hProcess, NULL, dwMemSize, MEM_COMMIT, PAGE_READWRITE);
    if(lpBuf)
    {
        if(WriteProcessMemory(hProcess, lpBuf, wzDll, dwMemSize, NULL))
        {
            LPVOID	pFunc = (LPVOID)GetRemoteLoadLibraryAddress(hProcess);
            if(NULL == pFunc)
            {
                DP((L"ERROR:: Fail to get remote LoadLibraryW address.\n"));
                return bRet;
            }
            HANDLE hThread= CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, lpBuf, 0, NULL);
            if(NULL != hThread)
            {
                bRet = TRUE;
                CloseHandle(hThread);
            }
            else
            {
                DP((L"ERROR:: Fail to create remote thread. error code = %d\n", GetLastError()));
            }
        }
        else
        {
            DP((L"ERROR:: Fail to write remote process memory. error code = %d\n", GetLastError()));
        }
    }
    else
    {
        DP((L"ERROR:: Fail to allocate remote process memory. error code = %d\n", GetLastError()));
    }

    return bRet;
}
//
//  Fix 7681 end
//

void CODHDUtilities::InjectDllToRemoteProcess(wchar_t* wszExecName)

{

	DWORD	dwProcId=0;

    BOOL    bRet = FALSE;

	GetDestProcessId(wszExecName, &dwProcId);

	if(dwProcId==0)

	{

		DP((L"Inject Dll to %s failed!\n",wszExecName));

		return;

	}

    WCHAR wzDll[MAX_PATH+1];    memset(wzDll, 0, sizeof(wzDll));

    ::GetModuleFileNameW(g_hInstance, wzDll, MAX_PATH);

    WCHAR* pwzEnd = wcsrchr(wzDll, L'\\');

    if(pwzEnd)

    {

        *pwzEnd = 0;

#if defined(_WIN64)

        wcsncat_s(wzDll, MAX_PATH+1, L"\\WindowBlocker.dll", _TRUNCATE);

#elif defined (_WIN32)

        wcsncat_s(wzDll, MAX_PATH+1, L"\\WindowBlocker32.dll", _TRUNCATE);

#else

#error Unsupported platform.

#endif

    }

    else

    {

		DP((L"Inject Dll to %s failed!\n",wszExecName));

        return;

    }



    DP((L"\nTrying to inject DLL into remote process:\n    DLL = %s\n    Process id = %d\n\n", wzDll, dwProcId));



    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcId);

    if(NULL != hProcess)

    {

         bRet = InjectDll(hProcess, wzDll);

        if(bRet)

        {

            DP((L"[InjectDllToRemoteProcess] Inject DLL to remote process done\n"));

        }

        else

        {

            DP((L"[InjectDllToRemoteProcess] Fail to inject DLL to remote process\n"));

        }



        CloseHandle(hProcess);

        hProcess = NULL;

    }

    else

    {

        DP((L"[InjectDllToRemoteProcess] Fail to open remote process\n"));

    }

}

#endif //WSO2K7

void CODHDUtilities::SetPptMicroTrustLevel2003(DWORD dwLevel)

{

    SetKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\PowerPoint", L"Security", L"Level", dwLevel);

}

void CODHDUtilities::SetWordMicroTrustLevel2003(DWORD dwLevel)

{

    SetKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\Word", L"Security", L"Level", dwLevel);

}

void CODHDUtilities::SetExcelMicroTrustLevel2003(DWORD dwLevel)

{

    SetKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\Excel", L"Security", L"Level", dwLevel);

}



void CODHDUtilities::ReplaceFileName(std::wstring &OriginalMessage, std::wstring &FileName, std::wstring &NewMessage)

{

	NewMessage = OriginalMessage;

	

	while(true)

	{

		std::wstring::size_type pos(0);



		if ((pos = NewMessage.find(L"{file}")) != std::wstring::npos)

			NewMessage.replace(pos, wcslen(L"{file}"), FileName);

		else

			break;

	}

}



DWORD CODHDUtilities::GetPptMicroTrustLevel2003(DWORD dwDefaultLevel)

{

    DWORD dwLevel = dwDefaultLevel;

    dwLevel = GetSecurityLevelValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\PowerPoint\\Security", L"Level");

    if(0xFFFFFFFF == dwLevel)

    {

        dwLevel = dwDefaultLevel;

        SetPptMicroTrustLevel2003(dwLevel);

    }

    return dwLevel;

}

DWORD CODHDUtilities::GetExcelMicroTrustLevel2003(DWORD dwDefaultLevel)

{

    DWORD dwLevel = dwDefaultLevel;

    dwLevel = GetSecurityLevelValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\Excel\\Security", L"Level");

    if(0xFFFFFFFF == dwLevel)

    {

        dwLevel = dwDefaultLevel;

        SetExcelMicroTrustLevel2003(dwLevel);

    }

    return dwLevel;

}

DWORD CODHDUtilities::GetWordMicroTrustLevel2003(DWORD dwDefaultLevel)

{

    DWORD dwLevel = dwDefaultLevel;

    dwLevel = GetSecurityLevelValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\11.0\\Word\\Security", L"Level");

    if(0xFFFFFFFF == dwLevel)

    {

        dwLevel = dwDefaultLevel;

        SetWordMicroTrustLevel2003(dwLevel);

    }

    return dwLevel;

}

BOOL CODHDUtilities::GetDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPWSTR pwzValue, int nSize)

{

	HRESULT hr   = S_OK;

	BOOL    bGet = FALSE;

	VARIANT result; VariantInit(&result);

	VARIANT x; x.vt = VT_BSTR; x.bstrVal = ::SysAllocString(pwzName);

	DispatchCallWraper(pProps,DISPATCH_PROPERTYGET, &result,  L"Item", 1, x);

	CComPtr<IDispatch> pProp = result.pdispVal;

	::SysFreeString(x.bstrVal);



	if(pProp)

	{

		DispatchCallWraper(pProp,DISPATCH_PROPERTYGET, &result, L"Value", 0);

		if(result.bstrVal)

		{

			bGet = TRUE;

			wcsncpy_s(pwzValue, nSize, result.bstrVal, _TRUNCATE);

		}

	}



	return bGet;

}

BOOL CODHDUtilities::EmptyDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue)

{

    HRESULT hr   = S_OK;

    BOOL    bSet = FALSE;

	BSTR bstrNull=::SysAllocString(pwzValue);

    // Looking for existing prop

    VARIANT result; VariantInit(&result);

    VARIANT x; x.vt = VT_BSTR; x.bstrVal = ::SysAllocString(pwzName);

    hr = DispatchCallWraper(pProps,DISPATCH_PROPERTYGET, &result,  L"Item", 1, x);

    if( SUCCEEDED(hr) && result.pdispVal )

    {

		VARIANT		vtTypeResult;

		VARIANT		parm1;

		memset(&parm1,0,sizeof(parm1));

        CComPtr<IDispatch>	pProp = result.pdispVal;

		if(0==_wcsicmp(pwzName, L"Last save time"))

			DP((L"here\n"));

		hr = DispatchCallWraper(pProp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtTypeResult, L"Type", 0);

		if(SUCCEEDED(hr))

		{

			switch(vtTypeResult.intVal)

			{

			case msoPropertyTypeNumber:

				parm1.vt=VT_EMPTY;//VT_I4;

				parm1.intVal=0;

				break;

			case msoPropertyTypeBoolean:

				parm1.vt=VT_EMPTY;//VT_BOOL;

				parm1.boolVal=0;

				break;

			case msoPropertyTypeDate:

				parm1.vt=VT_NULL;//VT_DATE;

				parm1.date=0.0;

				break;

			case msoPropertyTypeString:

				parm1.vt=VT_EMPTY;//VT_BSTR;

				parm1.bstrVal=bstrNull;

				break;

			case msoPropertyTypeFloat:

				DP((L"float not support"));

				break;

			default:

				break;

			}

		}

        hr = DispatchCallWraper(pProp,DISPATCH_PROPERTYPUT, NULL,  L"Value", 1, parm1);

    }

	::SysFreeString(bstrNull);

    return bSet;

}

BOOL CODHDUtilities::RemoveCustomProperties(CComPtr<IDispatch> lpCustomProps)

{

    BOOL    bRet= FALSE;

    HRESULT hr  = S_OK;

    if(NULL == lpCustomProps)

        return FALSE;



    VARIANT vtResult;

    long    lCount = 0;



    hr = DispatchCallWraper(lpCustomProps,DISPATCH_PROPERTYGET, &vtResult, L"Count", 0);

    lCount = vtResult.lVal;

    if(SUCCEEDED(hr) && lCount)

    {

        long i = 0;

        for(i=lCount; i>0; i--)

        {


            CComVariant varIndex(i);

            hr = DispatchCallWraper(lpCustomProps,DISPATCH_PROPERTYGET, &vtResult,  L"Item", 1, varIndex);
            CComPtr<IDispatch> lpDispProp = vtResult.pdispVal;

            if(SUCCEEDED(hr) && lpDispProp)

            {

                DispatchCallWraper(lpDispProp,DISPATCH_METHOD, NULL,  L"Delete", 0);

            }

        }

    }



    return bRet;

}

void CODHDUtilities::ShowPasswordWindows(void *dummy)

{

	HWND hwnd = NULL;

	//while(1)

	//{

		Sleep(3000);

		hwnd = ::FindWindow(NULL, L"Password");

		if (hwnd != NULL)

		{

			wchar_t szCls[255] = {0};

			GetClassName(hwnd,szCls,255);

			int evalue = _wcsicmp(szCls, L"bosa_sdm_XL9") * _wcsicmp(szCls, L"#32770") * _wcsicmp(szCls, L"bosa_sdm_Microsoft Office Word 11.0") * _wcsicmp(szCls, L"bosa_sdm_Microsoft Office Word 12.0") * _wcsicmp(szCls, L"bosa_sdm_msword");

			if(evalue == 0)

			{

				::ShowWindow(hwnd, SW_RESTORE);

				//::SetForegroundWindow(hwnd);

				::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

				//break;

			}	

		} 

	//}
	::_endthread();
}

void CODHDUtilities::ShowPromtWindows(void *dummy)
{
	HWND hwnd = NULL;
	Sleep(3000);

	//Word2003
	hwnd = ::FindWindow(L"bosa_sdm_Microsoft Office Word 11.0", L"Microsoft Office Word"); 
	if (hwnd != NULL)
	{
		::ShowWindow(hwnd, SW_RESTORE);
		//::SetForegroundWindow(hwnd);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::_endthread();
	} 

	//Word2007
	hwnd = ::FindWindow(L"bosa_sdm_Microsoft Office Word 12.0", L"Microsoft Office Word"); 
	if (hwnd != NULL)
	{
		::ShowWindow(hwnd, SW_RESTORE);
		//::SetForegroundWindow(hwnd);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::_endthread();
	} 

	//Word2010
	hwnd = ::FindWindow(L"bosa_sdm_msword", L"Microsoft Word");
	if (hwnd != NULL)
	{
		::ShowWindow(hwnd, SW_RESTORE);
		//::SetForegroundWindow(hwnd);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::_endthread();
	} 

}

BOOL CODHDUtilities::RemoveDocumentProperties(CComPtr<IDispatch> lpDocumentProps,LPCWSTR pPropNames[],int iPropCount)

{

    BOOL    bRet= FALSE;

    HRESULT hr  = S_OK;

    if(NULL == lpDocumentProps)

        return FALSE;



    //EnumDocumentProperties(lpDocumentProps);

	for(int iIndex=0;iIndex<iPropCount;iIndex++)

	{

		EmptyDocumentPropValue(lpDocumentProps, pPropNames[iIndex], L"");

	}



    return bRet;

}

BOOL CODHDUtilities::IsWordFile(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



    if(0 == _wcsicmp(pSuffix, L".DOC") ||

        0 == _wcsicmp(pSuffix, L".DOT") ||

		0 == _wcsicmp(pSuffix, L".RTF") 

#ifndef WSO2K3

        || 0 == _wcsicmp(pSuffix, L".DOCX") ||

		0 == _wcsicmp(pSuffix, L".DOCM") ||

		0 == _wcsicmp(pSuffix, L".DOTX") ||

		0 == _wcsicmp(pSuffix, L".RTF") ||

		0 == _wcsicmp(pSuffix, L".DOTM")

#endif



        )

		return TRUE;



	return FALSE;

}

BOOL CODHDUtilities::IsWordTemplateFile(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



    if(0 == _wcsicmp(pSuffix, L".DOT")



#ifndef WSO2K3

    ||0 == _wcsicmp(pSuffix, L".DOTM") ||

	0 == _wcsicmp(pSuffix, L".DOTX")

#endif



        )

		return TRUE;



	return FALSE;

}

BOOL CODHDUtilities::IsExcelFile(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



    if(0 == _wcsicmp(pSuffix, L".XLS") ||

        0 == _wcsicmp(pSuffix, L".XLT")

#ifndef WSO2K3

        || 0 == _wcsicmp(pSuffix, L".XLSX") ||

		0 == _wcsicmp(pSuffix, L".XLSM") ||

		0 == _wcsicmp(pSuffix, L".XLSB") ||

		0 == _wcsicmp(pSuffix, L".XLTX") ||

		0 == _wcsicmp(pSuffix, L".XLTM")

#endif

        )

		return TRUE;



	return FALSE;

}

BOOL CODHDUtilities::IsExcelTemplateFile(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



    if(0 == _wcsicmp(pSuffix, L".XLT")

#ifndef WSO2K3

        ||

		0 == _wcsicmp(pSuffix, L".XLTX") ||

		0 == _wcsicmp(pSuffix, L".XLTM")

#endif

		)



		return TRUE;



	return FALSE;

}

BOOL CODHDUtilities::IsPptFile(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



    if(0 == _wcsicmp(pSuffix, L".PPT") ||

        0 == _wcsicmp(pSuffix, L".POT")

#ifndef WSO2K3

        || 0 == _wcsicmp(pSuffix, L".PPTX") ||

		0 == _wcsicmp(pSuffix, L".PPTM") ||

		0 == _wcsicmp(pSuffix, L".POTX") ||

        0 == _wcsicmp(pSuffix, L".POTM")

#endif

        )

		return TRUE;



	return FALSE;

}



BOOL CODHDUtilities::IsOffice2007File(LPCWSTR pwzFile)

{

	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');

	if(NULL == pSuffix) return FALSE;



	if (0 == _wcsicmp(pSuffix, L".DOCX") ||

		0 == _wcsicmp(pSuffix, L".DOCM") ||

		0 == _wcsicmp(pSuffix, L".DOTX") ||

		0 == _wcsicmp(pSuffix, L".XLSX") ||

		0 == _wcsicmp(pSuffix, L".XLSM") ||

		0 == _wcsicmp(pSuffix, L".XLSB") ||

		0 == _wcsicmp(pSuffix, L".XLTX") ||

		0 == _wcsicmp(pSuffix, L".XLTM") ||

		0 == _wcsicmp(pSuffix, L".PPTX") ||

		0 == _wcsicmp(pSuffix, L".PPTM") ||

		0 == _wcsicmp(pSuffix, L".POTX") ||

        0 == _wcsicmp(pSuffix, L".POTM"))

		return TRUE;



	return FALSE;

}



bool operator == (CODHDCategory * srcHDClass,const  ODHDCATEGORY hdCat)

{

	return srcHDClass->m_Category==hdCat;

}

bool operator == (CODHDItem * srcHDItem,const  ODHDITEM indItem)

{

	return srcHDItem->m_indItem==indItem;

}

static LRESULT RunProgressWnd(LPVOID lpVoid)

{

	CHdrProgDlg* pHdrProgDlg=(CHdrProgDlg*)lpVoid;

	pHdrProgDlg->DoModal(pHdrProgDlg->get_ParentHwnd());



	return 0L;

}

BOOL InterestedAttachExist(AttachVector &vecAttachments)

{

	AttachVector::iterator iterAttach;

	EnumODType attachType;



	for(iterAttach=vecAttachments.begin();iterAttach!=vecAttachments.end();iterAttach++)

	{

		attachType=CODHDInspectorFactory::GetAttachType((*iterAttach).first.c_str());

		if(attachType==ODTYPE_WORD||attachType==ODTYPE_PPT||attachType==ODTYPE_EXCEL)

		{

			return TRUE;

		}



	}

	

	return FALSE;



}



BOOL InterestedFileExist(HDRFileList &hdrFileList)

{

	HDRFileList::iterator iterFile;

	EnumODType fileType;



	for(iterFile = hdrFileList.begin(); iterFile != hdrFileList.end(); iterFile++)

	{

		fileType = CODHDInspectorFactory::GetAttachType((*iterFile)->m_strFileName);

		if(fileType==ODTYPE_WORD||fileType==ODTYPE_PPT||fileType==ODTYPE_EXCEL)

		{

			return TRUE;

		}



	}

	

	return FALSE;



}



long HDRObligation(HWND hDialogParentWnd, RecipientVector &vecRecipients,

                   AttachVector &vecAttachments, HelpUrlVector &vecHelpUrls,VARIANT_BOOL isWordMail)

{

	BOOL		bCancelByProgDlg=FALSE;

	AttachVector::iterator iterAttach;

	AttachVector::size_type nAttachCount=vecAttachments.size();

	if(nAttachCount==0||!InterestedAttachExist(vecAttachments))

		return TRUE;



	std::vector<CODHDInspector *> inspectors;

	CHdrProgDlg	progDlg;

	progDlg.put_ParentHwnd(hDialogParentWnd);

	progDlg.put_ProgRange((int)nAttachCount*MAX_HDRDLG_PROGRESS);

	HANDLE hPrograss=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunProgressWnd, &progDlg, 0, NULL);

	if(hPrograss == NULL)

		return FALSE;

	progDlg.SetInspectItem(L"Begin to inspect hidden data...");

	for(iterAttach=vecAttachments.begin();iterAttach!=vecAttachments.end();iterAttach++)

	{

		if(progDlg.get_Canceled())

		{

			bCancelByProgDlg=TRUE;

			break;

		}

		CODHDInspector *pInspector=CODHDInspectorFactory::CreateODHDInspector((*iterAttach).first,&progDlg);

		if(pInspector)

		{

			long lSize=0,lIndex=0;

			ODHDSTATUS	odhdStatus=INSPECT_NONE,retStatus=INSPECT_NONE;

			pInspector->SetProgDlg(&progDlg);

			pInspector->SetProgTitle((*iterAttach).first.c_str());
			_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
			if(pInspector->Inspect((*iterAttach).first,(*iterAttach).second)==FALSE)

			{

				bCancelByProgDlg=TRUE;

				progDlg.CloseProgWnd();

				delete pInspector;

				break;

			}

			else

			{

				pInspector->PrintInspectResult();

				lSize=pInspector->GetSize();

				for(lIndex=0;lIndex<lSize;lIndex++)

				{

					retStatus=pInspector->GetCatStatus(lIndex);

					if(retStatus==INSPECT_HAVE||retStatus==INSPECT_FAILED)

						odhdStatus=INSPECT_HAVE;

				}

				if(odhdStatus==INSPECT_HAVE)

					inspectors.push_back(pInspector);

				else

					delete pInspector;

			}

			

		}

	}

	if(progDlg.get_Canceled())

		bCancelByProgDlg=TRUE;

	

	//MSOAppInstance::Instance()->ReleaseAppInstance();

	LRESULT	lRet=IDOK;

	if(bCancelByProgDlg==FALSE)

	{

		progDlg.CloseProgWnd();

		if(inspectors.size())

		{
			CHdrAssistDlg hdrDlg(&inspectors, std::wstring(HDR_DLG_INFO), &vecHelpUrls, 0xFFFFFFFF);

            hdrDlg.AddHdrAttachData();

            lRet = hdrDlg.DoModal(hDialogParentWnd/*::GetForegroundWindow()*/);

		}

	}

	else

		lRet=IDCANCEL;

	MSOAppInstance::Instance()->ReleaseAppInstance();



	if(hPrograss)

	{

		CloseHandle(hPrograss);

	}



	std::vector<CODHDInspector *>::size_type iIndex=0;

	for(iIndex;iIndex<inspectors.size();iIndex++)

	{

		delete inspectors[iIndex];

	}

	if(lRet==2)//

		return FALSE;

	else

		return TRUE;



}



ODHD_API HRESULT RemoveHiddenData(HWND hParentWnd, HDRFileList &hdrFileList, std::wstring &strMsg, std::vector<std::wstring> &vecHelpUrls, LONG lBtnVisible, BOOL *pbCanceled)

{

	HRESULT hr = S_OK;



	BOOL bCancelByProgDlg = FALSE;

	HDRFileList::iterator iterFile;

	HDRFileList::size_type nFileCount = hdrFileList.size();

	if(nFileCount==0)

		return hr;

#ifdef WSO2K3
	DWORD dwHalfPart=lBtnVisible&0xFF0303;
	DWORD dwWordHalfPart=lBtnVisible&0x18;
	DWORD dwExcelHalfPart=lBtnVisible&0xF800;
	lBtnVisible=dwHalfPart|(dwWordHalfPart>>1)|(dwExcelHalfPart>>1);
#endif
#ifdef WSO2K3

	for(iterFile = hdrFileList.begin(); iterFile != hdrFileList.end(); iterFile++)

	{

		HDRFile *pHDRFile = (*iterFile);

		DWORD dwHalfPart=pHDRFile->m_dwRemoveButton&0x03;
		DWORD dwShiftHalfPart=pHDRFile->m_dwRemoveButton&0xF8;
		pHDRFile->m_dwRemoveButton=dwHalfPart|(dwShiftHalfPart>>1);


		if (pHDRFile == NULL || pHDRFile->m_dwNeedDetectCatType == 0)

			continue;



		if (CODHDUtilities::IsOffice2007File(pHDRFile->m_strFileName.c_str()))

		{

			//std::wstring strMsg = L"Policy Assistant cannot scan attached Office 2007 document ";

			//strMsg += pHDRFile->m_strFileName;

			//strMsg += L". Please open the document manually and use File | Save As to save it in Office 2003 format before reattaching the document.";



			std::wstring strMsg = L"";

			CODHDUtilities::ReplaceFileName(g_wstrNoSupportOffice2K7, pHDRFile->m_strFileName, strMsg);



			MessageBoxW(hParentWnd, strMsg.c_str(), L"Hidden Data Removal", MB_OK | MB_ICONWARNING);

			if (pbCanceled != NULL)

			{

				*pbCanceled = TRUE;

			}

			return hr;

		}

	}

#endif



	if (!InterestedFileExist(hdrFileList))

		return hr;



	std::vector<CODHDInspector *> inspectors;

	CHdrProgDlg	progDlg;

	progDlg.put_ParentHwnd(hParentWnd);

	progDlg.put_ProgRange((int)nFileCount*MAX_HDRDLG_PROGRESS);

	HANDLE hPrograss=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunProgressWnd, &progDlg, 0, NULL);

	if(hPrograss == NULL)

		return E_FAIL;

	progDlg.SetInspectItem(L"Begin to inspect hidden data...");

	for(iterFile = hdrFileList.begin(); iterFile != hdrFileList.end(); iterFile++)

	{

		HDRFile *pHDRFile = (*iterFile);

		if(progDlg.get_Canceled())

		{

			bCancelByProgDlg=TRUE;

			break;

		}

		if (pHDRFile == NULL || pHDRFile->m_dwNeedDetectCatType == 0)

			continue;

		CODHDInspector *pInspector=CODHDInspectorFactory::CreateODHDInspector(pHDRFile->m_strFileName,&progDlg);

		if(pInspector)

		{

			long lSize=0,lIndex=0;

			ODHDSTATUS	odhdStatus=INSPECT_NONE,retStatus=INSPECT_NONE;

			pInspector->SetProgDlg(&progDlg);

			pInspector->SetProgTitle(pHDRFile->m_strFileName.c_str());

			pInspector->SetHDRFile(pHDRFile);
			_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
			if(pInspector->Inspect(pHDRFile->m_strFileName, pHDRFile->m_strFilePath)==FALSE)

			{

				bCancelByProgDlg=TRUE;

				progDlg.CloseProgWnd();

				delete pInspector;

				break;

			}

			else

			{

				pInspector->PrintInspectResult();

				lSize=pInspector->GetSize();

				for(lIndex=0;lIndex<lSize;lIndex++)

				{

					if(!pInspector->GetFilter(ODHDCATEGORY(lIndex)))

					{

						retStatus=pInspector->GetCatStatus(lIndex);

						if(retStatus==INSPECT_HAVE||retStatus==INSPECT_FAILED)

							odhdStatus=INSPECT_HAVE;

					}

				}

				if(odhdStatus==INSPECT_HAVE)

					inspectors.push_back(pInspector);

				else

					delete pInspector;

			}

			

		}

	}

	if(progDlg.get_Canceled())

		bCancelByProgDlg=TRUE;

	

	LRESULT	lRet=IDOK;

	if(bCancelByProgDlg==FALSE)

	{

		progDlg.CloseProgWnd();

		if(inspectors.size())

		{

            CHdrAssistDlg hdrDlg(&inspectors, strMsg, &vecHelpUrls, lBtnVisible);

            hdrDlg.AddHdrAttachData();

            lRet = hdrDlg.DoModal(hParentWnd);

			DP((L"Assitant Dialog return %d", lRet));

		}

	}

	else

		lRet=IDCANCEL;

	MSOAppInstance::Instance()->ReleaseAppInstance();



	if(hPrograss)

	{

		CloseHandle(hPrograss);

	}



	std::vector<CODHDInspector *>::size_type iIndex=0;

	for(iIndex;iIndex<inspectors.size();iIndex++)

	{

		delete inspectors[iIndex];

	}



	if (pbCanceled != NULL)

	{

		*pbCanceled = FALSE;

		if (lRet == IDCANCEL)

			*pbCanceled = TRUE;

	}



	return hr;

}



void CODHDInspector::PrintInspectResult()

{

#ifdef _DEBUG

	long iCatIndex=0;

	std::wstring strTitle,strResult,strFilePath;

	long iCatCount=GetSize();

	strFilePath=GetFilePath();

	DP((L"%s\n",strFilePath.c_str()));

	

	for(iCatIndex=0;iCatIndex<iCatCount;iCatIndex++)

	{

		strTitle=GetTitle(iCatIndex);

		strResult=GetResult(iCatIndex);

		DP((L"\tTitle :%s\n",strTitle.c_str()));

		DP((L"\tResult:%s\n",strResult.c_str()));



	}



#endif

	return;



}

long CODHDInspector::GetContent(ODHDCATEGORY odhdType,ODHDITEM odhdItem,std::wstring &strContent)

{

	return TRUE;

}



long CODHDInspector::SetContent(ODHDCATEGORY odhdType,ODHDITEM odhdItem,std::wstring &strContent)

{

	return TRUE;

}

void CODHDInspector::GetTitle(ODHDCATEGORY odhdCat,std::wstring &strTitle)

{

	CODHDCategory* pCat= m_pCatManager->GetHDCategory(odhdCat);

	if(pCat)

		strTitle=pCat->GetCatTitle();

	return ;

}

std::wstring	CODHDInspector::GetTitle(ODHDCATEGORY odhdCat)

{

	std::wstring strResult;

	GetTitle(odhdCat,strResult);

	return strResult;

}

std::wstring	CODHDInspector::GetTitle(int odhdCat)

{

	std::wstring strResult;

	GetTitle((ODHDCATEGORY)odhdCat,strResult);

	return strResult;

}



void CODHDInspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)

{

	CODHDCategory* pCat= m_pCatManager->GetHDCategory(odhdCat);

	if(pCat)

	{

#ifdef WSO2K3

		pCat->GetItemsResult(strResult);



#else



		if(odhdCat>1)

			pCat->GetItemsResult2K7(strResult,1);

		else

			pCat->GetItemsResult2K7(strResult);

#endif

	}



}



std::wstring	CODHDInspector::GetResult(ODHDCATEGORY odhdCat)

{

	std::wstring strResult;

	GetResult(odhdCat,strResult);

	return strResult;

}

std::wstring	CODHDInspector::GetResult(int odhdCat)

{

	std::wstring strResult;

	GetResult((ODHDCATEGORY)odhdCat,strResult);

	return strResult;

}

ODHDSTATUS	CODHDInspector::GetCatStatus(ODHDCATEGORY odhdCat)

{

	CODHDCategory* pCat= m_pCatManager->GetHDCategory(odhdCat);

	if(pCat)

		return pCat->GetCatStatus();

	else

		return INSPECT_NONE;

}

ODHDSTATUS	CODHDInspector::GetCatStatus(int odhdCat)

{

	return GetCatStatus((ODHDCATEGORY)odhdCat);

}

#ifndef WSO2K3

BOOL		CODHDInspector::InspectorExist(Office::DocumentInspectors* spInspectors,int iInspectorIndex,TCHAR *szInspectorName)

{

	HRESULT			hr=S_OK;

	int nInspCount = 0;

	hr = spInspectors->get_Count(&nInspCount);

	if(SUCCEEDED(hr) && nInspCount>0&&nInspCount>=iInspectorIndex)

	{

		CComPtr<Office::DocumentInspector> spInspector=NULL;

		hr = spInspectors->get_Item(iInspectorIndex, &spInspector);

		if(SUCCEEDED(hr) &&spInspector)

		{

			CComBSTR              name;

			hr = spInspector->get_Name(&name); 

			if(FAILED(hr))

				return FALSE;

			if(_tcsstr(name, szInspectorName))

				return TRUE;

		}

	}

	return FALSE;



}

BOOL		CODHDInspector::RemoveByInspector(Office::DocumentInspectors* spInspectors,ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

{

	ODHDSTATUS		retStatus=INSPECT_REMOVED;

	HRESULT			hr=S_OK;

	BOOL	bRet=FALSE;

	int nInspCount = 0;

	hr = spInspectors->get_Count(&nInspCount);

	if(SUCCEEDED(hr) && nInspCount>0&&nInspCount>=odhdCat-1)

	{

		CComPtr<Office::DocumentInspector> spInspector=NULL;

		hr = spInspectors->get_Item(odhdCat-1, &spInspector);

		if(SUCCEEDED(hr) &&spInspector)

		{

			CComBSTR              name;

			CComBSTR              result;

			MsoDocInspectorStatus status;

			hr = spInspector->get_Name(&name);

			if(FAILED(hr))

				return bRet;

			hr=spInspector->Fix(&status, &result);

			if(SUCCEEDED(hr))

			{

				if(msoDocInspectorStatusIssueFound==status)

					retStatus=INSPECT_HAVE;

				if(msoDocInspectorStatusError==status)

					retStatus=INSPECT_FAILED;

				if(msoDocInspectorStatusDocOk==status)

					bRet=TRUE;

				if(wcsstr(result, L"did not complete successfully"))

					retStatus=INSPECT_FAILED;

			}

			RecordInspectResult(odhdCat,odhdItem,retStatus,name,result);

		}

	}

	return bRet;

}

BOOL		CODHDInspector::RemoveByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

{

	ODHDSTATUS		retStatus=INSPECT_REMOVED;

	HRESULT			hr=S_OK;

	BOOL	bRet=FALSE;

	int nInspCount = 0;

	if(iInspectorIndex<=0)

	{

		DP((L"iInspectorIndex<=0")) ;

		return TRUE;

	}

	hr = spInspectors->get_Count(&nInspCount);

	if(SUCCEEDED(hr) && nInspCount>0&&nInspCount>=iInspectorIndex)

	{

		CComPtr<Office::DocumentInspector> spInspector=NULL;

		hr = spInspectors->get_Item(iInspectorIndex, &spInspector);

		if(SUCCEEDED(hr) &&spInspector)

		{

			CComBSTR              name;

			CComBSTR              result;

			MsoDocInspectorStatus status;

			hr = spInspector->get_Name(&name);

			if(FAILED(hr))

				return bRet;

			hr=spInspector->Fix(&status, &result);

			if(SUCCEEDED(hr))

			{

				if(msoDocInspectorStatusIssueFound==status)

					retStatus=INSPECT_HAVE;

				if(msoDocInspectorStatusError==status)

					retStatus=INSPECT_FAILED;

				if(msoDocInspectorStatusDocOk==status)

				{

					DP((L"Fix status:[%d]",status)) ;

					bRet=TRUE;

				}

				if(wcsstr(result, L"did not complete successfully"))

				{

					DP((L"not complete ")) ;

					retStatus=INSPECT_FAILED;

				}

			}

			RecordInspectResult(odhdCat,odhdItem,retStatus,name,result);

		}

	}

	return bRet;



}

ODHDSTATUS		CODHDInspector::InspectByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD *dwInspectFail)

{

	ODHDSTATUS		retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	int nInspCount = 0;

	hr = spInspectors->get_Count(&nInspCount);

	if(SUCCEEDED(hr) && nInspCount>0&&nInspCount>=iInspectorIndex)

	{

		CComPtr<Office::DocumentInspector> spInspector=NULL;

		hr = spInspectors->get_Item(iInspectorIndex, &spInspector);

		if(SUCCEEDED(hr) &&spInspector)

		{

			CComBSTR              name;

			CComBSTR              result;

			MsoDocInspectorStatus status;

			hr = spInspector->get_Name(&name);

			if(SUCCEEDED(hr))

			{

				SetProgTitle(name);

			}

			hr=spInspector->Inspect(&status, &result);

			if(SUCCEEDED(hr))

			{

				if(msoDocInspectorStatusIssueFound==status)

					retStatus=INSPECT_HAVE;

				if(msoDocInspectorStatusError==status)

				{

					if(wcsstr(result, L"This document was saved in a previous version of this Microsoft Office program with fast save enabled.\nTo inspect this document, you must save it first.")||

						wcsstr(result,L"This document is protected. To inspect this document, remove document protection, and then run the Document Inspector."))

					{

						retStatus=INSPECT_DETECTFAIL;

						if(dwInspectFail)*dwInspectFail=1;

					}



					else

						retStatus=INSPECT_FAILED;

				}

				if(wcsstr(result, L"did not complete successfully"))

					retStatus=INSPECT_FAILED;

			}
			else
			{
				if(msoDocInspectorStatusError==status)
				{
					retStatus=INSPECT_FAILED;
				}
			}

			RecordInspectResult(odhdCat,

				odhdItem,

				retStatus,

				name,

				result);

		}

	}

	return retStatus;

}





ODHDSTATUS		CODHDInspector::InspectByInspector(Office::DocumentInspectors* spInspectors,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD *dwInspectFail)

{

	ODHDSTATUS		retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	int nInspCount = 0;

	hr = spInspectors->get_Count(&nInspCount);

	if(SUCCEEDED(hr) && nInspCount>0&&nInspCount>=odhdCat-1)

	{

		CComPtr<Office::DocumentInspector> spInspector=NULL;

		hr = spInspectors->get_Item(odhdCat-1, &spInspector);

		if(SUCCEEDED(hr) &&spInspector)

		{

			CComBSTR              name;

			CComBSTR              result;

			MsoDocInspectorStatus status;

			hr = spInspector->get_Name(&name);

			if(SUCCEEDED(hr))

			{

				SetProgTitle(name);

			}

			hr=spInspector->Inspect(&status, &result);

			if(SUCCEEDED(hr))

			{

				if(msoDocInspectorStatusIssueFound==status)

					retStatus=INSPECT_HAVE;

				if(msoDocInspectorStatusError==status)

				{

					if(wcsstr(result, L"This document was saved in a previous version of this Microsoft Office program with fast save enabled.\nTo inspect this document, you must save it first.")||

						wcsstr(result,L"This document is protected. To inspect this document, remove document protection, and then run the Document Inspector."))

					{

						retStatus=INSPECT_DETECTFAIL;

						if(dwInspectFail)*dwInspectFail=1;

					}



					else

						retStatus=INSPECT_FAILED;

				}

				if(wcsstr(result, L"did not complete successfully"))

					retStatus=INSPECT_FAILED;

			}
			else
			{
				if(msoDocInspectorStatusError==status)
				{
					retStatus=INSPECT_FAILED;
				}
			}

				RecordInspectResult(odhdCat,

						odhdItem,

						retStatus,

						name,

						result);

		}

	}

	return retStatus;

}

#endif //WSO2K7

BOOL CODHDInspector::HasDocumentProperties(CComPtr<IDispatch> pDisp)

{

	BOOL    bRet   = FALSE;

    HRESULT hr     = S_OK;

    LONG    lCount = 0;

    VARIANT vtResult, vtIndex;

    int     i = 0;



    hr = DispatchCallWraper(pDisp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtResult, L"Count", 0);

    lCount = vtResult.lVal;

	if(lCount>0)

    {

        DP((L"\nOffice Document:Find BuiltInDocumentProperties!\n"));

        vtIndex.vt = VT_INT;

        for(i=0; i<lCount; i++)

        {

            vtIndex.intVal = i+1;

            hr = DispatchCallWraper(pDisp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtResult,L"Item", 1, vtIndex);

            if(SUCCEEDED(hr) && vtResult.vt==VT_DISPATCH && vtResult.pdispVal)

            {

                CComPtr<IDispatch> pDispProp = vtResult.pdispVal;

                VARIANT    vtPropResult;

                VARIANT    vtTypeResult;

                WCHAR      wzProp[1024]; memset(wzProp, 0, sizeof(wzProp));

                hr = DispatchCallWraper(pDispProp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtPropResult,L"Name", 0);

                if (SUCCEEDED(hr))

                {

                    BOOL bNeedCheck = FALSE;

                    wcsncpy_s(wzProp, 1024, vtPropResult.bstrVal,_TRUNCATE);

					bNeedCheck=IsValidPropertyName(vtPropResult.bstrVal);



                    if(bNeedCheck)

                    {

                        WCHAR wzValue[513]; memset(wzValue, 0, sizeof(wzValue));

                        wcsncat_s(wzProp, 1024, L" : ", _TRUNCATE);

                        hr = DispatchCallWraper(pDispProp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtPropResult,L"Value", 0);

                        if(SUCCEEDED(hr))

                        {

                            hr = DispatchCallWraper(pDispProp,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtTypeResult, L"Type", 0);

                            if (SUCCEEDED(hr))

                            {

                                switch(vtTypeResult.intVal)

                                {

                                case msoPropertyTypeNumber:

                                    _snwprintf_s(wzValue, 512, _TRUNCATE, L"%d", vtPropResult.intVal);

                                    break;

                                case msoPropertyTypeBoolean:

                                    _snwprintf_s(wzValue, 512, _TRUNCATE, L"%s", vtPropResult.boolVal?L"True":L"False");

                                    break;

                                case msoPropertyTypeDate:

                                    {

                                        CComVariant varDate(vtPropResult);

                                        varDate.ChangeType(VT_BSTR, NULL);

                                        _snwprintf_s(wzValue, 512, _TRUNCATE, L"%s", varDate.bstrVal);



                                    }

                                    break;

                                case msoPropertyTypeString:

                                    if(vtPropResult.bstrVal) _snwprintf_s(wzValue, 512, _TRUNCATE, L"%s", vtPropResult.bstrVal);

                                    break;

                                case msoPropertyTypeFloat:

                                    _snwprintf_s(wzValue, 512, _TRUNCATE, L"%f", vtPropResult.fltVal);

                                    break;

                                default:

                                    break;

                                }

                                wcsncat_s(wzProp, 1024, wzValue, _TRUNCATE);

                            }

                        }

                        if(0 != wzValue[0])

							bRet = TRUE;

                    }

                    wcsncat_s(wzProp, 1024, L"\n", _TRUNCATE);

                    //DP((wzProp));

                }

                else

                {

                    DP((L"Fail to get property!\n"));

                }



            }

        }

        DP((L"\n"));

    }



    return bRet;



}

HRESULT DispatchCallWraper(CComPtr<IDispatch> pDisp,int dispatchType, VARIANT *pvResult,  LPOLESTR propName, int cArgs...)

{


    // Variables used...

    DISPPARAMS  dispParams  = { NULL, NULL, 0, 0 };

    DISPID      dispidNamed = DISPID_PROPERTYPUT;

    DISPID      dispID;

    HRESULT     hr;

    char        szName[200];



    // Convert down to ANSI

    WideCharToMultiByte(CP_ACP, 0, propName, -1, szName, 256, NULL, NULL);



    // Get DISPID for name passed...

    hr = pDisp->GetIDsOfNames(IID_NULL, &propName, 1, LOCALE_USER_DEFAULT, &dispID);

    if(FAILED(hr))

    {

        return hr;

    }



    // Allocate memory for arguments...

    VARIANT *pArgs = new VARIANT[cArgs+1];



    // Extract arguments...

    // Begin variable-argument list...

    va_list marker;

    va_start(marker, cArgs);


    for(int i=0; i<cArgs; i++) {

        pArgs[i] = va_arg(marker, VARIANT);

    }

    // End variable-argument section...

    va_end(marker);


    // Build DISPPARAMS

    dispParams.cArgs = cArgs;

    dispParams.rgvarg = pArgs;



    // Handle special-case for property-puts!

    if(dispatchType & DISPATCH_PROPERTYPUT)

    {

        dispParams.cNamedArgs = 1;

        dispParams.rgdispidNamedArgs = &dispidNamed;

    }



    // Make the call!

    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, dispatchType, &dispParams, pvResult, NULL, NULL);

    if(FAILED(hr))

    {

        return hr;

    }

    delete [] pArgs;

    return hr;

}



/*****************************************************************************/

/* CLASS HdrAction                                                           */

/*****************************************************************************/



void HdrAction::OnRemove(int nAttachmentIndex, int nItemIndex,

						 std::wstring& strBody, std::wstring& strNote,

						 int* pnStatus,LPVOID pContext)

{

    const CONTEXT* pHdrActionContext = (LPCONTEXT) pContext;

    const std::vector<CODHDInspector *>* pInspectors =

        pHdrActionContext->pInspectors;



    *pnStatus = (*pInspectors)[nAttachmentIndex]->Remove(nItemIndex) ?

        REMOVEOK : REMOVEFAIL;

    strBody = (*pInspectors)[nAttachmentIndex]->GetResult(nItemIndex);

	if((*pInspectors)[nAttachmentIndex]->GetNote(strNote)==FALSE)

		strNote=L"";

}



void HdrAction::OnNext(int nAttachmentIndex, std::wstring& strAttach,

                       std::wstring& strHelpUrl, LPVOID pContext)

{

    const CONTEXT* pHdrActionContext = (LPCONTEXT) pContext;

    const std::vector<CODHDInspector *>* pInspectors =

        pHdrActionContext->pInspectors;

    const HelpUrlVector* pvecHelpUrls = pHdrActionContext->pvecHelpUrls;



    int i = nAttachmentIndex + 1;



    strAttach = (*pInspectors)[i]->GetFilePath();



    for (int j = 0; j < (*pInspectors)[i]->GetSize(); j++)

    {

        m_vData.push_back(YLIB::smart_ptr<HdrData>(new HdrData(

            (*pInspectors)[i]->GetTitle(j).c_str(),

            (*pInspectors)[i]->GetResult(j).c_str(),

            (*pInspectors)[i]->GetCatStatus(j))));

    }



    strHelpUrl = (*pvecHelpUrls)[i];

}

//////////////////////////////////////////ntapi///////////////////////////////////////////////////
int WINAPI NumOfSections(LPVOID lpFile)
{
    return (int)(((PIMAGE_FILE_HEADER)PEFHDROFFSET(lpFile))->NumberOfSections);
}

LPVOID  WINAPI ImageDirectoryOffset (
                                     LPVOID    lpFile,
                                     DWORD     dwIMAGE_DIRECTORY)
{
    PIMAGE_OPTIONAL_HEADER   poh=0;
    PIMAGE_SECTION_HEADER    psh=0;
    int                      nSections = NumOfSections (lpFile);
    int                      i = 0;
    LPVOID                   VAImageDir=0;

    /* Retrieve offsets to optional and section headers. */
    poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (lpFile);
    psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (lpFile);

    /* Must be 0 thru (NumberOfRvaAndSizes-1). */
    if (NULL==poh || NULL==psh || dwIMAGE_DIRECTORY >= poh->NumberOfRvaAndSizes)
        return NULL;

    /* Locate image directory's relative virtual address. */
    VAImageDir = (LPVOID)poh->DataDirectory[dwIMAGE_DIRECTORY].VirtualAddress;

    /* Locate section containing image directory. */
    while (i++<nSections)
    {
        if (psh->VirtualAddress <= (DWORD)VAImageDir
            && (psh->VirtualAddress + psh->SizeOfRawData) > (DWORD)VAImageDir)
            break;
        psh++;
    }

    if (i > nSections)
        return NULL;

    /* Return image import directory offset. */
    return (LPVOID)( ((int)lpFile + (int)VAImageDir - psh->VirtualAddress) + (int)psh->PointerToRawData );
}

BOOL    WINAPI GetSectionHdrByAddress (
                                       LPVOID                   lpFile,
                                       IMAGE_SECTION_HEADER     *sh,
                                       DWORD                    addr)
{
    PIMAGE_SECTION_HEADER    psh=0;
    int                      nSections = NumOfSections (lpFile);
    int                      i=0;

    if ((psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (lpFile)) != NULL)
    {
        /* find the section by name */
        for (i=0; i<nSections; i++)
        {
            if (addr >= psh->VirtualAddress && addr < psh->VirtualAddress + psh->SizeOfRawData)
            {
                /* copy data to header */
                memcpy ((LPVOID)sh,
                    (LPVOID)psh,
                    sizeof (IMAGE_SECTION_HEADER));
                return TRUE;
            }
            else
                psh++;
        }
    }

    return FALSE;
}

DWORD
WINAPI
FindProcOffset2FileBase(
                        IN PVOID pFile,
                        IN PCHAR pchName
                        )
{
    PIMAGE_OPTIONAL_HEADER     poh=0;
    PIMAGE_EXPORT_DIRECTORY    ped=0;
    IMAGE_SECTION_HEADER       sh;
    DWORD                      VAImageDir=0;
    PULONG                     pNames=0, pCnt=0, pFunctions=0;
    PSHORT					   pOrdinals = 0;
    ULONG					   ord=0;
    PCHAR                      pOffset=0;
    int                        i=0, nCnt=0;
    char                       *pSrc=0;
    DWORD					   dwOffset = 0;

    memset(&sh, 0, sizeof(IMAGE_SECTION_HEADER));

    ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryOffset(pFile, IMAGE_DIRECTORY_ENTRY_EXPORT);
    if(NULL == ped) return 0;
    poh = (PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET (pFile);
    if(NULL == poh) return 0;
    VAImageDir = poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if(NULL == VAImageDir) return 0;

    if (FALSE == GetSectionHdrByAddress (pFile, &sh, VAImageDir)) return 0;
    pOffset = (PCHAR)pFile + (sh.PointerToRawData -  sh.VirtualAddress);
    pNames = (PULONG)(pOffset + (DWORD)ped->AddressOfNames);
    pFunctions = (PULONG)(pOffset + (DWORD)ped->AddressOfFunctions);
    pOrdinals = (PSHORT)(pOffset + (DWORD)ped->AddressOfNameOrdinals);

    nCnt = 1;
    for (i=0, pCnt = pNames; i<(int)ped->NumberOfNames; i++)
    {
        ord = pOrdinals[i];
        pSrc = (pOffset + *pCnt++);
        if(pSrc && 0==strcmp(pSrc, pchName))
        {
            dwOffset = (DWORD)pOffset + (DWORD)pFunctions[ord];
            break;
        }
        if (pSrc) nCnt += (int)strlen(pSrc)+1;
    }

    if(dwOffset > (DWORD)pFile) dwOffset = dwOffset - (DWORD)pFile;
    else dwOffset = 0;

    return dwOffset;
}

DWORD
WINAPI
FindProcOffset2ModuleBase(
                          IN PVOID pFile,
                          IN PCHAR pchName
                          )
{
	DWORD                      *pNames=0, *pCnt=0, *pFunctions=0;
	PSHORT					   pOrdinals = 0;
	ULONG					   ord=0;
	char                       *pOffset=0;
	int                        i=0, nCnt=0;
	char                       *pSrc=0;
	DWORD					   dwOffset = 0;

	PIMAGE_SECTION_HEADER psh = NULL;
	ULONG ulSize = 0;
	PIMAGE_EXPORT_DIRECTORY ped = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx (pFile,FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ulSize, &psh);
	if ( NULL == ped )
	{
		return 0;
	}

	pOffset = (char *)pFile + psh->PointerToRawData - psh->VirtualAddress;
	pNames = (DWORD *)(pOffset + (DWORD)ped->AddressOfNames);
	pFunctions = (DWORD *)(pOffset + (DWORD)ped->AddressOfFunctions);
	pOrdinals = (short *)(pOffset + (DWORD)ped->AddressOfNameOrdinals);

	nCnt = 1;
	for (i=0, pCnt = pNames; i<(int)ped->NumberOfNames; i++)
	{
		ord = pOrdinals[i];
		pSrc = (pOffset + *pCnt++);
		if(pSrc && 0==strcmp(pSrc, pchName))
		{
			dwOffset = pFunctions[ord];
			break;
		}
		if (pSrc) nCnt += (int)strlen(pSrc)+1;
	}
	return dwOffset;
}

PVOID WINAPI MyGetProcAddress(HMODULE hMod, LPCSTR pczFuncName)
{
    PVOID  pFunc = NULL;
    DWORD  dwFuncOffset = 0;
    WCHAR  wzPath[MAX_PATH+1]; memset(wzPath, 0, sizeof(wzPath));
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap  = NULL;
    DWORD  dwFileSize = 0;
    BYTE*  lpMod      = 0;

    if(NULL==pczFuncName) goto _exit;
    if(0 == GetModuleFileNameW(hMod, wzPath, MAX_PATH)) goto _exit;

    hFile = CreateFileW(wzPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) goto _exit;

    dwFileSize = GetFileSize(hFile, NULL);
    if(0 == dwFileSize) goto _exit;

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL);
    if(0 == hMap) goto _exit;

    lpMod = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, dwFileSize);
    if(0 == lpMod) goto _exit;

    dwFuncOffset = FindProcOffset2ModuleBase(lpMod, "LoadLibraryW");
    if(0 == dwFuncOffset) goto _exit;

    pFunc = (PVOID)((DWORD)hMod + (DWORD)dwFuncOffset);

_exit:
    if(0 != lpMod) UnmapViewOfFile(lpMod); lpMod = 0;
    if(0 != hMap) CloseHandle(hMap); hMap = 0;
    if(INVALID_HANDLE_VALUE != hFile) CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;
    return pFunc;
}


int CODHDInspector::GetInstalledOfficeVersion(void)

{

	HKEY    hKeyOffice   = NULL;

	LONG    lResult       = 0;

	WCHAR   wzKeyName[MAX_PATH+1];memset(wzKeyName, 0, sizeof(wzKeyName));

	WCHAR   wzValue[MAX_PATH+1];memset(wzValue, 0, sizeof(wzValue));

	DWORD dwValueLen = MAX_PATH*sizeof(WCHAR);

	DWORD dwType = REG_SZ;



	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"Word.Application\\CurVer");

	lResult = RegOpenKeyExW(HKEY_CLASSES_ROOT, wzKeyName, 0, KEY_READ, &hKeyOffice);

	if(ERROR_SUCCESS == lResult)

	{

		dwType = REG_SZ;

		dwValueLen = MAX_PATH*sizeof(WCHAR);

		lResult = RegQueryValueExW(hKeyOffice, L"", 0, &dwType, (LPBYTE)wzValue, &dwValueLen);

		if (ERROR_SUCCESS == lResult && dwValueLen != 0)

		{

			DP((L"Current Word version: %s", wzValue));

			if (!_wcsicmp(wzValue, L"Word.Application.16"))
			{

				DP((L"EnOfficeVersion_2016!\n"));

				return EnOfficeVersion_2016;

			}

			if (!_wcsicmp(wzValue, L"Word.Application.15"))
			{

				DP((L"EnOfficeVersion_2013!\n"));

				return EnOfficeVersion_2013;

			}
			else if (!_wcsicmp(wzValue, L"Word.Application.14"))

			{

				DP((L"EnOfficeVersion_2010!\n"));

				return EnOfficeVersion_2010;

			}

			else if (!_wcsicmp(wzValue, L"Word.Application.12"))

			{

				DP((L"EnOfficeVersion_2007!\n"));

				return EnOfficeVersion_2007;

			}

			else if (!_wcsicmp(wzValue, L"Word.Application.11"))

			{

				DP((L"EnOfficeVersion_2003!\n"));

				return EnOfficeVersion_2003;

			}

		}

	}





	return EnOfficeVersion_Other;

}

