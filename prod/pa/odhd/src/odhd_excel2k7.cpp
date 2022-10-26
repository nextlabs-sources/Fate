#include "stdafx.h"

#include "odhd_excel2k7.h"
#include "Strsafe.h"

#ifndef WSO2K3



extern std::wstring g_wstrCantOpenFile;



BOOL	CODHDExcel2K7Inspector::IsValidPropertyName(LPCWSTR strPropName)

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





BOOL	CODHDExcel2K7Inspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)

{

	int		nStep=MAX_HDRDLG_PROGRESS;



	if(m_pExcelApp == NULL)	return FALSE;

	m_strSrcFileName = strSrcFileName;

	m_strTempFileName = strTempFileName;

	m_bWin10AndTemplateFile = IsWin10AndTemplateFile();

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



		SetProgTitle(ODHDCAT_EXCEL_XML);

		bTrue = InspectCustomXMLData();

		MakeStep(1);nStep-=1;



		SetProgTitle(ODHDCAT_EXCEL_AUTOFILTER);

		bTrue = InspectAutoFilter();

		MakeStep(1);nStep-=1;

		

	

	if(nStep > 0)

		MakeStep(nStep);

	CComVariant varResult;

	CComVariant varDontSaveChanges((short)FALSE);

	HRESULT hr=AutoWrap(DISPATCH_METHOD,&varResult,m_pWorkbook,L"Close",1,varDontSaveChanges);

	SAFE_RELEASE(m_pWorkbook);

	return TRUE;

}

BOOL	CODHDExcel2K7Inspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

{

	bool bTrue = false;

	
	

	if( this->m_bReadonly == FALSE && !m_bWin10AndTemplateFile)

	{

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

			bTrue = InspectHeadersFooters(true);

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

		case ODHDCAT_EXCEL_XML:

			bTrue = InspectCustomXMLData(true);

			if (bTrue && m_pHDRFile != NULL)

			{

				m_pHDRFile->m_dwRemovedCatType |= ExcelHDCat_XML;

			}

			break;

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
		
		if (bTrue)
		{
			SafeExcelDoc();
		}
		
	}

	if(bTrue)

		SetCatStatus(odhdCat,INSPECT_REMOVED);

	else

		SetCatStatus(odhdCat,INSPECT_FAILED);



	//SAFE_RELEASE(m_pWorkbook);

	return bTrue?TRUE:FALSE;

}



long	CODHDExcel2K7Inspector::GetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent)

{

	return TRUE;

}

long	CODHDExcel2K7Inspector::SetContent(ODHDCATEGORY odhdCat,ODHDITEM odhdItem,std::wstring &strContent)

{

	return 0;

}

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& private function&&&&&&&&&&&&&&&&&&&& */

// get Workbook object

bool CODHDExcel2K7Inspector::GetWorkBook(void)

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

	if(FAILED(hr)||varWorkBooks.pdispVal==NULL)	

	{
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

		covOptional,    // Editable

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
		return false;

	}



	///VARIANT_BOOL         varVisible   = VARIANT_FALSE;

	CComVariant varVisible(VARIANT_FALSE);

	CComVariant varNumb((int)0);

	//	CComVariant varFalse((short)FALSE);

	varResult.Clear();
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"MultiUserEditing",0);

	if(SUCCEEDED(hr))

		m_bShared=varResult.boolVal;

	hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"CheckCompatibility",1,varFalse);

	hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"DoNotPromptForConvert",1,varFalse);



	hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pExcelApp,L"Visible",1,varVisible);

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



bool CODHDExcel2K7Inspector::InspectComments(bool bRemove)

{

	if(!GetWorkBook())	return false;



	bool bRemoveOK=true;

	HRESULT hr = S_OK;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;

	

	if(bRemove==true)

	{

		hr=pWorkbook->RemoveDocumentInformation(xlRDIComments);

		hr=pWorkbook->RemoveDocumentInformation(xlRDIInkAnnotations);

	}

	else

	{

		if(m_bShared==0)

		{

			BOOL           bHasComments= FALSE;    

			CComPtr<Excel::Sheets> spSheets    = 0;

			long           lSheetCount = 0;

			HRESULT        hr          = S_OK;

			int            i = 0,j = 0;


			hr = pWorkbook->get_Sheets(&spSheets);

			if(SUCCEEDED(hr) && spSheets)

			{

				hr = spSheets->get_Count(&lSheetCount);

				if(SUCCEEDED(hr) && lSheetCount)

				{

					for(i=1; i<=lSheetCount; i++)

					{

            			CComVariant     vtComments;

            			CComVariant     vtCommentCount;
            			
						VARIANT vtIndex; vtIndex.vt = VT_INT; vtIndex.intVal = i;

						CComPtr<Excel::_Worksheet>  spSheet = 0;

						hr = spSheets->get_Item(vtIndex, (IDispatch**)&spSheet);

						if(SUCCEEDED(hr) && spSheet)

						{

							CComPtr<IDispatch>   spComments = 0;



							hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtComments, spSheet, L"Comments", 0);

							if(SUCCEEDED(hr) && vtComments.vt==VT_DISPATCH && vtComments.pdispVal)

							{

								long lCommentCount = 0;

								spComments = vtComments.pdispVal;



								hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtCommentCount, spComments, L"Count", 0);

								if(SUCCEEDED(hr) && vtCommentCount.lVal)

								{

									bHasComments = TRUE;

								}


							}




						}



						if(bHasComments) break;

					}

				}

			}

			if(bHasComments==TRUE)

				RecordInspectResult(ODHDCAT_EXCEL_COMMENTS,

								ODHDITEM_EXCEL_COMMENTS,

								INSPECT_HAVE,

								gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_COMMENTS].m_itemTitle,

								gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_COMMENTS].m_itemFound);

			return bHasComments==TRUE?true:false;

		}

	}

	return bRemoveOK;

}



bool CODHDExcel2K7Inspector::InspectDocPropertiesPersInfo(bool bRemove)

{

	if(!GetWorkBook())	return false;



	bool bRemoveOK=true;

	HRESULT	hr=S_OK;



	if(m_bShared)

		return true;

	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	bool bTrue=false;

	CComVariant varResult;

	bool bProperties=false;

	bool bRemoveOk=true;

	if(bRemove==true)

	{

		hr=pWorkbook->RemoveDocumentInformation(xlRDIDocumentProperties);if(FAILED(hr))bRemoveOK=false;

		hr=pWorkbook->RemoveDocumentInformation(xlRDIRemovePersonalInformation);if(FAILED(hr))bRemoveOK=false;

		hr=pWorkbook->RemoveDocumentInformation(xlRDIRoutingSlip);if(FAILED(hr))bRemoveOK=false;

		//hr=pWorkbook->RemoveDocumentInformation(xlRDIDocumentServerProperties);if(FAILED(hr))bRemoveOK=false;

		hr=pWorkbook->RemoveDocumentInformation(xlRDIDocumentManagementPolicy);if(FAILED(hr))bRemoveOK=false;

		if(m_HasContentType==TRUE)

		{

			bRemoveOK=InspectCustomXMLData(true);

			hr=pWorkbook->RemoveDocumentInformation(xlRDIContentType);if(FAILED(hr))bRemoveOK=false;

		}

		

		hr=pWorkbook->RemoveDocumentInformation(xlRDIScenarioComments);if(FAILED(hr))bRemoveOK=false;

		hr=pWorkbook->RemoveDocumentInformation(xlRDIPublishInfo);if(FAILED(hr))bRemoveOK=false;

		hr=pWorkbook->RemoveDocumentInformation(xlRDIInactiveDataConnections);if(FAILED(hr))bRemoveOK=false;

		return bRemoveOK;

	}

	// Doc Server Information

	CComPtr<Office::DocumentLibraryVersions> spDocLibVersions = NULL;

	hr = AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pWorkbook,L"DocumentLibraryVersions",0);





    hr = pWorkbook->get_DocumentLibraryVersions(&spDocLibVersions);

    if(SUCCEEDED(hr) && spDocLibVersions)

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_SRVPROP,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_SRVPROP].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_SRVPROP].m_itemFound);

    }



	// properties

	unsigned long lProCount =0;

	CComVariant varEmpty;

	varEmpty.vt=VT_EMPTY;



	{

	BOOL        bFind   = FALSE;

    HRESULT     hr             = S_OK;

    CComPtr<IDispatch>  lpBIPropDisp   = 0;

    CComPtr<IDispatch>  lpCUPropDisp   = 0;

    long        i = 0, lCount  = 0;

    CComVariant     vtResult;

    WCHAR       wzTemplateName[MAX_PATH];  memset(wzTemplateName, 0, sizeof(wzTemplateName));



    if(NULL==pWorkbook) return FALSE;



    // Find document properties

	bool bFindDocmentProperties = false;
    hr = pWorkbook->get_BuiltinDocumentProperties(&lpBIPropDisp);

    if(SUCCEEDED(hr) && lpBIPropDisp)

    {

        if(HasDocumentProperties(lpBIPropDisp))

        {
			
			bFindDocmentProperties = true;

        }


    }

   
	if (!bFindDocmentProperties)
	{

		hr = pWorkbook->get_CustomDocumentProperties(&lpCUPropDisp);

		if(SUCCEEDED(hr) && lpCUPropDisp)

		{

			// Get ourself property

			int nOurProp = 0;

			WCHAR wzOurProp[MAX_PATH+1];	memset(wzOurProp, 0, sizeof(wzOurProp));

			if( GetPropertyValueByName(lpCUPropDisp, OCP_REALPATH, wzOurProp, MAX_PATH) )

				nOurProp = 1;



			hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtResult, lpCUPropDisp, L"Count", 0);

			lCount = vtResult.lVal;

			if(lCount>nOurProp)

			{
			
				bFindDocmentProperties = true;

			}

		}


	}
	

	if (bFindDocmentProperties)
	{

		RecordInspectResult(ODHDCAT_EXCEL_PROP,

			ODHDITEM_EXCEL_DOCPROP,

			INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_DOCPROP].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_DOCPROP].m_itemFound);	
	}
	






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

				hr = AutoWrap(DISPATCH_PROPERTYPUT,&varResult,m_pWorkbook,L"HasRoutingSlip",1,varFalse);

				if(FAILED(hr))		bRemoveOk=false;

			}

			else

			{

				RecordInspectResult(ODHDCAT_EXCEL_PROP,

					ODHDITEM_EXCEL_ROUTESLIP,

					INSPECT_HAVE,

					gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_ROUTESLIP].m_itemTitle,

					gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_ROUTESLIP].m_itemFound);		

			}

		}

	}

	}

	

	// Document Management Policy information

	CComPtr<Office::ServerPolicy> spServerPolicy = 0;

    hr = pWorkbook->get_ServerPolicy(&spServerPolicy);

    if(SUCCEEDED(hr) && spServerPolicy)

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_MANAGEPOLICY,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_MANAGEPOLICY].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_MANAGEPOLICY].m_itemFound);

    }

	

	 // Find Content Type

    CComPtr<Office::MetaProperties> spMetaProperties = 0;

    hr = pWorkbook->get_ContentTypeProperties(&spMetaProperties);

    if(SUCCEEDED(hr) && spMetaProperties)

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_CONTENTTYPE,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_CONTENTTYPE].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_CONTENTTYPE].m_itemFound);

		m_HasContentType=TRUE;

    }



	// secenario comments

	if(HasScenarioComments(pWorkbook))

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_SCENARIO,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_SCENARIO].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_SCENARIO].m_itemFound);

    }



	// get Publish Object Information

    if(HasPublishInformation(pWorkbook))

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_WEBPUBLISH,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WEBPUBLISH].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WEBPUBLISH].m_itemFound);

    }



    // get Connections

    if(HasConnections(pWorkbook))

    {

        RecordInspectResult(ODHDCAT_EXCEL_PROP,ODHDITEM_EXCEL_EXTDATACONN,INSPECT_HAVE,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_EXTDATACONN].m_itemTitle,

			gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_EXTDATACONN].m_itemFound);

    }

	return bRemoveOk;

}

BOOL CODHDExcel2K7Inspector::HasConnections(Excel::_Workbook *spWbk)

{

    HRESULT             hr = S_OK;

    BOOL                bHasConnections = FALSE;

    CComPtr<Excel::Connections> spConnect = NULL;



    hr = spWbk->get_Connections(&spConnect);

    if(SUCCEEDED(hr) && spConnect)

    {

        CComVariant vtConnCount;

        hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtConnCount, spConnect, L"Count", 0);

        if(SUCCEEDED(hr) && vtConnCount.lVal>0)

        {

            bHasConnections = TRUE;

        }



    }



    return bHasConnections;

}

BOOL CODHDExcel2K7Inspector::HasPublishInformation(Excel::_Workbook* spWbk)

{

    HRESULT             hr = S_OK;

    BOOL                bHasPublishObj = FALSE;

    CComPtr<Excel::PublishObjects> spPublishObjs = NULL;



    hr = spWbk->get_PublishObjects(&spPublishObjs);

    if(SUCCEEDED(hr) && spPublishObjs)

    {

        CComVariant vtConnCount;

        hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtConnCount, spPublishObjs, L"Count", 0);

        if(SUCCEEDED(hr) && vtConnCount.lVal>0)

        {

            bHasPublishObj = TRUE;

        }



    }



    return bHasPublishObj;

}

BOOL CODHDExcel2K7Inspector::HasScenarioComments(Excel::_Workbook* pWorkbook)

{

    BOOL           bHasScenarioComments= FALSE;    

    CComPtr<Excel::Sheets> spSheets    = 0;

    long           lSheetCount = 0;

    HRESULT        hr          = S_OK;

    int            i = 0,j = 0;


    hr = pWorkbook->get_Sheets(&spSheets);

    if(SUCCEEDED(hr) && spSheets)

    {

        hr = spSheets->get_Count(&lSheetCount);

        if(SUCCEEDED(hr) && lSheetCount)

        {

            for(i=1; i<=lSheetCount; i++)

            {

                VARIANT vtIndex; vtIndex.vt = VT_INT; vtIndex.intVal = i;

                CComPtr<Excel::_Worksheet>  spSheet = 0;

                hr = spSheets->get_Item(vtIndex, (IDispatch**)&spSheet);

                if(SUCCEEDED(hr) && spSheet)

                {

                    VARIANT vtScenarioIndex; vtScenarioIndex.vt = VT_INT;

                    for(j=0; ; j++)

                    {
                        CComVariant        vtScenarios;

                        CComVariant        vtScenarioComment;

                        vtScenarioIndex.intVal = j+1;

                        hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtScenarios, spSheet, L"Scenarios", 1, vtScenarioIndex);

                        if(FAILED(hr) || vtScenarios.vt!=VT_DISPATCH || NULL==vtScenarios.pdispVal)

                            break;


                        CComPtr<IDispatch> spDispatch = vtScenarios.pdispVal;
                        hr = AutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtScenarioComment, spDispatch, L"Comment", 0);

                        if(SUCCEEDED(hr) || vtScenarioComment.vt==VT_BSTR || NULL!=vtScenarioComment.bstrVal)

                        {

                            bHasScenarioComments = TRUE;

                        }


                    }



                }

            }

        }



    }



    return bHasScenarioComments;

}

bool CODHDExcel2K7Inspector::InspectCustomXMLData(bool bRemove)

{

	if(!GetWorkBook())	return false;



	HRESULT			hr   = S_OK;

	bool			bRet=true;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pWorkbook->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if(bRemove==false)

		{
			BOOL bSuccess = FALSE;
			if (m_IsOffice2016)
			{
				bSuccess = InspectorExist(spInspectors,1,_T("Custom &XML Data"));
				if (!bSuccess)
				{
					bSuccess = InspectorExist(spInspectors,1,_T("Custom XML Data"));
				}
				
			}
			else
			{
				bSuccess = InspectorExist(spInspectors,1,_T("Custom XML"));
			}


			if(bSuccess)

			{

				m_InspectorIndex1=1;

				if(InspectByInspector(spInspectors,ODHDCAT_EXCEL_XML,ODHDITEM_EXCEL_CUSTOMXML)==FALSE)

					bRet=false;

			}

			else

			{

				m_InspectorIndex1=0;

			}

		}

		else

		{

			if(m_InspectorIndex1!=0)

			{

				if(RemoveByInspector(spInspectors,m_InspectorIndex1,ODHDCAT_EXCEL_XML,ODHDITEM_EXCEL_CUSTOMXML)==FALSE)

					bRet=false;

			}

		}

	}

	else

		bRet=false;

	

	return bRet;

}





bool CODHDExcel2K7Inspector::ExistHeaderFooterInSheet(IDispatch*pSheet,bool bRemove)

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

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WSHEADER].m_itemTitle,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WSHEADER].m_itemFound);

		}

		if(bFooters)

		{

			RecordInspectResult(ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSFOOTER,INSPECT_HAVE,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WSFOOTER].m_itemTitle,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_WSFOOTER].m_itemFound);

		}

		return (bHeaders||bFooters);

	}

	

	return bRemoveOk;



}

bool CODHDExcel2K7Inspector::InspectHeadersFooters(bool bRemove)

{

	if(!GetWorkBook())	return false;



	HRESULT			hr   = S_OK;

	bool			bRet=true;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pWorkbook->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if(bRemove==false)

		{

			if(InspectorExist(spInspectors,2,_T("Headers and Footers")))

			{

				m_InspectorIndex2=2;

				if(InspectByInspector(spInspectors,ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSHEADER)==FALSE)

					bRet=false;

			}

			else

			{

				if(InspectorExist(spInspectors,1,_T("Headers and Footers")))

				{

					m_InspectorIndex2=1;

					if(InspectByInspector(spInspectors,1,ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSHEADER)==FALSE)

						bRet=false;

				}

				else

					m_InspectorIndex2=0;

			}

		}

		else

		{

			if(m_InspectorIndex2!=0)

			{

				if(RemoveByInspector(spInspectors,m_InspectorIndex2,ODHDCAT_EXCEL_HEADFOOT,ODHDITEM_EXCEL_WSHEADER)==FALSE)

					bRet=false;

			}

		}

	}

	else

		bRet=false;

	

	return bRet;

}

bool CODHDExcel2K7Inspector::InspectRowsColumns(bool bRemove)

{

	if(!GetWorkBook())	return false;



	HRESULT			hr   = S_OK;

	bool			bRet=true;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pWorkbook->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if(bRemove==false)

		{

			BOOL bSuccess = FALSE;
			if (m_IsOffice2016)
			{
				bSuccess = InspectorExist(spInspectors,3,_T("Hidden R&ows and Columns"));
				if (!bSuccess)
				{
					bSuccess = InspectorExist(spInspectors,3,_T("Hidden Rows and Columns"));
				}
				
			}
			else
			{
				bSuccess = InspectorExist(spInspectors,3,_T("Hidden Rows and Columns"));
			}

			if(bSuccess)

			{

				m_InspectorIndex3=3;

				if(InspectByInspector(spInspectors,ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENROWS)==FALSE)

					bRet=false;

			}

			else

			{

				if(InspectorExist(spInspectors,2,_T("Hidden Rows and Columns")))

				{

					m_InspectorIndex3=2;

					if(InspectByInspector(spInspectors,2,ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENROWS)==FALSE)

						bRet=false;

				}

				else

				{

					if(InspectorExist(spInspectors,1,_T("Hidden Rows and Columns")))

					{

						m_InspectorIndex3=1;

						if(InspectByInspector(spInspectors,1,ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENROWS)==FALSE)

							bRet=false;

					}

					else

					{

						m_InspectorIndex3=0;

					}

				}

			}

			

		}

		else

		{

			if(m_InspectorIndex3!=0)

			{

				if(RemoveByInspector(spInspectors,m_InspectorIndex3,ODHDCAT_EXCEL_ROWCOL,ODHDITEM_EXCEL_HIDDENROWS)==FALSE)

					bRet=false;

			}

		}

	}

	else

		bRet=false;

	

	return bRet;

}



bool CODHDExcel2K7Inspector::InspectWorksheets(bool bRemove)

{

	if(!GetWorkBook())	return false;



	HRESULT			hr   = S_OK;

	bool			bRet=true;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pWorkbook->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if(bRemove==false)

		{
			BOOL bSuccess = FALSE;
			if (m_IsOffice2016)
			{
				bSuccess = InspectorExist(spInspectors,4,_T("H&idden Worksheets"));
				if (!bSuccess)
				{
					bSuccess = InspectorExist(spInspectors,4,_T("Hidden Worksheets"));
				}
				
			}
			else
			{
				bSuccess = InspectorExist(spInspectors,4,_T("Hidden Worksheets"));
			}


			if(bSuccess)

			{

				m_InspectorIndex4=4;

				if(InspectByInspector(spInspectors,ODHDCAT_EXCEL_WORKSHEET,ODHDITEM_EXCEL_HIDDENWS)==FALSE)

					bRet=false;

			}

			else

			{

				if(InspectorExist(spInspectors,3,_T("Hidden Worksheets")))

				{

					m_InspectorIndex4=3;

					if(InspectByInspector(spInspectors,3,ODHDCAT_EXCEL_WORKSHEET,ODHDITEM_EXCEL_HIDDENWS)==FALSE)

						bRet=false;

				}

				else

				{

					if(InspectorExist(spInspectors,2,_T("Hidden Worksheets")))

					{

						m_InspectorIndex4=2;

						if(InspectByInspector(spInspectors,2,ODHDCAT_EXCEL_WORKSHEET,ODHDITEM_EXCEL_HIDDENWS)==FALSE)

							bRet=false;

					}

					else

					{

						if(InspectorExist(spInspectors,1,_T("Hidden Worksheets")))

						{

							m_InspectorIndex4=1;

							if(InspectByInspector(spInspectors,1,ODHDCAT_EXCEL_WORKSHEET,ODHDITEM_EXCEL_HIDDENWS)==FALSE)

								bRet=false;

						}

						else

						{

							m_InspectorIndex4=0;



						}

					}

				}

			}

		}

		else

		{

			if(m_InspectorIndex4!=0)

			{

				if(RemoveByInspector(spInspectors,m_InspectorIndex4,ODHDCAT_EXCEL_WORKSHEET,ODHDITEM_EXCEL_HIDDENWS)==FALSE)

					bRet=false;

			}

			

		}

			

	}

	else

		bRet=false;

	

	return bRet;

}




bool CODHDExcel2K7Inspector::InspectInvisibleContentbyShapes(bool bRemove,int &nInvisibleContentNum)
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
		nInvisibleContentNum = 0;
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

								long lShapesCount=varShapesCount.lVal;
								for(int index=lShapesCount;index>0;index--)
								{
									CComVariant varShapeIndex(index);
									CComVariant varShape;
									hr=AutoWrap(DISPATCH_METHOD,&varShape,varShapes.pdispVal,L"Item",1,varShapeIndex);
									if(SUCCEEDED(hr)&&varShape.pdispVal)
									{
										CComVariant varVisible;
										hr=AutoWrap(DISPATCH_PROPERTYGET,&varVisible,varShape.pdispVal,L"Visible",0);
										if (SUCCEEDED(hr))
										{
											if (varVisible.lVal == Office::MsoTriState::msoFalse)
											{
												CComVariant varType;
												hr=AutoWrap(DISPATCH_PROPERTYGET,&varType,varShape.pdispVal,L"Type",0);
												if (SUCCEEDED(hr))
												{
													if (varType.lVal == Office::MsoShapeType::msoAutoShape)
													{
														nInvisibleContentNum++;
														if (bRemove)
														{
															CComVariant varDelete;
															hr=AutoWrap(DISPATCH_METHOD,&varDelete,varShape.pdispVal,L"Delete",0);
															if(FAILED(hr))
															{
																return false;
															}
														}

													}
												}
											}
										}


									}
								}//for index

							}

						}
					}
				}
			}
		}
	}

	return true;
}






ODHDSTATUS	CODHDExcel2K7Inspector::InspectInvisibleContentByInspector(Office::DocumentInspectors* spInspectors,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD *dwInspectFail)

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

		
		
			if (INSPECT_FAILED == retStatus)
			{
				int nInvisibleContent = 0;
				InspectInvisibleContentbyShapes(false,nInvisibleContent);
				wchar_t szResult[MAX_PATH] = {0};
				StringCbPrintf(szResult,MAX_PATH,L"Number of invisible objects found: %d",nInvisibleContent);
				if (nInvisibleContent > 0)
				{
					CComBSTR ShapesRst(szResult);
					result = ShapesRst;
					retStatus = INSPECT_HAVE;
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



ODHDSTATUS		CODHDExcel2K7Inspector::InspectInvisibleContentByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem,DWORD *dwInspectFail)
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


			if (INSPECT_FAILED == retStatus)
			{
				int nInvisibleContent = 0;
				InspectInvisibleContentbyShapes(false,nInvisibleContent);
				wchar_t szResult[MAX_PATH] = {0};
				StringCbPrintf(szResult,MAX_PATH,L"Number of invisible objects found: %d",nInvisibleContent);
				if (nInvisibleContent > 0)
				{
					CComBSTR ShapesRst(szResult);
					result = ShapesRst;
					retStatus = INSPECT_HAVE;
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


BOOL	CODHDExcel2K7Inspector::RemoveInvisibleContentByInspector(Office::DocumentInspectors* spInspectors,int iInspectorIndex,ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

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

			if (retStatus == INSPECT_FAILED)
			{
				int nInvisibleContentNum  = 0;
				bool bRetShapes = InspectInvisibleContentbyShapes(true,nInvisibleContentNum);
				if (bRetShapes && nInvisibleContentNum > 0)
				{
					retStatus = INSPECT_REMOVED;
					CComBSTR ShapesResult(L"Invisible objects were successfully removed.");
					result = ShapesResult;
					bRet=TRUE;
				}

			}

			RecordInspectResult(odhdCat,odhdItem,retStatus,name,result);
		}
	}

	return bRet;



}



bool CODHDExcel2K7Inspector::InspectInvisibleContent(bool bRemove)

{

	if(!GetWorkBook())	return false;



	HRESULT			hr   = S_OK;

	bool			bRet=true;



	CComPtr<Excel::_Workbook> pWorkbook=NULL;

	hr=m_pWorkbook->QueryInterface(__uuidof(_Workbook),(void**)&pWorkbook);

	if(FAILED(hr)||pWorkbook==NULL)

		return false;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pWorkbook->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if(bRemove==false)

		{

			BOOL bSuccess = FALSE;
			if (m_IsOffice2016)
			{
				bSuccess = InspectorExist(spInspectors,5,_T("In&visible Content"));
				if (!bSuccess)
				{
					bSuccess = InspectorExist(spInspectors,5,_T("Invisible Content"));
				}
			}
			else
			{
				bSuccess = InspectorExist(spInspectors,5,_T("Invisible Content"));
			}

			if(bSuccess)

			{

				m_InspectorIndex5=5;

				if(InspectInvisibleContentByInspector(spInspectors,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

					bRet=false;

			}

			else

			{

				if(InspectorExist(spInspectors,4,_T("Invisible Content")))

				{

					m_InspectorIndex5=4;

					if(InspectInvisibleContentByInspector(spInspectors,4,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

						bRet=false;

				}

				else

				{

					if(InspectorExist(spInspectors,3,_T("Invisible Content")))

					{

						m_InspectorIndex5=3;

						if(InspectInvisibleContentByInspector(spInspectors,3,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

							bRet=false;

					}

					else

					{

						if(InspectorExist(spInspectors,2,_T("Invisible Content")))

						{

							m_InspectorIndex5=2;

							if(InspectInvisibleContentByInspector(spInspectors,2,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

								bRet=false;

						}

						else

						{

							if(InspectorExist(spInspectors,1,_T("Invisible Content")))

							{

								m_InspectorIndex5=1;

								if(InspectInvisibleContentByInspector(spInspectors,1,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

									bRet=false;

							}

							else

							{

								m_InspectorIndex5=0;

							}

						}

					}

				}

			}

		}

		else

		{

			if(m_InspectorIndex5!=0)

			{

				if(RemoveInvisibleContentByInspector(spInspectors,m_InspectorIndex5,ODHDCAT_EXCEL_INVISCONTENT,ODHDITEM_EXCEL_INVIOBJECT)==FALSE)

					bRet=false;

			}

			

		}

	}

	else

		bRet=false;

	

	return bRet;

}



bool CODHDExcel2K7Inspector::InspectAutoFilter(bool bRemove)

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

	if(!bRemove)

	{

		if (nCount>0)

		{

			RecordInspectResult(ODHDCAT_EXCEL_AUTOFILTER,

				ODHDITEM_EXCEL_AUTOFILTER,

				INSPECT_HAVE,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_AUTOFILTER].m_itemTitle,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_AUTOFILTER].m_itemFound);

		}

		else

		{

			RecordInspectResult(ODHDCAT_EXCEL_AUTOFILTER,

				ODHDITEM_EXCEL_AUTOFILTER,

				INSPECT_NONE,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_AUTOFILTER].m_itemTitle,

				gs_ItemTitle_Excel2K7[ODHDITEM_EXCEL_AUTOFILTER].m_itemNotFound);

		}

	}



	return bRemoveOk;

}



void CODHDExcel2K7Inspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)

{

	if(m_bShared&&(odhdCat==ODHDCAT_EXCEL_COMMENTS||odhdCat==ODHDCAT_EXCEL_PROP))

		strResult=L"Hidden information cannot be removed from this workbook because it is a shared workbook.";

	else

	{

		CODHDInspector::GetResult(odhdCat,strResult);

	}

	

}



BOOL CODHDExcel2K7Inspector::GetNote(std::wstring &strNote)

{

	if(m_bReadonly&&m_bSaveAsed)

	{

		//strNote.assign(L"The file was opened in read-only mode. If the file has password protection on modify, Remove All will also remove password protection from the attachment.");

		strNote.assign(L"Password protection on modify is removed \nfor read-only files.");

		return TRUE;

	}

	return FALSE;

}



BOOL CODHDExcel2K7Inspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)

{

	BOOL bFiltered = FALSE;



	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_EXCEL_COMMENTS && !(dwNeedCat & ExcelHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_EXCEL_PROP && !(dwNeedCat & ExcelHDCat_PROP))

			|| (odhdCat == ODHDCAT_EXCEL_XML && !(dwNeedCat & ExcelHDCat_XML))

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

			case ODHDCAT_EXCEL_XML:

				m_pHDRFile->m_dwFoundCatType |= ExcelHDCat_XML;

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

			default:

				break;

			}

		}

	}



	return bFiltered;

}



BOOL CODHDExcel2K7Inspector::GetFilter(ODHDCATEGORY odhdCat)

{

	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_EXCEL_COMMENTS && !(dwNeedCat & ExcelHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_EXCEL_PROP && !(dwNeedCat & ExcelHDCat_PROP))

			|| (odhdCat == ODHDCAT_EXCEL_XML && !(dwNeedCat & ExcelHDCat_XML))

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



#endif //WSO2K7

