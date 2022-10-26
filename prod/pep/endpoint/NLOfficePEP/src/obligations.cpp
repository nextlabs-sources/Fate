#include "stdafx.h"
#include "utils.h"
#include "NLOfficePEP_Comm.h"

#pragma warning(push)
#pragma warning(disable:6273 4819 4996 4313)
#include "PALoad.h"
#pragma warning(pop)


#include "EnumWnd.h"
#define PPT_FILEPATH_LENGTH_IN_IE 90
#include "NLObMgr.h"

#pragma warning(push)
#pragma warning(disable:4819 4995)
#include "eframework\platform\cesdk.hpp"
#include "COverlayTool.h"
#pragma warning(pop)

#include "dllmain.h"

#include "obligations.h"


#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 


//
//	Encrypt
//
#ifdef _WIN64
static const wchar_t* g_pwchSeLibName = L"nl_sysenc_lib.dll";
#else
static const wchar_t* g_pwchSeLibName = L"nl_sysenc_lib32.dll";
#endif

static const char* g_pchIsEncryptFuncName = "SE_IsEncrypted";
static const char* g_pchEncryptFuncName = "SE_EncryptFile";
static const char* g_pchMarkEncryptedFiletFuncName = "SE_MarkFileAsDRMOneShot";

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLENCRYPT)
//////////////////////////////////////////////////////////////////////////

CNLEncrypt::CNLEncrypt(void) : m_bInit(false), m_hEncryptLib(NULL), m_pFuncIsEncrypted(NULL), m_pFuncEncryptFile(NULL), m_pFuncMarkEncryptedFile(NULL)
{
	SetInitFlag(false);
	wstring strCommonSELibPath = GetCommonComponentsDir() + g_pwchSeLibName;
	m_hEncryptLib = LoadLibrary(strCommonSELibPath.c_str());
	if (NULL != m_hEncryptLib)
	{
		m_pFuncIsEncrypted = (fnSE_IsEncrypted)GetProcAddress(m_hEncryptLib, g_pchIsEncryptFuncName);
		m_pFuncEncryptFile = (fnSE_EncryptFile)GetProcAddress(m_hEncryptLib, g_pchEncryptFuncName);
		m_pFuncMarkEncryptedFile = (fnSE_MarkFileAsDRMOneShot)GetProcAddress(m_hEncryptLib, g_pchMarkEncryptedFiletFuncName);
	}

	if (NULL != m_pFuncIsEncrypted && NULL != m_pFuncEncryptFile && NULL != m_pFuncMarkEncryptedFile)
	{
		SetInitFlag(true);
	}
}

CNLEncrypt::~CNLEncrypt(void)
{
	if (IsInited() && NULL == m_hEncryptLib)
	{
		FreeLibrary(m_hEncryptLib);
		m_hEncryptLib = NULL;
		SetInitFlag(false);
	}
}

bool CNLEncrypt::IsInited()
{
	return m_bInit;
}

void CNLEncrypt::SetInitFlag(const bool& bFlag)
{
	m_bInit = bFlag;
}

// The parameter bForSave: true it means save action, false save as action
bool CNLEncrypt::EncryptTempFile(const AppType& emOfficeAppType, const wstring& wstrFile, const bool bForSave)
{
	if (wstrFile.empty() || !IsLocalDriver(wstrFile))
	{
		return false;
	}

	bool bRet = false;
	wstring wstrFileDir = GetFilePath(wstrFile);

	switch (emOfficeAppType)
	{
	case kAppWord:
	{
								bRet = PreEncryptFile(wstrFileDir + L"*.tmp");
	}
		break;
	case kAppExcel:
	{
								 bRet = PreEncryptFile(wstrFileDir + L"*");
								 if (!bForSave)
								 {
									 // this is save as action, we need encrypt twice( test result )
									 bRet = PreEncryptFile(wstrFileDir + L"*");
								 }
	}
		break;
	case kAppPPT:
		if (bForSave)
		{
			bRet = PreEncryptFile(wstrFileDir + L"*");
			bRet = PreEncryptFile(wstrFileDir + L"*");
		}
		else
		{
			bRet = PreEncryptFile(wstrFile);
		}
		break;
	default:
		break;
	}
	return bRet;
}

bool CNLEncrypt::EncryptFile(const wstring& wstrPath)
{
	// here ,we only care the local file
	if (wstrPath.empty() || !IsLocalDriver(wstrPath) || !IsInited())
	{
		return false;
	}
	return m_pFuncEncryptFile(wstrPath.c_str()) ? true : false;
}

bool CNLEncrypt::IsEncryptedFile(const wstring& wstrPath)
{
	// here ,we only care the local file
	if (wstrPath.empty() || !IsLocalDriver(wstrPath) || !IsInited())
	{
		return false;
	}
	return m_pFuncIsEncrypted(wstrPath.c_str()) ? true : false;
}

bool CNLEncrypt::PreEncryptFile(const wstring& wstrPath)
{
	// here ,we only care the local file
	bool bRet = false;
	if (wstrPath.empty() || !IsLocalDriver(wstrPath) || !IsInited())
	{
		return bRet;
	}

	wstring wstrNewFile = L"";
	if (PathFileExists(wstrPath.c_str()))
	{
		wstrNewFile = wstrPath + L"_nextlabs_temp.nxtfile";
		if (!MoveFile(wstrPath.c_str(), wstrNewFile.c_str()))
		{
			wstrNewFile = L"";
		}
	}

	if (m_pFuncMarkEncryptedFile(wstrPath.c_str()))
	{
		if (!wstrNewFile.empty())
		{
			DeleteFileW(wstrNewFile.c_str());
		}
		bRet = true;
	}
	else if (!wstrNewFile.empty())
	{
		// here if we encrypt the file failed, we need revert whats we changed.
		MoveFile(wstrNewFile.c_str(), wstrPath.c_str());
	}
	return bRet;
}



//
//	FileTagging
//

////////////////////////////////library name//////////////////////////////////////////
#ifdef _M_IX86
#define NLPAFILETAGGING_MODULE_NAME L"pa_filetagging32.dll"
#else
#define NLPAFILETAGGING_MODULE_NAME L"pa_filetagging.dll"
#endif
////////////////////////////////function name//////////////////////////////////////////
static const char* g_szReleaseBufName = "ReleaseTagBuf";

////////////////////////////////buffer separator, used to prase tag buffer to tag vector//////////////////////////////////////////
#define NLBUFFERSEPARATOR L"\n"


//////////////////////////////////////////////////////////////////////////

CNLFileTagging::CNLFileTagging(void) : m_hModule(NULL), m_bInitFlag(false), m_pfuncReaseTagBuf(NULL)
{
	NLSetInitFlag(NLInitialize());
}

CNLFileTagging::~CNLFileTagging(void)
{
	NLUnInitialize();
	NLSetInitFlag(false);
}

CNLFileTagging& CNLFileTagging::NLGetInstance()
{
	static CNLFileTagging theFileTaggingIns;
	return theFileTaggingIns;
}

bool CNLFileTagging::NLInitialize()
{
	// check if load
	if (NULL != m_hModule)
	{
		NLPRINT_DEBUGLOG(L"file tagging has been load \n");
		return true;
	}

	// load library
	wstring wstrDllPath = GetCommonComponentsDir() + NLPAFILETAGGING_MODULE_NAME;
	m_hModule = ::LoadLibraryW(wstrDllPath.c_str());

	if (NULL == m_hModule)
	{
		NLPRINT_DEBUGLOG(L"file tagging load failed, wstrDllPath=%s \n", wstrDllPath.c_str());
		return false;
	}

	// Get process address
	m_pfuncReaseTagBuf = (PFUNC_RELEASETAGBUF)GetProcAddress(m_hModule, g_szReleaseBufName);

	bool bRet = NULL != m_pfuncReaseTagBuf;
	NLPRINT_DEBUGLOG(L"file tagging initialize result:[%s] \n", bRet ? L"TRUE" : L"FALSE");
	return bRet;
}

void CNLFileTagging::NLUnInitialize()
{
	if (NULL != m_hModule)
	{
        // fixed bugs that about office crash after do file tagging and close file (bug 36052,36054,36056)
		//::FreeLibrary(m_hModule);
	}
}

void CNLFileTagging::NLSetInitFlag(_In_ const bool& bFlag)
{
	m_bInitFlag = bFlag;
}

bool CNLFileTagging::NLGetInitFlag()
{
	return m_bInitFlag;
}

bool CNLFileTagging::NLGetVecStringPairFromBuf(_In_ const wchar_t* pwszBuf, _In_ const DWORD& dwLen, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	if (NULL == pwszBuf || dwLen < 1)
	{
		return false;
	}

	vector<wstring> vecNxlTags;
	boost::algorithm::split(vecNxlTags, pwszBuf, boost::algorithm::is_any_of(NLBUFFERSEPARATOR));

	if (0 == vecNxlTags.size() % 2)
	{
		for (size_t i = 0; i < vecNxlTags.size(); i += 2)
		{
			vecTagPair.push_back(pair<wstring, wstring>(vecNxlTags[i], vecNxlTags[i + 1]));
		}
	}
	return true;
}

bool CNLFileTagging::NLDoFileTagging(_In_ const wchar_t* pwchSrcFilePath, _In_ const wchar_t* pwchDesFilePath,
	_In_ nextlabs::Obligations& obs, _In_ const OfficeAction& emAction, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	NLPRINT_DEBUGLOG(L" The Parameters are: pwchSrcFilePath=%ls, pwchDesFilePath=%ls, obs=%p, emAction=%d \n", print_long_string(pwchSrcFilePath), print_long_string(pwchDesFilePath), &obs, emAction);
	// check parameter 
	if (NULL == pwchSrcFilePath)
	{
		NLPRINT_DEBUGLOG(L"the source file path is NULL \n");
		return false;
	}

	//check file tagging initialize flag
	if (!NLGetInitFlag())
	{
		NLPRINT_DEBUGLOG(L"the file tagging initialize failed \n");
		return false;
	}

	PA_Mngr::CPAMngr pam(NULL);
	pam.SetObligations(pwchSrcFilePath, pwchSrcFilePath, obs);

	// get obligation tag pairs (source,destination).
	PABase::ACTION theAction = kOA_COPY == emAction ? PABase::AT_COPY : PABase::AT_MOVE;

	wchar_t* pSrcTag = NULL;
	DWORD dwSrcLen = 0;

	wchar_t* pDstTag = NULL;
	DWORD dwDstLen = 0;

	//	LONG lRet = pam.DoFileTagging2WithErrHandling( m_hModule, NULL, &pSrcTag, dwSrcLen, &pDstTag, dwDstLen, theAction, TRUE, L"OK", NULL, false, NULL, NULL, NULL, NLPAPEP_OFFICE );

	LONG lRet = pam.DoFileTagging2(m_hModule, NULL, &pSrcTag, dwSrcLen, &pDstTag, dwDstLen, theAction, TRUE, L"OK", NULL, false);

	if (PA_SUCCESS != lRet)
	{
		NLPRINT_DEBUGLOG(L"do file tagging failed or user choose cancel button \n");
		return false;
	}

	if (NULL != pSrcTag)
	{
		NLGetVecStringPairFromBuf(pSrcTag, dwSrcLen, vecTagPair);
		m_pfuncReaseTagBuf(pSrcTag);
		pSrcTag = NULL;
	}

	vector<pair<wstring, wstring>> vecDesTagPair;
	if (NULL != pDstTag)
	{
		NLGetVecStringPairFromBuf(pDstTag, dwDstLen, vecDesTagPair);
		m_pfuncReaseTagBuf(pDstTag);
		pSrcTag = NULL;
	}
	vecTagPair.insert(vecTagPair.end(), vecDesTagPair.begin(), vecDesTagPair.end());
	return true;
}

//
// View Overlay
//


wstring g_strModulePath = L"";
HMODULE pa_labeling_mod = NULL;
extern wchar_t g_szLog[1024];
extern wchar_t g_strWinCaption[1025];
#define WM_OVPRINT	WM_USER+4
#define WM_OVPRINTCLOSE	WM_USER+5

CNLOvelayView::CNLOvelayView() :m_bSharePointExcelOnce(true)
{
	InitializeCriticalSection(&m_mapViewSec);
	InitializeCriticalSection(&m_vecViewSec);
	InitializeCriticalSection(&m_vecFilePathSec);
}
CNLOvelayView::~CNLOvelayView()
{
	DelOverlayPath();
	DeleteCriticalSection(&m_vecFilePathSec);
	DeleteCriticalSection(&m_vecViewSec);
	DeleteCriticalSection(&m_mapViewSec);
}
CNLOvelayView& CNLOvelayView::GetInstance()
{
	static CNLOvelayView theOV;

	return theOV;
}
void CNLOvelayView::AddCach(HWND hView, bool bUsed, const wstring& filepath)
{
	EnterCriticalSection(&m_mapViewSec);
	pair<bool, wstring> one(bUsed, filepath);
	m_mapViewUsed[hView] = one;
	LeaveCriticalSection(&m_mapViewSec);
}
void CNLOvelayView::AddView(HWND hView)
{
	::EnterCriticalSection(&m_vecViewSec);
	m_vechView.push_back(hView);
	::LeaveCriticalSection(&m_vecViewSec);
}
void CNLOvelayView::ClearCreatewindowCache(void)
{
	::EnterCriticalSection(&m_vecViewSec);
	m_vechView.clear();
	::LeaveCriticalSection(&m_vecViewSec);
}
bool CNLOvelayView::IsEmptyCreatewindowCache(void)
{
	bool bRet = false;
	::EnterCriticalSection(&m_vecViewSec);
	bRet = m_vechView.empty();
	::LeaveCriticalSection(&m_vecViewSec);
	return bRet;
}
void CNLOvelayView::GetDoOverlayHwnd(AppType AppType, vector<HWND> &vecHwnd)
{
	::EnterCriticalSection(&m_vecViewSec);
	if (AppType == kAppWord)
	{
		wchar_t lintitle[MAX_PATH + 1] = { 0 }, className[MAX_PATH + 1] = { 0 };
		for (vector<HWND>::iterator it = m_vechView.begin(); it != m_vechView.end(); it++)
		{
			GetClassName(*it, className, MAX_PATH);
			GetWindowText(*it, lintitle, MAX_PATH);
			if (wcscmp(className, L"_WwB") == 0 && boost::algorithm::icontains(lintitle, L"(Protected View)"))	//precondition:_WwB -> _WwG  order
			{
				vecHwnd.push_back(*it);
				break;
			}
			else if (wcscmp(className, L"_WwG") == 0)
			{
				vecHwnd.push_back(*it);
			}
		}
	}
	else if (AppType == kAppPPT)
	{
		wchar_t lintitle[MAX_PATH + 1] = { 0 }, className[MAX_PATH + 1] = { 0 };
		for (vector<HWND>::iterator it = m_vechView.begin(); it != m_vechView.end(); it++)
		{
			GetClassName(*it, className, MAX_PATH);
			if (wcscmp(className, L"childClass") == 0)	//IE
			{
				GetWindowText(*it, lintitle, MAX_PATH);
				if (boost::algorithm::icontains(lintitle, L"PowerPoint Slide Show"))
				{
					vecHwnd.push_back(*it);
					break;
				}
			}
			else
				vecHwnd.push_back(*it);
		}
	}
	else
	{
		vecHwnd = m_vechView;
	}
	m_vechView.clear();
	::LeaveCriticalSection(&m_vecViewSec);
}

void CNLOvelayView::SetPathTryDoOverlay(const wstring &strFilePath)
{
	bool bFind = false;
	::EnterCriticalSection(&m_vecViewSec);
	vector<wstring>::iterator itor;
	for (itor = m_vecFilePath.begin(); itor != m_vecFilePath.end(); itor++)
	{
		if (_wcsicmp((*itor).c_str(), strFilePath.c_str()) == 0)
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)	m_vecFilePath.push_back(strFilePath);
	::LeaveCriticalSection(&m_vecViewSec);
}
bool CNLOvelayView::PathIsTryDoOverlay(const wstring &strFilePath)
{
	bool bRet = false;
	::EnterCriticalSection(&m_vecViewSec);
	vector<wstring>::iterator itor;
	for (itor = m_vecFilePath.begin(); itor != m_vecFilePath.end(); itor++)
	{
		if (_wcsicmp((*itor).c_str(), strFilePath.c_str()) == 0)
		{
			bRet = true;
			break;
		}
	}
	::LeaveCriticalSection(&m_vecViewSec);
	return bRet;
}

void CNLOvelayView::DelOverlayPath()
{
	::EnterCriticalSection(&m_vecViewSec);
	m_vecFilePath.clear();
	::LeaveCriticalSection(&m_vecViewSec);
}

bool CNLOvelayView::GetNormalOfficeView(const wstring& strPath, HWND& hViewWnd, AppType	AppType)
{
	bool bDot = false;
	AppWndParams theCaption;
	theCaption.hView = NULL;
	wstring strFileName = GetFileNameEx(strPath, bDot);
	bool bGet = false;
	// enum the viewwnd
	switch (AppType)
	{
	case kAppWord:
	{
								swprintf_s(g_strWinCaption, L"%s", strFileName.c_str());
								theCaption.strFileTypeCaption = g_strWinCaption;
								swprintf_s(g_strWinCaption, L"%s - Microsoft Word", strFileName.c_str());
								theCaption.strFileTitle = g_strWinCaption;
								theCaption.nAppType = AppType;
								if (!::EnumWindows(EnumLocalOfficeAppFrameProc, (LPARAM)&theCaption) && theCaption.hView != NULL)
								{
									bGet = true;
									hViewWnd = theCaption.hView;
								}
	}
		break;
	case kAppExcel:
	{
								 swprintf_s(g_strWinCaption, L"Microsoft Excel - %s", strFileName.c_str());
								 theCaption.strFileTypeCaption = g_strWinCaption;
								 swprintf_s(g_strWinCaption, L"%s", strFileName.c_str());
								 theCaption.strChildWndTitle = g_strWinCaption;
								 theCaption.nAppType = AppType;
								 if (!::EnumWindows(EnumLocalOfficeAppFrameProc, (LPARAM)&theCaption) && theCaption.hView != NULL)
								 {
									 bGet = true;
									 hViewWnd = theCaption.hView;
								 }
	}
		break;
	case kAppPPT:
	{
							   swprintf_s(g_strWinCaption, L"Microsoft PowerPoint - [%s]", strFileName.c_str());
							   theCaption.strFileTypeCaption = g_strWinCaption;
							   theCaption.strFileTitle = L"Microsoft PowerPoint";
							   theCaption.strChildWndTitle = strFileName;
							   swprintf_s(g_strWinCaption, L"%s - Microsoft PowerPoint", strFileName.c_str());
							   theCaption.str2010PPTCaption = g_strWinCaption;
							   theCaption.nAppType = AppType;
							   theCaption.dwOfficeVer = pep::getVersion();
							   if (!::EnumWindows(EnumLocalOfficeAppFrameProc, (LPARAM)&theCaption) && theCaption.hView != NULL)
							   {
								   bGet = true;
								   hViewWnd = theCaption.hView;
							   }
	}
		break;
	}
	return bGet;
}

bool CNLOvelayView::CallPaf(__in PA_Mngr::CPAMngr& pam, __in const PABase::ACTION pam_action, __in const std::wstring& new_source)
{
	if (pa_labeling_mod == NULL)
	{
		if (g_strModulePath.empty())
		{
			wchar_t szModulePath[1025] = { 0 };
			GetModuleFileNameW(g_hInstance, szModulePath, 1024);
			wchar_t* pPos = wcsrchr(szModulePath, L'\\');
			if (pPos == NULL)	return false;
			*++pPos = L'\0';
			g_strModulePath = szModulePath;
		}
		wstring strDllPath = g_strModulePath;
		strDllPath += PA_LOAD::PA_MODULE_NAME_NLVISUALABELING;
		pa_labeling_mod = LoadLibraryW(strDllPath.c_str());


		StringCchPrintfW(g_szLog, 1024, L"PA_LOAD::LoadModuleByName: %s module = 0x%p,(last error = %d), path:%s\n", PA_LOAD::PA_MODULE_NAME_NLVISUALABELING,
			pa_labeling_mod, GetLastError(), strDllPath.c_str());
		OutputDebugString(g_szLog);
		if (pa_labeling_mod == NULL)	return false;
	}


	vector<HWND> vecView;
	HWND hViewWnd = NULL;
	AppType AppType = pep::appType();
	GetDoOverlayHwnd(AppType, vecView);
	if (vecView.empty())
	{
		if (!PathIsTryDoOverlay(new_source))
		{
			GetNormalOfficeView(new_source, hViewWnd, AppType);
			if (hViewWnd != NULL)
			{
				vecView.push_back(hViewWnd);
			}
		}
	}

	SetPathTryDoOverlay(new_source);

	for (vector<HWND>::const_iterator it = vecView.begin(); it != vecView.end(); it++)
	{
		StringCchPrintf(g_szLog, 1024, L"will pass into DoVisualLabeling param is:hwnd:%p,filepath:%s\n", *it, new_source.c_str());
		::OutputDebugStringW(g_szLog);
		pam.DoVisualLabeling(pa_labeling_mod, NULL, pam_action, TRUE, _T("OK"), *it);
	}


	return true;
}
const wchar_t* CNLOvelayView::COPYOF_CEM_GetString(CEString cestr)
{
	if (cestr == NULL)
	{
		return NULL;
	}
	return (((_CEString*)cestr)->buf);
}/* COPYOF_CEM_GetString */
void CNLOvelayView::DoViewOverlay(nextlabs::Obligations& obs, const wchar_t* szFile)
{
	PA_Mngr::CPAMngr pam(CNLOvelayView::COPYOF_CEM_GetString);
	wstring filePath = szFile;
	if (boost::algorithm::istarts_with(filePath, L"http"))
	{
		ConvertURLCharacterW(filePath);
		boost::algorithm::replace_all(filePath, L"\\", L"/");
	}
	else
	{
		boost::algorithm::replace_all(filePath, L"/", L"\\");
	}
	pam.SetObligations(filePath.c_str(), filePath.c_str(), obs);
	CallPaf(pam, PABase::AT_READ, filePath);
}
void CNLOvelayView::SendPrintPreview(bool switchstatus)
{
	HWND hFore = GetForegroundWindow();
	HWND hParent = FindWindowExW(hFore, NULL, L"XLDESK", L"");
	if (hParent)
	{
		HWND hView = GetWindow(hParent, GW_CHILD);
		if (switchstatus)
			PostMessage(hView, WM_OVPRINT, NULL, NULL);
		else
			PostMessage(hView, WM_OVPRINTCLOSE, NULL, NULL);
	}
}
void CNLOvelayView::TriggerDoViewOverlay(void)
{
	if (IsEmptyCreatewindowCache())	return;
	wstring strPath = L"";
	getDocumentPathEx(strPath, getActiveDoc());
	ProcessResult rt = NLOBMGRINSTANCE.NLProcessActionCommon(NULL, strPath.c_str(), NULL, kOA_OPEN);
	if (pep::isExcelApp)
	{
		if (kFSSuccess != rt.kFuncStat || kPSDeny == rt.kPolicyStat )
		{
			wstring wstrActiveFilePath;
			CComPtr<IDispatch> pActiveDoc = getActiveDoc();
			getDocumentPathEx( wstrActiveFilePath, pActiveDoc );
			NLOBMGRINSTANCE.NLSetCloseFlag(wstrActiveFilePath, true);
		}
		
	}
	
}


//
// Print Overlay
//

extern HINSTANCE g_hInstance;

CNLPrintOverlay::CNLPrintOverlay(void)
{
	m_hDC = NULL;
	m_gdiplusToken = NULL;
	m_pPrintVLFunc = NULL;
	m_hPrintVLDLL = NULL;
	GetPrintVLFunAddr();
	//m_overlayInfo
}

CNLPrintOverlay::~CNLPrintOverlay(void)
{
	FreePrintVLAddr();
}
CNLPrintOverlay& CNLPrintOverlay::GetInstance()
{
	static CNLPrintOverlay theOV;

	return theOV;
}
void CNLPrintOverlay::StartGDIPlus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}
void CNLPrintOverlay::ShutDownGDIPlus()
{
	if (m_gdiplusToken != NULL)
	{
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		m_gdiplusToken = NULL;
	}

}
FuncPrintVL CNLPrintOverlay::GetPrintVLFunAddr()
{
	wchar_t wzPath[MAX_PATH + 1] = { 0 };
	DWORD dwLen = GetModuleFileNameW((HMODULE)g_hInstance, wzPath, MAX_PATH);
	DWORD i = 1;
	while (i++ < dwLen)
	{
		if (wzPath[dwLen - i] == L'\\')
		{
			wzPath[dwLen - i + 1] = L'\0';
			break;
		}
	}
#ifdef _WIN64
	wcscat_s(wzPath, MAX_PATH, L"NLVLViewPrint.dll");
#else
	wcscat_s(wzPath, MAX_PATH, L"NLVLViewPrint32.dll");
#endif

	if (m_hPrintVLDLL == NULL)
	{
		m_hPrintVLDLL = LoadLibraryW(wzPath);
	}
	if (m_hPrintVLDLL)
	{
		m_pPrintVLFunc = (FuncPrintVL)GetProcAddress(m_hPrintVLDLL, PRINT_VL_FUNC);
	}

	return m_pPrintVLFunc;
}

void CNLPrintOverlay::FreePrintVLAddr()
{
	if (m_pPrintVLFunc != NULL)
	{
		m_pPrintVLFunc = NULL;
	}
	if (m_hPrintVLDLL != NULL)
	{
		FreeLibrary(m_hPrintVLDLL);
		m_hPrintVLDLL = NULL;
	}
}

void CNLPrintOverlay::DoPrintOverlay()
{
	if (m_pPrintVLFunc)
	{
		m_pPrintVLFunc(m_hDC, m_theInfo);
	}
	return;
}
void CNLPrintOverlay::SetHDC(const HDC& theHDC)
{
	m_hDC = theHDC;
}
void CNLPrintOverlay::releaseHDC(const HDC& theHDC)
{
	if (m_hDC == theHDC)
		m_hDC = NULL;
}
bool CNLPrintOverlay::IsSameHDC(const HDC& theHDC)
{
	if (m_hDC == theHDC)	return true;
	return false;
}

void CNLPrintOverlay::ReleaseOverlayData()
{
	memset((void *)&m_theInfo, 0, sizeof(m_theInfo));
}


bool CNLPrintOverlay::SetOverlayData(_In_ nextlabs::Obligations& obs, _In_ const wstring & strFilePath)
{
	bool bRet = false;
	if (strFilePath.empty())
	{
		return bRet;
	}

	const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();
	std::list<nextlabs::Obligation>::const_iterator it;
	for (it = ob_list.begin(); it != ob_list.end(); ++it)
	{
		if (boost::algorithm::iequals(it->name, OBLIGATION_PRINT_OVERLAY))
		{
			StringCchPrintf(m_theInfo.strFilePath, 2048, L"%s", strFilePath.c_str());
			StringCchPrintf(m_theInfo.strPolicyName, 512, L"%s", it->policy.c_str());

			nextlabs::ObligationOptions::const_iterator options_it;
			for (options_it = it->options.begin(); options_it != it->options.end(); ++options_it)
			{
				if (options_it->first == NM_VLObligation::str_text_name)
				{
					StringCchPrintf(m_theInfo.strTextValue, 1024, L"%s", options_it->second.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_classification_map_name)
				{
					StringCchPrintf(m_theInfo.strClassificationMap, 512, L"%s", options_it->second.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_transparency_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"15";
					}
					m_theInfo.dwTransparency = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_font_name)
				{
					StringCchPrintf(m_theInfo.strFontName, 64, L"%s", options_it->second.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_font_size1_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"36";
					}
					m_theInfo.dwFontSize1 = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_font_size2_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"28";
					}
					m_theInfo.dwFontSize2 = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_font_bold_name)
				{
					if (_wcsicmp(options_it->second.c_str(), L"false") == 0)		m_theInfo.bFontBold = false;
					else m_theInfo.bFontBold = true;

					continue;
				}
				else if (options_it->first == NM_VLObligation::str_font_color_name)
				{
					wstring strColor = options_it->second;
					if (strColor.empty())
					{
						strColor = L"#888888";
					}
					boost::algorithm::replace_all(strColor, L"#", L"");
					m_theInfo.dwFontColor = COverlayTool::GetFontColor(strColor);
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_placement_view_print_name)
				{
					StringCchPrintf(m_theInfo.strPlacement, 64, L"%s", options_it->second.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_left_margins_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"1";
					}
					m_theInfo.dwLeftMargin = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_top_margins_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"2";
					}
					m_theInfo.dwTopMargin = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_hor_space_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"2";
					}
					m_theInfo.dwHorSpace = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_ver_space_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"3";
					}
					m_theInfo.dwVerSpace = boost::lexical_cast<unsigned long>(strTemp.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_placement_view_print_name)
				{
					StringCchPrintf(m_theInfo.strPlacement, 64, L"%s", options_it->second.c_str());
					continue;
				}
				else if (options_it->first == NM_VLObligation::str_date_format_name)
				{
					StringCchPrintf(m_theInfo.strDateFormat, 128, L"%s", options_it->second.c_str());
				}
				else if (options_it->first == NM_VLObligation::str_time_format_name)
				{
					StringCchPrintf(m_theInfo.strTimeFormat, 128, L"%s", options_it->second.c_str());
					continue;
				}
			}

			wstring strTemp = L"";
			COverlayTool::ConvertTxT(m_theInfo.strTextValue, m_theInfo.strFilePath, m_theInfo.strPolicyName,
				m_theInfo.strDateFormat, m_theInfo.strTimeFormat, strTemp);
			StringCchPrintf(m_theInfo.strTextValue, 1024, L"%s", strTemp.c_str());
			bRet = true;
			break;
		}
	}
	return bRet;
}
bool CNLPrintOverlay::IsDoOverlay()
{
	return m_bIsDoOverLay;
}
void CNLPrintOverlay::SetDoOverlayFlag(bool b)
{
	m_bIsDoOverLay = b;
}

//
// SE Synchronize Golden Tags
//

#define NLOFFICE_SYNCHROTAGS_MAX_CONCURRENCY 100

CNLSESynchroGoldenTags::CNLSESynchroGoldenTags(void)
{
	InitializeCriticalSection(&m_csMapGoldenTag);
	NLSetThreadEndFlag(FALSE);
	NLSetInitializeFlag(FALSE);
	m_hSemaphore = NULL;
	m_hThread = NULL;
}

CNLSESynchroGoldenTags::~CNLSESynchroGoldenTags(void)
{
	DeleteCriticalSection(&m_csMapGoldenTag);
	NLUnInitialize();
}

///////////////////thread create and destroy///////////////////////////////////////////////////////
void CNLSESynchroGoldenTags::NLInitialize()
{
	if (!NLGetInitializeFlag())
	{
		// here initialize for asynchronous I/O 
		// use semaphore to receive synchronize golden tag message
		m_hSemaphore = CreateSemaphore(NULL, 0, NLOFFICE_SYNCHROTAGS_MAX_CONCURRENCY, NULL);
		if (NULL != m_hSemaphore)
		{
			NLSetThreadEndFlag(FALSE);

			// create a thread used to synchronize the golden tags
			m_hThread = (HANDLE)_beginthreadex(NULL, 0, &NLSynchronizeGoldenTagsThread, this, 0, NULL);
			if (NULL == m_hThread)
			{
				CloseHandle(m_hSemaphore);
				m_hSemaphore = NULL;
			}
			else
			{
				NLSetInitializeFlag(TRUE);
			}
		}
	}
}

void CNLSESynchroGoldenTags::NLUnInitialize()
{
#pragma chMSG( "Here we must make sure the thread is closed, cache exception, use SEH" )
	// send a message to destroy the thread
	if (NLGetInitializeFlag())
	{
		NLSetInitializeFlag(FALSE);
		NLSetThreadEndFlag(TRUE);
		ReleaseSemaphore(m_hSemaphore, 1, NULL);

		if (NULL != m_hThread)
		{
			WaitForSingleObject(m_hThread, INFINITE);
		}

		CloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;

		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}
}

void CNLSESynchroGoldenTags::NLSetInitializeFlag(_In_ BOOL bInitialize)
{
	InterlockedExchange(reinterpret_cast<LONG*>(&m_bInitialize), static_cast<LONG>(bInitialize));
}

bool CNLSESynchroGoldenTags::NLGetInitializeFlag()
{
	return m_bInitialize;
}
//////////////////////////////////////////////////////////////////////////

///////////////////////////////synchronize golden tags, map thread safe///////////////////////////////////////////
bool CNLSESynchroGoldenTags::NLSynchronizeGoldenTags(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags)
{
	NLCELOG_ENTER
		NLPRINT_DEBUGLOG(L" --- Begin to synchronize the golden tags, file path:[%s] --- \n", wstrFilePath.c_str());
	NLPRINT_TAGPAIRLOG(vecGoldenTags, L"--- synchronize golden tags ---", L"end");
	NLCELOG_RETURN_VAL(NLOBMGRINSTANCE.NLSynchroniseFileTags(wstrFilePath, vecGoldenTags))
}

void CNLSESynchroGoldenTags::NLCopyIntoProcessContainer(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags)
{
	EnterCriticalSection(&m_csMapGoldenTag);
	m_mapGoldenTag[wstrFilePath].vecGoldeTags = vecGoldenTags;
	LeaveCriticalSection(&m_csMapGoldenTag);
}

void CNLSESynchroGoldenTags::NLCopyIntoProcessContainer(_In_ const map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags)
{
	EnterCriticalSection(&m_csMapGoldenTag);
	m_mapGoldenTag = mapGoldenTags;
	LeaveCriticalSection(&m_csMapGoldenTag);
}

void CNLSESynchroGoldenTags::NLGetProcessGoldenTags(_Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags)
{
	EnterCriticalSection(&m_csMapGoldenTag);
	mapGoldenTags = m_mapGoldenTag;
	m_mapGoldenTag.clear();
	LeaveCriticalSection(&m_csMapGoldenTag);
}

void CNLSESynchroGoldenTags::NLSetThreadEndFlag(_In_ BOOL bNeedEndThread)
{
	InterlockedExchange(reinterpret_cast<LONG*>(&m_bNeedEndThread), static_cast<LONG>(bNeedEndThread));
}

bool CNLSESynchroGoldenTags::NLGetThreadEndFlag()
{
	return m_bNeedEndThread;
}
///////////////////////////logic start to synchronize golden tags///////////////////////////////////////////////
/*
*\ Brief: this function used to start up synchronize the golden tags
*\ Parameter:
*		[IN] wstrFilePath: the file full path which will be synchronize the golden tags. we can save the golden tags by NLSetGoldenTags before we start synchronize golden tags.
*   [IN] vecGoldenTags: the golden tags need synchronize
*		[in] bIOSynchronize:
*					true:  I/O synchronous
*					false: I/O Asynchronous
*\ Return value:
*		return false: means start synchronize golden tags failed.
*		return true:  only means the function invoke success.
*	 Note: function return true no means the golden tags synchronize success, because we only can known write tags success but we don't known the tags is the golden tags or not.
*		Success synchronize golden tags need: SE file, right start up time and success start synchronize
*/
bool CNLSESynchroGoldenTags::NLStartSynchronizeGoldenTags(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldenTags, _In_ const bool bIOSynchronize)
{
	NLCELOG_ENTER
		bool bRet = false;
	if (bIOSynchronize)
	{
		bRet = NLSynchronizeGoldenTags(wstrFilePath, vecGoldenTags);
	}
	else
	{
		if (NLGetInitializeFlag())
		{
			NLCopyIntoProcessContainer(wstrFilePath, vecGoldenTags);
			bRet = ReleaseSemaphore(m_hSemaphore, 1, NULL);
		}
	}
	NLCELOG_RETURN_VAL(bRet)
}

bool CNLSESynchroGoldenTags::NLStartSynchronizeGoldenTags(_In_ const map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags, _In_ const bool bIOSynchronize)
{
	NLCELOG_ENTER
		bool bRet = false;
	if (bIOSynchronize)
	{
		map<wstring, STUNLOFFICE_GOLDENTAG>::const_iterator itr = mapGoldenTags.begin();
		for (; itr != mapGoldenTags.end(); itr++)
		{
			const STUNLOFFICE_GOLDENTAG& stuGoldenTag = itr->second;
			if (!NLSynchronizeGoldenTags(itr->first, stuGoldenTag.vecGoldeTags))
			{
				bRet = false;
				break;
			}
		}
	}
	else
	{
		if (NLGetInitializeFlag())
		{
			NLCopyIntoProcessContainer(mapGoldenTags);
			bRet = ReleaseSemaphore(m_hSemaphore, 1, NULL);
		}
	}
	NLCELOG_RETURN_VAL(bRet)
}
//////////////////////////////////////////////////////////////////////////

/////////////////////////////synchronize tags/////////////////////////////////////////////
unsigned __stdcall CNLSESynchroGoldenTags::NLSynchronizeGoldenTagsThread(void* pArguments)
{
	NLCELOG_ENTER
		NLPRINT_DEBUGLOG(L"----- synchronize golden tags thread ID is:[%d] ----- \n", GetCurrentThreadId());
	CNLSESynchroGoldenTags* pThis = static_cast<CNLSESynchroGoldenTags*>(pArguments);
	while (true)
	{
		WaitForSingleObject(pThis->m_hSemaphore, INFINITE);

		if (pThis->NLGetThreadEndFlag())
		{
			NLPRINT_DEBUGLOG(L"synchronize golden tags thread end \n");
			break;
		}

		map<wstring, STUNLOFFICE_GOLDENTAG> mapGoldenTag;
		pThis->NLGetProcessGoldenTags(mapGoldenTag);

		map< wstring, STUNLOFFICE_GOLDENTAG >::iterator itr = mapGoldenTag.begin();
		for (; itr != mapGoldenTag.end(); itr++)
		{
			STUNLOFFICE_GOLDENTAG& stuGoldenTag = itr->second;
			pThis->NLSynchronizeGoldenTags(itr->first, stuGoldenTag.vecGoldeTags);
		}
	}
	pThis->NLSetThreadEndFlag(FALSE);
	NLCELOG_RETURN_VAL(0)
}
//////////////////////////////////////////////////////////////////////////



//
// Tag obligations
//
#ifdef _WIN64
#define  NLOFFICERESATTRLIB_MODULE_NAME	L"resattrlib.dll"
#else
#define  NLOFFICERESATTRLIB_MODULE_NAME	L"resattrlib32.dll"
#endif

#ifdef _WIN64
#define	  NLOFFICERESATTRMGR_MODULE_NAME	L"resattrmgr.dll"
#else
#define  NLOFFICERESATTRMGR_MODULE_NAME	L"resattrmgr32.dll"
#endif

// SE File inherent tags
static const wchar_t* g_pwchSeTagNameEncrypted = L"NXL_encrypted";
static const wchar_t* g_pwchSeTagNameKeyRingName = L"NXL_keyRingName";
static const wchar_t* g_pwchSeTagNameRequires = L"NXL_requiresLocalEncryption";
static const wchar_t* g_pwchSeTagNameWrapped = L"NXL_wrapped";

CNLTag::CNLTag(void) : m_hLib(NULL), m_hMgr(NULL), m_dwIEVer(0), m_bIsGetFunSuc(false)
{
	if (GetTagDllHandle())
	{
		SetGetFunSucFlag(GetTagFunAddr());
	}
}

CNLTag::~CNLTag(void)
{
	if (NULL != m_hLib)
	{
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
	if (NULL != m_hMgr)
	{
		FreeLibrary(m_hMgr);
		m_hMgr = NULL;
	}
}

bool CNLTag::IsGetFunSuc()
{
	return m_bIsGetFunSuc;
}

void CNLTag::SetGetFunSucFlag(_In_ bool bSuccess)
{
	m_bIsGetFunSuc = bSuccess;
}

CComPtr<IDispatch> CNLTag::GetCustomProperties(_In_ IDispatch* pCurDoc)
{
	if (NULL != pCurDoc)
	{
		CComVariant var2;
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &var2, pCurDoc, L"CustomDocumentProperties", 0);
		if (SUCCEEDED(hr) && NULL != var2.pdispVal)
		{
			return var2.pdispVal;
		}
	}
	return NULL;
}

bool CNLTag::ReadTag(_In_ const wstring& wstrPath, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	NLCELOG_ENTER
		// 1. check tag library
	if (!IsGetFunSuc())
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. get file path: HTTP path we should convert it to UNC path and need check the path if it is exist.
	wstring wstrFilePath = wstrPath;  

	if (!NLGetEffectFilePath(wstrFilePath))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 3. alloc resource
	ResourceAttributeManager* pMgr = NULL;
	ResourceAttributes* pAttrs = NULL;

	if (!NLAlloceResource(pMgr, pAttrs))
	{
		NLCELOG_RETURN_VAL(false)
	}

	bool bRet = false;

#pragma chMSG( "Here we can not make sure what the return value means" )
	// here I can not make sure what the return value means
	int nReadRet = m_lfReadResourceAttributesW(pMgr, wstrFilePath.c_str(), pAttrs);

	if (1 == nReadRet)
	{
		bRet = true;
		NLGetAttributeToVetor(pAttrs, vecTagPair);
	}

	NLFreeResource(pMgr, pAttrs);
	NLCELOG_RETURN_VAL(bRet)
}

bool CNLTag::ReadTag(_In_ IDispatch* pDocObj, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	NLCELOG_ENTER
		// 1. check parameter & tag library, we use pDocObj to read tags but we also need use tag library to convert the long tags
	if (!IsGetFunSuc() || NULL == pDocObj)
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. get custom interface
	CComPtr<IDispatch> pCustomProperties = GetCustomProperties(pDocObj);
	if (NULL == pCustomProperties)
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 3. read tags by pDoc
	CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties, L"Count", 0);
	if (SUCCEEDED(hr))
	{
		if (theResult.lVal <= 0)
		{
			NLPRINT_DEBUGLOG(L"the file no tags, count:[%d] \n", theResult.lVal);
			NLCELOG_RETURN_VAL(true)
		}

		long lCount = theResult.lVal;
		for (long i = 0; i < lCount; i++)
		{
			CComVariant varIndex(i + 1);
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties, L"Item", 1, varIndex);
			if (SUCCEEDED(hr) && theResult.pdispVal)
			{
				IDispatch* pCustomProperty = theResult.pdispVal;	// here if use the CComPtr, the process can't exit when excel open in IE
				hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperty, L"Name", 0);
				if (SUCCEEDED(hr) && VT_BSTR == theResult.vt && NULL != theResult.bstrVal)
				{
					wstring wstrTagName(theResult.bstrVal);
					hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperty, L"Value", 0);
					if (SUCCEEDED(hr) && VT_BSTR == theResult.vt && NULL != theResult.bstrVal)
					{
						wstring wstrTagValue(theResult.bstrVal);
						vecTagPair.push_back(pair<wstring, wstring>(wstrTagName, wstrTagValue));
					}
				}
				pCustomProperty->Release();
			}
		}
	}
	else
	{
		NLPRINT_DEBUGLOG(L"!!!Failed to get custom tag count, please check \n");
		NLCELOG_RETURN_VAL(false)
	}

	// original tags
	NLPRINT_TAGPAIRLOG(vecTagPair, L"Original tags read by pDoc");

	// 4. alloc resource for convert the long tags 
	ResourceAttributeManager* pMgr = NULL;
	vector<ResourceAttributes*> vecpAttr;

	if (!NLAlloceResource(pMgr, vecpAttr, 2))
	{
		NLCELOG_RETURN_VAL(false)
	}

	NLAddAttributeFromVector(vecpAttr[0], vecTagPair);
	m_lConvert4GetAttr(vecpAttr[1], vecpAttr[0]);					// this function always return 1, no means
	vecTagPair.clear();
	NLGetAttributeToVetor(vecpAttr[1], vecTagPair);

	NLFreeResource(pMgr, vecpAttr);

	NLPRINT_TAGPAIRLOG(vecTagPair, L"read tags by pDoc");
	NLCELOG_RETURN_VAL(true)
}

bool CNLTag::AddTag(_In_ const wstring& wstrPath, _In_ const vector<pair<wstring, wstring>>& vecTagPair)
{
	NLCELOG_ENTER
		// 1. check tag library
	if (!IsGetFunSuc())
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. check tag pair
	if (vecTagPair.empty())
	{
		NLCELOG_RETURN_VAL(true)
	}

	// add logs
	NLPRINT_TAGPAIRLOG(vecTagPair);

	// 2. get file path: HTTP path we should convert it to UNC path and need check the path if it is exist.
	wstring wstrFilePath = wstrPath;
	if (!NLGetEffectFilePath(wstrFilePath))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 3. alloc resource attributes
	ResourceAttributeManager* pMgr = NULL;
	ResourceAttributes* pAttrs = NULL;

	if (!NLAlloceResource(pMgr, pAttrs))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 4. add tags into attribute, SE file need ignore its inherent tags, for bug:23431
	NLAddAttributeFromVector(pAttrs, vecTagPair, NLOBMGRINSTANCE.NLIsEncryptedFile(wstrFilePath));

	// 5. write tags
	int nWriteRet = m_lfWriteResourceAttributesW(pMgr, wstrFilePath.c_str(), pAttrs);
	NLPRINT_DEBUGLOG(L"the WriteResourceAttributesW return value is:[%d] \n", nWriteRet);

	NLFreeResource(pMgr, pAttrs);

#pragma chMSG( "Maybe here we don't need to check this return value" )
	NLCELOG_RETURN_VAL(1 == nWriteRet ? true : false)
}

bool CNLTag::AddTag(_In_ IDispatch* pDocObj, _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_ const bool bIsSave)
{
	NLCELOG_ENTER
		// 1. check tag library & parameter, we need use tag library to covert the long tags
	if (!IsGetFunSuc() || NULL == pDocObj)
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. check tag pair
	if (vecTagPair.empty())
	{
		NLCELOG_RETURN_VAL(true)
	}

	// add some logs
	NLPRINT_TAGPAIRLOG(vecTagPair);

	// 3. alloc resource attributes
	ResourceAttributeManager* pMgr = NULL;
	vector<ResourceAttributes*> vecpResource;

	if (!NLAlloceResource(pMgr, vecpResource, 2))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 4. delete old custom tags
	if (!RemoveAllTag(pDocObj, false))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// add tags
	vector<pair<wstring, wstring>> vecTagPairToSet;
	NLAddAttributeFromVector(vecpResource[0], vecTagPair);
	m_lConvert4SetAttr(vecpResource[1], vecpResource[0]);
	NLGetAttributeToVetor(vecpResource[1], vecTagPairToSet);

	CComVariant theResult;
	HRESULT hr;

	CComPtr<IDispatch> pCustomProperties = GetCustomProperties(pDocObj);
	if (NULL == pCustomProperties)
	{
		NLCELOG_RETURN_VAL(false)
	}

	// use IDispatch to add custom tags
	vector<pair<wstring, wstring>>::const_iterator iter;
	for (iter = vecTagPairToSet.begin(); iter != vecTagPairToSet.end(); iter++)
	{
		wstring wstrName = (*iter).first;
		wstring wstrValue = (*iter).second;

		boost::algorithm::trim(wstrName);
		boost::algorithm::trim(wstrValue);
		if (wstrName.empty() || wstrValue.empty()) continue;

		CComVariant pa1(wstrName.c_str());
		CComVariant pa2(VARIANT_FALSE);
		CComVariant pa3(4);
		CComVariant pa4(wstrValue.c_str());

		hr = AutoWrap(DISPATCH_METHOD, &theResult, pCustomProperties, L"Add", 4, pa4, pa3, pa2, pa1);
		if (!SUCCEEDED(hr))
		{
			NLPRINT_DEBUGLOG(L"Add Property fail! the name is [%s], the value is [%s],hr is [%x]\n", wstrName.c_str(), wstrValue.c_str(), hr);
			continue;
		}
	}

	// save the custom tags change, if need.
	bool bRet = true;
	if (bIsSave)
	{
		bRet = NLSaveCustomTags(pDocObj);	// for PPT this function invoke failed. ( invoke Com interface "Save" failed, hr = 080002009, unknown reason )
	}
	NLCELOG_RETURN_VAL(bRet)
}

bool CNLTag::RemoveTag(_In_ const wstring& wstrPath, _In_ const vector<pair<wstring, wstring>>& vecTagPair)
{
	NLCELOG_ENTER
		// 1. check tag library
	if (!IsGetFunSuc())
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. check parameter
	if (vecTagPair.empty())
	{
		NLCELOG_RETURN_VAL(true)
	}

	// 3. get file path: HTTP path we should convert it to UNC path and need check the path if it is exist.
	wstring wstrFilePath = wstrPath;
	if (!NLGetEffectFilePath(wstrFilePath))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 3. alloc
	ResourceAttributeManager* pMgr = NULL;
	ResourceAttributes* pAttrs = NULL;

	if (NLAlloceResource(pMgr, pAttrs))
	{
		NLCELOG_RETURN_VAL(false)
	}

	NLAddAttributeFromVector(pAttrs, vecTagPair);  // this function if we can make sure the pAttr is not NULL, if will return true
	int nRet = m_lfRemoveResourceAttributesW(pMgr, wstrFilePath.c_str(), pAttrs);

	NLFreeResource(pMgr, pAttrs);
	NLCELOG_RETURN_VAL(1 == nRet ? true : false)
}

bool CNLTag::RemoveAllTag(_In_ const wstring& wstrPath)
{
	NLCELOG_ENTER
		// 1. check tag library
	if (!IsGetFunSuc())
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 2. get file path: HTTP path we should convert it to UNC path and need check the path if it is exist.
	wstring wstrFilePath = wstrPath;
	if (!NLGetEffectFilePath(wstrFilePath))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 3. alloc
	ResourceAttributeManager* pMgr = NULL;
	ResourceAttributes* pAttrs = NULL;

	if (NLAlloceResource(pMgr, pAttrs))
	{
		NLCELOG_RETURN_VAL(false)
	}

	// 4. remove all tags
	int nRet = m_lfReadResourceAttributesW(pMgr, wstrFilePath.c_str(), pAttrs);
	if (1 == nRet)
	{
		nRet = m_lfRemoveResourceAttributesW(pMgr, wstrFilePath.c_str(), pAttrs);
	}

	NLFreeResource(pMgr, pAttrs);
	NLCELOG_RETURN_VAL(1 == nRet ? true : false)
}

bool CNLTag::RemoveAllTag(_In_ IDispatch* pDocObj, _In_ const bool& bIsSave)
{
	NLCELOG_ENTER
		// 1. check parameter
	if (NULL == pDocObj)
	{
		NLCELOG_RETURN_VAL(false)
	}

	bool bRet = false;
	CComPtr<IDispatch> pCustomProperties = GetCustomProperties(pDocObj);
	if (NULL != pCustomProperties)
	{
		if (NLDeleteTags(pCustomProperties))
		{
			bRet = bIsSave ? NLSaveCustomTags(pDocObj) : true;
		}
	}
	// save the custom tags change, if need.
	NLCELOG_RETURN_VAL(bRet)
}

bool CNLTag::NLDeleteTags(IDispatch* pCustomProperties)
{
	NLCELOG_ENTER
		CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties, L"Count", 0);

	if (SUCCEEDED(hr))
	{
		if (0 == theResult.lVal)
		{
			// No custom tags
			NLCELOG_RETURN_VAL(true)
		}

		for (long i = 0; i < theResult.lVal; i++)
		{
			CComVariant varIndex(1);
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties, L"Item", 1, varIndex);
			if (SUCCEEDED(hr) && theResult.pdispVal)
			{
				CComPtr<IDispatch> pCustomProperty = theResult.pdispVal;
				hr = AutoWrap(DISPATCH_METHOD, &theResult, pCustomProperty, L"Delete", 0);

				NLCELOG_RETURN_VAL(SUCCEEDED(hr))
			}
		}
	}
	NLCELOG_RETURN_VAL(false)
}

bool CNLTag::NLSaveCustomTags(IDispatch* pDoc)
{
	NLCELOG_ENTER
		CComVariant theResult;
	CComVariant varFalse(VARIANT_FALSE);

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYPUT, &theResult, pDoc, L"Saved", 1, varFalse);

	if (SUCCEEDED(hr))
	{
#pragma chMSG( " !!!!!!!!!!! Please note here, PPT edit, invoke this COM interface failed " )
		hr = AutoWrap(DISPATCH_METHOD, &theResult, pDoc, L"Save", 0);
		NLCELOG_RETURN_VAL(SUCCEEDED(hr))
	}
	NLCELOG_RETURN_VAL(false)
}

void CNLTag::NLFreeResource(_Inout_ ResourceAttributeManager*& pMgr, _Inout_ ResourceAttributes*& pAttr)
{
	if (NULL != pMgr)
	{
		m_lfCloseAttributeManager(pMgr);
		pMgr = NULL;
	}

	if (NULL != pAttr)
	{
		m_lfFreeAttributes(pAttr);
	}
}

void CNLTag::NLFreeResource(_Inout_ ResourceAttributeManager*& pMgr, _Inout_ vector<ResourceAttributes*>& vecpAttr)
{
	if (NULL != pMgr)
	{
		m_lfCloseAttributeManager(pMgr);
		pMgr = NULL;
	}

	for (vector<ResourceAttributes*>::iterator itr = vecpAttr.begin(); itr != vecpAttr.end(); itr++)
	{
		if (NULL != *itr)
		{
			m_lfFreeAttributes(*itr);
		}
	}
	vecpAttr.clear();
}

bool CNLTag::NLAlloceResource(_Out_ ResourceAttributeManager*& pMgr, _Out_ ResourceAttributes*& pAttr)
{
	// 1. initialize
	pMgr = NULL;
	pAttr = NULL;

	// 2. alloc mgr
	ResourceAttributeManager* pMgrTemp = NULL;
	m_lfCreateAttributeManager(&pMgrTemp);
	pMgr = pMgrTemp;

	// 2. check mgr
	if (NULL == pMgr)
	{
		return false;
	}

	ResourceAttributes* pResourcceAttr = NULL;
	m_lfAllocAttributes(&pResourcceAttr);
	pAttr = pResourcceAttr;

	if (NULL == pAttr)
	{
		NLFreeResource(pMgr, pAttr);
		return false;
	}
	return true;
}

bool CNLTag::NLAlloceResource(_Out_ ResourceAttributeManager*& pMgr, _Out_ vector<ResourceAttributes*>& vecpAttr, _In_ unsigned int nCount)
{
	// 1. initialize
	pMgr = NULL;
	vecpAttr.clear();

	// 2. alloc mgr
	ResourceAttributeManager* pMgrTemp = NULL;
	m_lfCreateAttributeManager(&pMgrTemp);
	pMgr = pMgrTemp;

	// 2. check mgr
	if (NULL == pMgr)
	{
		return false;
	}

	for (unsigned int i = 0; i < nCount; i++)
	{
		ResourceAttributes* pResourcceAttr = NULL;
		m_lfAllocAttributes(&pResourcceAttr);

		if (NULL != pResourcceAttr)
		{
			vecpAttr.push_back(pResourcceAttr);
		}
		else
		{
			NLFreeResource(pMgr, vecpAttr);
			return false;
		}
	}
	return true;
}

void CNLTag::NLGetAttributeToVetor(_In_   ResourceAttributes*& pAttr, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	int nSze = m_lfGetAttributeCount(pAttr);
	for (int i = 0; i < nSze; ++i)
	{
		wstring wstrTagName = (WCHAR *)m_lfGetAttributeName(pAttr, i);
		wstring wstrTagValue = (WCHAR *)m_lfGetAttributeValue(pAttr, i);

		boost::algorithm::trim(wstrTagName);
		boost::algorithm::trim(wstrTagValue);

		vecTagPair.push_back(pair<wstring, wstring>(wstrTagName, wstrTagValue));
	}
	NLPRINT_TAGPAIRLOG(vecTagPair);
}

void CNLTag::NLAddAttributeFromVector(_Inout_ ResourceAttributes*& pAttr, _In_ const vector<pair<wstring, wstring>>& vecTagPair, _In_ const bool bIgnoreSEInherent)
{
	vector<pair<wstring, wstring>>::const_iterator itr;
	for (itr = vecTagPair.begin(); itr != vecTagPair.end(); itr++)
	{
		wstring wstrTagName = itr->first;
		wstring wstrTagValue = itr->second;

		boost::algorithm::trim(wstrTagName);
		boost::algorithm::trim(wstrTagValue);

		if (!wstrTagName.empty() && !wstrTagValue.empty())
		{
			if (!(bIgnoreSEInherent && NLIsEncryptFileInherentTag(wstrTagName.c_str())))
			{
				m_lfAddAttributeW(pAttr, wstrTagName.c_str(), wstrTagValue.c_str());
			}
		}
	}
	NLPRINT_TAGPAIRLOG(vecTagPair);
}

bool CNLTag::GetTagDllHandle()
{
	if (NULL != m_hLib && NULL != m_hMgr)// the libraries have been loaded
	{
		return true;
	}

	std::wstring strCommonPath = GetCommonComponentsDir();
	wstring wstrLib = strCommonPath + NLOFFICERESATTRLIB_MODULE_NAME;
	wstring wstrMgr = strCommonPath + NLOFFICERESATTRMGR_MODULE_NAME;

	m_hLib = LoadLibrary(wstrLib.c_str());
	if (NULL == m_hLib)
	{
		NLPRINT_DEBUGLOG(L"Load Library [%s] Failed! The error code is [%d]", wstrLib.c_str(), GetLastError());
		return false;
	}

	wchar_t szDllPathBak[2048] = { 0 };
	GetDllDirectoryW(2048, szDllPathBak);
	SetDllDirectoryW(strCommonPath.c_str());
	m_hMgr = LoadLibrary(wstrMgr.c_str());
	SetDllDirectoryW(szDllPathBak);

	if (NULL == m_hMgr)
	{
		NLPRINT_DEBUGLOG(L"Load Library [%s] Failed! The error code is [%d]", wstrMgr.c_str(), GetLastError());
		return false;
	}
	SetDllDirectoryW(NULL);
	return true;
}

bool CNLTag::GetTagFunAddr()
{
	m_lfCreateAttributeManager = (CreateAttributeManagerType)GetProcAddress(m_hMgr, "CreateAttributeManager");
	m_lfAllocAttributes = (AllocAttributesType)GetProcAddress(m_hLib, "AllocAttributes");
	m_lfReadResourceAttributesW = (ReadResourceAttributesWType)GetProcAddress(m_hMgr, "ReadResourceAttributesW");
	m_lfGetAttributeCount = (GetAttributeCountType)GetProcAddress(m_hLib, "GetAttributeCount");
	m_lfFreeAttributes = (FreeAttributesType)GetProcAddress(m_hLib, "FreeAttributes");
	m_lfCloseAttributeManager = (CloseAttributeManagerType)GetProcAddress(m_hMgr, "CloseAttributeManager");
	m_lfAddAttributeW = (AddAttributeWType)GetProcAddress(m_hLib, "AddAttributeW");
	m_lfGetAttributeName = (GetAttributeNameType)GetProcAddress(m_hLib, "GetAttributeName");
	m_lfGetAttributeValue = (GetAttributeValueType)GetProcAddress(m_hLib, "GetAttributeValue");
	m_lfWriteResourceAttributesW = (WriteResourceAttributesWType)GetProcAddress(m_hMgr, "WriteResourceAttributesW");
	m_lfRemoveResourceAttributesW = (RemoveResourceAttributesWType)GetProcAddress(m_hMgr, "RemoveResourceAttributesW");

	m_lConvert4GetAttr = (Convert4GetAttr)GetProcAddress(m_hMgr, "Convert_GetAttributes");
	m_lConvert4SetAttr = (Convert4SetAttr)GetProcAddress(m_hMgr, "Convert_SetAttributes");

	if (!(m_lfCreateAttributeManager && m_lfAllocAttributes &&
		m_lfReadResourceAttributesW && m_lfGetAttributeCount &&
		m_lfFreeAttributes && m_lfCloseAttributeManager && m_lfAddAttributeW &&
		m_lfGetAttributeName && m_lfGetAttributeValue &&
		m_lfWriteResourceAttributesW&& m_lfRemoveResourceAttributesW &&
		m_lConvert4GetAttr && m_lConvert4SetAttr))
	{
		NLPRINT_DEBUGLOG(L"failed to get resattrlib/resattrmgr functions\r\n");
		return false;
	}
	return true;
}

DWORD CNLTag::GetIEVersion()
{
	LONG    lResult = 0;
	HKEY    hKey = NULL;

#pragma chMSG( "Why the default value is eight not zero" )
	DWORD dwIEVersion = 8;

	lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS == lResult && NULL != hKey)
	{
		DWORD dwType = 0, cbData = MAX_PATH;
		char  szData[MAX_PATH + 1] = { 0 };
		lResult = RegQueryValueEx(hKey, L"Version", 0, &dwType, (LPBYTE)szData, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
#if 0
			if ('7' == szData[0])	dwIEVersion = 7;
			else if ('6' == szData[0])	dwIEVersion = 6;
			else if ('8' == szData[0])	dwIEVersion = 8;
			else if ('9' == szData[0])	dwIEVersion = 9;
#else
			dwIEVersion = szData[0] - '0';
#endif
		}
		RegCloseKey(hKey);
	}
	return dwIEVersion;
}

bool CNLTag::NLIsEncryptFileInherentTag(_In_ const wchar_t* pwchTagName)
{
	if (NULL == pwchTagName)
	{
		return false;
	}

	return (0 == _wcsicmp(pwchTagName, g_pwchSeTagNameEncrypted) ||
		0 == _wcsicmp(pwchTagName, g_pwchSeTagNameKeyRingName) ||
		0 == _wcsicmp(pwchTagName, g_pwchSeTagNameRequires) ||
		0 == _wcsicmp(pwchTagName, g_pwchSeTagNameWrapped));
}

bool CNLTag::NLGetEffectFilePath(_Inout_ wstring& wstrEffectFilePath)
{
	if (NLIsHttpPath(wstrEffectFilePath))
	{
		NLHttpPathToUncPath(wstrEffectFilePath);
	}
    else if (NLIsFilePath(wstrEffectFilePath))
    {
        NLFilePathToUncPath(wstrEffectFilePath);
    }
	return PathFileExists(wstrEffectFilePath.c_str());
}

bool CNLTag::NLHttpPathToUncPath(_Inout_ wstring& wstrFilePath)
{
	/* 1. convert the HTTP path: http://, https://, http:\\, https:\\ */
	if (boost::algorithm::istarts_with(wstrFilePath, L"http://"))
	{
		ConvertURLCharacterW(wstrFilePath);
		boost::algorithm::ireplace_first(wstrFilePath, L"http://", L"");
	}
	else if (boost::algorithm::istarts_with(wstrFilePath, L"https://"))
	{
		ConvertURLCharacterW(wstrFilePath);
		boost::algorithm::ireplace_first(wstrFilePath, L"https://", L"");
	}
	else
	{
		return false;
	}

#pragma chMSG( "If we run office and add/read tags in IE, and then we uninstall IE and install a new IE, our PEP the IE version is the old one, may wrong, If we need consider this ?" )
	if (0 == m_dwIEVer)
	{
		m_dwIEVer = GetIEVersion();
	}

	if (m_dwIEVer >= 8)
	{
		size_t nPos = wstrFilePath.find('/');
		if (wstring::npos == nPos)
		{
			wstrFilePath.insert(0, L"\\\\");
		}
		else
		{
			wstrFilePath = L"//" + wstrFilePath.substr(0, nPos) + L"/DavWWWRoot/" + wstrFilePath.substr(nPos + 1);
		}
	}
	else 
	{
		wstrFilePath.insert(0, L"\\\\");
	}

	boost::algorithm::ireplace_all(wstrFilePath, L"/", L"\\");
	boost::algorithm::replace_all(wstrFilePath, L":", L"@");

	return true;
}

  bool CNLTag::NLFilePathToUncPath(_Inout_ wstring& wstrFilePath)
  {
       /* convert the path that starts with file:// */
      if (boost::algorithm::istarts_with(wstrFilePath, L"file://")) 
      {
           ConvertURLCharacterW(wstrFilePath);
      } 
      else
      {
           return false;
      }

      boost::algorithm::replace_all(wstrFilePath, L":", L"@");

      return true;
  }
