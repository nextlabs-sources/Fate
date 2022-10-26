#include "stdafx.h"
#include "odhd_ppt2k7.h"
#ifndef WSO2K3


static LPCWSTR gs_PropertiesNames_Ppt2K7[]={
		{L"Title"},
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
};

extern std::wstring g_wstrCantOpenFile;

BOOL CODHDPpt2K7Inspector::IsValidPropertyName(LPCWSTR strPropName)
{
	int iIndex=0;
	
	for(iIndex=0;iIndex<sizeof(gs_PropertiesNames_Ppt2K7)/sizeof(LPCWSTR);iIndex++)
	{
		if(0 == _wcsicmp(strPropName, gs_PropertiesNames_Ppt2K7[iIndex]))
			return TRUE;
	}
	return FALSE;
}


ODHDSTATUS	CODHDPpt2K7Inspector::InspectComments(PPT::_Presentation *pDoc)
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
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_COMMENTS].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_COMMENTS].m_itemFound);
	return retStatus;
}
ODHDSTATUS CODHDPpt2K7Inspector::InspectPresentationNote(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
	{
		if(InspectorExist(spInspectors,4,_T("Presentation Notes")))
		{
			m_InspectorIndex4=4;
			retStatus= InspectByInspector(spInspectors,ODHDCAT_PPT_NOTE,ODHDITEM_PPT_PRESENTNOTE,&m_InspectorFail4);
		}
		else
		{
			if(InspectorExist(spInspectors,3,_T("Presentation Notes")))
			{
				m_InspectorIndex4=3;
				retStatus= InspectByInspector(spInspectors,3,ODHDCAT_PPT_NOTE,ODHDITEM_PPT_PRESENTNOTE,&m_InspectorFail4);
			}
			else
			{
				if(InspectorExist(spInspectors,2,_T("Presentation Notes")))
				{
					m_InspectorIndex4=2;
					retStatus= InspectByInspector(spInspectors,2,ODHDCAT_PPT_NOTE,ODHDITEM_PPT_PRESENTNOTE,&m_InspectorFail4);
				}
				else
				{
					if(InspectorExist(spInspectors,1,_T("Presentation Notes")))
					{
						m_InspectorIndex4=1;
						retStatus= InspectByInspector(spInspectors,1,ODHDCAT_PPT_NOTE,ODHDITEM_PPT_PRESENTNOTE,&m_InspectorFail4);
					}
					else
					{
						m_InspectorIndex4=0;
						retStatus=INSPECT_NONE;
					}
				}
					
			}
		}
	}
	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectBuiltinProperty(PPT::_Presentation *pDoc)
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
ODHDSTATUS	CODHDPpt2K7Inspector::InspectCustomProperty(PPT::_Presentation *pDoc)
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
ODHDSTATUS	CODHDPpt2K7Inspector::InspectDocProperty(PPT::_Presentation *pDoc)
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
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_DOCPROP].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_DOCPROP].m_itemFound);
		retStatus= INSPECT_NONE;
	}
	return retStatus;
}

ODHDSTATUS	CODHDPpt2K7Inspector::InspectDocSrvProp(PPT::_Presentation *pDoc)
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
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_SRVPROP].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_SRVPROP].m_itemFound);

	}
	return retStatus;
}
ODHDSTATUS CODHDPpt2K7Inspector::InspectEmailHeader(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;

	return retStatus;
}
ODHDSTATUS CODHDPpt2K7Inspector::InspectRoutingSlip(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;

	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectSend4Review(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	

	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectOnSlideInvisible(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
	{
		if(InspectorExist(spInspectors,2,_T("Invisible On-Slide Content")))
		{
			m_InspectorIndex2=2;
			retStatus= InspectByInspector(spInspectors,ODHDCAT_PPT_ONSLIDE,ODHDITEM_PPT_INVIOBJECT,&m_InspectorFail2);
		}
		else
		{
			if(InspectorExist(spInspectors,1,_T("Invisible On-Slide Content")))
			{
				m_InspectorIndex2=1;
				retStatus= InspectByInspector(spInspectors,1,ODHDCAT_PPT_ONSLIDE,ODHDITEM_PPT_INVIOBJECT,&m_InspectorFail2);
			}
			else
			{
				m_InspectorIndex2=0;
				retStatus=INSPECT_NONE;
			}
		}
	}
	return retStatus;
}

ODHDSTATUS	CODHDPpt2K7Inspector::InspectCustomXML(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
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
			retStatus= InspectByInspector(spInspectors,ODHDCAT_PPT_XML,ODHDITEM_PPT_CUSTOMXML,&m_InspectorFail1);
		}
		else
		{
			m_InspectorIndex1=0;
			retStatus=INSPECT_NONE;
		}
		
	}
	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectOffSlide(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	 retStatus=INSPECT_NONE;
	HRESULT			hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
	{
		if(InspectorExist(spInspectors,3,_T("Off-Slide Content")))
		{
			m_InspectorIndex3=3;
			retStatus= InspectByInspector(spInspectors,ODHDCAT_PPT_OFFSLIDE,ODHDITEM_PPT_OFFSLIDE,&m_InspectorFail3);
		}
		else
		{
			if(InspectorExist(spInspectors,2,_T("Off-Slide Content")))
			{
				m_InspectorIndex3=2;
				retStatus= InspectByInspector(spInspectors,2,ODHDCAT_PPT_OFFSLIDE,ODHDITEM_PPT_OFFSLIDE,&m_InspectorFail3);
			}
			else
			{
				if(InspectorExist(spInspectors,1,_T("Off-Slide Content")))
				{
					m_InspectorIndex3=1;
					retStatus= InspectByInspector(spInspectors,1,ODHDCAT_PPT_OFFSLIDE,ODHDITEM_PPT_OFFSLIDE,&m_InspectorFail3);
				}
				else
				{
					m_InspectorIndex3=0;
					retStatus=INSPECT_NONE;
				}
			}
		}
	}
	
	
	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectWebPublish(PPT::_Presentation *pDoc)
{
	ODHDSTATUS	retStatus=INSPECT_NONE;
	HRESULT             hr = S_OK;
    CComPtr<PPT::PublishObjects> spPublishObjs = NULL;


    hr = pDoc->get_PublishObjects(&spPublishObjs);
    if(SUCCEEDED(hr) && spPublishObjs)
    {
        VARIANT vtConnCount;
        CComPtr<IDispatch> spDispatch = spPublishObjs;
        hr = DispatchCallWraper(spDispatch,DISPATCH_METHOD|DISPATCH_PROPERTYGET, &vtConnCount,  L"Count", 0);
        if(SUCCEEDED(hr) && vtConnCount.lVal>0)
		{
			CComPtr<PPT::pptPublishObject> pPublishObj=NULL;
			hr=spPublishObjs->Item(1,&pPublishObj);
			if(SUCCEEDED(hr)&&pPublishObj)
			{
				int lStart=0,lEnd=0;
				PPT::PpPublishSourceType sourceType;
				CComBSTR	fileName;
				hr=pPublishObj->get_FileName(&fileName);
				hr=pPublishObj->get_SourceType(&sourceType);
				hr=pPublishObj->get_RangeStart(&lStart);
				hr=pPublishObj->get_RangeEnd(&lEnd);
				if(lStart!=lEnd)
				{
					retStatus=INSPECT_HAVE;
					RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_WEBPUBLISH,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_WEBPUBLISH].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_WEBPUBLISH].m_itemFound);
				}
			}
		}
    }

	return retStatus;
}
ODHDSTATUS	CODHDPpt2K7Inspector::InspectContentType(PPT::_Presentation *pDoc)
{
	HRESULT			hr   = S_OK;
	ODHDSTATUS	 retStatus=INSPECT_NONE;

	CComPtr<Office::MetaProperties> spMetaProperties = 0;
	hr = pDoc->get_ContentTypeProperties(&spMetaProperties);
	if(SUCCEEDED(hr) && spMetaProperties)
	{
		retStatus=INSPECT_HAVE;
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_CONTENTTYPE,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_CONTENTTYPE].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_CONTENTTYPE].m_itemFound);
		m_HasContentType=TRUE;
	}

	return retStatus;
}

ODHDSTATUS	CODHDPpt2K7Inspector::InspectSrvPolicy(PPT::_Presentation *pDoc)
{
	HRESULT			hr   = S_OK;
	ODHDSTATUS	 retStatus=INSPECT_NONE;

	CComPtr<Office::ServerPolicy> spServerPolicy = 0;
	hr = pDoc->get_ServerPolicy(&spServerPolicy);
	if(SUCCEEDED(hr) && spServerPolicy)
	{
		retStatus=INSPECT_HAVE;
		RecordInspectResult(ODHDCAT_PPT_PROP,
							ODHDITEM_PPT_MANAGEPOLICY,
							INSPECT_HAVE,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_MANAGEPOLICY].m_itemTitle,
							gs_ItemTitle_Ppt2K7[ODHDITEM_PPT_MANAGEPOLICY].m_itemFound);
	}

	return retStatus;
}
BOOL CODHDPpt2K7Inspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)
{
	BOOL    bRet = TRUE;
    HRESULT hr   = S_OK;
	int		nStep=MAX_HDRDLG_PROGRESS;

	m_strTempFileName=strTempFileName;
	m_strSrcFileName =strSrcFileName;
	
	CComPtr<PPT::Presentations> spDocuments  = NULL;

    hr = m_pApp->get_Presentations(&spDocuments);

	if(SUCCEEDED(hr) && spDocuments)
    {
		CComPtr<PPT::_Presentation>	spDocument   = NULL;
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
			//InspectEmailHeader(spDocument);
			//InspectRoutingSlip(spDocument);
			MakeStep(2);nStep-=2;
			//InspectSend4Review(spDocument);
			InspectDocSrvProp(spDocument);
			InspectSrvPolicy(spDocument);
			MakeStep(2);nStep-=2;
			InspectContentType(spDocument);
			//InspectWebPublish(spDocument);
			
			SetProgTitle(ODHDCAT_PPT_ONSLIDE);
			InspectOnSlideInvisible(spDocument);
			MakeStep(1);nStep-=1;

			SetProgTitle(ODHDCAT_PPT_OFFSLIDE);
			InspectOffSlide(spDocument);
			MakeStep(1);nStep-=1;

			SetProgTitle(ODHDCAT_PPT_NOTE);
			InspectPresentationNote(spDocument);
			InspectCustomXML(spDocument);
			MakeStep(2);nStep-=2;

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
        spDocuments = NULL;
    }
	if(nStep > 0)
		MakeStep(nStep);
	return bRet;


}

BOOL	CODHDPpt2K7Inspector::SaveDocAfterRemove(PPT::_Presentation*pDoc)
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
BOOL CODHDPpt2K7Inspector::RemoveCategoryComments(PPT::_Presentation*pDoc)
{
	HRESULT hr=S_OK;
	BOOL	bRet=TRUE;
	hr=pDoc->RemoveDocumentInformation(ppRDIComments);if(FAILED(hr))bRet=FALSE;
	hr=pDoc->RemoveDocumentInformation(ppRDIInkAnnotations);if(FAILED(hr))bRet=FALSE;
	return bRet;
}

BOOL	CODHDPpt2K7Inspector::RemoveCategoryDocProp(PPT::_Presentation*pDoc)
{
	HRESULT	hr=S_OK;
	BOOL	bRet=TRUE;
	hr=pDoc->RemoveDocumentInformation(ppRDIDocumentProperties);if(FAILED(hr))bRet=FALSE;
	hr=pDoc->RemoveDocumentInformation(ppRDIRemovePersonalInformation);if(FAILED(hr))bRet=FALSE;
	if(m_HasContentType==TRUE)
	{
		bRet=RemoveCategoryCustomXML(pDoc);
		hr=pDoc->RemoveDocumentInformation(ppRDIContentType);if(FAILED(hr))bRet=FALSE;
	}
	
	hr=pDoc->RemoveDocumentInformation(ppRDIDocumentManagementPolicy);if(FAILED(hr))bRet=FALSE;
	
	return bRet;
}
BOOL	CODHDPpt2K7Inspector::RemoveCategoryOnSlide(PPT::_Presentation*pDoc)
{
	BOOL		bRet=FALSE;
	HRESULT		hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	if(m_InspectorIndex2==0)
		return TRUE;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
		return RemoveByInspector(spInspectors,m_InspectorIndex2,ODHDCAT_PPT_ONSLIDE,ODHDITEM_PPT_INVIOBJECT);
	return bRet;
}
BOOL	CODHDPpt2K7Inspector::RemoveCategoryOffSlide(PPT::_Presentation*pDoc)
{
	BOOL		bRet=FALSE;
	HRESULT		hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	if(m_InspectorIndex3==0)
		return TRUE;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
		return RemoveByInspector(spInspectors,m_InspectorIndex3,ODHDCAT_PPT_OFFSLIDE,ODHDITEM_PPT_OFFSLIDE);
	return bRet;
}
BOOL	CODHDPpt2K7Inspector::RemoveCategoryCustomXML(PPT::_Presentation*pDoc)
{
	BOOL		bRet=FALSE;
	HRESULT		hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	if(m_InspectorIndex1==0)
		return TRUE;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
		return RemoveByInspector(spInspectors,m_InspectorIndex1,ODHDCAT_PPT_XML,ODHDITEM_PPT_CUSTOMXML);
	return bRet;
}
BOOL	CODHDPpt2K7Inspector::RemoveCategoryNote(PPT::_Presentation*pDoc)
{
	BOOL		bRet=FALSE;
	HRESULT		hr   = S_OK;
	CComPtr<Office::DocumentInspectors> spInspectors=NULL;
	if(m_InspectorIndex4==0)
		return TRUE;
	hr = pDoc->get_DocumentInspectors(&spInspectors);
	if(SUCCEEDED(hr) && spInspectors)
		return RemoveByInspector(spInspectors,m_InspectorIndex4,ODHDCAT_PPT_NOTE,ODHDITEM_PPT_PRESENTNOTE);
	return bRet;
}

BOOL CODHDPpt2K7Inspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)
{
	BOOL	bRet=FALSE;
	HRESULT	hr=S_OK;

	CComPtr<PPT::Presentations>	pDocuments=NULL;
	
	assert(m_pApp);
#pragma warning(push)
#pragma warning(disable: 6011)
	hr = m_pApp->get_Presentations(&pDocuments);
#pragma warning(pop)
	if(SUCCEEDED(hr)&&pDocuments)
	{
		assert(m_strTempFileName.length());
		CComPtr<PPT::_Presentation>	pDoc   = NULL;
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
			BOOL bReadonly = FALSE ;
			Office::MsoTriState msoTriState = msoFalse;
			hr = pDoc->get_ReadOnly(&msoTriState);
			if(SUCCEEDED(hr) && msoTrue==msoTriState)
			{
				bReadonly = TRUE;
				bRet = FALSE ;
			}

			BOOL bHasPassword=FALSE;
            if(bHasPassword==FALSE)
			{
				if( bReadonly == FALSE )
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
					case ODHDCAT_PPT_ONSLIDE:
						bRet=RemoveCategoryOnSlide(pDoc);
						if (bRet && m_pHDRFile != NULL)
						{
							m_pHDRFile->m_dwRemovedCatType |= PptHDCat_ONSLIDE;
						}
						break;
					case ODHDCAT_PPT_OFFSLIDE:
						bRet=RemoveCategoryOffSlide(pDoc);
						if (bRet && m_pHDRFile != NULL)
						{
							m_pHDRFile->m_dwRemovedCatType |= PptHDCat_OFFSLIDE;
						}
						break;
					case ODHDCAT_PPT_XML:
						bRet=RemoveCategoryCustomXML(pDoc);
						if (bRet && m_pHDRFile != NULL)
						{
							m_pHDRFile->m_dwRemovedCatType |= PptHDCat_XML;
						}
						break;
					case ODHDCAT_PPT_NOTE:
						bRet=RemoveCategoryNote(pDoc);
						if (bRet && m_pHDRFile != NULL)
						{
							m_pHDRFile->m_dwRemovedCatType |= PptHDCat_NOTE;
						}
						break;
					default:
						DP((L"Error Parameter for Hidden Data Category \n"));
						break;
					}
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

void CODHDPpt2K7Inspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)
{
	switch(odhdCat)
	{
	case ODHDCAT_PPT_XML:
		if(m_InspectorFail1==1)
		{
			strResult=L"This document was saved in previous version of Microsoft Office.To inspect this document, you must save it first.";
			return;
		}
	case ODHDCAT_PPT_ONSLIDE:
		if(m_InspectorFail2==1)
		{
			strResult=L"This document was saved in previous version of Microsoft Office.To inspect this document, you must save it first.";
			return;
		}
	case ODHDCAT_PPT_OFFSLIDE:
		if(m_InspectorFail3==1)
		{
			strResult=L"This document was saved in previous version of Microsoft Office.To inspect this document, you must save it first.";
			return;
		}
	case ODHDCAT_PPT_NOTE:
		if(m_InspectorFail4==1)
		{
			strResult=L"This document was saved in previous version of Microsoft Office.To inspect this document, you must save it first.";
			return;
		}
	default:
		CODHDInspector::GetResult(odhdCat,strResult);
		return;
	}
	return;
}

BOOL CODHDPpt2K7Inspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)
{
	BOOL bFiltered = FALSE;

	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_PPT_COMMENTS && !(dwNeedCat & PptHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_PPT_PROP && !(dwNeedCat & PptHDCat_PROP))
			|| (odhdCat == ODHDCAT_PPT_XML && !(dwNeedCat & PptHDCat_XML))
			|| (odhdCat == ODHDCAT_PPT_ONSLIDE && !(dwNeedCat & PptHDCat_ONSLIDE))
			|| (odhdCat == ODHDCAT_PPT_OFFSLIDE && !(dwNeedCat & PptHDCat_OFFSLIDE))
			|| (odhdCat == ODHDCAT_PPT_NOTE && !(dwNeedCat & PptHDCat_NOTE)))
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
			case ODHDCAT_PPT_XML:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_XML;
				break;
			case ODHDCAT_PPT_ONSLIDE:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_ONSLIDE;
				break;
			case ODHDCAT_PPT_OFFSLIDE:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_OFFSLIDE;
				break;
			case ODHDCAT_PPT_NOTE:
				m_pHDRFile->m_dwFoundCatType |= PptHDCat_NOTE;
				break;
			default:
				break;
			}
		}
	}

	return bFiltered;
}

BOOL CODHDPpt2K7Inspector::GetFilter(ODHDCATEGORY odhdCat)
{
	if (m_pHDRFile != NULL)
	{
		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;
		if (dwNeedCat == 0
			|| (odhdCat == ODHDCAT_PPT_COMMENTS && !(dwNeedCat & PptHDCat_COMMENTS))
			|| (odhdCat == ODHDCAT_PPT_PROP && !(dwNeedCat & PptHDCat_PROP))
			|| (odhdCat == ODHDCAT_PPT_XML && !(dwNeedCat & PptHDCat_XML))
			|| (odhdCat == ODHDCAT_PPT_ONSLIDE && !(dwNeedCat & PptHDCat_ONSLIDE))
			|| (odhdCat == ODHDCAT_PPT_OFFSLIDE && !(dwNeedCat & PptHDCat_OFFSLIDE))
			|| (odhdCat == ODHDCAT_PPT_NOTE && !(dwNeedCat & PptHDCat_NOTE)))
		{
			return TRUE;
		}
	}

	return FALSE;
}

#endif //WSO2K7
