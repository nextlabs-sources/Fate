#include "OverLay.h"
#include "COverlayTool.h"
#include "celog.h"


#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 
#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_FORCEADOBEPEP_SRC_OVERLAY_CPP

static HMODULE pa_labeling_mod=NULL;

HWND  COverLay::m_hViewWnd = NULL;

COverLay::COverLay(void)
{
	m_hDC = NULL;
	m_gdiplusToken = NULL;
	m_pPrintVLFunc = NULL;
	m_hPrintVLDLL = NULL;
	GetPrintVLFunAddr();
}

COverLay::~COverLay(void)
{
	FreePrintVLAddr();
}

wstring GetFileName(const wstring& str)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: str=%ls \n",str.c_str() );

	if(str.empty())
	{
		return str;
	}
	std::wstring::size_type pos, len, dotpos;
	char cslash = '\\';
	pos = str.find('/');
	if(pos != std::wstring::npos)
	{
		cslash = '/';
	}
	pos = str.rfind(cslash);
	if(pos == std::wstring::npos)
	{
		len = str.length();
		dotpos = str.rfind('.');
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: cslash=%c, pos=%u, len=%u, dotpos=%u \n", cslash,pos,len,dotpos );
		if(dotpos == std::wstring::npos)
		{
			return str;
		}
		return str.substr(0,dotpos);
	}
	len = str.length();
	dotpos = str.rfind('.');
    CELOG_LOG(CELOG_DUMP, L"Local variables are: cslash=%c, pos=%u, len=%u, dotpos=%u \n", cslash,pos,len,dotpos );

	if((dotpos == std::wstring::npos)||(dotpos < pos))
	{
		return str.substr(pos + 1,len - pos);
	}
	return str.substr(pos + 1,dotpos - pos - 1);
}

const wchar_t* COPYOF_CEM_GetString(CEString cestr)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: cestr=%p \n",&cestr );

	if( cestr == NULL )
	{
		return NULL;
	}
	return (((_CEString*)cestr)->buf);
}/* COPYOF_CEM_GetString */

BOOL CALLBACK EnumAdobeChildProc(HWND hwnd, LPARAM lParam) 
{ 
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hwnd=%p, lParam=%ld \n",hwnd,lParam );

	if(hwnd != NULL)
	{
		wchar_t strWinCaption[MAX_PATH] = {0};
		::GetWindowText(hwnd,strWinCaption,MAX_PATH);
		
		if(0 == wcscmp(strWinCaption,L"AVPageView"))
		{
			PADOBE_WND_INFO pAdobeWndInfo = reinterpret_cast<PADOBE_WND_INFO>(lParam);
			if(pAdobeWndInfo != NULL)	pAdobeWndInfo->hAdobeView = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CALLBACK EnumAdobeProc(HWND hwnd, LPARAM lParam) 
{ 
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: hwnd=%p, lParam=%ld \n",hwnd,lParam );

	if(hwnd != NULL)
	{
		PADOBE_WND_INFO pAdobeWndInfo = reinterpret_cast<PADOBE_WND_INFO>(lParam);
		if(pAdobeWndInfo == NULL)	return TRUE;
		wchar_t* strFileName = pAdobeWndInfo->strFilePath;
		wchar_t strWinCaption[MAX_PATH] = {0};
		::GetWindowText(hwnd,strWinCaption,MAX_PATH);

		wchar_t strWinCls[MAX_PATH] = {0};
		::GetClassName(hwnd,strWinCls,MAX_PATH);
	
		if(boost::algorithm::iends_with(strWinCaption,ADOBE_READER_SUFFIX) || boost::algorithm::iends_with(strWinCaption,ADOBE_BAT_SUFFIX) || 0 == _wcsicmp(strWinCls,L"IEFrame"))
		{
			wstring strTitle = GetFileName(strWinCaption);
			if(strTitle.empty())
			{
				CELOG_LOG(CELOG_DUMP, L"Local variables are: strFileName=%ls, strWinCaption=%ls, strWinCls=%ls \n", print_long_string(strFileName),print_long_string(strWinCaption),print_long_string(strWinCls) );
				return TRUE;
			}

			boost::algorithm::ireplace_last(strTitle,ADOBE_READER_SUFFIX,L"");

			wstring strFilePath(strFileName);
			wstring strname = GetFileName(strFilePath);

			if(_wcsicmp(strname.c_str(),strTitle.c_str()) == 0)
			{
				::EnumChildWindows(hwnd,EnumAdobeChildProc,lParam);
				if(pAdobeWndInfo->hAdobeView != NULL)
				{
					pAdobeWndInfo->hAdobeFrame = hwnd;
					CELOG_LOG(CELOG_DUMP, L"Local variables are: strFileName=%ls, strWinCaption=%ls, strWinCls=%ls \n", print_long_string(strFileName),print_long_string(strWinCaption),print_long_string(strWinCls) );
					return FALSE;
				}
			}
		}
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: strFileName=%ls, strWinCaption=%ls, strWinCls=%ls \n", print_long_string(strFileName),print_long_string(strWinCaption),print_long_string(strWinCls) );

	}
	return TRUE;
}


HWND COverLay::GetViewHwnd()
{
	return COverLay::m_hViewWnd;
}

void COverLay::SetViewHwnd(const HWND hView)
{
	COverLay::m_hViewWnd = hView;
}

HWND COverLay::GetPdfViewHandle(const wstring & strFilePath)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: strFilePath=%ls \n",strFilePath.c_str() );

	wstring strPath(strFilePath); 
	
	ADOBE_WND_INFO theAdobeInfo;
	theAdobeInfo.hAdobeFrame = NULL;
	theAdobeInfo.hAdobeView = NULL;
	int len = strPath.length() + 1;
	theAdobeInfo.strFilePath = new wchar_t[len]();
	if (theAdobeInfo.strFilePath  != NULL)
	{
		wsprintf(theAdobeInfo.strFilePath,L"%s",strPath.c_str());
		// First ,we check with the name 
		if(theAdobeInfo.hAdobeView == NULL)
		{
			::EnumWindows(EnumAdobeProc,(LPARAM)&theAdobeInfo);
		}
		delete []theAdobeInfo.strFilePath;
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: len=%d \n", len );

	return theAdobeInfo.hAdobeView;
}

void COverLay::DoViewOL(nextlabs::Obligations& obs,const wstring &strFilePath)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: strFilePath=%ls, obs=%p \n",strFilePath.c_str(),&obs );

	HWND hwnd = NULL;
	bool bRightWnd = false;
	
	if (GetViewHwnd() != NULL)
	{
		//because we had tested it. adobe will just do overlay in open event after calling createwindow function.
		//adobe 11 we can't get window text. so we dont cache the path to judge the file.
		hwnd = GetParent(GetParent(GetParent(GetParent(GetParent(GetViewHwnd())))));
		
		if (hwnd != NULL)
		{
			wchar_t strWinTxt[MAX_PATH] = {0};
			GetWindowText(hwnd,strWinTxt,MAX_PATH);	

			if (_wcsicmp(strWinTxt,L"AVSplitterView") == 0)
			{

#if ACRO_SDK_LEVEL==0x000A0000	

				HWND hFrameWnd  = GetParent(GetParent(hwnd));
				if (hFrameWnd != NULL)
				{
					wchar_t strClsName[MAX_PATH] = {0};
					::GetClassName(hFrameWnd,strClsName,MAX_PATH);
					if (_wcsicmp(strClsName,L"AcrobatSDIWindow") == 0)
					{
						COverLayData &Data =  COverLayData::GetInstance();
						Data.AddFrameAndPath(hFrameWnd,strFilePath);
					}
				}
#endif
				bRightWnd  = true;
			}
		}
	}
	
	if (hwnd != NULL&&bRightWnd)
	{
		PA_Mngr::CPAMngr pam(COPYOF_CEM_GetString);
		pam.SetObligations(strFilePath.c_str(),strFilePath.c_str(),obs);


		if (pa_labeling_mod == NULL)
		{
			std::wstring bin_install_path;////
			bin_install_path = GetEnforceBinDir();
		
			wstring strDllPath = bin_install_path + L"\\";
			strDllPath += PA_MODULE_NAME_NLVISUALABELING;
			pa_labeling_mod = LoadLibraryW(strDllPath.c_str());

			
			if( pa_labeling_mod == NULL )
			{
			    CELOG_LOG(CELOG_DUMP, L"Local variables are: hwnd=%p \n", hwnd );
				return ;
			}

		}


		long lRt = pam.DoVisualLabeling(pa_labeling_mod,NULL,PABase::AT_READ,TRUE,_T("OK"),hwnd);    
		if(0 != lRt)
		{
			CELOG_LOG(CELOG_DEBUG, L"DoVisualLabeling fail!\n");
			CELOG_LOG(CELOG_DUMP, L"Local variables are: hwnd=%p \n", hwnd );
			return;
		}
		
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"Get PDF VIEW is NULL\n");
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hwnd=%p \n", hwnd );

}





void COverLay::StartGDIPlus()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}
void COverLay::ShutDownGDIPlus()
{
	if (m_gdiplusToken != NULL)
	{
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		m_gdiplusToken = NULL;
	}

}
FuncPrintVL COverLay::GetPrintVLFunAddr()
{
	
	std::wstring bin_install_path;
	bin_install_path = GetEnforceBinDir();
	wstring strPath = bin_install_path + L"\\";

#ifdef _WIN64
	strPath += L"NLVLViewPrint.dll";
#else
	strPath += L"NLVLViewPrint32.dll";
#endif

	if (m_hPrintVLDLL == NULL)
	{
		m_hPrintVLDLL = LoadLibraryW(strPath.c_str());
	}
	if(m_hPrintVLDLL)
	{
		m_pPrintVLFunc = (FuncPrintVL)GetProcAddress(m_hPrintVLDLL,PRINT_VL_FUNC);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"load overlay dll failed \n");
	}

	return m_pPrintVLFunc;
}

void COverLay::FreePrintVLAddr()
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

void COverLay::GetInfoValue(const wstring &strInfo,const wstring strName,wstring &strValue)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: strInfo=%ls, strName=%ls, strValue=%ls \n",strInfo.c_str(),strName.c_str(),strValue.c_str() );

	wstring strTemp = L"";
	strValue = strTemp;
	size_t nEndPos = 0;
	size_t nStartPos = 0;
	size_t nStrLen = 0;

	if(_wcsicmp(strName.c_str(),L"strTextValue=") == 0)
	{
		//in order to deal with strTextValue contains ";";
		nStartPos = strInfo.find(strName);
		if(nStartPos != wstring::npos)
		{
			strTemp = strInfo.substr(nStartPos);
			nStrLen = wcslen(strName.c_str());
			strValue = strTemp.substr(nStrLen);
			return;
		}
	}
	nStartPos = strInfo.find(strName);
	if(nStartPos != wstring::npos)
	{
		strTemp = strInfo.substr(nStartPos);
		nEndPos = strTemp.find(L";");
		if(nEndPos != wstring::npos)
		{
			nStrLen = wcslen(strName.c_str());
			strValue = strTemp.substr(nStrLen,nEndPos - nStrLen);
		}
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: strTemp=%ls, nEndPos=%u, nStartPos=%u, nStrLen=%u \n", strTemp.c_str(),nEndPos,nStartPos,nStrLen );

}
void COverLay::GetVLInfo(wstring strInfo,bool &bDeny,bool &bIsExistOL,NM_VLObligation::VisualLabelingInfo &VLInfo)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: strInfo=%ls, bDeny=%ls, bIsExistOL=%ls, VLInfo=%p \n",strInfo.c_str(),bDeny?L"TRUE":L"FALSE",bIsExistOL?L"TRUE":L"FALSE",&VLInfo );

	wstring strValue = L"";
	GetInfoValue(strInfo,L"bDeny=",strValue);
	if(_wcsicmp(strValue.c_str(),L"true") == 0)
	{
		bDeny = true;
	}
	else
	{
		bDeny = false;
	}

	GetInfoValue(strInfo,L"bExistOL=",strValue);
	if(_wcsicmp(strValue.c_str(),L"true") == 0)
	{
		bIsExistOL = true;
	}
	else
	{
		bIsExistOL = false;
		return;
	}

	GetInfoValue(strInfo,L"bFontBold=",strValue);
	if(_wcsicmp(strValue.c_str(),L"0") == 0)
	{
		VLInfo.bFontBold = false;
	}
	else
	{
		VLInfo.bFontBold = true;
	}

	GetInfoValue(strInfo,L"dwFontColor=",strValue);
	VLInfo.dwFontColor = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwFontSize1=",strValue);
	VLInfo.dwFontSize1 = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwFontSize2=",strValue);
	VLInfo.dwFontSize2 = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwTransparency=",strValue);
	VLInfo.dwTransparency = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwLeftMargin=",strValue);
	VLInfo.dwLeftMargin = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwTopMargin=",strValue);
	VLInfo.dwTopMargin = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwHorSpace=",strValue);
	VLInfo.dwHorSpace = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"dwVerSpace=",strValue);
	VLInfo.dwVerSpace = _wtoi(strValue.c_str());

	GetInfoValue(strInfo,L"strText=",strValue);
	wcsncpy_s(VLInfo.strText,512,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strClassificationMap=",strValue);
	wcsncpy_s(VLInfo.strClassificationMap,512,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strFontName=",strValue);
	wcsncpy_s(VLInfo.strFontName,64,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strPlacement=",strValue);
	wcsncpy_s(VLInfo.strPlacement,64,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strType=",strValue);
	wcsncpy_s(VLInfo.strType,64,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strConfHFtooer=",strValue);
	wcsncpy_s(VLInfo.strConfHFtooer,64,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strConfWatermark=",strValue);
	wcsncpy_s(VLInfo.strConfWatermark,64,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strPolicyName=",strValue);
	wcsncpy_s(VLInfo.strPolicyName,512,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strFilePath=",strValue);
	wcsncpy_s(VLInfo.strFilePath,2048,strValue.c_str(), _TRUNCATE);
	
	GetInfoValue(strInfo,L"strTextValue=",strValue);
	wcsncpy_s(VLInfo.strTextValue,1024,strValue.c_str(), _TRUNCATE);
	
    CELOG_LOG(CELOG_DUMP, L"Local variables are: strValue=%ls \n", strValue.c_str() );
}




void COverLay::SetOverLayDate(NM_VLObligation::VisualLabelingInfo VLInfo)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: VLInfo=%p \n",&VLInfo);

	m_OverLayInfo.bFontBold = VLInfo.bFontBold;
	m_OverLayInfo.dwFontColor= VLInfo.dwFontColor;
	m_OverLayInfo.dwFontSize1= VLInfo.dwFontSize1;
	m_OverLayInfo.dwFontSize2= VLInfo.dwFontSize2;
	m_OverLayInfo.dwHorSpace= VLInfo.dwHorSpace;
	m_OverLayInfo.dwLeftMargin= VLInfo.dwLeftMargin;
	m_OverLayInfo.dwTopMargin= VLInfo.dwTopMargin;
	m_OverLayInfo.dwTransparency= VLInfo.dwTransparency;
	m_OverLayInfo.dwVerSpace= VLInfo.dwVerSpace;

	wcsncpy_s(m_OverLayInfo.strClassificationMap,512,VLInfo.strClassificationMap, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strConfHFtooer,64,VLInfo.strConfHFtooer, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strConfWatermark,64,VLInfo.strConfWatermark, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strFilePath,2048,VLInfo.strFilePath, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strFontName,64,VLInfo.strFontName, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strLocation,128,VLInfo.strLocation, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strPlacement,64,VLInfo.strPlacement, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strPolicyName,64,VLInfo.strPolicyName, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strText,512,VLInfo.strText, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strType,64,VLInfo.strType, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strWatermarkOrig,64,VLInfo.strWatermarkOrig, _TRUNCATE);
	wcsncpy_s(m_OverLayInfo.strTextValue,1024,VLInfo.strTextValue, _TRUNCATE);

}

void COverLay::DoPrintOverlay()
{

	if(_wcsicmp(m_OverLayInfo.strTextValue,L"")==0)
	{
		return;
	}
	if(m_pPrintVLFunc)
	{
		NM_VLObligation::IVisualLabelingInfo theInfo;
		theInfo.bFontBold = m_OverLayInfo.bFontBold;
		theInfo.dwFontColor = m_OverLayInfo.dwFontColor;
		theInfo.dwFontSize1 = m_OverLayInfo.dwFontSize1;
		theInfo.dwFontSize2 = m_OverLayInfo.dwFontSize2;
		theInfo.dwHorSpace = m_OverLayInfo.dwHorSpace;
		theInfo.dwLeftMargin = m_OverLayInfo.dwLeftMargin;
		theInfo.dwTopMargin = m_OverLayInfo.dwTopMargin;
		theInfo.dwTransparency = m_OverLayInfo.dwTransparency;
		theInfo.dwVerSpace = m_OverLayInfo.dwVerSpace;

		wcsncpy_s(theInfo.strClassificationMap,512,m_OverLayInfo.strClassificationMap, _TRUNCATE);
		wcsncpy_s(theInfo.strConfHFtooer,64,m_OverLayInfo.strConfHFtooer, _TRUNCATE);
		wcsncpy_s(theInfo.strConfWatermark,64,m_OverLayInfo.strConfWatermark, _TRUNCATE);
		wcsncpy_s(theInfo.strFilePath,2048,m_OverLayInfo.strFilePath, _TRUNCATE);
		wcsncpy_s(theInfo.strFontName,64,m_OverLayInfo.strFontName, _TRUNCATE);
		wcsncpy_s(theInfo.strLocation,128,m_OverLayInfo.strLocation, _TRUNCATE);
		wcsncpy_s(theInfo.strPlacement,64,m_OverLayInfo.strPlacement, _TRUNCATE);
		wcsncpy_s(theInfo.strPolicyName,512,m_OverLayInfo.strPolicyName, _TRUNCATE);
		wcsncpy_s(theInfo.strText,512,m_OverLayInfo.strText, _TRUNCATE);
		wcsncpy_s(theInfo.strTextValue,1024,m_OverLayInfo.strTextValue, _TRUNCATE);
		wcsncpy_s(theInfo.strType,64,m_OverLayInfo.strType, _TRUNCATE);
		wcsncpy_s(theInfo.strWatermarkOrig,64,m_OverLayInfo.strWatermarkOrig, _TRUNCATE);
		m_pPrintVLFunc(m_hDC,theInfo);
	}
	return ;
}
void COverLay::SetHDC(const HDC& theHDC)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: theHDC=%p \n",theHDC);

	m_hDC = theHDC;
}
void COverLay::releaseHDC(const HDC& theHDC)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: theHDC=%p \n",theHDC);

	if(m_hDC == theHDC)
		m_hDC = NULL;
}
bool COverLay::IsSameHDC(const HDC& theHDC)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: theHDC=%p \n",theHDC);

	if(m_hDC == theHDC)	return true;
	return false;
}

void COverLay::ReleaseOverlayData()
{
	memset(&m_OverLayInfo,0,sizeof(NM_VLObligation::VisualLabelingInfo));
}

wstring COverLay::SetOLInfo()
{
	wchar_t strInfo[1024] = {0};
	wstring strData = L"";
	wsprintf(strInfo,L"bFontBold=%d;dwFontColor=%d;dwFontSize1=%d;dwFontSize2=%d;dwTransparency=%d;dwLeftMargin=%d;dwTopMargin=%d;dwHorSpace=%d;dwVerSpace=%d;",m_OverLayInfo.bFontBold,m_OverLayInfo.dwFontColor,m_OverLayInfo.dwFontSize1,m_OverLayInfo.dwFontSize2,m_OverLayInfo.dwTransparency,m_OverLayInfo.dwLeftMargin,m_OverLayInfo.dwTopMargin,m_OverLayInfo.dwHorSpace,m_OverLayInfo.dwVerSpace);
	
	strData = strInfo;
	strData = strData + L"strText=" + m_OverLayInfo.strText + L";" \
		L"strClassificationMap=" + m_OverLayInfo.strClassificationMap + L";" \
		L"strFontName=" + m_OverLayInfo.strFontName + L";" \
		L"strPlacement=" + m_OverLayInfo.strPlacement + L";" \
		L"strPolicyName=" + m_OverLayInfo.strPolicyName + L";" \
		L"strFilePath=" + m_OverLayInfo.strFilePath + L";" \
		L"strType=" + m_OverLayInfo.strType + L";" \
		L"strConfHFtooer=" + m_OverLayInfo.strConfHFtooer + L";" \
		L"strConfWatermark=" + m_OverLayInfo.strConfWatermark + L";" \
		L"strTextValue=" + m_OverLayInfo.strTextValue;

	CELOG_LOG(CELOG_DUMP, L"Local variables are: strInfo=%ls, strData=%ls \n", print_long_string(strInfo),strData.c_str() );

	return strData;

}

bool COverLay::ReadKey( const wchar_t* in_key , void* in_value , size_t in_value_size )
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: in_key=%ls, in_value=%p, in_value_size=%u \n",print_long_string(in_key),in_value,in_value_size );

	assert( in_key != NULL );
	assert( in_value != NULL );
	if( in_key == NULL || in_value == NULL )
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	/* Parse out key path and key name */
	std::wstring key_path(in_key);
	std::wstring::size_type i = key_path.find_last_of(L"\\//");
	if( i == std::wstring::npos )
	{
		return false;
	}
	std::wstring key_name(key_path,i+1); // capture key name
	key_path.erase(i);                   // trim tail which includes key name.

	HKEY hKey = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: hKey=%p, rstatus=%ld \n", hKey,rstatus );
		return false;
	}

	DWORD result_size = (DWORD)in_value_size;
	rstatus = RegQueryValueExW(hKey,key_name.c_str(),NULL,NULL,(LPBYTE)in_value,&result_size);
	RegCloseKey(hKey);
    CELOG_LOG(CELOG_DUMP, L"Local variables are: hKey=%p, rstatus=%ld, result_size=%lu \n", hKey,rstatus,result_size );


	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return true;
}/* ReadKey */

wstring COverLay::GetEnforceBinDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer\\InstallDir", szDir, MAX_PATH))
	{
		wcscat_s(szDir, MAX_PATH, L"\\bin");
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: szDir=%ls \n", print_long_string(szDir) );
		return szDir;
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: szDir=%ls \n", print_long_string(szDir) );

	return L"";
}

wstring COverLay::ConstructOLData(bool bDeny,bool bExistOL)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: bDeny=%ls, bExistOL=%ls \n",bDeny?L"TRUE":L"FALSE",bExistOL?L"TRUE":L"FALSE" );

	wstring strBuf = L"";
	if(bDeny)
	{
		strBuf = strBuf + L"bDeny=true;" + L"bExistOL=false;";
	    CELOG_LOG(CELOG_DUMP, L"Local variables are: strBuf=%ls \n", strBuf.c_str() );
		return strBuf;
	}
	else
	{
		strBuf = strBuf + L"bDeny=false;";
	}

	if(bExistOL)
	{
		strBuf += L"bExistOL=true;";
	}
	else
	{
		strBuf += L"bExistOL=false;";
		CELOG_LOG(CELOG_DUMP, L"Local variables are: strBuf=%ls \n", strBuf.c_str() );
		return strBuf;
	}

	strBuf += SetOLInfo();
    CELOG_LOG(CELOG_DUMP, L"Local variables are: strBuf=%ls \n", strBuf.c_str() );
	return strBuf;
}

bool COverLay::IsExistViewOL(_In_ nextlabs::Obligations& obs)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: obs=%p \n",&obs );

	const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();
	std::list<nextlabs::Obligation>::const_iterator it;
	for( it = ob_list.begin() ; it != ob_list.end() ; ++it )
	{
		if( boost::algorithm::iequals(it->name,OBLIGATION_VIEW_OVERLAY) )
		{
			return true;
		}
	}
	return false;
}

bool COverLay::SetOverlayData(_In_ nextlabs::Obligations& obs,_In_ const wstring & strFilePath)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: obs=%p, strFilePath=%ls \n",&obs,strFilePath.c_str() );

	bool bRet = false;
	const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();
	std::list<nextlabs::Obligation>::const_iterator it;
	for( it = ob_list.begin() ; it != ob_list.end() ; ++it )
	{
		if( boost::algorithm::iequals(it->name,OBLIGATION_PRINT_OVERLAY) )
		{
			wcsncpy_s(m_OverLayInfo.strFilePath,2048,strFilePath.c_str(), _TRUNCATE);
			wcsncpy_s(m_OverLayInfo.strPolicyName,512,it->policy.c_str(), _TRUNCATE);
			nextlabs::ObligationOptions::const_iterator options_it;
			for( options_it = it->options.begin() ; options_it != it->options.end() ; ++options_it )
			{
				if(options_it->first == NM_VLObligation::str_text_name)
				{
					wcsncpy_s(m_OverLayInfo.strTextValue,1024,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_classification_map_name)
				{
					wcsncpy_s(m_OverLayInfo.strClassificationMap,512,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_transparency_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"15";
					}
					try
					{
						m_OverLayInfo.dwTransparency = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwTransparency = 35;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_font_name)
				{
					wcsncpy_s(m_OverLayInfo.strFontName,64,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_font_size1_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"36";
					}
					try
					{
						m_OverLayInfo.dwFontSize1 = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwFontSize1 = 36;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_font_size2_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"28";
					}
					try
					{
						m_OverLayInfo.dwFontSize2 = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwTransparency = 28;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_font_bold_name)
				{
					if(_wcsicmp(options_it->second.c_str(),L"false") == 0)		m_OverLayInfo.bFontBold = false;
					else m_OverLayInfo.bFontBold = true;

					continue;
				}
				else if(options_it->first == NM_VLObligation::str_font_color_name)
				{
					wstring strColor = options_it->second;
					if (strColor.empty())
					{
						strColor = L"#888888";
					}
					boost::algorithm::replace_all(strColor,L"#",L"");
					m_OverLayInfo.dwFontColor = COverlayTool::GetFontColor(strColor);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_placement_view_print_name)
				{
					wcsncpy_s(m_OverLayInfo.strPlacement,64,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_left_margins_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"1";
					}
					try
					{
						m_OverLayInfo.dwLeftMargin = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwLeftMargin = 100;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_top_margins_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"2";
					}
					try
					{
						m_OverLayInfo.dwTopMargin = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwTopMargin = 100;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_hor_space_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"2";
					}
					try
					{
						m_OverLayInfo.dwHorSpace = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwHorSpace = 200;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_ver_space_print_name)
				{
					wstring strTemp = options_it->second;
					if (strTemp.empty())
					{
						strTemp = L"3";
					}	
					try
					{
						m_OverLayInfo.dwVerSpace = boost::lexical_cast<DWORD>(strTemp.c_str());
					}
					catch (...)
					{
						m_OverLayInfo.dwVerSpace = 300;
					}
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_placement_view_print_name)
				{
					wcsncpy_s(m_OverLayInfo.strPolicyName,512,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
				else if(options_it->first == NM_VLObligation::str_date_format_name)
				{
					wcsncpy_s(m_OverLayInfo.strDateFormat,128,options_it->second.c_str(), _TRUNCATE);
				}
				else if(options_it->first == NM_VLObligation::str_time_format_name)
				{
					wcsncpy_s(m_OverLayInfo.strTimeFormat,128,options_it->second.c_str(), _TRUNCATE);
					continue;
				}
			}
			if (_wcsicmp(m_OverLayInfo.strTextValue,L"")!=0 && _wcsicmp(m_OverLayInfo.strFilePath,L"")!=0)
			{
				wstring tempString = L"";
				COverlayTool::ConvertTxT(m_OverLayInfo.strText,m_OverLayInfo.strFilePath,m_OverLayInfo.strPolicyName,m_OverLayInfo.strDateFormat,m_OverLayInfo.strTimeFormat,tempString);
				wcsncpy_s(m_OverLayInfo.strTextValue,1024,tempString.c_str(), _TRUNCATE);

			}
			bRet = true;
			break;
		}
	}
	CELOG_LOG(CELOG_DUMP, L"Local variables are: bRet=%ls \n", bRet?L"TRUE":L"FALSE" );

	return bRet;
}



COverLayData::COverLayData(void)
{
	InitializeCriticalSection(&m_ViewSec);
}

COverLayData::~COverLayData(void)
{
	DelAllInfo();
	DeleteCriticalSection(&m_ViewSec);
}


COverLayData & COverLayData::GetInstance()
{
	static COverLayData instance;
	return instance;
}
void COverLayData::AddFrameAndPath(const HWND &hFrame,const wstring &strFilePath)
{
	if (hFrame != NULL)
	{
		EnterCriticalSection(&m_ViewSec);
		m_mapInfor[hFrame] = strFilePath;
		LeaveCriticalSection(&m_ViewSec);
	}
}


wstring COverLayData::GetFilePathFromFrameWnd(const HWND &hFrame)
{
	wstring strTemp = L"";
	EnterCriticalSection(&m_ViewSec);	
	map<HWND,wstring>::iterator itor = m_mapInfor.find(hFrame) ;   
	if (itor != m_mapInfor.end())
	{
		strTemp = itor->second;
	}
	LeaveCriticalSection(&m_ViewSec);
	return strTemp;
}

void COverLayData::DelAllInfo()
{
	EnterCriticalSection(&m_ViewSec);
	if (!m_mapInfor.empty())
	{
		m_mapInfor.clear();
	}
	LeaveCriticalSection(&m_ViewSec);
}
