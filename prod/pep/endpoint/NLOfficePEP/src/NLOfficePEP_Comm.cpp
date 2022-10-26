#include "stdafx.h"
#include "dllmain.h"
#include <tlhelp32.h>

#include "nlconfig.hpp"
#include <winhttp.h>

#include "NLObMgr.h"
#include "NLOfficePEP_Comm.h"

wchar_t g_szLog[1024]={0};


#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 
//////////////////////////////////////////////////////////////////////////


#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLOFFICE_Comm)

HWND g_hBubbleWnd = NULL;
type_notify2 notify2 = NULL;
//#define NLBYTECOUNT 8



void CNLObligationType::init()
{
	map_[kOblUnknown] = L"UNKNOWN OBLIGATION";
	map_[kOblClassification] = L"DOCUMENT_ENFORCEMENT_ASSISTANT";
	map_[kOblAutoFileTagging] = L"AUTOMATIC_FILE_TAGGING";
	map_[kOblInteractiveFileTagging] = L"INTERACTIVE_FILE_TAGGING";
	map_[kOblViewOverlay] = L"VIEW_OVERLAY";
	map_[kOblPrintOverlay] = L"PRINT_OVERLAY";
	map_[kOblSEEncryption] = L"SE_ENCRYPTION";
}

OblMap CNLObligationType::NLSetObligationFlag(_In_ const UINT32 unFlag)
{
	OblMap stuOfficeObligationMap;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblUnknown] = kOblUnknown & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblClassification] = kOblClassification  & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblAutoFileTagging] = kOblAutoFileTagging & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblInteractiveFileTagging] = kOblInteractiveFileTagging & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblViewOverlay] = kOblViewOverlay  & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblPrintOverlay] = kOblPrintOverlay & unFlag;
	stuOfficeObligationMap.m_mapNLDoObligaitonFlag[kOblSEEncryption] = kOblSEEncryption & unFlag;

	return stuOfficeObligationMap;
}

wstring CNLObligationType::NLGetStringObligationTypeByEnum(_In_ const ObligationType& emObligationType)
{
	map<ObligationType, wstring>::iterator itr = map_.find(emObligationType);
	if (itr != map_.end())
	{
		return map_[emObligationType];
	}
	return map_[kOblUnknown];
}

ObligationType CNLObligationType::NLGetEnumObligationTypeByString(_In_ const wstring& wstrObligationType)
{
	map<ObligationType, wstring>::iterator itr = map_.begin();
	for (; itr != map_.end(); itr++)
	{
		if (wstrObligationType == itr->second)
		{
			return itr->first;
		}
	}
	return kOblUnknown;
}

CNLActionType::CNLActionType()
{
	NLInitialize();
}

CNLActionType::~CNLActionType()
{

}

CNLActionType& CNLActionType::NLGetInstance()
{
	static CNLActionType theActionIns;
	return theActionIns;
}

wstring CNLActionType::NLGetStringActionTypeByEnum(_In_ const OfficeAction& emActionType)
{
	map<OfficeAction, wstring>::iterator itr = map_.find(emActionType);
	if (itr != map_.end())
	{
		return map_[emActionType];
	}
	return map_[kOA_Unknown];
}

OfficeAction CNLActionType::NLGetEnumActionTypeByString(_In_ const wstring& wstrActionType)
{
	map<OfficeAction, wstring>::iterator itr = map_.begin();
	for (; itr != map_.end(); itr++)
	{
		if (wstrActionType == itr->second)
		{
			return itr->first;
		}
	}
	return kOA_Unknown;
}

void CNLActionType::NLInitialize()
{
	map_[kOA_Unknown] = L"UNKNOWN_ACTION";
	map_[kOA_CLASSIFY] = L"CLASSIFY";
	map_[kOA_OPEN] = L"OPEN";
	map_[kOA_EDIT] = L"EDIT";
	map_[kOA_COPY] = L"COPY";
	map_[kOA_PASTE] = L"PASTE";
	map_[kOA_PRINT] = L"PRINT";
	map_[kOA_SEND] = L"SEND";
	map_[kOA_INSERT] = L"CONVERT";
	map_[kOA_CONVERT] = L"CONVERT";
	map_[kOA_CHANGEATTRIBUTES] = L"CHANGE_ATTRIBUTES";
	map_[kOA_CLOSE] = L"CLOSE";
	map_[kOA_SCREENCAPTURE] = L"SCREEN_CAPTURE";
}


CNLFileType::CNLFileType()
{
	NLInitialize();
}

CNLFileType::~CNLFileType()
{

}

CNLFileType& CNLFileType::NLGetInstance()
{
	static CNLFileType theInstance;
	return theInstance;
}

wstring CNLFileType::NLGetStringFileTypeByEnum(_In_ const OfficeFile& emOfficeFileType)
{
	return map_[emOfficeFileType];
}

OfficeFile CNLFileType::NLGetEnumFileTypeByString(_In_ const wstring& wstrFileType)
{
	map<OfficeFile, wstring>::iterator itr = map_.begin();
	for (; itr != map_.end(); itr++)
	{
		if (wstrFileType == itr->second)
		{
			return itr->first;
		}
	}
	return kFileUnknown;
}

void CNLFileType::NLInitialize()
{
	map_[kFileUnknown] = L"UnknownType";

	// word type ( 7 )
	map_[kDoc] = L"doc";
	map_[kDocx] = L"docx";
	map_[kDocm] = L"docm";
	map_[kDotx] = L"dotx";
	map_[kDotm] = L"dotm";
	map_[kDot] = L"dot";
	map_[kOdt] = L"odt";

	// word type ( 10 )
	map_[kXls] = L"xls";
	map_[kXlsx] = L"xlsx";
	map_[kXlsm] = L"xlsm";
	map_[kXlsb] = L"xlsb";
	map_[kXlam] = L"xlam";
	map_[kXla] = L"xla";
	map_[kXlt] = L"xlt";
	map_[kXltm] = L"xltm";
	map_[kXltx] = L"xltx";
	map_[kOds] = L"ods";

	// power point type ( 12 )
	map_[lPptx] = L"pptx";
	map_[kPptm] = L"pptm";
	map_[kPpt] = L"ppt";
	map_[kPotx] = L"potx";
	map_[kPotm] = L"potm";
	map_[kPot] = L"pot";
	map_[kPpsx] = L"ppsx";
	map_[kPpsm] = L"ppsm";
	map_[kPps] = L"pps";
	map_[kPpam] = L"ppam";
	map_[kPpa] = L"ppa";
	map_[kOdp] = L"odp";

	// for convert type ( 2 )
	map_[kPDF] = L"pdf";
	map_[kXPS] = L"xps";
};

//////////////////////////////////////////////////////////////////////////



tySTUNLOFFICE_CACHEDATA::tySTUNLOFFICE_CACHEDATA() : pDoc(NULL), 
bIsEncryptFile(false), bNeedEncrypt(false), bNeedClose(false), 
bNeedReleaseDoc(!pep::isExcelApp())
{

}

tySTUNLOFFICE_CACHEDATA::~tySTUNLOFFICE_CACHEDATA()
{
	if (bNeedReleaseDoc && NULL != pDoc)
	{
		pDoc->Release();
	}
}

// for policy
ActionResult CNxtSDK::DoEvaluation(const WCHAR* pwchSource,
	const OfficeAction& emAction,
	nextlabs::Obligations& obs,
	const WCHAR* pwchDest,
	vector< pair<wstring, wstring> >* pvecSrcAttrs,
	CENoiseLevel_t noise_level,
	const wstring wstrapp_attr_value,
	bool bForceNoCache)
{
	wstring wstrAction = NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum(emAction);
	return CNxtSDK::DoEvaluation(pwchSource, wstrAction.c_str(), obs, pwchDest, pvecSrcAttrs, noise_level, wstrapp_attr_value, bForceNoCache);
}

ActionResult CNxtSDK::DoEvaluation(const WCHAR* source, const WCHAR* action, nextlabs::Obligations& obs, const WCHAR* dest,
	vector< pair<wstring, wstring> >* psrcAttrs,
	CENoiseLevel_t noise_level, const wstring wstrapp_attr_value, bool bForceNoCache)
{
	NLCELOG_ENUM_ENTER(ActionResult)
		NLPRINT_DEBUGLOG(L" The Parameters are: source=%ls, action=%ls, obs=%p, dest=%ls, psrcAttrs=%p, noise_level=%d, wstrapp_attr_value=%ls, bForceNoCache=%ls \n", print_long_string(source), print_long_string(action), &obs, print_long_string(dest), psrcAttrs, noise_level, wstrapp_attr_value.c_str(), bForceNoCache ? L"TRUE" : L"FALSE");


	static const wchar_t* WDE_PLATFORM_RESOURCE_DOCUMENT = L"fso";     // document resource
	ActionResult emOfficeResult = kRtUnknown;
	static nextlabs::cesdk_context context;
	if (NULL == context.m_pApp)
	{
		nextlabs::comm_helper::Init_Cesdk(&context);	// if no initialize, initialize it.
	}

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &context);
	nextlabs::ATTRS theAttrs;
	nextlabs::eval_parms parm;
	parm.SetNoiseLevel(noise_level);

	wstring strAction(action);
	wstring strSrc(source);
	wstring strDst(L"");
	wstring strSType = WDE_PLATFORM_RESOURCE_DOCUMENT;
	wstring strDType = WDE_PLATFORM_RESOURCE_DOCUMENT;

	if (dest != NULL)	strDst = dest;

	/*
	*\ for local path or unc path ,we just disable PC  reading tag.
	*	for http or ftp or other network path, we will disable pc reading tag and reading content.
	* disable read tag, just set "ce::file_custom_attributes_included = yes"
	* disable read tag/content, set "ce::filesystemcheck=no"
	*/
	static const wchar_t* cstr_disablereadtagname = L"ce::file_custom_attributes_included";
	static const wchar_t* cstr_disablereadtagcontentname = L"ce::filesystemcheck";

	pair<wstring, wstring> pairWstrExtraTag;
	if (psrcAttrs != NULL && !psrcAttrs->empty())
	{
		psrcAttrs->push_back(pair<wstring, wstring>(cstr_disablereadtagcontentname, L"no"));
		pairWstrExtraTag.first = cstr_disablereadtagcontentname;
		pairWstrExtraTag.second = L"no";
	}

	parm.SetAction(strAction.c_str());

	//revert to the original value if the converted path is not existed.
	wstring wstrDstBak = strDst;
	ConvertURLCharacterW(strDst);
	if (!PathFileExistsW(strDst.c_str()))
		strDst = wstrDstBak;

	if (dest != NULL) parm.SetTarget(strDst.c_str(), strDType.c_str());
	if (psrcAttrs != NULL && !psrcAttrs->empty())
	{
		vector<pair<wstring, wstring>> vecTag;
		vector<pair<wstring, wstring>>::const_iterator inner, it = psrcAttrs->begin();
		for (; it != psrcAttrs->end(); it++)
		{
			const wstring& strname = (*it).first;
			const wstring& strvalue = (*it).second;

			// we should delete empty tags
			if (strname.empty() || strvalue.empty())
			{
				::OutputDebugStringW(L"**** policy, has empty tags **** \n");
				continue;
			}

			bool bHas = false;
			for (inner = vecTag.begin(); inner != vecTag.end(); inner++)
			{
				const wstring& striname = (*inner).first;
				const wstring& strivalue = (*inner).second;
				if (_wcsicmp(strname.c_str(), striname.c_str()) == 0 &&
					_wcsicmp(strvalue.c_str(), strivalue.c_str()) == 0)
				{
					bHas = true; // delete repeat tags
					break;
				}
			}
			if (!bHas)
			{
				vecTag.push_back(pair<wstring, wstring>(strname, strvalue));
			}
		}
		theAttrs.insert(vecTag.begin(), vecTag.end());
	}
	ConvertURLCharacterW(strSrc);

	if (bForceNoCache)
	{
		wstring strTime = utils::timedate::GetCurrentStringTime();
		theAttrs.insert(pair<wstring, wstring>(L"modified_date", strTime));	// modify the time and the file update, PC will query again and not use the policy cache.
	}
	parm.SetSrc(strSrc.c_str(), strSType.c_str(), &theAttrs);
	nextlabs::ATTRS app_attr;
	wstring app_attr_name(L"NextLabs Enforcer");
	pair<wstring, wstring> app_attr_tag(app_attr_name, wstrapp_attr_value);
	app_attr.insert(app_attr_tag);
	parm.SetApplicationAttrs(&app_attr);

	ptr->Query(&parm);
	obs = ptr->GetObligations();
	if (ptr->IsDenied())	emOfficeResult = kRtPCDeny;

	else emOfficeResult = kRtPCAllow;

	if (kRtPCDeny == emOfficeResult)
	{
		NLPRINT_DEBUGLOG(L"DoEvaluation: denied by the policy, source [%s], dst is [%s],thread id is [%d].\n ", strSrc.c_str(), strDst.c_str(), ::GetCurrentThreadId());
	}
	else
	{
		NLPRINT_DEBUGLOG(L"DoEvaluation: allowed by the policy, source [%s], dst is [%s],thread id is [%d].\n ", strSrc.c_str(), strDst.c_str(), ::GetCurrentThreadId());
	}

	if (psrcAttrs != NULL && psrcAttrs->size() > 0)
	{
		for (vector<std::pair<std::wstring, std::wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++)
		{
			NLPRINT_DEBUGLOG(L"\tTag Name: [%s], Tag Value: [%s]\n", (*it).first.c_str(), (*it).second.c_str());
		}
	}

	// delete the extra tag
	if (NULL != psrcAttrs && !pairWstrExtraTag.first.empty())
	{
		for (vector<pair<wstring, wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++)
		{
			if (0 == wcscmp(it->first.c_str(), pairWstrExtraTag.first.c_str()) &&
				0 == wcscmp(it->second.c_str(), pairWstrExtraTag.second.c_str()))
			{
				psrcAttrs->erase(it);
				break;
			}
		}
	}

    if (noise_level == CE_NOISE_LEVEL_USER_ACTION)
    {
        if(obs.IsObligationSet(OBLIGATION_RICH_USER_MESSAGE) == true)
        {
            const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();

            for( std::list<nextlabs::Obligation>::const_iterator it = ob_list.begin() ; it != ob_list.end() ; ++it ) 
            {
                if( it->name == OBLIGATION_RICH_USER_MESSAGE )
                {
                    nextlabs::ObligationOptions::const_iterator options_it = it->options.begin();

                    std::wstring FirstValue = options_it->second.c_str();
                    ++options_it;
                    std::wstring SecondValue = options_it->second.c_str();

                    ShowBubble(FirstValue.c_str(), 1000 * _wtoi(SecondValue.c_str()), action);

                    break;
                }
            }   
        }
    }

	if (g_bDebugMode_L2)
	{
		wchar_t wszlog[2048] = { 0 };
		StringCchPrintfW(wszlog, 2047, L"DoEvaluation[%s]: source [%s], dst is [%s],thread id is [%d].\n", strAction.c_str(), strSrc.c_str(), strDst.c_str(), ::GetCurrentThreadId());

		wstring wstrTestLog = wszlog;

		if (NULL != psrcAttrs && !pairWstrExtraTag.first.empty())
		{
			for (vector<pair<wstring, wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++)
			{
				// add file tags here
				wchar_t wszlogTag[1024] = { 0 };
				StringCchPrintfW(wszlogTag, 1023, L"\n tag:[ %s = %s ] \n", it->first.c_str(), it->second.c_str());
				wstrTestLog += wszlogTag;
			}
		}

		if (MessageBoxW(GetActiveWindow(), wstrTestLog.c_str(), L"Allow?", MB_YESNO) == IDNO)
		{
			NLCELOG_ENUM_RETURN_VAL(kRtPCDeny)
		}
	}
	NLCELOG_ENUM_RETURN_VAL(emOfficeResult)
}
// end policy



HRESULT AutoWrap( WORD autoType, VARIANT *pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...)
{
	// Check the parameter
	if ( NULL == pDisp )
	{
		return E_FAIL;
	}

	// Variables used...
	DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
	DISPID      dispidNamed = DISPID_PROPERTYPUT;
	DISPID      dispID;
	HRESULT     hr = E_FAIL;
	char        szName[256] = { 0 };
	
	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

	// Get DISPID for name passed...
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
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
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;

	// Handle special-case for property-puts!
	if(autoType & DISPATCH_PROPERTYPUT)
	{
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call!
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
	delete [] pArgs;

	return hr;
}



bool getDocumentPath(_Out_ wstring& wstrPath, _In_ CComPtr<IDispatch> pDoc)
{
	// 1. check parameter
	wstrPath.clear();
	if (NULL != pDoc)
	{
		CComVariant var;
		HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &var, pDoc, L"FullName", 0);
		if (SUCCEEDED(hr) && NULL != var.bstrVal)
		{
			wstrPath = var.bstrVal;
			NLConvertNetFilePathBySepcifiedSeparator(wstrPath);
		}
	}
	return !wstrPath.empty();
}

bool getDocumentPathEx(_Out_ wstring& wstrPath, _In_ CComPtr<IDispatch> pCurDoc)
{
	wstrPath.clear();
	if (getDocumentPath(wstrPath, pCurDoc))
	{
		// judge if it is right file path( word/excel open in IE can not get the right file path )
		if (isOpenInIEFlag(wstrPath))
		{
			wstrPath = NLOBMGRINSTANCE.NLGetIEFilePathCache(pCurDoc);
			NLPRINT_DEBUGLOG(L"Get the file path from IEFilePathCache, filePath:[%s] \n", wstrPath.c_str());

		}
	}

	if (wstrPath.empty())
	{
		wstrPath = NLOBMGRINSTANCE.NLGetCurActiveFilePath();
		NLPRINT_DEBUGLOG(L"Fail to get file path by IDispatch or from IEFilePathCache, now we get the path from current active file cache:[%s] \n", wstrPath.c_str());
	}
	return !wstrPath.empty();
}


CComPtr<IDispatch> getActiveDoc()
{
	CComVariant theComResult;
	wchar_t* pMethod = NULL;

	switch (pep::appType())
	{
	case kAppWord:
		pMethod = L"ActiveDocument";
		break;
	case kAppExcel:
		pMethod = L"ActiveWorkbook";
		break;
	case kAppPPT:
		pMethod = L"ActivePresentation";
		break;
	default:
		return NULL;
		break;
	}

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &theComResult, pep::getApp(), pMethod, 0);
	if (SUCCEEDED(hr) && NULL != theComResult.pdispVal)
	{
		CComPtr<IDispatch> pDoc = theComResult.pdispVal;
		return pDoc;
	}
	else
	{
		NLPRINT_DEBUGLOG(L"Get active pDoc failed \n");
	}
	return NULL;
}

wstring getProctedViewPath(_Inout_ CComPtr<IDispatch>& pDisp, _Out_ CComPtr<IDispatch>& pActDoc)
{
	pActDoc = NULL;

	AppType app_type = pep::appType();

	HRESULT hr = S_OK;
	CComVariant varResult;
	if (app_type == kAppWord || pDisp == NULL)
	{
		CComPtr<IDispatch> theApp = pep::getApp();
		if (NULL == theApp)
		{
			return L"";
		}

		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, theApp, L"ActiveProtectedViewWindow", 0);
		if (FAILED(hr) || varResult.pdispVal == NULL)
		{
			return L"";
		}
		pDisp = varResult.pdispVal;
	}

	// get active pDoc
	wstring wstrAppString;
	switch (app_type)
	{
	case kAppWord:
		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pDisp, L"Document", 0);
		break;
	case kAppExcel:
		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pDisp, L"Workbook", 0);
		break;
	case kAppPPT:
		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, pDisp, L"Presentation", 0);
		break;
	default:
		break;
	}

	// get active file pDoc	
	if (FAILED(hr) || NULL == varResult.pdispVal)
	{
		NLPRINT_DEBUGLOG(L"Get Presentation Failed hr:[0x%x] \n", hr);
	}
	else
	{
		pActDoc = varResult.pdispVal;
	}

	/* Get protect view file path.
	*  Most time we can get the right file path either of the following methods.
	*  But for bug23994 the path that we get from pActDoc is different with pDisp
	*/
	// 1. sometimes the file path we get by pDoc is different pDisp 
	wstring wstrFilePath;
	if (getDocumentPath(wstrFilePath, pActDoc) && !wstrFilePath.empty())
	{
		NLPRINT_DEBUGLOG(L"the protect view file path is:[%s] \n", wstrFilePath.c_str());
	}
	else
	{
#pragma chMSG( "Here we can optimization the code to the function end when we can't get the file path by pActDoc" )
		CComVariant varResultPath;
		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResultPath, pDisp, L"SourcePath", 0);
		if (FAILED(hr))
		{
			return L"";
		}

		CComVariant varResultName;
		hr = AutoWrap(DISPATCH_PROPERTYGET, &varResultName, pDisp, L"SourceName", 0);
		if (FAILED(hr))
		{
			return L"";
		}

		// 2. get protect view file path
		wchar_t wszPath[1024] = { 0 };
		StringCchPrintfW(wszPath, 1023, L"%s\\%s", varResultPath.bstrVal != NULL ? varResultPath.bstrVal : L"", varResultName.bstrVal != NULL ? varResultName.bstrVal : L"");

		wstrFilePath = wszPath;
		NLPRINT_DEBUGLOG(L"the protect view source path varResultName:[%s] \n", wstrFilePath.c_str());
	}

	NLConvertNetFilePathBySepcifiedSeparator(wstrFilePath);
	return wstrFilePath;
}

bool isOpenInIEFlag(_In_ const wstring& wstrPathFlag)
{
//	// I don't sure if this flag depend by IE version
//	// xp 2007, win7 2010
//#define FILENAMEONE_WORDOPENINIE	 L"Document in Windows Internet Explorer" 
//#define FILENAMEONE_EXCELOPENINIE  L"Worksheet in Windows Internet Explorer"
//
//	// win7 2007
//#define FILENAMETWO_WORDOPENINIE	 L"Document in Internet Explorer" 
//#define FILENAMETWO_EXCELOPENINIE  L"Worksheet in Internet Explorer"
//
//#define FILENAME_EXCEL_OBJECT L"Object"

	static const wchar_t* FILENAMEONE_WORDOPENINIE = L"Document in Windows Internet Explorer";
	static const wchar_t* FILENAMEONE_EXCELOPENINIE = L"Worksheet in Windows Internet Explorer";
	static const wchar_t* FILENAMETWO_WORDOPENINIE = L"Document in Internet Explorer";
	static const wchar_t* FILENAMETWO_EXCELOPENINIE = L"Worksheet in Internet Explorer";
	static const wchar_t* FILENAME_EXCEL_OBJECT = L"Object";


	bool bIsOpenInIE = false;
	if (
		(	pep::isWordApp() &&
			(boost::algorithm::istarts_with(wstrPathFlag, FILENAMEONE_WORDOPENINIE) ||
			boost::algorithm::istarts_with(wstrPathFlag, FILENAMETWO_WORDOPENINIE))
		)
		||
		(	pep::isExcelApp() &&
			(boost::algorithm::istarts_with(wstrPathFlag, FILENAMEONE_EXCELOPENINIE) ||
			boost::algorithm::istarts_with(wstrPathFlag, FILENAMETWO_EXCELOPENINIE))
		)
		||
		boost::algorithm::iequals(wstrPathFlag, FILENAME_EXCEL_OBJECT)
		)
	{
		// PPT no this flag
		bIsOpenInIE = true;
	}
	return bIsOpenInIE;
}

bool isOpenInIE()
{
	bool bIsOpenInIE = false;

	CComPtr<IDispatch> pDoc = getActiveDoc();
	if (NULL == pDoc)
	{
		return false;
	}

	wstring wstrFilePath;
	if (getDocumentPath(wstrFilePath, pDoc))
	{
		bIsOpenInIE = isOpenInIEFlag(wstrFilePath);
	}
	return bIsOpenInIE;
}


bool isNewOfficeFile(_In_ const wstring& wstrFilePath)
{
	bool bNewDoc = (
		(pep::isWordApp()	&& boost::algorithm::istarts_with(wstrFilePath, L"Document"))	|| 
		(pep::isExcelApp()	&& boost::algorithm::istarts_with(wstrFilePath, L"Book"))	|| 
		(pep::isPPtApp()	&& boost::algorithm::istarts_with(wstrFilePath, L"Presentation")));
	return bNewDoc;
}

bool NLIsOfficeFile(_In_ const wstring& wstrFilePath)
{
	return isNewOfficeFile(wstrFilePath) ||
		pep::isWordFile(wstrFilePath) ||
		pep::isExcelFile(wstrFilePath) ||
		pep::isPPTFile(wstrFilePath);
}

bool isTemplateFile(_In_ const wstring& wstrFilePath, 
	_In_ const bool bNeedCheckNewDocFlag, 
	_In_ const bool bNeedCheckIEFlag)
{
	// 1. check new document flag
	if (bNeedCheckNewDocFlag)
	{
		if (isNewOfficeFile(wstrFilePath))
		{
			NLPRINT_DEBUGLOGEX(true, L" this file maybe is the new document:[%s] \n", wstrFilePath.c_str());
			return false;
		}
	}

	// 2. check if it is file opened in IE
	if (bNeedCheckIEFlag)
	{
		if (isOpenInIEFlag(wstrFilePath))
		{
			NLPRINT_DEBUGLOGEX(true, L" this file maybe is opened in IE [%s] \n", wstrFilePath.c_str());
			return false;
		}
	}


	/*
	* for template file we just can get a file name, not the file full name. so we just check if the path contain window path separator.
	*/
	return !((wstring::npos != wstrFilePath.find(L'\\')) || (wstring::npos != wstrFilePath.find(L'/')));
}



//////////////////////////////////////////////////////////////////////////


bool NLOpenedByOtherApp()
{
	DWORD dwProcessID = 0;
	DWORD dwCurPID = ::GetCurrentProcessId();

	HWND hWnd = ::GetForegroundWindow();
	if (hWnd != NULL)
	{
		if (::GetWindowThreadProcessId(hWnd, &dwProcessID))
		{
			return (dwProcessID != dwCurPID);
		}
	}
	return true;
}

bool CloseProtectedView(IDispatch * pDoc)
{
	bool bRet = true;
	if (pep::isPPtApp())
	{
		long lHandle;
		PPT::_pptApplicationPtr theApp((IDispatch*)pep::getApp());
		HRESULT hr = theApp->get_HWND(&lHandle);
		if (SUCCEEDED(hr))	::PostMessage((HWND)lHandle, WM_CLOSE, 0, 0);
		else	bRet = false;
	}
	else
	{
		CComVariant varResult;
		HRESULT hr = AutoWrap(DISPATCH_METHOD, &varResult, pDoc, L"Close", 0);
		if (FAILED(hr))	bRet = false;
	}
	return bRet;
}

bool NLIsValidFileForInsertPicture(_In_ const wstring& wstrFile)
{
	if (boost::algorithm::iends_with(wstrFile, L".emf") ||
		boost::algorithm::iends_with(wstrFile, L".wmf") ||
		boost::algorithm::iends_with(wstrFile, L".jpg") ||
		boost::algorithm::iends_with(wstrFile, L".jpeg") ||
		boost::algorithm::iends_with(wstrFile, L".jfif") ||
		boost::algorithm::iends_with(wstrFile, L".jpe") ||
		boost::algorithm::iends_with(wstrFile, L".png") ||
		boost::algorithm::iends_with(wstrFile, L".bmp") ||
		boost::algorithm::iends_with(wstrFile, L".dib") ||
		boost::algorithm::iends_with(wstrFile, L".bmz") ||
		boost::algorithm::iends_with(wstrFile, L".gif") ||
		boost::algorithm::iends_with(wstrFile, L".gfa") ||
		boost::algorithm::iends_with(wstrFile, L".emz") ||
        boost::algorithm::iends_with(wstrFile, L".tif") ||
        boost::algorithm::iends_with(wstrFile, L".tiff"))
	{
		return true;
	}
	return false;
}
