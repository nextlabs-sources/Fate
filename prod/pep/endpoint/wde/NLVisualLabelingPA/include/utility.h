#pragma  once
#include <boost/algorithm/string.hpp>
#include <Sddl.h>

using std::pair;
#define CELOG_MAX_MESSAGE_SIZE_CHARS 1024
static bool LogOut(bool bError, _In_ const wchar_t* fmt , ... )
{
	bool bRet = true;
	va_list args;
	va_start(args,fmt);
	wchar_t sline[CELOG_MAX_MESSAGE_SIZE_CHARS]={0};
	vswprintf(sline,_countof(sline),fmt,args);
	va_end(args);
	wstring strlog = sline;
	if(bError)
	{
		DWORD dw = GetLastError();
		LPWSTR lpMsgBuf=NULL;
		FormatMessageW( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			0, // Default language
			(LPWSTR) &lpMsgBuf,
			0,
			NULL 
			);
		strlog += L"\tErrorMsg is:";
		strlog += lpMsgBuf;
		strlog += L"\tFile is:";
		strlog += __FILEW__;
		strlog += L"\tLine is:";
		wchar_t szLine[32]={0};
		wsprintfW(szLine,L"%d",__LINE__);
		strlog += szLine;
		strlog +=L"\n";
		// Free the buffer.
		LocalFree( lpMsgBuf );
	}
	//OutputDebugStringW(strlog.c_str());
	return bRet;
}

static HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, 
				 LPOLESTR ptName, int cArgs...) 
{
	if(!pvResult || !pDisp) 
		return HRESULT_FROM_WIN32((unsigned long)E_POINTER);

	// Variables used...
	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;

	// Get DISPID for name passed...
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if(FAILED(hr)) {
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
	if(autoType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call!
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, 
		&dp, pvResult, NULL, NULL);

	delete [] pArgs;

	return hr;
}

static bool IsWordType(const wstring szFilePath)
{
	if(boost::algorithm::iends_with(szFilePath,L".docx") ||
		boost::algorithm::iends_with(szFilePath,L".docm") ||
		boost::algorithm::iends_with(szFilePath,L".doc") ||
		boost::algorithm::iends_with(szFilePath,L".dotx") ||
		boost::algorithm::iends_with(szFilePath,L".dotm") ||
		boost::algorithm::iends_with(szFilePath,L".dot") ||
		boost::algorithm::iends_with(szFilePath,L".rtf")
		)
	{
		return true;
	}
	return false;
}

static bool IsExcelType(const wstring szFilePath)
{
	if(boost::algorithm::iends_with(szFilePath,L".xlsx") ||
		boost::algorithm::iends_with(szFilePath,L".xlsm") ||
		boost::algorithm::iends_with(szFilePath,L".xlsb") ||
		boost::algorithm::iends_with(szFilePath,L".xls") ||
		boost::algorithm::iends_with(szFilePath,L".xltx") ||
		boost::algorithm::iends_with(szFilePath,L".xltm") ||
		boost::algorithm::iends_with(szFilePath,L".xlt")||
		boost::algorithm::iends_with(szFilePath,L".xls"))
	{
		return true;
	}
	return false;
}

static bool IsPPTType(const wstring szFilePath)
{
	if(boost::algorithm::iends_with(szFilePath,L".pptx") ||
		boost::algorithm::iends_with(szFilePath,L".pptm") ||
		boost::algorithm::iends_with(szFilePath,L".ppt") ||
		boost::algorithm::iends_with(szFilePath,L".potx") ||
		boost::algorithm::iends_with(szFilePath,L".potm") ||
		boost::algorithm::iends_with(szFilePath,L".pot") ||
		boost::algorithm::iends_with(szFilePath,L".ppsx")||
		boost::algorithm::iends_with(szFilePath,L".ppsm") ||
		boost::algorithm::iends_with(szFilePath,L".pps")||
		boost::algorithm::iends_with(szFilePath,L".ppam")||
		boost::algorithm::iends_with(szFilePath,L".ppa"))
	{
		return true;
	}
	return false;
}

/*!
* \brief: Try to get current office document object
*/
static IDispatch* GetOfficeDocObject(const wchar_t* szFilePath)
{
	CoInitialize(NULL);
	if(IsWordType(szFilePath))
	{
		CLSID wdClsid;
		HRESULT hr = CLSIDFromProgID(L"Word.Application",&wdClsid);
		if(FAILED(hr))	return NULL;
		IUnknown* pUnk=NULL;
		hr = GetActiveObject(wdClsid,NULL,&pUnk);
		if(FAILED(hr))	return NULL;
		IDispatch* pDisp=NULL;
		hr = pUnk->QueryInterface(IID_IDispatch,(void**)&pDisp);
		CComQIPtr<Word::_wordApplication> pWordApp(pDisp);
		Word::DocumentsPtr pWordDocs=NULL;
		pWordApp->get_Documents(&pWordDocs);
		long Count;
		pWordDocs->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			Word::_DocumentPtr pWordDoc;
			CComVariant theIndex(i+1);
			pWordDocs->Item(&theIndex, &pWordDoc);
			BSTR FullName;
			pWordDoc->get_FullName(&FullName);
			if (_wcsicmp(FullName,szFilePath) == 0)
			{
				return pWordDoc;
			}
		}
	}
	else if(IsExcelType(szFilePath))
	{
		CLSID wdClsid;
		HRESULT hr = CLSIDFromProgID(L"Excel.Application",&wdClsid);
		if(FAILED(hr))	return NULL;
		IUnknown* pUnk=NULL;
		hr = GetActiveObject(wdClsid,NULL,&pUnk);
		if(FAILED(hr))	return NULL;
		IDispatch* pDisp=NULL;
		hr = pUnk->QueryInterface(IID_IDispatch,(void**)&pDisp);
		CComQIPtr<Excel::_excelApplication> pExcelApp(pDisp);
		Excel::WorkbooksPtr pExcelWbooks=NULL;
		pExcelApp->get_Workbooks(&pExcelWbooks);
		long Count;
		pExcelWbooks->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			Excel::_WorkbookPtr pExcelWbook;
			pExcelWbooks->get_Item(_variant_t(i+1), &pExcelWbook);
			BSTR FullName;
			pExcelWbook->get_FullName(1,&FullName);
			if (_wcsicmp(FullName,szFilePath) == 0)
			{
				return pExcelWbook;
			}
		}
	}
	else if(IsPPTType(szFilePath))
	{
		CLSID wdClsid;
		HRESULT hr = CLSIDFromProgID(L"PPT.Application",&wdClsid);
		if(FAILED(hr))	return NULL;
		IUnknown* pUnk=NULL;
		hr = GetActiveObject(wdClsid,NULL,&pUnk);
		if(FAILED(hr))	return NULL;
		IDispatch* pDisp=NULL;
		hr = pUnk->QueryInterface(IID_IDispatch,(void**)&pDisp);
		CComQIPtr<PPT::_pptApplication> pPPTApp(pDisp);
		PPT::PresentationsPtr pPPTPresents=NULL;
		pPPTApp->get_Presentations(&pPPTPresents);
		long Count;
		pPPTPresents->get_Count(&Count);
		for (int i=0; i<Count; i++)
		{
			PPT::_PresentationPtr pPPTPresent;
			CComVariant theIndex(i+1);
			pPPTPresents->Item(theIndex, &pPPTPresent);
			BSTR FullName;
			pPPTPresent->get_FullName(&FullName);
			if (_wcsicmp(FullName,szFilePath) == 0)
			{
				return pPPTPresent;
			}
		}
	}

	CoUninitialize();
	return NULL;
}


static CComPtr<IDispatch> GetCustomProperties(CComPtr<IDispatch> pCurDoc)
{
	if(pCurDoc == NULL)	return NULL;

	CComVariant var2;
	HRESULT hr  = AutoWrap(DISPATCH_PROPERTYGET,&var2,pCurDoc,L"CustomDocumentProperties",0);
	if(SUCCEEDED(hr) && var2.pdispVal != NULL)
	{
		CComPtr<IDispatch> pCusPors = var2.pdispVal;
		return pCusPors;
	}
	return NULL;
}

static bool GetCustomPropertyBaseOnName(IDispatch* pCusProperties,const std::wstring& strName,
										std::wstring& strValue)
{
	CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCusProperties,L"Count",0);
	if(SUCCEEDED(hr) && theResult.lVal > 0)
	{
		long lCount = theResult.lVal;
		for(long i=0;i<lCount; i++)
		{
			CComVariant varIndex(i+1);
			theResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCusProperties,L"Item", 1, varIndex);
			if(SUCCEEDED(hr) && theResult.pdispVal)
			{
				CComPtr<IDispatch> pCustomProperty = theResult.pdispVal;
				theResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Name",0);
				if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
				{
					if(_wcsicmp(theResult.bstrVal,strName.c_str()) == 0)
					{
						SysFreeString(theResult.bstrVal);
						theResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Value",0);
						if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
						{
							strValue = theResult.bstrVal;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

static bool SetCustomProperties(IDispatch* pCustomProperties,
								const std::wstring& strName,const std::wstring& strValue)
{
	// if we can find the tag name ,compare the value's level and replace it if it's priority is lower than new values
	// if we can't find the tag name ,just add the tag pair.
	CComVariant pa1(strName.c_str());
	CComVariant pa2(VARIANT_FALSE);
	CComVariant pa3(4);
	CComVariant pa4(strValue.c_str());
	CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperties,L"Count",0);
	if(SUCCEEDED(hr) && theResult.lVal > 0)
	{
		long lCount = theResult.lVal;
		for(long i=0;i<lCount; i++)
		{
			CComVariant varIndex(i+1);
			theResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties,L"Item", 1, varIndex);
			if(SUCCEEDED(hr) && theResult.pdispVal)
			{
				CComPtr<IDispatch> pCustomProperty = theResult.pdispVal;
				theResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Name",0);
				if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
				{
					if(_wcsicmp(theResult.bstrVal,strName.c_str()) == 0)
					{
						SysFreeString(theResult.bstrVal);
						theResult.Clear();
						hr = AutoWrap(DISPATCH_METHOD,NULL,pCustomProperty,L"Delete",0);
						//hr = AutoWrap(DISPATCH_PROPERTYPUT,&theResult,pCustomProperty,L"Value",1,strValue.c_str());
						if(SUCCEEDED(hr) )
						{
							hr = AutoWrap(DISPATCH_METHOD, NULL, pCustomProperties, L"Add", 4, pa4,pa3, pa2, pa1);
							if(SUCCEEDED(hr))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}  	

	hr = AutoWrap(DISPATCH_METHOD, NULL, pCustomProperties, L"Add", 4, pa4,pa3, pa2, pa1);
	if(SUCCEEDED(hr))	return true;

	return false;
}


static bool RemoveCustomProperty(IDispatch* pCustomProperties,
								 const std::wstring& strName)
{
	bool bRet = false;
	CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperties,L"Count",0);
	if(SUCCEEDED(hr) && theResult.lVal > 0)
	{
		long lCount = theResult.lVal;
		for(long i=0;i<lCount; i++)
		{
			CComVariant varIndex(i+1);
			theResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCustomProperties,L"Item", 1, varIndex);
			if(SUCCEEDED(hr) && theResult.pdispVal)
			{
				CComPtr<IDispatch> pCustomProperty = theResult.pdispVal;
				theResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Name",0);
				if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
				{
					if(_wcsicmp(theResult.bstrVal,strName.c_str()) == 0)
					{
						SysFreeString(theResult.bstrVal);
						theResult.Clear();
						hr = AutoWrap(DISPATCH_METHOD,NULL,pCustomProperty,L"Delete",0);
						if(SUCCEEDED(hr))	bRet = true;
					}
				}
			}
		}
	}  	

	return bRet;
}

static vector<pair<wstring,wstring>> GetOfficeTag(const wchar_t* szFilePath)
{
	vector<pair<wstring,wstring>> tagPair;
	CComPtr<IDispatch> pDoc = GetOfficeDocObject(szFilePath);
	if(pDoc == NULL)	return tagPair;
	CComPtr<IDispatch> pCusPro = GetCustomProperties(pDoc);
	CComVariant theResult;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCusPro,L"Count",0);
	wstring strname,strvalue;
	if(SUCCEEDED(hr) && theResult.lVal > 0)
	{
		long lCount = theResult.lVal;
		for(long i=0;i<lCount; i++)
		{
			CComVariant varIndex(i+1);
			theResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET, &theResult, pCusPro,L"Item", 1, varIndex);
			if(SUCCEEDED(hr) && theResult.pdispVal)
			{
				CComPtr<IDispatch> pCustomProperty = theResult.pdispVal;
				theResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Name",0);
				if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
				{
					strname = theResult.bstrVal;
					SysFreeString(theResult.bstrVal);
					theResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&theResult,pCustomProperty,L"Value",0);
					if(SUCCEEDED(hr) && theResult.bstrVal != NULL)
					{
						strvalue = theResult.bstrVal;
					}
					tagPair.push_back(pair<wstring,wstring>(strname,strvalue));
				}
			}
		}
	}
	return tagPair;
}


