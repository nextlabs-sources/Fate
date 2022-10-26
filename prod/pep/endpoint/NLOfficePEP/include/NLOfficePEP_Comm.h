#pragma once
#include "utils.h"
#include <winhttp.h>	// for HINTERNET

#pragma warning( push )
#pragma warning( disable: 4995 4819 )
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/policy/comm_helper.hpp"
#pragma warning( pop )

using namespace nextlabs;

extern HINSTANCE g_hInstance;
extern wstring	 g_strModulePath;
extern CRITICAL_SECTION showbubbleCriticalSection;


struct BubbleStruct
{
    const wchar_t* pHyperLink;
    const wchar_t* pAction;
};

typedef struct  
{
    ULONG ulSize;
    WCHAR methodName [64];
    WCHAR params [7][256];
}NOTIFY_INFO;

typedef void (__stdcall* type_notify2)(NOTIFY_INFO* pInfo, int nDuration, HWND hCallingWnd, void* pReserved, const WCHAR* pHyperLink);
extern type_notify2 notify2;

extern HWND g_hBubbleWnd;



typedef enum tagOfficeVer
{
	kUnknown,
	kVer2003,
	kVer2007,
	kVer2010,
	kVer2013,
	kVer2016
}OfficeVer;

// office application type
typedef enum tagAppType
{
	kAppUnk = 0,
	kAppWord,
	kAppExcel,
	kAppPPT
}AppType;

typedef enum tagObligationType
{
	kOblUnknown = 0x00,
	kOblClassification = 0x01,
	kOblAutoFileTagging = 0x02,
	kOblInteractiveFileTagging = 0x04,
	kOblViewOverlay = 0x08,
	kOblPrintOverlay = 0x10,
	kOblSEEncryption = 0x20
}ObligationType;

// this struct is only used for action and obligation relations
typedef struct tagOblMap
{
	map<ObligationType, bool> m_mapNLDoObligaitonFlag;
}OblMap;



#define NLOBLIGATIONTYPEINSTANCE ( CNLObligationType::NLGetInstance() )
#define OBLIGATION_RICH_USER_MESSAGE     L"RICH_USER_MESSAGE"
class CNLObligationType
{
private:
	CNLObligationType() { init(); }
	~CNLObligationType() {}
public:
	inline static CNLObligationType& NLGetInstance()
	{
		static CNLObligationType theInstance;
		return theInstance;
	}

public:
	wstring NLGetStringObligationTypeByEnum(_In_ const ObligationType& emObligationType);
	ObligationType NLGetEnumObligationTypeByString(_In_ const wstring& wstrObligationType);

	OblMap NLSetObligationFlag(_In_ const UINT32 unFlag = 0);

private:
	void init();

private:
	map<ObligationType, wstring> map_;
};


//////////////////////////////office action type ///////////////////////////////////////////////////////////
#define NLACTIONTYPEINSTANCE ( CNLActionType::NLGetInstance() )

typedef enum tagOfficeAction
{
	kOA_Unknown = 0,
	kOA_CLASSIFY,
	kOA_OPEN,
	kOA_EDIT,
	kOA_COPY,
	kOA_PASTE,				// copy content, cut content, copy select slides
	kOA_PRINT,
	kOA_SEND,				// send, publish: don't care the destination path
	kOA_INSERT,			// Insert/Data use this, but there are convert action, but it need process dependence
	kOA_CONVERT,			// save as other format: don't care the destination path
	kOA_CHANGEATTRIBUTES,	// office pep may be no this action
	kOA_CLOSE,
	kOA_SCREENCAPTURE
}OfficeAction;

class CNLActionType
{
private:
	CNLActionType();
	~CNLActionType();
public:
	static CNLActionType& NLGetInstance();

public:
	wstring NLGetStringActionTypeByEnum(_In_ const OfficeAction& emActionType);
	OfficeAction NLGetEnumActionTypeByString(_In_ const wstring& wstrActionType);

private:
	void NLInitialize();
private:
	map<OfficeAction, wstring> map_;
};


///////////////////////////////office type: file type, event type///////////////////////////////////////////
typedef enum tagOfficeFileType
{
	kFileUnknown = 0,

	// word type ( 7 )
	kDoc,	// 97-2003
	kDocx,
	kDocm,
	kDotx,
	kDotm,
	kDot,
	kOdt,

	// word type ( 10 )
	kXls,
	kXlsx,
	kXlsm,
	kXlsb,
	kXlam,
	kXla,
	kXlt,
	kXltm,
	kXltx,
	kOds,

	// power point type ( 12 )
	lPptx,
	kPptm,
	kPpt,
	kPotx,
	kPotm,
	kPot,
	kPpsx,
	kPpsm,
	kPps,
	kPpam,
	kPpa,
	kOdp,

	// for convert type ( 2 )
	kPDF,
	kXPS
}OfficeFile;

#define NLFILETYPEINSTANCE ( CNLFileType::NLGetInstance() )

class CNLFileType
{
private:
	CNLFileType();
	~CNLFileType();
public:
	static CNLFileType& NLGetInstance();

public:
	wstring NLGetStringFileTypeByEnum(_In_ const OfficeFile& emOfficeFileType);
	OfficeFile NLGetEnumFileTypeByString(_In_ const wstring& wstrFileType);

private:
	void NLInitialize();
private:
	map<OfficeFile, wstring> map_;
};

//////////////////////////////
// get user choose from dialog, normal: ok, cancel, apply or dialog error
typedef enum tyEMNLOFFICE_USERCLICKONDIALOG
{
	emDialogClickUnknown = 0,

	emDialogClickOk,
	emDialogClickCancel,
	emDialogClickApply
}EMNLOFFICE_USERCLICKONDIALOG;



// office: insert type
typedef enum tyEMNLOFFICE_INSERTTYPE
{
	kInsertUnkown = 0,

	kInsertObj,
	kInsertText_WordOnly,					// insert->Object->Text From file[convert] only for word
	kInsertPic,
	kInsertDataFromAccess_ExcelOnly,			// only for excel data->from access
	kDataFromText_ExcelOnly				// only for excel data->from text
}InsertType;

// paste content type
typedef enum tyEMNLOFFICE_PASTECONTENTTYPE
{
	emPateContentUnknownType = 0,

	emCutContent,
	emCopyContent,
	emCopyAsAPicture,
	emPateContent,
	emCopySelectedSlides,		// only for PPT slides 
	emMoveOrCopy,
}EMNLOFFICE_PASTECONTENTTYPE;

/*
*	office XML ribbon event define
*/
typedef enum tagRibbonEvent
{
	kRibbonUnknown = 0,

	kRibbonSave,
	kRibbonSaveAs,
	kRibbonConvert,
	kRibbonSend,
	kRibbonPublish,
	kRibbonPasteContent,
	kRibbonInsert,
	kRibbonExcelDataTab,								// now this is only for excel

	kRibbonViewNewWindow,		// View->NewWindow, used for overlay
	kRibbonViewZoom,					// View->Zoom, used for overlay

	kRibbonNextLabsRibbonRightProtected	// for NextLabs define ribbon button: right protect
}RibbonEvent;

typedef union tyUNNLOFFICE_EVENTEXTRAFLAG
{
	OfficeFile					emExtraDesFileType;
	InsertType				emExtraInsertType;
	EMNLOFFICE_PASTECONTENTTYPE	emExtraPasteContentType;

	/*
	* if we want to use this union, we must initialize this by ourself.
	* Because: if we use the default value and can initialize one the enum. If all these enum are start as a same number, the initialize will be right.
	* if not, other enum will be initialize as a unknown number and this not we want. Sometimes it may be cause some error and very hard to find it.
	*/
	tyUNNLOFFICE_EVENTEXTRAFLAG(OfficeFile			emParamExtraDesFileType) : emExtraDesFileType(emParamExtraDesFileType) {  }
	tyUNNLOFFICE_EVENTEXTRAFLAG(InsertType	emParamExtraInsertType) : emExtraInsertType(emParamExtraInsertType) {  }
	tyUNNLOFFICE_EVENTEXTRAFLAG(EMNLOFFICE_PASTECONTENTTYPE	emParamExtraPasteContentType) : emExtraPasteContentType(emParamExtraPasteContentType) {  }

private:
	tyUNNLOFFICE_EVENTEXTRAFLAG(){};	// to make sure it is safe
}UNNLOFFICE_EVENTEXTRAFLAG;

//////////////////////////////////end////////////////////////////////////////

//////////////////////////////// office COM notification event begin//////////////////////////////////////////

// word event ( 7 )
typedef enum tagWordEvent
{
	kWdUnknown = 0x0,

	kWdStartup = 0x01,
	kWdDocChange = 0x03,
	kWdDocOpen = 0x04,
	kWdDocBeforeClose = 0x06,
	kWdDocBeforeSave = 0x08,
	kWdNewDoc = 0x09,
	kWdWndActivate = 0x0A,
	kWdProtectedViewWindowOpen = 0x1f,
	kWdProtectedViewWindowBeforeClose = 0x21
}WordEvent;

// excel event ( 11 )
typedef enum tagExcelEvent
{
	emExcelEventUnknown = 0x0,

	emExcelWindowsDeactivate = 0x615,
	emExcelOpen = 0x61F,
	emExcelSave = 0x623,
	emExcelClose = 0x622,
	emExcelPrint = 0x624,
	emExelWBActive = 0x620,		// cache the path in this event and used to API hook.!!!!!!!
	emProtectedViewOpen = 0xB57,
	emExcelSelectionChange = 0x616,		// for drag/drop cell from excel
	emProtectedViewWindowActivate = 0xB5D,
	emExcelHyperLink = 0x73E,
	emWindowsActive = 0x614,
	emExcelProtectedViewWindowBeforeClose = 0xb5a
}ExcelEvent;

// PPT event ( 12 )
typedef enum tagPPTEvent
{
	emPPTEventUnknown = 0x0,
	emPPTPresentationOpen = 0x7D6,	// before open
	emPPTAfterPresentationOpen = 0x7E5,	// after open
	emPPTAfterNewPresentation = 0x7E4,
	emPPTWinSelectionChange = 0x7D1,
	emPPTBeforeClose = 0x7D4,
	emPPTSlideShow = 0x7DB,
	emPPTSave = 0x7E2,
	emPPTAferSaveAs = 0x7D5,
	emPPTWindowActivate = 0x7D9,
	emPPTPrint = 0x7DF,
	emPPTProtectedViewWindowOpen = 0x7EA,
	emPPTSlideShowNextClick = 0x7E3,
	emPPTProtectedViewWindowBeforeClose = 0x7EC
}PPTEvent;

typedef union tagEventFlag
{
	WordEvent  fword;
	ExcelEvent fexcel;
	PPTEvent	 fppt;

	tagEventFlag(WordEvent we) : fword(we) {  }
	tagEventFlag(ExcelEvent ee) : fexcel(ee) {  }
	tagEventFlag(PPTEvent	pe) : fppt(pe) {  }
private:
	tagEventFlag(){};	// to make sure it is safe
}EventFlag;

////////////////////////////////////////////////////////////////////end////////////////////////////////////////////////////////////////////////////////

// office event trigger point
typedef enum tagEventType
{
	kEventTypeUnknown = 0,
	kFireByRibbonUI,
	kFireByComNotify,
	kFireByHookAPI,
	kFireByPEP
}EventType;

///////////////////////////////office pep function return value///////////////////////////////////////////

// query policy
typedef enum tagPolicyStat
{
	kPSUnknown = 0,		// default work flow is continue	
	kPSAllow,
	kPSDeny
}PolicyStat;

// normal error describe
typedef enum tagFuncStat
{
	kFSUnknown = 0,				// default value

	kFSSuccess,						// function invoke success
	kFSFailed,
	kFSParmErr,
	kFSWriteTagErr,
	KFSReadTagErr,
	kFSExcpt
}FuncStat;

// return value: enum 
typedef enum tagActionResult
{
	kRtUnknown = 0,		// Unknown error, no means

	// function failed
	kRtException,			// function run exception
	kRtFailed,					// function invoke failed

	// function success
	kRtSuccess = 0x100,	// function invoke success
	kRtPCAllow,			// function invoke success and the policy is allow 
	kRtPCDeny				// function invoke success but the policy is deny
}ActionResult;

typedef enum tyEMNLOFFICE_HOOKEVENT
{
	emOfficeHookEventUnknown,

	// this is only for excel key board event
	emOfficeHookKeyBoardCtrlC,
	emOfficeHookKeyBoardCtrlX
}EMNLOFFICE_HOOKEVENT;

typedef enum tyEMNLOFFICE_HOOKMETHOD
{
	emOfficeHookMethodUnknown,

	emOfficeHookMethodByHookAPI,
	emOfficeHookMethodByHookCode,
	emOfficeHookMethodByHookUI,
	emOfficeHookMethodByHookKeyBoard
}EMNLOFFICE_HOOKMETHOD;

// return value: structure 
typedef struct tagProcessResult
{
	FuncStat		kFuncStat;
	PolicyStat		kPolicyStat;
	OfficeAction			kAction;
	VARIANT_BOOL				vbCancel;

	// for debug, to record the function name and the file line: __FUNCTIONW__, __LINE__
	wstring						fname;
	int							line;

	tagProcessResult(FuncStat    emParamFunctionStatus = kFSUnknown,
		PolicyStat emParamPolicyStatus = kPSUnknown,
		OfficeAction                 emParamAction = kOA_Unknown,
		VARIANT_BOOL bParamVariantCancel = VARIANT_FALSE,
		wstring wstrParamFunctionName = L"",
		int         unParamline = 0)
		:kFuncStat(emParamFunctionStatus), kPolicyStat(emParamPolicyStatus), kAction(emParamAction),
		vbCancel(bParamVariantCancel), fname(wstrParamFunctionName), line(unParamline) {}
}ProcessResult;

// Ribbon event struct, used as a parameter to set event
typedef struct tySTUNLOFFICE_RIBBONEVENT
{
	RibbonEvent		rbEvent;					// XML ribbon event 
	UNNLOFFICE_EVENTEXTRAFLAG	unOfficeEventExtraType;	// Extra flag to describe the event: insert, data, save as, paste content
	VARIANT_BOOL							bVariantCancel;					// this is an output parameter, return false continue, true deny

	// for safe no default values, here we must initialize this structure by ourself.
	tySTUNLOFFICE_RIBBONEVENT(RibbonEvent   emParamRibbonEvent = kRibbonUnknown,
		UNNLOFFICE_EVENTEXTRAFLAG				unOfficeParamEventExtraType = kFileUnknown,
		VARIANT_BOOL								    bParamVariantCancel = VARIANT_FALSE)
		:rbEvent(emParamRibbonEvent), unOfficeEventExtraType(unOfficeParamEventExtraType), bVariantCancel(bParamVariantCancel) {  }
}STUNLOFFICE_RIBBONEVENT;

// Office COM notify event struct, used as a parameter to set event 
typedef struct tagNotifyEvent
{
	EventFlag eventflag;		// Com notify describe
	CComPtr<IDispatch>				 pDoc;					// Com interface
	VARIANT_BOOL bVariantSaveAsUI;		// for save event, if it is save action ( new widow or no write permission file is save as )
	VARIANT_BOOL bVariantCancel;			// VARIANT_FALSE, allow continue
	VARIANT_BOOL bProtectedViewOpen;	//indicate if current file opened in protected view
	VARIANT_BOOL bProtectedViewClose;	//indicate if current file opened in protected view

	tagNotifyEvent(EventFlag flag = kWdUnknown, 
		CComPtr<IDispatch> pdoc = NULL,
		VARIANT_BOOL vbSaveAsUI = VARIANT_FALSE, 
		VARIANT_BOOL vbCancel = VARIANT_FALSE, 
		VARIANT_BOOL vbProtectedViewOpen = VARIANT_FALSE, 
		VARIANT_BOOL vbProtectedViewClose = VARIANT_FALSE)
		:eventflag(flag), 
		pDoc(pdoc), 
		bVariantSaveAsUI(vbSaveAsUI), 
		bVariantCancel(vbCancel), 
		bProtectedViewOpen(vbProtectedViewOpen), 
		bProtectedViewClose(vbProtectedViewClose) {}
}NotifyEvent;

// Hook API event struct, used as a parameter to set event
typedef struct tagHookEvent
{
	EMNLOFFICE_HOOKEVENT  emOfficeHookEvent;
	EMNLOFFICE_HOOKMETHOD emOfficeHookMethod;

	tagHookEvent(EMNLOFFICE_HOOKEVENT emParamOfficeHookEvent = emOfficeHookEventUnknown,
		EMNLOFFICE_HOOKMETHOD emParamOfficeHookMethod = emOfficeHookMethodUnknown)
		: emOfficeHookEvent(emParamOfficeHookEvent), emOfficeHookMethod(emParamOfficeHookMethod) {}
}HookEvent;

///////////////////////////////////end///////////////////////////////////////

/////////////////////////////////////////////////////////this structure is used to record the current runtime status///////////////////////////////////////////////////////////////////////////////////////////
typedef struct tySTUNLOFFICE_EVENTSTATUS
{
	STUNLOFFICE_RIBBONEVENT    stuCurRibbonEvent;
	NotifyEvent stuCurComNotify;
	HookEvent	     stuCurHookEvent;

	tySTUNLOFFICE_EVENTSTATUS(){}
	tySTUNLOFFICE_EVENTSTATUS(STUNLOFFICE_RIBBONEVENT    stuParamCurRibbonEvent) : stuCurRibbonEvent(stuParamCurRibbonEvent){}
	tySTUNLOFFICE_EVENTSTATUS(NotifyEvent stuParamCurComNotify) : stuCurComNotify(stuParamCurComNotify) {}
	tySTUNLOFFICE_EVENTSTATUS(HookEvent	     stuParamCurHookEvent) : stuCurHookEvent(stuParamCurHookEvent) {}
}STUNLOFFICE_EVENTSTATUS;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////For golden tags cache///////////////////////////////////////////
typedef struct tySTUNLOFFICE_GOLDENTAG
{
	vector<pair<wstring, wstring>> vecGoldeTags;
}STUNLOFFICE_GOLDENTAG;


///////////////////////////////

typedef struct tagActionFlag
{
	UNNLOFFICE_EVENTEXTRAFLAG unActionExtraFlag;	// the detail flag is depend about the action

	tagActionFlag(UNNLOFFICE_EVENTEXTRAFLAG unParamActionExtraFlag = kInsertUnkown)
		: unActionExtraFlag(unParamActionExtraFlag) { }
}ActionFlag;

typedef struct tySTUNLOFFICE_CACHEDATA
{
	vector<pair<wstring, wstring>> vecTagPair;		// file tags
	IDispatch* pDoc;			// Change from CComPtr to normal pointer to fix a problem: exel process cannot quit if open a file in IE.
	bool bNeedReleaseDoc;	// this used for control pDoc release.for word new document or template file no need release FixBug24688

	bool bIsEncryptFile;	// if this is an encrypt file
	bool bNeedEncrypt;	// if there is file encryption requirement
	bool bNeedClose;		// Close file flag, some case, like PPT opened by other applications but our policy need close it and we can't close it at once we can set this flag

	// save the current event flag
	STUNLOFFICE_RIBBONEVENT stuCurRibbonEvent;
	NotifyEvent stuCurComNotify;

	// no need handle flag, we can check this flag to ignore the action that triggered by our PEP.
	map<OfficeAction, bool> mapNoNeedHandleFlag;

	// action startup flag, we can check this flag to do our event at the hook function. like: win7 save as, insert, data, classify ...  
	map<OfficeAction, pair<bool, ActionFlag> > mapActionStartupFlag;

	tySTUNLOFFICE_CACHEDATA();
	~tySTUNLOFFICE_CACHEDATA();
}STUNLOFFICE_CACHEDATA;





////////////////////////Helper///////////////////////
HRESULT AutoWrap( WORD autoType, VARIANT *pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...);


class CNxtSDK
{
public:
	/* DoEvaluation
	*	noise_level: default value is CE_NOISE_LEVEL_USER_ACTION, if no need to pop up bubble, can set it as: CE_NOISE_LEVEL_APPLICATION
	*	if it is new document it must force no cache( bForceNoCache = true )
	*/
	static ActionResult DoEvaluation(const WCHAR* pwchSource,
		const OfficeAction& emAction,
		nextlabs::Obligations& obs,
		const WCHAR* pwchDest = L"",
		vector<pair<wstring, wstring> >* pvecSrcAttrs = NULL,
		CENoiseLevel_t noise_level = CE_NOISE_LEVEL_USER_ACTION,
		const wstring wstrapp_attr_value = L"OfficePEP",
		bool bForceNoCache = true);

	static ActionResult DoEvaluation(const WCHAR* source,
		const WCHAR* action,
		nextlabs::Obligations& obs,
		const WCHAR* dest = L"",
		vector<pair<wstring, wstring> >* psrcAttrs = NULL,
		CENoiseLevel_t noise_level = CE_NOISE_LEVEL_USER_ACTION,
		const wstring wstrapp_attr_value = L"OfficePEP",
		bool bForceNoCache = true);

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

    static DWORD WINAPI BubbleThreadProc(LPVOID lpParameter)
    {
        WCHAR CurrentDir[MAX_PATH] = { 0 };

        GetModuleFileNameW(g_hInstance, CurrentDir, MAX_PATH);

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
        wcex.hInstance		= g_hInstance;
        wcex.hIcon			= NULL;
        wcex.hCursor		= NULL;
        wcex.hbrBackground	= NULL;
        wcex.lpszMenuName	= NULL;
        wcex.lpszClassName	= L"BubbleClass";
        wcex.hIconSm		= NULL;

        RegisterClassExW(&wcex);

        g_hBubbleWnd = CreateWindowW(L"BubbleClass", NULL, 0, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);;

        SetEvent((HANDLE)lpParameter);

        MSG msg = { 0 };

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return 0;
    }

    static void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction)
    {
        static bool bFirst = false;

        if (!bFirst)
        {
            ::EnterCriticalSection(&showbubbleCriticalSection);
            if (!bFirst)
            {
                HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

                CreateThread(NULL, 0, BubbleThreadProc, hEvent, 0, NULL);

                WaitForSingleObject(hEvent, INFINITE);
                CloseHandle(hEvent);

                bFirst = true;
            }    
            ::LeaveCriticalSection(&showbubbleCriticalSection);
        }

        BubbleStruct ThisBubble = {pHyperLink, pAction};

        SendMessage(g_hBubbleWnd, WM_USER + 1, (WPARAM)&ThisBubble, Timeout);
    }

};






inline bool NLIsHttpPath(_In_ const wstring& wstrFilePath)
{
	return boost::algorithm::istarts_with(wstrFilePath, L"http");
}

inline bool NLIsFilePath(_In_ const wstring& wstrFilePath)
{
    return boost::algorithm::istarts_with(wstrFilePath,L"file://");
}

inline void NLConvertNetFilePathBySepcifiedSeparator(wstring& wstrFilePath,
	const wchar_t* const pwchOriginalSeparator = L"\\",
	const wchar_t* const pwchUnionSeparator = L"/")
{
	if (!wstrFilePath.empty() && NLIsHttpPath(wstrFilePath))
	{
		boost::algorithm::replace_all(wstrFilePath, pwchOriginalSeparator, pwchUnionSeparator);
	}
}

inline void NLConvertFilePathBySepcifiedSeparator(wstring& wstrFilePath,
	const wchar_t* const pwchOriginalSeparator = L"\\",
	const wchar_t* const pwchUnionSeparator = L"/")
{
	boost::algorithm::replace_all(wstrFilePath, pwchOriginalSeparator, pwchUnionSeparator);
}


bool getDocumentPath(_Out_ wstring& wstrPath, _In_ CComPtr<IDispatch> pDoc);


// if we can not get right file path by pDoc, it will return the path in the cache 
bool getDocumentPathEx(_Out_ wstring& wstrPath, _In_ CComPtr<IDispatch> pCurDoc);


CComPtr<IDispatch> getActiveDoc();

// For protect view
/*
*\ Brief: this function used for protect view. it return file real path and the current file active document IDispath.
*\ Parameter
*		[inOut] pDisp: this is protect view active window IDispath and we can pass an empty pointer for it.
*									if it is NULl or word application, we will get it in the function and out to user.
*		[Out]   pActDoc: the current active document IDispath which opened with protect view.
*\ Return value:
*		return the current file path, if the function failed, the return value will be empty.
*/
wstring getProctedViewPath(_Inout_ CComPtr<IDispatch>& pDisp, _Out_ CComPtr<IDispatch>& pActDoc);


bool isOpenInIEFlag(_In_ const wstring& wstrPathFlag);


bool isOpenInIE();


bool isNewOfficeFile(_In_ const wstring& wstrFilePath);

bool NLIsOfficeFile(_In_ const wstring& wstrFilePath);

/*
*  this function used to judge if it is the template file, but this function is not accurately,
*  now this function just use to solve some special case, like Bug24714
*  if you want to use this function you must known the details about the template
*  the context when the invoked.
*/
bool isTemplateFile(_In_ const wstring& wstrFilePath,
	_In_ const bool bNeedCheckNewDocFlag,
	_In_ const bool bNeedCheckIEFlag);



bool NLOpenedByOtherApp();


bool CloseProtectedView(_In_ IDispatch* pDoc);

bool NLIsValidFileForInsertPicture(_In_ const wstring& wstrFile);
