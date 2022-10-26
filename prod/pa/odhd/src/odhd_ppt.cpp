#include "stdafx.h"
#include "odhd_ppt.h"

static LPCWSTR gs_PropertiesNames_Ppt[]={
	/*{L"Title"},*/
	{L"Subject"},
	{L"Author"},
	{L"Keywords"},
	{L"Comments"},
	{L"Last author"},
	{L"Category"},
	{L"Manager"},
	{L"Company"},
	{L"Content type"},
	{L"Content status"},
	{L"Language"},
	{L"Hyperlink base"}

	/*
	{L"Number of words"},
	{L"Number of characters"},
	{L"Number of bytes"},
	{L"Number of lines"},
	{L"Number of paragraphs"},
	{L"Number of slides"},
	{L"Number of notes"},
	{L"Number of pages"},
	{L"Last save time"},
	{L"Total editing time"},
	{L"Last print date"}
	{L"Application name"},
	{L"Security"},
	*/
};

extern std::wstring g_wstrCantOpenFile;

BOOL CODHDPptInspector::IsValidPropertyName(LPCWSTR strPropName)
{
	int iIndex=0;
	for(iIndex=0;iIndex<sizeof(gs_PropertiesNames_Ppt)/sizeof(LPCWSTR);iIndex++)
	{
		if(0 == _wcsicmp(strPropName, gs_PropertiesNames_Ppt[iIndex]))
			return TRUE;
	}
	return FALSE;
	/*
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
    if(0 == _wcsicmp(strPropName, L"Content type"))
        return TRUE;

    return FALSE;*/
}


ODHDSTATUS	CODHDPptInspector::InspectComments(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<PPT::Slides>    spSlides   = NULL;
	
	long            i          = 0;
	

	hr = pDoc->get_Slides(&spSlides);
	if(SUCCEEDED(hr) && spSlides)
	{
		long nSlideCount = 0;
		hr = spSlides->get_Count(&nSlideCount);
		if(SUCCEEDED(hr) && nSlideCount)
		{
			for(i=0; i<nSlideCount; i++)
			{
				CComPtr<PPT::_Slide>    spSlide = 0;
				CComVariant     nSlideIndex(i+1);
				hr = spSlides->Item(nSlideIndex, &spSlide);
				if(SUCCEEDED(hr) && spSlide)
				{
					CComPtr<PPT::pptComments>   spComments = 0;
					hr = spSlide->get_pptComments(&spComments);
					if(SUCCEEDED(hr) && spComments)
					{
						long nCmtCount = 0;
						hr = spComments->get_Count(&nCmtCount);
						if(SUCCEEDED(hr) && nCmtCount)
						{
							/*
							PPT::pptComment *pComment=NULL;
							BSTR	bstrText;
							long iIndex=0;
							for(iIndex;iIndex<nCmtCount;iIndex++)
							{
								hr=spComments->Item(iIndex+1,&pComment);
								if(SUCCEEDED(hr)&&pComment)
								{
									hr=pComment->get_Text(&bstrText);
									::SysFreeString(bstrText);
									pComment->Release();
								}
							}
							*/
							retStatus=INSPECT_HAVE;
							break;
						}
					}
				}
			}
		}
	}
	if(retStatus==INSPECT_HAVE)
		RecordInspectResult(ODHDCAT_PPT_COMMENTS,
							ODHDITEM_PPT_COMMENTS,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_COMMENTS].m_itemTitle,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_COMMENTS].m_itemFound);
	return retStatus;
}
ODHDSTATUS CODHDPptInspector::InspectPresentationNote(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<PPT::Slides>    spSlides   = NULL;
	
	long            i          = 0;
	

	hr = pDoc->get_Slides(&spSlides);
	if(SUCCEEDED(hr) && spSlides)
	{
		long nSlideCount = 0;
		hr = spSlides->get_Count(&nSlideCount);
		if(SUCCEEDED(hr) && nSlideCount)
		{
			for(i=0; i<nSlideCount; i++)
			{
				CComPtr<PPT::_Slide>    spSlide = 0;
				CComVariant     nSlideIndex(i+1);
				hr = spSlides->Item(nSlideIndex, &spSlide);
				if(SUCCEEDED(hr) && spSlide)
				{
					CComPtr<PPT::pptComments>   spComments = 0;
					hr = spSlide->get_pptComments(&spComments);
					if(SUCCEEDED(hr) && spComments)
					{
						long nCmtCount = 0;
						hr = spComments->get_Count(&nCmtCount);
						if(SUCCEEDED(hr) && nCmtCount)
						{
							/*
							PPT::pptComment *pComment=NULL;
							BSTR	bstrText;
							long iIndex=0;
							for(iIndex;iIndex<nCmtCount;iIndex++)
							{
								hr=spComments->Item(iIndex+1,&pComment);
								if(SUCCEEDED(hr)&&pComment)
								{
									hr=pComment->get_Text(&bstrText);
									::SysFreeString(bstrText);
									pComment->Release();
								}
							}
							*/
							retStatus=INSPECT_HAVE;
						}
					}
				}
			}
		}
	}

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectBuiltinProperty(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	CComPtr<IDispatch>  pDocPropertyDispatch   = NULL;
	HRESULT		hr = pDoc->get_BuiltInDocumentProperties(&pDocPropertyDispatch);
	if(SUCCEEDED(hr)&&pDocPropertyDispatch)
	{
		if(HasDocumentProperties(pDocPropertyDispatch))
			retStatus=INSPECT_HAVE;
	}
	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectCustomProperty(PPT::_Presentation *pDoc)
{
	CComPtr<IDispatch>  pDocPropertyDispatch   = NULL;
	HRESULT		hr = pDoc->get_CustomDocumentProperties(&pDocPropertyDispatch);
	VARIANT     vtResult;
	if(SUCCEEDED(hr)&&pDocPropertyDispatch)
	{
		int nOurProp = 0;
		long lCount=0;
		WCHAR wzOurProp[MAX_PATH+1];
		memset(wzOurProp, 0, sizeof(wzOurProp));
		if( CODHDUtilities::GetDocumentPropValue(pDocPropertyDispatch, OCP_REALPATH, wzOurProp, MAX_PATH) )
			nOurProp = 1;

		hr = DispatchCallWraper(pDocPropertyDispatch,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtResult, L"Count", 0);
		lCount = vtResult.lVal;
		if(lCount>nOurProp)
			return INSPECT_HAVE;
		
	}
	return INSPECT_NONE;
}
ODHDSTATUS	CODHDPptInspector::InspectDocProperty(PPT::_Presentation *pDoc)
{
	ODHDSTATUS builtinStatus,customStatus,retStatus=INSPECT_NONE;
	builtinStatus=InspectBuiltinProperty(pDoc);
	customStatus=InspectCustomProperty(pDoc);
	if(INSPECT_NONE==builtinStatus&&INSPECT_NONE==customStatus)
		retStatus= INSPECT_NONE;
	else
	{
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_DOCPROP,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_DOCPROP].m_itemTitle,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_DOCPROP].m_itemFound);
		retStatus= INSPECT_NONE;
	}
	return retStatus;
}

ODHDSTATUS	CODHDPptInspector::InspectDocSrvProp(PPT::_Presentation *pDoc)
{
	HRESULT			hr   = S_OK;
	ODHDSTATUS	 retStatus=INSPECT_NONE;

	CComPtr<Office::DocumentLibraryVersions> pDocLibVersion = 0;
	hr = pDoc->get_DocumentLibraryVersions(&pDocLibVersion);
	if(SUCCEEDED(hr) && pDocLibVersion)
	{
		retStatus=INSPECT_HAVE;
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_SRVPROP,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_SRVPROP].m_itemTitle,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_SRVPROP].m_itemFound);

	}
	return retStatus;
}
ODHDSTATUS CODHDPptInspector::InspectEmailHeader(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;

	return retStatus;
}
ODHDSTATUS CODHDPptInspector::InspectRoutingSlip(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectSend4Review(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectOnSlideInvisible(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	

	return retStatus;
}

ODHDSTATUS	CODHDPptInspector::InspectCustomXML(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectOffSlide(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectWebPublish(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	HRESULT             hr = S_OK;
    CComPtr<PPT::PublishObjects> spPublishObjs = NULL;
	CComPtr<PPT::pptPublishObject> pPublishObj=NULL;

    hr = pDoc->get_PublishObjects(&spPublishObjs);
    if(SUCCEEDED(hr) && spPublishObjs)
    {
        VARIANT vtConnCount;
        CComPtr<IDispatch> spDispatch = spPublishObjs;
        hr = DispatchCallWraper(spDispatch,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtConnCount,  L"Count", 0);
        if(SUCCEEDED(hr) && vtConnCount.lVal>0)
		{
			hr=spPublishObjs->Item(1,&pPublishObj);
			if(SUCCEEDED(hr)&&pPublishObj)
			{
				int lStart=0,lEnd=0;
				hr=pPublishObj->get_RangeStart(&lStart);
				hr=pPublishObj->get_RangeEnd(&lEnd);
				if(lStart!=lEnd)
					retStatus=INSPECT_HAVE;
			}
		}
    }

	return retStatus;
}
ODHDSTATUS	CODHDPptInspector::InspectContentType(PPT::_Presentation *pDoc)
{
	HRESULT			hr   = S_OK;
	ODHDSTATUS	 retStatus=INSPECT_NONE;
#ifndef  WSO2K3
	CComPtr<Office::MetaProperties> spMetaProperties = 0;
	hr = pDoc->get_ContentTypeProperties(&spMetaProperties);
	if(SUCCEEDED(hr) && spMetaProperties)
	{
		retStatus=INSPECT_HAVE;
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_CONTENTTYPE,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_CONTENTTYPE].m_itemTitle,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_CONTENTTYPE].m_itemFound);
	}
#endif
	return retStatus;
}

ODHDSTATUS	CODHDPptInspector::InspectSrvPolicy(PPT::_Presentation *pDoc)
{
	HRESULT			hr   = S_OK;
	ODHDSTATUS	 retStatus=INSPECT_NONE;
#ifndef  WSO2K3
	CComPtr<Office::ServerPolicy> spServerPolicy = 0;
	hr = pDoc->get_ServerPolicy(&spServerPolicy);
	if(SUCCEEDED(hr) && spServerPolicy)
	{
		retStatus=INSPECT_HAVE;
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_MANAGEPOLICY,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_MANAGEPOLICY].m_itemTitle,
							gs_ItemTitle_Ppt[ODHDITEM_PPT_MANAGEPOLICY].m_itemFound);
	}
#endif
	return retStatus;
}
BOOL CODHDPptInspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)
{
	BOOL    bRet = TRUE;
    HRESULT hr   = S_OK;
	int		nStep=MAX_HDRDLG_PROGRESS;

	m_strTempFileName=strTempFileName;
	m_strSrcFileName =strSrcFileName;
	
	CComPtr<PPT::Presentations> spDocuments  = NULL;
    CComPtr<PPT::_Presentation>	spDocument   = NULL;

    hr = m_pApp->get_Presentations(&spDocuments);

	if(SUCCEEDED(hr) && spDocuments)
    {
        CComBSTR    FileName(strTempFileName.c_str());
        CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		_beginthread(CODHDUtilities::ShowPasswordWindows, 0, NULL);
        hr = spDocuments->Open( FileName,       // FileName
            msoFalse,    // ReadOnly
            msoFalse,    // Untitled
            msoFalse,    // WithWindow
            &spDocument);
        if(SUCCEEDED(hr) && spDocument)
        {
            VARIANT_BOOL    bReadonly  = FALSE;

            m_pApp->put_Visible(msoFalse);
            m_pApp->put_ShowWindowsInTaskbar(msoFalse);

            Office::MsoTriState msoTriState = msoFalse;
            hr = spDocument->get_ReadOnly(&msoTriState);
            if(SUCCEEDED(hr) && msoTrue==msoTriState)
            {
                bReadonly = TRUE;
            }

            // Inspect Section 1 :: Comments and Annotations
			SetProgTitle(ODHDCAT_PPT_COMMENTS);
            InspectComments(spDocument);
			MakeStep(2);nStep-=2;
		
			SetProgTitle(ODHDCAT_PPT_PROP);
			InspectDocProperty(spDocument);
			InspectEmailHeader(spDocument);
			InspectRoutingSlip(spDocument);
			MakeStep(2);nStep-=2;
			InspectSend4Review(spDocument);
			InspectDocSrvProp(spDocument);
			InspectSrvPolicy(spDocument);
			MakeStep(2);nStep-=2;
			InspectContentType(spDocument);
			InspectWebPublish(spDocument);
			
			//SetProgTitle(ODHDCAT_PPT_ONSLIDE);
			//InspectOnSlideInvisible(spDocument);
			MakeStep(1);nStep-=1;

			//SetProgTitle(ODHDCAT_PPT_OFFSLIDE);
			//InspectOffSlide(spDocument);
			MakeStep(1);nStep-=1;

			//SetProgTitle(ODHDCAT_PPT_NOTE);
			//InspectPresentationNote(spDocument);
			//InspectCustomXML(spDocument);
			MakeStep(2);nStep-=2;

            // Inspect Section 2 :: Document Properties & Personal Information
            

			hr = spDocument->Close();
        }
        else
        {
			std::wstring strMsg = L"";
			CODHDUtilities::ReplaceFileName(g_wstrCantOpenFile, strSrcFileName, strMsg);

			m_pApp->put_Visible(msoFalse);
            m_pApp->put_ShowWindowsInTaskbar(msoFalse);
			MessageBox(m_progDlg->m_hWnd, strMsg.c_str(), L"Hidden Data Removal", MB_OK | MB_ICONWARNING);
			bRet=FALSE;
        }
    }
	if(nStep > 0)
		MakeStep(nStep);
	return bRet;


}

BOOL	CODHDPptInspector::SaveDocAfterRemove(PPT::_Presentation*pDoc)
{
	BOOL	bRet=FALSE;
	VARIANT_BOOL    bReadonly  = FALSE;
	HRESULT	hr=S_OK;

	Office::MsoTriState msoTriState = msoFalse;
	hr = pDoc->get_ReadOnly(&msoTriState);
	if(SUCCEEDED(hr) && msoTrue==msoTriState)
	{
		bReadonly = TRUE;
	}
	if(bReadonly)
	{
		std::wstring strTempSaveAsFile = m_strTempFileName;
		strTempSaveAsFile.append(L".SaveAsTemp");
		CComBSTR varTempSaveAsFile(strTempSaveAsFile.c_str());
		hr = pDoc->SaveAs(varTempSaveAsFile, ppSaveAsPresentation, msoTriStateMixed);
		bRet = SUCCEEDED(hr);
		hr = pDoc->Close();
		if(bRet)
		{
			::DeleteFileW(m_strTempFileName.c_str());
			::MoveFileW(strTempSaveAsFile.c_str(), m_strTempFileName.c_str());
		}
	}
	else
	{
		hr = pDoc->Save();
		bRet = SUCCEEDED(hr);
		hr = pDoc->Close();
	}
	return bRet;
}
BOOL CODHDPptInspector::RemoveCategoryComments(PPT::_Presentation*pDoc)
{
	HRESULT hr=S_OK;
	BOOL	bRet=TRUE;
	CComPtr<PPT::Slides>    spSlides   = NULL;
	
	long            i          = 0;
	long            j          = 0;
	
	hr = pDoc->get_Slides(&spSlides);
	if(SUCCEEDED(hr) && spSlides)
	{
		long nSlideCount = 0;
		hr = spSlides->get_Count(&nSlideCount);
		if(SUCCEEDED(hr) && nSlideCount)
		{
			for(i=0; i<nSlideCount; i++)
			{
				CComPtr<PPT::_Slide>    spSlide = 0;
				CComVariant     nSlideIndex(i+1);
				hr = spSlides->Item(nSlideIndex, &spSlide);
				if(SUCCEEDED(hr) && spSlide)
				{
					CComPtr<PPT::pptComments>   spComments = 0;
					hr = spSlide->get_pptComments(&spComments);
					if(SUCCEEDED(hr) && spComments)
					{
						long nCmtCount = 0;
						hr = spComments->get_Count(&nCmtCount);
						if(SUCCEEDED(hr) && nCmtCount)
						{
							for(j=nCmtCount; j>0; j--)
							{
								CComPtr<PPT::pptComment>    spComment = 0;
								//CComVariant varCmtIndex(j);
								hr = spComments->Item((int)j, &spComment);
								if(SUCCEEDED(hr) && spComment)
								{
									hr=spComment->Delete();
									if(FAILED(hr))
										bRet=FALSE;
								}
							}
						}
					}
				}
			}
		}
	}
	return bRet;
}
BOOL	CODHDPptInspector::RemoveWebPublish(PPT::_Presentation*pDoc)
{
	BOOL	bRet=TRUE;
	HRESULT             hr = S_OK;
    CComPtr<PPT::PublishObjects> pPublishObjs = NULL;
	CComPtr<PPT::pptPublishObject> pPublishObj=NULL;

    hr = pDoc->get_PublishObjects(&pPublishObjs);
    if(SUCCEEDED(hr) && pPublishObjs)
    {
		hr=pPublishObjs->Item(1,&pPublishObj);
		if(SUCCEEDED(hr)&&pPublishObj)
		{
			hr=pPublishObj->put_RangeStart(0);
			hr=pPublishObj->put_RangeEnd(0);
		}
		
    }
	return bRet;
}
BOOL	CODHDPptInspector::RemoveCategoryDocProp(PPT::_Presentation*pDoc)
{
	HRESULT	hr=S_OK;
	BOOL	bRet=TRUE;
	CComPtr<IDispatch>  lpDocumentProps = NULL;
	CComPtr<IDispatch>  lpCustomProps   = NULL;
	hr = pDoc->get_BuiltInDocumentProperties(&lpDocumentProps);
	if(SUCCEEDED(hr) && lpDocumentProps)
	{
		CODHDUtilities::RemoveDocumentProperties(lpDocumentProps,gs_PropertiesNames_Ppt,sizeof(gs_PropertiesNames_Ppt)/sizeof(LPCWSTR));
	}
	hr = pDoc->get_CustomDocumentProperties(&lpCustomProps);
	if(SUCCEEDED(hr) && lpCustomProps)
	{
		CODHDUtilities::RemoveCustomProperties(lpCustomProps);
	}
	
	//hr=pDoc->put_RemoveDateAndTime(removeTrue);
	hr=pDoc->put_RemovePersonalInformation(msoCTrue);
	bRet=RemoveWebPublish(pDoc);
	return bRet;
}

BOOL CODHDPptInspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)
{
	BOOL	bRet=FALSE;
	HRESULT	hr=S_OK;

	CComPtr<PPT::Presentations>	pDocuments=NULL;
	CComPtr<PPT::_Presentation>	pDoc   = NULL;
	//VARIANT_BOOL		varVisible = FALSE;

	assert(m_pApp);
#pragma warning(push)
#pragma warning(disable: 6011)
	hr = m_pApp->get_Presentations(&pDocuments);
#pragma warning(pop)
	if(SUCCEEDED(hr)&&pDocuments)
	{
		assert(m_strTempFileName.length());

		CComBSTR    FileName(m_strTempFileName.c_str());
        CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
        hr = pDocuments->Open( FileName,       // FileName
								msoFalse,    // ReadOnly
								msoFalse,    // Untitled
								msoFalse,    // WithWindow
								&pDoc);

		m_pApp->put_Visible(msoFalse);
		m_pApp->put_ShowWindowsInTaskbar(msoFalse);
		if(SUCCEEDED(hr)&&pDoc)
		{
			BOOL bHasPassword=FALSE;
            if(bHasPassword==FALSE)
			{
                switch(odhdCat)
				{
				case ODHDCAT_PPT_COMMENTS:
					bRet=RemoveCategoryComments(pDoc);
					if (bRet && m_pHDRFile != NULL)
					{
						m_pHDRFile->m_dwRemovedCatType |= PptHDCat_COMMENTS;
					}
					break;
				case ODHDCAT_PPT_PROP:
					bRet=RemoveCategoryDocProp(pDoc);
					if (bRet && m_pHDRFile != NULL)
					{
						m_pHDRFile->m_dwRemovedCatType |= PptHDCat_PROP;
					}
					break;
				/*case ODHDCAT_PPT_ONSLIDE:
					bRet=RemoveCategoryHeaderFooter(pDoc);
					break;
				case ODHDCAT_PPT_OFFSLIDE:
					bRet=RemoveCategoryHiddenText();
					break;
				case ODHDCAT_PPT_XML:
					break;
				case ODHDCAT_PPT_NOTE:
					break;
					*/
				default:
					DP((L"Error Parameter for Hidden Data Category \n"));
					break;
				}
				if(TRUE==bRet)
					SetCatStatus(odhdCat,INSPECT_REMOVED);
				else
					SetCatStatus(odhdCat,INSPECT_FAILED);
				SaveDocAfterRemove(pDoc);
				

			}
			else
			{
				DP((L"There is a password!\n"));
			}
		}
	}
	else
    {
        DP((L"Fail to get documents obj!\n"));
    }
	return bRet;
}

BOOL CODHDPptInspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)
{
	BOOL bFiltered = FALSE;

	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_PPT_COMMENTS && !(dwNeedCat & PptHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_PPT_PROP && !(dwNeedCat & PptHDCat_PROP)))
		{
			bFiltered = TRUE;
		}

		if (!bFiltered && odhdStatus == INSPECT_HAVE)
		{
			switch (odhdCat)
			{
			case ODHDCAT_PPT_COMMENTS:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_COMMENTS;
				break;
			case ODHDCAT_PPT_PROP:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_PROP;
				break;
			default:
				break;
			}
		}
	}

	return bFiltered;
}

BOOL CODHDPptInspector::GetFilter(ODHDCATEGORY odhdCat)
{
	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_PPT_COMMENTS && !(dwNeedCat & PptHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_PPT_PROP && !(dwNeedCat & PptHDCat_PROP)))
		{
			return TRUE;
		}
	}

	return FALSE;
}
