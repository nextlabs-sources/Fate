#ifndef __H_ODHD_EXCEL__
#define __H_ODHD_EXCEL__
#include "odhd.h"

#define Excel_2003	2003
#define Excel_2007	2007

#define SAFE_RELEASE(x)	{  x=NULL;	}

//following constant should be arranged according the order of ODHDCATEGORY definition
static StaticCatTitle gs_CatTitle_Excel[]={
	{
		L"Comments and Annotations",
		L"The following items were found:",
		L"No items were found.",
		L"Comments and Annotations were successfully removed.",
		L"Failed to remove comments or annotations.",
		L"nothing",
		1
	},
	{
		L"Document Properties and Personal Information",
		L"The following document information was found.",
		L"No document properties or personal information was found.",
		L"Document properties and personal information were successfully removed.",
		L"Failed to remove document properties and personal information.",
		L"nothing",
		1
	},
	{
		L"Headers and Footers",
		L"The following items were found:",
		L"No headers, footers were found.",
		L"Headers,footers were removed from the document.",
		L"Failed to remove headers and footers.",
		L"nothing",
		1
	},
	{
		L"Hidden Rows and Columns",
		L"The following items were found:",
		L"No hidden rows and columns were found.",
		L"All hidden rows and columns containing data were removed.",
		L"Failed to remove hidden rows and columns.",
		L"nothing",
		1
	},
	/*{
		L"Custom XML Data",
		L"Custom XML data was found.",
		L"No custom XML data was found.",
		L"Custom XML data was successfully removed.",
		L"Failed to remove custom XML data.",
		L"nothing",
		0
	},*/
	{
		L"Hidden Worksheets",
		L"Hidden Worksheets was found.",
		L"No Hidden Worksheets was found.",
		L"Hidden Worksheets was successfully removed.",
		L"Failed to remove Hidden Worksheets.",
		L"nothing",
		1
	},
	{
		L"Invisible Content",
		L"Invisible objects wre found.",
		L"No Invisible objects was found.",
		L"Invisible objects were successfully removed.",
		L"Failed to remove invisible objects.",
		L"nothing",
		0
	},
	{
		L"AutoFilter data",
		L"AutoFilter data was found.",
		L"No AutoFilter data was found.",
		L"AutoFilter data was successfully removed.",
		L"Failed to remove AutoFilter data.",
		L"nothing",
		1
	}
};

static StaticItemResult gs_ItemTitle_Excel[] = 
{
	{L"Comments",L"Comments.",L"No comment.",L"Can't remove comments(s).",0},
	{L"Ink annotations ",L"Ink annotations.",L"No ink annotations found.",L"Can't remove ink annotations.",0},
	{L"Document properties",L"Document properties.",L"No document properties found.",L"Can't remove document properties.",0},
	{L"E-mail headers",L"E-mail headers.",L"No E-mail headers found.",L"Can't remove E-mail headers.",0},
	{L"Routing slips",L"Routing slips.",L"No routing slips found.",L"Can't remove routing slips.",0},
	{L"Send-for-review information",L"Send-for-review information.",L"No send-for-review information found.",L"Can't remove send-for-review information.",0},
	{L"Document server properties",L"Document server properties.",L"No document server properties found.",L"Can't remove document server properties.",0},
	{L"Document Management Policy information",L"Document management policy information.",L"No document management policy information found.",L"Can't remove document management policy information.",0},
	{L"Content type information",L"Content type information.",L"No content type information found.",L"Can't remove content type information.",0},
	{L"User name",L"User name.",L"No user name found.",L"Can't remove user name.",0},
	{L"Printer path information ",L"Printer path information.",L"No printer path information found.",L"Can't remove printer path information.",0},
	{L"Scenario comments ",L"Scenario comments.",L"No scenario comments found.",L"Can't remove scenario comments.",0},
	{L"File path for publishing Web pages ",L"File path for publishing web pages.",L"No file path for publishing web pages.",L"Can't remove file path for publishing web pages.",0},
	{L"Comments for defined names and table names ",L"Comments for defined names and table names.",L"No comments for defined names and table names found.",L"Can't remove comments for defined names and table names.",0},
	{L"Inactive external data connections",L"Inactive external data connections.",L"No inactive external data connections found.",L"Can't remove inactive external data connections.",0},
	{L"Information in worksheet headers",L"Headers.",L"No information in worksheet headers.",L"Can't remove information in worksheet headers.",0},
	{L"Information in worksheet footers",L"Footers.",L"No information in worksheet footers.",L"Can't remove information in worksheet footers.",0},
	{L"Hidden rows",L"Number of hidden rows found",L"No hidden rows found.",L"Can't remove hidden rows.",1},
	{L"Hidden columns ",L"Number of hidden columns found",L"No hidden columns found.",L"Can't remove hidden columns",1},
	{L"Hidden worksheets",L"Number of hidden worksheets found",L"No hidden worksheets found.",L"Cant' remove hidden worksheets.",1},
	{L"Custom XML data ",L"Custom XML data.",L"No custom XML data found.",L"Can't remove custom XML data.",0},
	{L"Objects that are not visible because they are formatted as invisible",L"Invisible objects.",L"No invisible object found.",L"Can't remove invisible objects.",0},
	{L"AutoFilter data",L"",L"No AutoFilter data found.",L"Can't remove AutoFilter data.",0}
};


class CODHDExcelInspector:public CODHDInspector
{
public:
	CODHDExcelInspector():CODHDInspector((long)ODHDCAT_EXCEL_MAX,gs_CatTitle_Excel)
		  ,m_pExcelApp(NULL)
		  ,m_pWorkbook(NULL)
		  ,m_bReadonly(FALSE)
		  ,m_bSaveAsed(FALSE)
		  ,m_bUnhiddenRowCol(false)
	  {
		  HRESULT hr = S_OK;
		  Excel::_excelApplication* pExcelApp = MSOAppInstance::Instance()->GetExcelAppInstance();
		  if(pExcelApp != NULL)
		  {
			  hr = pExcelApp->QueryInterface(IID_IDispatch,(void**)&m_pExcelApp);
			  if(FAILED(hr))	return ;
			  /*
			  get version
			  */
			  CComVariant varResult;
			  hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pExcelApp,L"Version",0);
			  if(SUCCEEDED(hr))
			  {
				  if(_wcsicmp(varResult.bstrVal,L"11.0")==0)
					  m_nVersion = Excel_2003;
				  else if(_wcsicmp(varResult.bstrVal,L"12.0")==0)
					  m_nVersion = Excel_2007;
			  }
			  // remove all kinds of alert
			  CComVariant varDisable(VARIANT_FALSE);
			  varResult.Clear();
			  hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pExcelApp,L"DisplayAlerts",1,varDisable);
		  }
	  };
	virtual ~CODHDExcelInspector()
	{
		//SAFE_RELEASE(m_pWorkbook);
		CComVariant varResult;
		if(m_pExcelApp != NULL)
			AutoWrap(DISPATCH_METHOD,&varResult,m_pExcelApp,L"Quit",0);		
	};
public:
	virtual	BOOL			Inspect(std::wstring strSrcFileName,std::wstring strTempFileName);
	virtual BOOL			Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem=ODHDITEM_DEFAULT);
	virtual long			GetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);
	virtual long			SetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent);
	virtual BOOL			IsValidPropertyName(LPCWSTR strPropName);

	virtual void			GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult);
	virtual BOOL			GetNote(std::wstring &strNote);

	virtual BOOL			FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus);
	virtual BOOL			GetFilter(ODHDCATEGORY odhdCat);

private:
	BOOL			m_bSaveAsed;
	BOOL			m_bReadonly;
	CComPtr<IDispatch>		m_pExcelApp;
	CComPtr<IDispatch>		m_pWorkbook;
	unsigned int	m_nVersion;
	bool			m_bUnhiddenRowCol;//If pivottable or list header are unhidden,it will be true;
private:
	HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...)
	{
		// Variables used...
		DISPPARAMS  dp          = { NULL, NULL, 0, 0 };
		DISPID      dispidNamed = DISPID_PROPERTYPUT;
		DISPID      dispID;
		HRESULT     hr;
		char        szName[256];

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
		hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
		delete [] pArgs;
		return hr;
	}

	// get Workbook object
	bool GetWorkBook(void);
	bool InspectComments(bool bRemove=false);
	bool InspectDocPropertiesPersInfo(bool bRemove=false);
	bool InspectCustomXMLData(bool bRemove=false);
	bool InspectHeadersFooters(bool bRemove=false);
	bool InspectHeadersFootersEx(bool bRemove=false);
	bool InspectRowsColumns(bool bRemove=false);
	bool InspectWorksheets(bool bRemove=false);
	bool InspectInvisibleContent(bool bRemove=false);
	bool InspectAutoFilter(bool bRemove=false);
	bool ExistHeaderFooterInSheet(IDispatch*pSheet,bool bRemove=false);

	// remove properties
	BOOL EmptyDocumentPropValue(CComPtr<IDispatch> pProps, LPCWSTR pwzName, LPCWSTR pwzValue)
	{
		HRESULT hr   = S_OK;
		BOOL    bSet = FALSE;

		// Looking for existing prop
		CComVariant result;
		CComVariant varName(pwzName);
		hr = AutoWrap(DISPATCH_PROPERTYGET, &result, pProps, L"Item", 1, varName);
		if( SUCCEEDED(hr) && result.pdispVal )
		{
			CComPtr<IDispatch>	pProp = result.pdispVal;
			CComVariant varValue(pwzValue);
			hr = AutoWrap(DISPATCH_PROPERTYPUT, NULL, pProp, L"Value", 1, varValue);
		}
		return bSet;
	}
	// save doc after modify
	BOOL SafeExcelDoc()
	{
		if (m_pExcelApp == NULL)
		{
			return FALSE;
		}

		HRESULT hr = S_OK;
		CComVariant varResult;
		
		if(m_bReadonly||CODHDUtilities::IsExcelTemplateFile(m_strTempFileName.c_str()))
		{
			std::wstring strTempSaveAsFile = m_strTempFileName;
			strTempSaveAsFile.append(L".SaveAsTemp");
			CComVariant varTempSaveAsFile(strTempSaveAsFile.c_str());
			CComVariant varCR(xlLocalSessionChanges);
			CComVariant varZero((int)0);
			CComVariant varTrue((short)TRUE);
			CComVariant varFalse((short)FALSE);
			CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
			CComVariant varNoChange(xlNoChange);
			hr = AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"SaveAs",11,
				vtMissing,
				vtMissing,
				varFalse,
				varCR,
				varNoChange,
				varFalse,
				varFalse,
				covOptional,
				covOptional,
				covOptional,
				varTempSaveAsFile
				);
			if(SUCCEEDED(hr))
			{
				varResult.Clear();
				hr = AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"Close",0);
				if(SUCCEEDED(hr))
				{
					::DeleteFileW(m_strTempFileName.c_str());
					::MoveFileW(strTempSaveAsFile.c_str(), m_strTempFileName.c_str());
					m_bSaveAsed=TRUE;
				}
			}
		}
		else
		{
			varResult.Clear();
			hr = AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"Save",0);
			if(SUCCEEDED(hr))
			{
				varResult.Clear();
				hr = AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"Close",0);
			}

		}
		SAFE_RELEASE(m_pWorkbook);
		return SUCCEEDED(hr)?TRUE:FALSE;
	}
};


#endif //__H_ODHD_EXCEL__
