#include "stdafx.h"
#include "odhd_excel.h"

extern std::wstring g_wstrCantOpenFile;

BOOL	CODHDExcelInspector::IsValidPropertyName(LPCWSTR strPropName)
{
    if(0 == _wcsicmp(strPropName, L"Title"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Subject"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Author"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Keywords"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Comments"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Last author"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Category"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Manager"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Company"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Content type"))
        return TRUE;
    if(0 == _wcsicmp(strPropName, L"Content status"))
        return TRUE;
    //if(0 == _wcsicmp(strPropName, L"Language"))
    //    return TRUE;
    if(0 == _wcsicmp(strPropName, L"Hyperlink base"))
        return TRUE;
    return FALSE;


}


BOOL	CODHDExcelInspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)
{
	int		nStep=MAX_HDRDLG_PROGRESS;

	if(m_pExcelApp == NULL)	return FALSE;
	m_strSrcFileName = strSrcFileName;
	m_strTempFileName = strTempFileName;

	if(m_nVersion == Excel_2003)
	{
		if(!GetWorkBook())
		{
			std::wstring strMsg = L"";
			CODHDUtilities::ReplaceFileName(g_wstrCantOpenFile, strSrcFileName, strMsg);

			MessageBox(m_progDlg->m_hWnd, strMsg.c_str(), L"Hidden Data Removal", MB_OK | MB_ICONWARNING);
			return FALSE;
		}
		bool bTrue = false;
		
		SetProgTitle(ODHDCAT_EXCEL_COMMENTS);
		bTrue = InspectComments();
		MakeStep(1);nStep-=1;

		SetProgTitle(ODHDCAT_EXCEL_PROP);
		bTrue = InspectDocPropertiesPersInfo();
		MakeStep(1);nStep-=1;

		SetProgTitle(ODHDCAT_EXCEL_HEADFOOT);
		bTrue = InspectHeadersFooters();
		MakeStep(1);nStep-=1;

		SetProgTitle(ODHDCAT_EXCEL_ROWCOL);
		bTrue = InspectRowsColumns();
		MakeStep(2);nStep-=2;

		SetProgTitle(ODHDCAT_EXCEL_WORKSHEET);
		bTrue = InspectWorksheets();
		MakeStep(1);nStep-=1;

		SetProgTitle(ODHDCAT_EXCEL_INVISCONTENT);
		bTrue = InspectInvisibleContent();
		MakeStep(1);nStep-=1;

		//SetProgTitle(ODHDCAT_EXCEL_XML);
		//bTrue = InspectCustomXMLData();
		MakeStep(1);nStep-=1;

		SetProgTitle(ODHDCAT_EXCEL_AUTOFILTER);
		bTrue = InspectAutoFilter();
		MakeStep(1);nStep-=1;
		
	}
	else if(m_nVersion == Excel_2007)
	{

	}
	if(nStep > 0)
		MakeStep(nStep);
	CComVariant varResult;
	AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"Close",0);
	SAFE_RELEASE(m_pWorkbook);
	return TRUE;
}
BOOL	CODHDExcelInspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)
{
	bool bTrue = false;
	switch(odhdCat)
	{
	case ODHDCAT_EXCEL_COMMENTS:
		bTrue = InspectComments(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_COMMENTS;
		}
		break;
	case ODHDCAT_EXCEL_HEADFOOT:
		bTrue = InspectHeadersFootersEx(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_HEADFOOT;
		}
		break;
	case ODHDCAT_EXCEL_ROWCOL:
		bTrue = InspectRowsColumns(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_ROWCOL;
		}
		break;
	case ODHDCAT_EXCEL_WORKSHEET:
		bTrue = InspectWorksheets(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_WORKSHEET;
		}
		break;
	case ODHDCAT_EXCEL_INVISCONTENT:
		bTrue = InspectInvisibleContent(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_INVISCONTENT;
		}
		break;
	/*case ODHDCAT_EXCEL_XML:
		bTrue = InspectCustomXMLData(true);
		break;*/
	case ODHDCAT_EXCEL_PROP:
		bTrue = InspectDocPropertiesPersInfo(true);
		if (bTrue && m_pHDRFile != NULL)
		{
			m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_PROP;
		}
		break;
	default:
		return FALSE;
	}
	SafeExcelDoc();
	if(bTrue)
		SetCatStatus(odhdCat,INSPECT_REMOVED);
	else
		SetCatStatus(odhdCat,INSPECT_FAILED);

	SAFE_RELEASE(m_pWorkbook);
	return bTrue?TRUE:FALSE;
}

long	CODHDExcelInspector::GetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent)
{
	return TRUE;
}
long	CODHDExcelInspector::SetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent)
{
	return 0;
}
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& private function&&&&&&&&&&&&&&&&&&&& */
// get Workbook object
bool CODHDExcelInspector::GetWorkBook(void)
{
	if (m_pExcelApp == NULL)
	{
		m_pExcelApp = MSOAppInstance::Instance()->GetExcelAppInstance();
		if (m_pExcelApp == NULL)
		{
			return FALSE;
		}

	}
	if(m_pWorkbook != NULL)	return true;
	CComVariant varWorkBooks;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varWorkBooks,m_pExcelApp,L"Workbooks",0);
	if(FAILED(hr)||NULL==varWorkBooks.pdispVal)	
	{
		m_pExcelApp=NULL;
		return false;
	}

	CComPtr<IDispatch> spDispatch = varWorkBooks.pdispVal;

	CComVariant FileName;
	FileName.vt = VT_BSTR;
	FileName.bstrVal = _bstr_t(m_strTempFileName.c_str());
	CComVariant varZero((int)0);
	CComVariant varTrue((short)TRUE);
	CComVariant varFalse((short)FALSE);
	CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);


	CComVariant varResult;
	_beginthread(CODHDUtilities::ShowPasswordWindows, 0, NULL);
	hr = AutoWrap(DISPATCH_METHOD,&varResult,spDispatch,L"Open",15,
		covOptional,    // CorruptLoad
		covOptional,    // Local
		covOptional,    // AddToMru
		covOptional,    // Converter
		varFalse,    // Notify
		varTrue, //covOptional,    // Editable
		covOptional,    // Delimiter
		covOptional,    // Origin
		varTrue,    // IgnoreReadOnlyRecommended
		covOptional,    // WriteResPassword
		covOptional,    // Password
		covOptional,    // Format
		varFalse,    // ReadOnly
		varZero,    // UpdateLinks
		FileName
		);
	m_pWorkbook = varResult.pdispVal;
	if(FAILED(hr)||m_pWorkbook==NULL)
	{
		m_pExcelApp=NULL;
		return false;
	}


	//VARIANT_BOOL         varVisible   = VARIANT_FALSE;
	CComVariant varVisible(VARIANT_FALSE);
	CComVariant varNumb((int)0);
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pExcelApp,L"Visible",1,varVisible);
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pExcelApp,L"ShowWindowsInTaskbar",1,varVisible);
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"ReadOnly",0);
	if(varResult.vt & VT_BOOL && varResult.boolVal)
	{
		DP((L"Excel file is readonly\n"));		
		m_bReadonly = TRUE;		
	}
	return true;
}

bool CODHDExcelInspector::InspectComments(bool bRemove)
{
	if(!GetWorkBook())	return false;
	
	CComPtr<IDispatch> pSheets=NULL;
	CComVariant varResult;
	bool bRemoveOK=true;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Sheets",0);
	if(FAILED(hr)||NULL==varResult.pdispVal)	return false;
	pSheets = varResult.pdispVal;

	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheets,L"Count",0);
	if(FAILED(hr))	
	{
		return false;
	}

	long lCount = varResult.lVal;
	for(int i=1;i<=lCount;i++)
	{
		CComVariant varIndex(i);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheets,L"Item",1,varIndex);
		if(SUCCEEDED(hr)&&varResult.pdispVal)
		{
			CComPtr<IDispatch> pSheet = varResult.pdispVal;
			CComPtr<IDispatch> pComments = NULL;
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"Comments",0);
			if(SUCCEEDED(hr)&&varResult.pdispVal)
			{
				pComments = varResult.pdispVal;
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pComments,L"Count",0);
				if(SUCCEEDED(hr) && varResult.lVal > 0)
				{
					long lCommantCount = varResult.lVal;
					if(bRemove)
					{			
						for(int j=lCommantCount;j>0;j--)
						{
							VARIANT vtComment;
							VariantInit(&vtComment);
							CComVariant varIndexComment(j);
							hr = AutoWrap(DISPATCH_PROPERTYGET|DISPATCH_METHOD,&vtComment,pComments,L"Item",1,varIndexComment);
							if(SUCCEEDED(hr) && vtComment.pdispVal != NULL )
							{
								CComPtr<IDispatch> pComment = vtComment.pdispVal;
								hr = AutoWrap(DISPATCH_METHOD,&vtComment,pComment,L"Delete",0);
								DP((L"we have got comment!\r\n"));
								if(FAILED(hr))
								{
									bRemoveOK=false;
									break;
								}
							}
						}
					}
					else
					{
						RecordInspectResult(ODHDCAT_EXCEL_COMMENTS,
							ODHDITEM_EXCEL_COMMENTS,
							INSPECT_HAVE,
							gs_ItemTitle_Excel[ODHDITEM_EXCEL_COMMENTS].m_itemTitle,
							gs_ItemTitle_Excel[ODHDITEM_EXCEL_COMMENTS].m_itemFound,
							varResult.lVal);
						break;	
					}
				}
			}
		}
	}
	return bRemoveOK;
}

bool CODHDExcelInspector::InspectDocPropertiesPersInfo(bool bRemove)
{
	// get one
	if(!GetWorkBook())	return false;
	
	CComVariant varResult;
	
	bool bRemoveOk=true;
	// properties
	unsigned long lProCount =0;
	CComVariant varEmpty;
	varEmpty.vt=VT_EMPTY;
#if 1	// get custom Properties
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"CustomDocumentProperties",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		CComPtr<IDispatch> pCustomPros=varResult.pdispVal;
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pCustomPros,L"Count",0);
		if(SUCCEEDED(hr))
		{
			unsigned int nCustom = varResult.lVal;
			lProCount += nCustom;
			if(bRemove)
			{
				wchar_t strInfo[256]={0};
				for(unsigned int k=nCustom;k>=1;k--)
				{
					CComVariant varKIndex(k);
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pCustomPros,L"Item",1,varKIndex);
					if(SUCCEEDED(hr)&&varResult.pdispVal)
					{
						CComPtr<IDispatch> pCustomPro =	varResult.pdispVal;
#ifdef _DEBUG
						// get name
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pCustomPro,L"Name",0);
						if(SUCCEEDED(hr))	
							_snwprintf_s(strInfo,256, _TRUNCATE,L"Name is:%ws.\t Value is:",varResult.bstrVal);

						// get value
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pCustomPro,L"Value",0);
						if(SUCCEEDED(hr))
						{
							varResult.ChangeType(VT_BSTR);
							wcsncat_s(strInfo,256, varResult.bstrVal, _TRUNCATE);
						}
#endif
						varResult.Clear();
						hr = AutoWrap(DISPATCH_METHOD,&varResult,pCustomPro,L"Delete",0);
						if(FAILED(hr))
						{
							bRemoveOk=false;
							break;
						}
					}
				}//for
			}//if(bRemove)
		}
	}
#endif
#if 1	// get document properties
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"BuiltinDocumentProperties",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		// traverse the properties
		CComPtr<IDispatch> pProperties=varResult.pdispVal;
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pProperties,L"Count",0);
		if(SUCCEEDED(hr))
		{
			unsigned long lPropertiesCount = varResult.lVal;
			wchar_t strInfo[256]={'\0'};
			for(unsigned int i=1;i<=lPropertiesCount;i++)
			{
				CComVariant varIndex(i);
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pProperties,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					CComPtr<IDispatch>  spProp = varResult.pdispVal;
#if 1
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, spProp, L"Name", 0);
					if (SUCCEEDED(hr) && varResult.bstrVal)
						_snwprintf_s(strInfo,256, _TRUNCATE,L"the name is:%ws.\tthe value is:",varResult.bstrVal);
#endif
					if(IsValidPropertyName(varResult.bstrVal))
					{
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,spProp,L"Value",0);
						varResult.ChangeType(VT_BSTR);
						if(_wcsicmp(varResult.bstrVal,L"") != 0)
						{
							wcsncat_s(strInfo,256,varResult.bstrVal, _TRUNCATE);
							lProCount++;
							if(bRemove)
							{
								varResult.Clear();
								hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,spProp,L"Value",1,varEmpty);
								if(FAILED(hr))	bRemoveOk=false;
							}
						}
					}

					DP((strInfo));
					DP((L"\r\n"));
				}
			}
		}
	}
#endif
#if 1	// remove personal information
	if(bRemove)
	{
		CComVariant varRemove(VARIANT_TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"RemovePersonalInformation",1,varRemove);
		if(FAILED(hr))
		{
			bRemoveOk=false;
		}
	}

#endif
	// routing slip
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"HasRoutingSlip",0);
	if(SUCCEEDED(hr))
	{
		if(varResult.boolVal == VARIANT_TRUE)
		{
			if(bRemove)
			{
				CComVariant varFalse(VARIANT_FALSE);
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"HasRoutingSlip",1,varFalse);
				if(FAILED(hr))		bRemoveOk=false;
			}
			else
			{
				RecordInspectResult(ODHDCAT_EXCEL_PROP,
					ODHDITEM_EXCEL_DOCPROP,
					INSPECT_HAVE,
					gs_ItemTitle_Excel[ODHDITEM_EXCEL_DOCPROP].m_itemTitle,
					gs_ItemTitle_Excel[ODHDITEM_EXCEL_DOCPROP].m_itemFound);		
			}
		}
	}

#if 0
	// secenario comments
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Sheets",0);
	if(FAILED(hr)||NULL==varResult.pdispVal)	return false;
	IDispatch* pSheets = varResult.pdispVal;

	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheets,L"Count",0);
	if(FAILED(hr))	
	{
		SAFE_RELEASE(pSheets);
		return false;
	}

	long lCount = varResult.lVal;
	for(int i=lCount;i>0;i--)
	{
		CComVariant varIndex(i);
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheets,L"Item",1,varIndex);
		if(SUCCEEDED(hr)&&varResult.pdispVal)
		{
			IDispatch*pSheet = varResult.pdispVal;
			CComVariant varIndex(i);
			hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"Scenarios",2,varIndex,0);

		}
	}
#endif
	if(!bRemove && lProCount>0)
	{
		RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_DOCPROP,INSPECT_HAVE,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_DOCPROP].m_itemTitle,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_DOCPROP].m_itemFound,
			lProCount);
	}
	return bRemoveOk;
}
bool CODHDExcelInspector::InspectCustomXMLData(bool bRemove)
{
	return false;
}
bool CODHDExcelInspector::InspectHeadersFootersEx(bool bRemove)
{
	if(!GetWorkBook())	return false;
	//return true;
	CComVariant varResult;
	bool bRemoveOk=true;
	
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Worksheets",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=lCount;i>0;i--)
			{
				CComVariant varIndex(i);
				DP((L"Sheet Index:%d\n",i));
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					CComPtr<IDispatch> pSheet = varResult.pdispVal;
					bRemoveOk=ExistHeaderFooterInSheet(pSheet,bRemove);
					if(!bRemove&&bRemoveOk)
					{
						DP((L"break from for loop\n"));
						break;
					}
				}
			}//for
		}
	}
	varResult.vt=VT_EMPTY;
	return bRemoveOk;
}
/*bool InspectHeadersFootersEx(bool bRemove)
{
	CComVariant varEmpty(L"");
	CComVariant varResult;
	bool bRemoveOk=true;

	if(!GetWorkBook())	return false;
	
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Worksheets",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		IDispatch* pWorkSheets = varResult.pdispVal;

		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=1;i<=lCount;i++)
			{
				CComVariant varIndex(i);
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					IDispatch* pAllSheet = varResult.pdispVal;
					
					bRemoveOk=ExistHeaderFooterInSheet(pAllSheet,bRemove);
					if(!bRemove&&bRemoveOk)
					{
						SAFE_RELEASE(pAllSheet);
						break;
					}
					SAFE_RELEASE(pAllSheet);
				}
			}
		}
		SAFE_RELEASE(pWorkSheets);
	}

	return bRemoveOk;
}*/
bool CODHDExcelInspector::ExistHeaderFooterInSheet(IDispatch*pSheet,bool bRemove)
{
	HRESULT	hr=S_OK;
	bool bRemoveOk=true;
	CComVariant varResult;
	CComVariant varEmpty(L"");

	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"PageSetup",0);
	if(FAILED(hr)||NULL==varResult.pdispVal)
		return false;

	CComPtr<IDispatch> pPageSetup=varResult.pdispVal;
	BOOL bHeaders = false,bFooters = false;
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		DP((L"Footer:Center:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		DP((L"Header:Center:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterHeader",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		DP((L"Footer:Left:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		DP((L"Header:Left:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftHeader",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		DP((L"Footer:Right:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		DP((L"Header:Right:%s\n",varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightHeader",1,varEmpty);
			if(FAILED(hr))
				bRemoveOk=false;
		}
	}
	if(!bRemove)
	{
		if(bHeaders)
		{
			RecordInspectResult(ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSHEADER,INSPECT_HAVE,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSHEADER].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSHEADER].m_itemFound);
		}
		if(bFooters)
		{
			RecordInspectResult(ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSFOOTER,INSPECT_HAVE,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSFOOTER].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSFOOTER].m_itemFound);
		}
		return (bHeaders||bFooters);
	}
	
	return bRemoveOk;

}
bool CODHDExcelInspector::InspectHeadersFooters(bool bRemove)
{
	if(!GetWorkBook())	return false;

	bool bRemoveOk=true;
	CComPtr<IDispatch> pRange=NULL;
	CComVariant varResult;
	CComVariant varEmpty(L"");
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pExcelApp,L"Cells",0);
	if(FAILED(hr)||NULL== varResult.pdispVal)
		return false;

	pRange = varResult.pdispVal;
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRange,L"Worksheet",0);
	if(FAILED(hr)||NULL==varResult.pdispVal)
	{
		return false;
	}

	CComPtr<IDispatch> pWorksheet=varResult.pdispVal;
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorksheet,L"PageSetup",0);
	if(FAILED(hr)||NULL==varResult.pdispVal)
	{
		return false;
	}

	CComPtr<IDispatch> pPageSetup=varResult.pdispVal;
	BOOL bHeaders = false,bFooters = false;
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"CenterHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"CenterHeader",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"LeftHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"LeftHeader",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightFooter",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bFooters = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightFooter",1,varEmpty);
			if(FAILED(hr))
			{
				bRemoveOk=false;
			}
		}
	}
	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pPageSetup,L"RightHeader",0);
	if(SUCCEEDED(hr) && _wcsicmp(varResult.bstrVal,L"") != 0)
	{
		bHeaders = true;
		DP((varResult.bstrVal));
		if(bRemove)
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pPageSetup,L"RightHeader",1,varEmpty);
			if(FAILED(hr))
				bRemoveOk=false;
		}
	}
	if(!bRemove)
	{
		if(bHeaders)
		{
			RecordInspectResult(ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSHEADER,INSPECT_HAVE,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSHEADER].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSHEADER].m_itemFound);
		}
		if(bFooters)
		{
			RecordInspectResult(ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSFOOTER,INSPECT_HAVE,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSFOOTER].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_WSFOOTER].m_itemFound);
		}
	}
	return bRemoveOk;
}
bool CODHDExcelInspector::InspectRowsColumns(bool bRemove)
{
	if(!GetWorkBook())	return false;
	CComVariant varResult;
	bool bRemoveOk=true;
	
	unsigned long lColumns=0,lRows=0;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Worksheets",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=1;i<=lCount;i++)
			{
				CComVariant varIndex(i);
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					CComPtr<IDispatch> pAllSheet = varResult.pdispVal;
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pAllSheet,L"UsedRange",0);
					if(SUCCEEDED(hr)&&varResult.pdispVal)
					{
						CComPtr<IDispatch> pSheet = varResult.pdispVal;
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"Columns",0);
						if(SUCCEEDED(hr)&&varResult.pdispVal)
						{
							CComPtr<IDispatch> pRange = varResult.pdispVal;
							varResult.Clear();
							hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRange,L"Count",0);
							if(SUCCEEDED(hr))
							{
								long lTempColumns = varResult.lVal;
								for(int j=lTempColumns;j>=1;j--)
								{
									CComVariant varJndex(j);
									varResult.Clear();
									hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRange,L"Item",1,varJndex);
									if(SUCCEEDED(hr)&&varResult.pdispVal)
									{
										CComPtr<IDispatch> pColumn = varResult.pdispVal;
										varResult.Clear();
										hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pColumn,L"Hidden",0);
										if(SUCCEEDED(hr))
										{
											if(varResult.vt == VT_BOOL && varResult.boolVal & VARIANT_TRUE)
											{
												lColumns++;
												DP((L"Here exist hidden data!\r\n"));
												if(bRemove)
												{
													VARIANT varHiddenTrue;	
													varHiddenTrue.vt=VT_BOOL;
													varHiddenTrue.boolVal=0;
													varResult.Clear();
													hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pColumn,L"Hidden",1,varHiddenTrue);
													varResult.Clear();
													hr = AutoWrap(DISPATCH_METHOD,&varResult,pColumn,L"Delete",0);
													if(FAILED(hr))
													{
														varResult.Clear();
														hr=AutoWrap(DISPATCH_PROPERTYGET,&varResult,pColumn,L"PivotField",0);
														CComPtr<IDispatch> spDispatch = varResult.pdispVal;
														if(SUCCEEDED(hr)&&spDispatch)
														{
															varResult.Clear();
															hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pColumn,L"Hidden",1,varHiddenTrue);
															if(FAILED(hr))
																bRemoveOk=false;
															else
																m_bUnhiddenRowCol=true;
														}
														else
														{
															varResult.Clear();
															hr=AutoWrap(DISPATCH_PROPERTYGET,&varResult,pColumn,L"ListHeaderRows",0);
															if(SUCCEEDED(hr))
															{
																varResult.Clear();
																hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pColumn,L"Hidden",1,varHiddenTrue);
																if(FAILED(hr))
																	bRemoveOk=false;
																else
																	m_bUnhiddenRowCol=true;

															}
															else
																bRemoveOk=false;
														}
													}
														
												}
											}
										}
									}
								}//for
							}
						}
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"Rows",0);
						if(SUCCEEDED(hr)&&varResult.pdispVal)
						{
							CComPtr<IDispatch> pRange = varResult.pdispVal;
							varResult.Clear();
							hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRange,L"Count",0);
							if(SUCCEEDED(hr))
							{
								long lTempRows = varResult.lVal;
								for(int j=lTempRows;j>=1;j--)
								{
									CComVariant varJndex(j);
									varResult.Clear();
									hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRange,L"Item",1,varJndex);
									if(SUCCEEDED(hr)&&varResult.pdispVal)
									{
										CComPtr<IDispatch> pRow = varResult.pdispVal;
										varResult.Clear();
										hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRow,L"Hidden",0);
										if(SUCCEEDED(hr))
										{
											if(varResult.vt == VT_BOOL && varResult.boolVal & VARIANT_TRUE)
											{
												DP((L"Here exist hidden data!\r\n"));
												lRows++;
												if(bRemove)
												{
													VARIANT varHiddenTrue;	
													varHiddenTrue.vt=VT_BOOL;
													varHiddenTrue.boolVal=0;
													varResult.Clear();
													hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pRow,L"Hidden",1,varHiddenTrue);
													varResult.Clear();
													hr = AutoWrap(DISPATCH_METHOD,&varResult,pRow,L"Delete",0);
													if(FAILED(hr))
													{
														varResult.Clear();
														hr=AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRow,L"PivotField",0);
														CComPtr<IDispatch> spDispatch = varResult.pdispVal;
														if(SUCCEEDED(hr)&&spDispatch)
														{
															varResult.Clear();
															hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pRow,L"Hidden",1,varHiddenTrue);
															if(FAILED(hr))
																bRemoveOk=false;
															else
																m_bUnhiddenRowCol=true;

														}
														else
														{
															varResult.Clear();
															hr=AutoWrap(DISPATCH_PROPERTYGET,&varResult,pRow,L"ListHeaderRows",0);
															if(SUCCEEDED(hr))
															{
																varResult.Clear();
																hr=AutoWrap(DISPATCH_PROPERTYPUT,&varResult,pRow,L"Hidden",1,varHiddenTrue);
																if(FAILED(hr))
																	bRemoveOk=false;
																else 
																	m_bUnhiddenRowCol=true;

															}
															else
																bRemoveOk=false;
														}
													}
												}//bRemove
											}
										}
									}
								}//for
							}
						}
					}
				}
			}//for
		}
	}
	if(!bRemove)
	{
		if(lColumns>0)
		{
			RecordInspectResult(ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENCOLS,
				INSPECT_HAVE,gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENCOLS].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENCOLS].m_itemFound,lColumns,1);
		}
		if(lRows>0)
		{
			RecordInspectResult(ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENROWS,
				INSPECT_HAVE,gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENROWS].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENROWS].m_itemFound,lRows,1);
		}
	}
	return bRemoveOk;
}

bool CODHDExcelInspector::InspectWorksheets(bool bRemove)
{
	if(!GetWorkBook())	return false;
	CComVariant varResult;
	bool bRemoveOk=true;
	unsigned int nCount = 0;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Worksheets",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=lCount;i>0;i--)
			{
				CComVariant varIndex(i);
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					CComPtr<IDispatch> pSheet = varResult.pdispVal;
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"Visible",0);
					if(SUCCEEDED(hr) && varResult.lVal == VARIANT_FALSE )
					{
						if(bRemove)
						{
							varResult.Clear();
							hr = AutoWrap(DISPATCH_METHOD,&varResult,pSheet,L"Delete",0);
							if(FAILED(hr) || varResult.boolVal == VARIANT_FALSE)	bRemoveOk=false;
						}
						else
							nCount++;
					}
				}
			}
		}
	}
	if(!bRemove && nCount>0)
	{
		RecordInspectResult(ODHDCAT_EXCEL_WORKSHEET,
			ODHDITEM_EXCEL_HIDDENWS,
			INSPECT_HAVE,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENWS].m_itemTitle,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_HIDDENWS].m_itemFound,
			nCount,1);
	}
	return bRemoveOk;
}
bool CODHDExcelInspector::InspectInvisibleContent(bool bRemove)
{
	bool bHave=false,bRemoveFailed=false,bHide=false;
	if(!GetWorkBook())	return false;
	CComVariant varResult;
	HRESULT hr=S_OK;
	if(bRemove==false)
	{
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"DisplayDrawingObjects",0);
		if(varResult.dblVal==3)
			bHide=true;
	}
	else
	{
		CComVariant varDisplayObject=-4104;
		hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"DisplayDrawingObjects",1,varDisplayObject);
		if(FAILED(hr))
			return false;
	}
	
	{
		CComVariant varWorksheets;
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varWorksheets,m_pWorkbook,L"Worksheets",0);
		if(SUCCEEDED(hr)&&varWorksheets.pdispVal)
		{
			CComVariant varTrue(TRUE);
			varResult.Clear();
			hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,varWorksheets.pdispVal,L"Count",0);
			if(SUCCEEDED(hr))
			{
				long lCount = varResult.lVal;
				for(int i=1;i<=lCount;i++)
				{
					CComVariant varIndex(i);
					CComVariant varWorkSheet;
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varWorkSheet,varWorksheets.pdispVal,L"Item",1,varIndex);
					if(SUCCEEDED(hr)&&varWorkSheet.pdispVal)
					{
						CComVariant varShapes;
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varShapes,varWorkSheet.pdispVal,L"Shapes",0);
						if(SUCCEEDED(hr)&&varShapes.pdispVal)
						{
							CComVariant varShapesCount=0;
							hr=AutoWrap(DISPATCH_PROPERTYGET,&varShapesCount,varShapes.pdispVal,L"Count",0);
							if(SUCCEEDED(hr))
							{
								if(bRemove)
								{
									long lShapesCount=varShapesCount.lVal;
									for(int index=lShapesCount;index>0;index--)
									{
										CComVariant varShapeIndex(index);
										CComVariant varShape;
										hr=AutoWrap(DISPATCH_METHOD,&varShape,varShapes.pdispVal,L"Item",1,varShapeIndex);
										if(SUCCEEDED(hr)&&varShape.pdispVal)
										{
											CComVariant varDelete;
											hr=AutoWrap(DISPATCH_METHOD,&varDelete,varShape.pdispVal,L"Delete",0);
											if(FAILED(hr))
												bRemoveFailed=true;
										}
									}
								}
								else
								{
									if(varShapesCount.lVal&&bHide==true)
									{
										bHave=true;
										break;
									}
								}
								
							}

						}
					}
				}
			}
		}
	}
	
	if(bRemove==false)
	{
		if(bHave==true)
		{
			RecordInspectResult(ODHDCAT_EXCEL_INVISCONTENT,
				ODHDITEM_EXCEL_INVIOBJECT,
				INSPECT_HAVE,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_INVIOBJECT].m_itemTitle,
				gs_ItemTitle_Excel[ODHDITEM_EXCEL_INVIOBJECT].m_itemFound);
			return true;
		}
		else
			return false;
		
	}
	else
	{
		if(bRemoveFailed==true)
			return false;
		else
			return true;

	}
	return false;
}

bool CODHDExcelInspector::InspectAutoFilter(bool bRemove)
{
	if(!GetWorkBook())	return false;
	CComVariant varResult;
	bool bRemoveOk=true;
	unsigned int nCount = 0;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"Worksheets",0);
	if(SUCCEEDED(hr)&&varResult.pdispVal)
	{
		CComPtr<IDispatch> pWorkSheets = varResult.pdispVal;
		CComVariant varTrue(TRUE);
		varResult.Clear();
		hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Count",0);
		if(SUCCEEDED(hr))
		{
			long lCount = varResult.lVal;
			for(int i=lCount;i>0;i--)
			{
				CComVariant varIndex(i);
				varResult.Clear();
				hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pWorkSheets,L"Item",1,varIndex);
				if(SUCCEEDED(hr)&&varResult.pdispVal)
				{
					CComPtr<IDispatch> pSheet = varResult.pdispVal;
					varResult.Clear();
					hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pSheet,L"AutoFilter",0);
					if(SUCCEEDED(hr) && varResult.pdispVal)
					{
						CComPtr<IDispatch> pAutoFilter = varResult.pdispVal;
						varResult.Clear();
						hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pAutoFilter,L"Filters",0);
						if(SUCCEEDED(hr) && varResult.pdispVal)
						{
							CComPtr<IDispatch> pFilters = varResult.pdispVal;
							varResult.Clear();
							hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,pFilters,L"Count",0);
							if (SUCCEEDED(hr) && varResult.lVal != 0)
								nCount++;
						}
					}
				}
			}
		}
	}
	if(!bRemove && nCount>0)
	{
		RecordInspectResult(ODHDCAT_EXCEL_AUTOFILTER,
			ODHDITEM_EXCEL_AUTOFILTER,
			INSPECT_HAVE,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_AUTOFILTER].m_itemTitle,
			gs_ItemTitle_Excel[ODHDITEM_EXCEL_AUTOFILTER].m_itemFound,
			nCount,1);
	}
	return bRemoveOk;
}

void CODHDExcelInspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)
{
	CODHDCategory *pCat=NULL;
	
	ODHDSTATUS	odhdStatus=INSPECT_HAVE;
	assert(m_pCatManager);
#pragma warning(push)
#pragma warning(disable: 6011)
	pCat=m_pCatManager->GetHDCategory(odhdCat);
#pragma warning(pop)	
	switch(odhdCat)
	{
	case ODHDCAT_EXCEL_ROWCOL:
		odhdStatus=pCat->GetCatStatus();
		pCat->GetItemsResult(strResult,0);
		if(m_bUnhiddenRowCol==true)
		{
			switch(odhdStatus)
			{
			case INSPECT_HAVE:
				strResult+=L"\nIf subsets of PivotTables or list headers have been hidden, then these subset rows or columns cannot be removed. These rows or columns are unhidden when you click Remove All.";
				break;
			case INSPECT_REMOVED:
				strResult=L"Some hidden rows and columns cannot be removed because they are part of a PivotTable or a list header. These rows and columns have been unhidden.";
				break;
			}
		
		}
		else
		{
			if(odhdStatus==INSPECT_HAVE)
				strResult+=L"\nIf subsets of PivotTables or list headers have been hidden, then these subset rows or columns cannot be removed. These rows or columns are unhidden when you click Remove All.";
		}
		break;
	case ODHDCAT_EXCEL_WORKSHEET:
		pCat->GetItemsResult(strResult,0);
		break;
	default:
		CODHDInspector::GetResult(odhdCat,strResult);
		break;
	}
}

BOOL CODHDExcelInspector::GetNote(std::wstring &strNote)
{
	if(m_bReadonly&&m_bSaveAsed)
	{
		//strNote.assign(L"The file was opened in read-only mode. If the file has password protection on modify, Remove All will also remove password protection from the attachment.");
		strNote.assign(L"Password protection on modify is removed \nfor read-only files.");
		return TRUE;
	}
	return FALSE;
}

BOOL CODHDExcelInspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)
{
	BOOL bFiltered = FALSE;

	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_EXCEL_COMMENTS && !(dwNeedCat & ExcelHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_EXCEL_PROP && !(dwNeedCat & ExcelHDCat_PROP))
			|| (odhdCat == ODHDCAT_EXCEL_HEADFOOT && !(dwNeedCat & ExcelHDCat_HEADFOOT))
			|| (odhdCat == ODHDCAT_EXCEL_ROWCOL && !(dwNeedCat & ExcelHDCat_ROWCOL))
			|| (odhdCat == ODHDCAT_EXCEL_WORKSHEET && !(dwNeedCat & ExcelHDCat_WORKSHEET))
			|| (odhdCat == ODHDCAT_EXCEL_INVISCONTENT && !(dwNeedCat & ExcelHDCat_INVISCONTENT)))
		{
			bFiltered = TRUE;
		}

		if (!bFiltered && odhdStatus == INSPECT_HAVE)
		{
			switch (odhdCat)
			{
			case ODHDCAT_EXCEL_COMMENTS:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_COMMENTS;
				break;
			case ODHDCAT_EXCEL_PROP:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_PROP;
				break;
			case ODHDCAT_EXCEL_HEADFOOT:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_HEADFOOT;
				break;
			case ODHDCAT_EXCEL_ROWCOL:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_ROWCOL;
				break;
			case ODHDCAT_EXCEL_WORKSHEET:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_WORKSHEET;
				break;
			case ODHDCAT_EXCEL_INVISCONTENT:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_INVISCONTENT;
				break;
			case ODHDCAT_EXCEL_AUTOFILTER:
				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_FILTER;
				break;
			default:
				break;
			}
		}
	}

	return bFiltered;
}

BOOL CODHDExcelInspector::GetFilter(ODHDCATEGORY odhdCat)
{
	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_EXCEL_COMMENTS && !(dwNeedCat & ExcelHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_EXCEL_PROP && !(dwNeedCat & ExcelHDCat_PROP))
			|| (odhdCat == ODHDCAT_EXCEL_HEADFOOT && !(dwNeedCat & ExcelHDCat_HEADFOOT))
			|| (odhdCat == ODHDCAT_EXCEL_ROWCOL && !(dwNeedCat & ExcelHDCat_ROWCOL))
			|| (odhdCat == ODHDCAT_EXCEL_WORKSHEET && !(dwNeedCat & ExcelHDCat_WORKSHEET))
			|| (odhdCat == ODHDCAT_EXCEL_INVISCONTENT && !(dwNeedCat & ExcelHDCat_INVISCONTENT))
			|| (odhdCat == ODHDCAT_EXCEL_AUTOFILTER && !(dwNeedCat & ExcelHDCat_FILTER)))
		{
			return TRUE;
		}
	}

	return FALSE;
}
