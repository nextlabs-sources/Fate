#include "stdafx.h"

#include "odhd_word.h"

static LPCWSTR gs_PropertiesNames_Word[]={

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



extern std::wstring g_wstrNeedOfficeSP3;

extern std::wstring g_wstrCantOpenFile;



BOOL CODHDWordInspector::RemoveCategoryComments(Word::_Document*pDoc)

{

	BOOL			bRet=TRUE;

	CComPtr<Word::Versions> spVersions = NULL;

	CComPtr<Word::Version>  spVersion  = NULL;

	long            lCount     = 0;

	long            iIndex     = 0;

	HRESULT			hr		   =S_OK;

	hr=pDoc->DeleteAllComments();

	hr=pDoc->DeleteAllInkAnnotations();



	hr=pDoc->AcceptAllRevisions();



	hr=pDoc->put_TrackRevisions(FALSE);

	hr = pDoc->get_Versions(&spVersions);

	if(SUCCEEDED(hr) && spVersions)

	{

		hr=spVersions->put_AutoVersion(wdAutoVersionOff);

		if(SUCCEEDED(hr))

		{

			hr = spVersions->get_Count(&lCount);

			if(SUCCEEDED(hr) && lCount)

			{

				for(iIndex=lCount; iIndex>0; iIndex--)

				{

					hr = spVersions->Item(iIndex, &spVersion);

					if(SUCCEEDED(hr) && spVersion)

					{

						spVersion->Delete();

					}

				}

			}

		}

		else

			bRet=FALSE;

	}

	return bRet;

}



BOOL	CODHDWordInspector::SaveDocAfterRemove(Word::_Document*pDoc)

{

	BOOL    bRet = FALSE;

	

	CComVariant varFalse((short)FALSE);

	CComVariant varSaveChanges(0);  // WdSaveOptions::wdDoNotSaveChanges

	CComVariant varOrigFmt(1);      // WdOriginalFormat::wdOriginalDocumentFormat

	CComVariant varDontSaveChanges(wdDoNotSaveChanges);  // WdSaveOptions::wdDoNotSaveChanges

	DWORD       dwOldMicroSecurityLevel = 0x3;

	VARIANT_BOOL    bReadonly  = FALSE;

	HRESULT		hr=S_OK;



	hr = pDoc->get_ReadOnly(&bReadonly);

	if(FAILED(hr))

		bReadonly = FALSE;

	if(bReadonly||CODHDUtilities::IsWordTemplateFile(m_strTempFileName.c_str()))

	{

		std::wstring strTempSaveAsFile = m_strTempFileName;

		strTempSaveAsFile.append(L".SaveAsTemp");

		CComVariant varTempSaveAsFile(strTempSaveAsFile.c_str());
		_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
		hr = pDoc->SaveAs(&varTempSaveAsFile);

		bRet = SUCCEEDED(hr);

		DP((L"SaveAs Word file %s\n", bRet?L"OK":L"Fail"));

		hr = pDoc->Close(&varSaveChanges, &varOrigFmt, &varFalse);

		//spDocuments->Close(&varSaveChanges, &varOrigFmt, &varFalse);

		if(bRet)

		{

			::DeleteFileW(m_strTempFileName.c_str());

			::MoveFileW(strTempSaveAsFile.c_str(), m_strTempFileName.c_str());

		}

	}

	else

	{

		DP((L"Before SaveAs Word file\n"));
		_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
		hr = pDoc->Save();

		bRet = SUCCEEDED(hr);

		DP((L"Save Word file %s\n", bRet?L"OK":L"Fail"));

		hr = pDoc->Close(&varDontSaveChanges, &varOrigFmt, &varFalse);

		//spDocuments->Close(&varSaveChanges, &varOrigFmt, &varFalse);

	}
	return bRet;
}

BOOL CODHDWordInspector::RemoveEmailHeader(Word::_Document* pDoc)

{

	BOOL	bRet=TRUE;



	HRESULT			hr   = S_OK;

	CComPtr<Office::IMsoEnvelopeVB>		pEmailHeader=NULL;

	CComPtr<IDispatch>	pMailItem=NULL;

	BSTR			bstrIntrodution=NULL;

	VARIANT			varResult;

	hr=pDoc->get_MailEnvelope(&pEmailHeader);

	if(SUCCEEDED(hr)&&pEmailHeader)

	{

		BSTR bstrIntro = ::SysAllocString(L"introduce from inspect");

		hr=pEmailHeader->put_Introduction(bstrIntro);

		::SysFreeString(bstrIntro);

		hr=pEmailHeader->get_Item(&pMailItem);



		if(SUCCEEDED(hr)&&pMailItem)

		{

			hr=DispatchCallWraper(pMailItem,DISPATCH_METHOD|DISPATCH_PROPERTYGET,&varResult,L"Subject",0);

			::SysFreeString( bstrIntrodution );



		}

	}



	return bRet;

}

BOOL CODHDWordInspector::RemoveDocSvrProp(Word::_Document* pDoc)

{

	BOOL bRet=TRUE;

	HRESULT			hr   = S_OK;

	ODHDSTATUS	 retStatus=INSPECT_NONE;



	CComPtr<Office::DocumentLibraryVersions> pDocLibVersions = NULL;

	CComPtr<Office::DocumentLibraryVersion> pDocLibVersion=NULL;

	hr = pDoc->get_DocumentLibraryVersions(&pDocLibVersions);

	if(SUCCEEDED(hr) && pDocLibVersion)

	{

		long iIndex=0,nCount=0;

		hr=pDocLibVersions->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			for(iIndex=0;iIndex<nCount;iIndex++)

			{

				hr=pDocLibVersions->get_Item(iIndex,&pDocLibVersion);

				if(SUCCEEDED(hr)&&pDocLibVersion)

				{

					hr=pDocLibVersion->Delete();

					if(FAILED(hr))

						bRet=FALSE;

				}

			}

		}

	}



	return bRet;

}

BOOL	CODHDWordInspector::RemoveCategoryDocProp(Word::_Document*pDoc)

{

	BOOL	bRet=FALSE;

	BOOL	bDocSvrRet=FALSE,bEmailHeaderRet=TRUE;

	HRESULT	hr=S_OK;

	CComPtr<IDispatch>  lpDocumentProps = NULL;

	CComPtr<IDispatch>  lpCustomProps   = NULL;

	hr = pDoc->get_BuiltInDocumentProperties(&lpDocumentProps);

	if(SUCCEEDED(hr) && lpDocumentProps)

	{

		CODHDUtilities::RemoveDocumentProperties(lpDocumentProps,gs_PropertiesNames_Word,sizeof(gs_PropertiesNames_Word)/sizeof(LPCWSTR));

	}

	hr = pDoc->get_CustomDocumentProperties(&lpCustomProps);

	if(SUCCEEDED(hr) && lpCustomProps)

	{

		CODHDUtilities::RemoveCustomProperties(lpCustomProps);

	}

	

	// Remove template

	CComVariant varTemplate(L"Normal.dot");

	pDoc->put_AttachedTemplate(&varTemplate);

	VARIANT_BOOL	removeTrue=-1;

	hr=pDoc->put_RemoveDateAndTime(removeTrue);

	hr=pDoc->put_RemovePersonalInformation(removeTrue);

	

	VARIANT_BOOL bHasRoutingSlip = FALSE;

	hr = pDoc->get_HasRoutingSlip(&bHasRoutingSlip);

	if(SUCCEEDED(hr) && bHasRoutingSlip)

	{

		pDoc->put_HasRoutingSlip(FALSE);

	}

	bRet=TRUE;

	bDocSvrRet=RemoveDocSvrProp(pDoc);

	bEmailHeaderRet=RemoveEmailHeader(pDoc);

	if(bDocSvrRet==FALSE||bEmailHeaderRet==FALSE)

		bRet=FALSE;

	/*{

		BSTR bstrUserName=::SysAllocString(L"");

		

		hr=m_pApp->put_UserName(bstrUserName);

		hr=m_pApp->get_UserInitials(&bstrUserName);

		bstrUserName=::SysAllocString(L"");

		hr=m_pApp->put_UserInitials(bstrUserName);

		hr=m_pApp->get_UserInitials(&bstrUserName);



		if(FAILED(hr))

			bRet=FALSE;

		::SysFreeString(bstrUserName);

	}*/

	

	return bRet;

}

BOOL CODHDWordInspector::RemoveOneHeaderFooter(Word::wordHeaderFooter*pHeaderFooter,Word::_Document* pDoc)

{

	BOOL		bRet=TRUE;

	HRESULT		hr   = S_OK;

	CComPtr<Word::Range> pRange=NULL;

	long	lStart=0,lEnd=0;

	BSTR	bstrNull=::SysAllocString(L"");

	hr=pHeaderFooter->get_Range(&pRange);

	if(SUCCEEDED(hr)&&pRange)

	{

		hr=pRange->put_Text(bstrNull);

		if(FAILED(hr))

		{

			DP((L"put HeaderFooter text failed:%d\n",hr));

			BSTR bstrVerify=NULL;

			hr=pRange->get_Text(&bstrVerify);

			if(FAILED(hr)||bstrVerify!=NULL)

				bRet=FALSE;

			::SysFreeString(bstrVerify);

		}

	}

	::SysFreeString(bstrNull);

	return bRet;

}

BOOL CODHDWordInspector::RemoveHeadersFooters(Word::wordHeadersFooters*pHeadersFooters,Word::_Document* pDoc)

{

	BOOL		bRet=TRUE,bRevision=FALSE;

	HRESULT		hr   = S_OK;

	VARIANT_BOOL	bExists=FALSE;

	VARIANT_BOOL	oldTrackRevision,noTrackRevision=0;

	CComPtr<Word::wordHeaderFooter> pHeaderFooter=NULL;

	long nCount=0;

	WdHeaderFooterIndex indexArr[]={wdHeaderFooterPrimary,wdHeaderFooterFirstPage,wdHeaderFooterEvenPages};

	int index=0;

	hr=pDoc->get_TrackRevisions(&oldTrackRevision);

	if(SUCCEEDED(hr))

	{

		bRevision=TRUE;

		hr=pDoc->put_TrackRevisions(noTrackRevision);

		if(FAILED(hr)&&oldTrackRevision)

		{

			return FALSE;

		}

	}



	for(;index<sizeof(indexArr)/sizeof(WdHeaderFooterIndex);index++)

	{

		hr=pHeadersFooters->Item(indexArr[index],&pHeaderFooter);

		if(SUCCEEDED(hr)&&pHeaderFooter)

		{

			hr=pHeaderFooter->get_Exists(&bExists);

			if(SUCCEEDED(hr)&&bExists)

			{

				if(RemoveOneHeaderFooter(pHeaderFooter,pDoc)==FALSE)

				{

					DP((L"===RemoveOneHeaderFooter Fail:index %d\n",index));

					bRet= FALSE;

				}

			}

		}

	}

	pDoc->put_TrackRevisions(oldTrackRevision);

	return bRet;

}

BOOL	CODHDWordInspector::RemoveCategoryHeaderFooter(Word::_Document*pDoc)

{

	BOOL		bRet=FALSE,bHeaderRet=FALSE,bFooterRet=FALSE;

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	CComPtr<Word::Sections>	pSections=NULL;

	CComPtr<Word::Section>	pSection=NULL;

	CComPtr<Word::wordHeadersFooters>	pHeaders=NULL,pFooters=NULL;

	long			nCount=0,iIndex=0;

	long	nHeaderCount=0,nFooterCount=0;

	long	iIndexHeader=0,iIndexFooter=0;



	// Check Word 2003 build version

	BSTR bstrBuildVersion = NULL;

	m_pApp->get_Build(&bstrBuildVersion);

	if (bstrBuildVersion != NULL)

	{

		LPCWSTR wstrBuild = wcsrchr(bstrBuildVersion, L'.');

		if (wstrBuild != NULL)

		{

			int iBuild = _wtoi(wstrBuild+1);

			if (iBuild < 6568) // Office 2003 SP2 version

			{

				// Does not support to remove water marker for Office2003 below SP2 version

				std::wstring wstrMsg = L"";

				CODHDUtilities::ReplaceFileName(g_wstrNeedOfficeSP3, m_strSrcFileName, wstrMsg);



				SysFreeString(bstrBuildVersion);

				bstrBuildVersion = NULL;

				MessageBoxW(GetForegroundWindow(), wstrMsg.c_str(), L"Hidden Data Removal", MB_OK | MB_ICONWARNING);



				return FALSE;

			}

		}



		SysFreeString(bstrBuildVersion);

		bstrBuildVersion = NULL;

	}

	

	hr=pDoc->get_Sections(&pSections);

	if(SUCCEEDED(hr)&&pSections)

	{

		hr=pSections->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			for(;iIndex<nCount;iIndex++)

			{

				nHeaderCount=0;nFooterCount=0;

				hr=pSections->Item(iIndex+1,&pSection);

				if(SUCCEEDED(hr)&&pSection)

				{

					//Header

					hr=pSection->get_Headers(&pHeaders);

					if(SUCCEEDED(hr)&&pHeaders)

					{

						bHeaderRet=RemoveHeadersFooters(pHeaders,pDoc);

					}



					//Footer

					hr=pSection->get_Footers(&pFooters);

					if(SUCCEEDED(hr)&&pFooters)

					{

						bFooterRet=RemoveHeadersFooters(pFooters,pDoc);

					}


				}//one section

			}//for sections

		}

	}

	DP((L"===Remove Header:%d\n",bHeaderRet));

	DP((L"===Remove Footer:%d\n",bFooterRet));

	if(bHeaderRet==TRUE&&bFooterRet==TRUE)

		bRet=TRUE;

	return bRet;

}


BOOL	CODHDWordInspector::RemoveCategoryHiddenText(Word::_Document*pDoc)

{



	HRESULT hr=S_OK;

	BOOL bRet=TRUE;

	CComPtr<Word::Range>pDocRange=NULL;

	CComPtr<Word::Find> pFind=NULL;

	CComPtr<Word::_Font> pFindFont=NULL;

	CComPtr<Word::TextRetrievalMode> pMode=NULL;

	hr=pDoc->get_Content(&pDocRange);

	if(FAILED(hr)||pDocRange==NULL)

		return FALSE;

	hr=pDocRange->get_Find(&pFind);

	if(FAILED(hr)||pFind==NULL)

		bRet=FALSE;

	else

	{

		hr=pDocRange->get_TextRetrievalMode(&pMode);

		if(FAILED(hr)||pMode==NULL)

		{

			return FALSE;

		}

		hr=pMode->put_IncludeHiddenText(TRUE);

		if(FAILED(hr))

		{

			return FALSE;

		}

		pFind->ClearFormatting();

		CComVariant wordFindText=L"";

		CComVariant varReplaceWith=L"";

		CComVariant varTrue=TRUE,varFalse=FALSE;

		CComVariant varWrap=wdFindContinue;

		CComVariant varReplace=wdReplaceAll;

		CComVariant varOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

		VARIANT_BOOL varBool=FALSE;

		hr=pFind->get_wordFont(&pFindFont);

		if(FAILED(hr)||pFindFont==NULL)

		{

			return FALSE;

		}

		hr=pFindFont->put_Hidden(TRUE);

		hr=pFind->Execute(&wordFindText,//wordFindText

							&varFalse,//MatchCase

							&varFalse,//MatchWholeWord

							&varFalse,//MatchWildcards

							&varFalse,//MatchSoundsLike

							&varFalse,//MatchAllWordForms

							&varTrue,//Forward

							&varWrap,//Wrap

							&varTrue,//Format

							&wordFindText,//ReplaceWith

							&varReplace,//Replace

							&varOptional,

							&varOptional,

							&varOptional,

							&varOptional,

							&varBool

							);

		if(FAILED(hr)||varBool==FALSE)

			bRet=FALSE;



	}

	return bRet;

}

BOOL CODHDWordInspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

{

	BOOL	bRet=FALSE;

	HRESULT	hr=S_OK;



	CComPtr<Word::Documents>	pDocuments=NULL;

	CComPtr<Word::_Document>	pDoc   = NULL;

	VARIANT_BOOL		varVisible = 0;



	MSOAppInstance::Instance()->ReleaseWordApp();

	m_pApp=NULL;

	GetWordApp();



	if(NULL==m_pApp)

	{

		SetCatStatus(odhdCat,INSPECT_FAILED);

		return bRet;

	}



	hr = m_pApp->get_Documents(&pDocuments);

	if(SUCCEEDED(hr)&&pDocuments)

	{

		VARIANT_BOOL isValid=0;

		long lCount=0;

		hr=pDocuments->get_Count(&lCount);

		hr=m_pApp->get_IsObjectValid(pDocuments,&isValid);

		assert(m_strTempFileName.length());



		CComVariant FileName(/**/m_strTempFileName.c_str());

        CComVariant varZero((short)0);

        CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

        CComVariant varTrue((short)TRUE);

        CComVariant varFalse((short)FALSE);

		CComVariant varSaveChanges(0);  // WdSaveOptions::wdDoNotSaveChanges

		CComVariant varOrigFmt(1);      // WdOriginalFormat::wdOriginalDocumentFormat



		hr = pDocuments->Open( &FileName,      // File

            &varFalse,      // ConfirmConversions

            &varFalse,      // ReadOnly

            &varFalse,      // AddToRecentFiles

            &covOptional,   // PasswordDocument

            &covOptional,   // PasswordTemplate

            &varFalse,      // Revert

            &covOptional,   // WritePasswordDocument

            &covOptional,   // WritePasswordTemplate

            &covOptional,   // Format

            &covOptional,   // Encoding

            &varFalse,      // Visible

            &covOptional,	// OpenAndRepair

            &varZero,       // DocumentDirection

            &covOptional,   // NoEncodingDialog

            &covOptional,   // XMLTransform

            &pDoc);

		

		pDocuments->get_Count(&lCount);
		VARIANT_BOOL bIsTrack = FALSE;
		m_pApp->put_Visible(varVisible);

		m_pApp->put_ShowWindowsInTaskbar(varVisible);

		if(SUCCEEDED(hr)&&pDoc)

		{

            WdProtectionType	wdProetctType = wdNoProtection;

            hr = pDoc->get_ProtectionType(&wdProetctType);

			/*if(wdNoProtection!=wdProetctType)

			{

				pDoc->Unprotect();

				hr=pDoc->get_ProtectionType(&wdProetctType);

			}*/

            if(wdNoProtection==wdProetctType)

			{

                m_pApp->put_Visible(varVisible);

                m_pApp->put_ShowWindowsInTaskbar(varVisible);



                switch(odhdCat)

				{

				case ODHDCAT_WORD_COMMENTS:

					bRet=RemoveCategoryComments(pDoc);
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_COMMENTS;

					}

					break;

				case ODHDCAT_WORD_PROP:

					bRet=RemoveCategoryDocProp(pDoc);
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_PROP;

					}

					break;

				case ODHDCAT_WORD_HEADFOOT:

					bRet=RemoveCategoryHeaderFooter(pDoc);
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_HEADFOOT;

					}

					break;

				case ODHDCAT_WORD_TEXT:
				    pDoc->get_TrackRevisions(&bIsTrack);
					if(bIsTrack != FALSE)
					{
						pDoc->put_TrackRevisions(FALSE);
					}
					bRet=RemoveCategoryHiddenText(pDoc);
					if(bIsTrack != FALSE)
					{
						pDoc->put_TrackRevisions(bIsTrack);
					}
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_TEXT;

					}

					break;

				/*case ODHDCAT_WORD_XML:

					break;*/

				default:

					DP((L"Error Parameter for Hidden Data Category \n"));

					break;

				}

				



			}

			else

			{
				_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
				SaveDocAfterRemove(pDoc);

				DP((L"There is a password!\n"));

			}

			//pDocuments->Close(&varSaveChanges, &varOrigFmt, &varFalse);

		}

		else

		{

			

			hr=m_pApp->get_IsObjectValid(pDocuments,&isValid);

			hr=pDocuments->get_Count(&lCount);

			if(SUCCEEDED(hr)&&lCount)

			{

				CComVariant index(lCount);

				hr=pDocuments->Item(&index,&pDoc);

			}

			hr=pDocuments->Close(&varSaveChanges, &varOrigFmt, &varFalse);

		}

	}

	else

    {

        DP((L"Fail to get documents obj!\n"));

    }

	if(TRUE==bRet)

		SetCatStatus(odhdCat,INSPECT_REMOVED);

	else

		SetCatStatus(odhdCat,INSPECT_FAILED);

	return bRet;

}



ODHDSTATUS	CODHDWordInspector::InspectComments(Word::_Document *pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	HRESULT	hr=S_OK;

	CComPtr<Word::wordComments> pComments=NULL;

	long	nCount=0;

	hr=pDoc->get_wordComments(&pComments);

	if(SUCCEEDED(hr)&&pComments)

	{

		hr=pComments->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			RecordInspectResult(ODHDCAT_WORD_COMMENTS,

								ODHDITEM_WORD_COMMENTS,

								INSPECT_HAVE,

								gs_ItemTitle_Word[ODHDITEM_WORD_COMMENTS].m_itemTitle,

								gs_ItemTitle_Word[ODHDITEM_WORD_COMMENTS].m_itemFound,

								nCount);

			retStatus= INSPECT_HAVE;

		}

		

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectRevision(Word::_Document *pDoc)

{

	ODHDSTATUS retStatus=INSPECT_NONE;

	HRESULT	hr=S_OK;

	CComPtr<Word::Revisions> pRevisions=NULL;

	long	nCount=0;

	hr=pDoc->get_Revisions(&pRevisions);

	if(SUCCEEDED(hr)&&pRevisions)

	{

		hr=pRevisions->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			

			RecordInspectResult(ODHDCAT_WORD_COMMENTS,

								ODHDITEM_WORD_REVISION,

								INSPECT_HAVE,

								gs_ItemTitle_Word[ODHDITEM_WORD_REVISION].m_itemTitle,

								gs_ItemTitle_Word[ODHDITEM_WORD_REVISION].m_itemFound,

								nCount);

			retStatus= INSPECT_HAVE;

		}

		

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectVersion(Word::_Document *pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	HRESULT	hr=S_OK;

	CComPtr<Word::Versions> pVersions=NULL;

	long	nCount=0;

	hr=pDoc->get_Versions(&pVersions);

	if(SUCCEEDED(hr)&&pVersions)

	{

		hr=pVersions->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			

			RecordInspectResult(ODHDCAT_WORD_COMMENTS,

								ODHDITEM_WORD_VERSION,

								INSPECT_HAVE,

								gs_ItemTitle_Word[ODHDITEM_WORD_VERSION].m_itemTitle,

								gs_ItemTitle_Word[ODHDITEM_WORD_VERSION].m_itemFound,

								nCount);

			retStatus= INSPECT_HAVE;

		}

		

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectBuiltinProperty(Word::_Document *pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	CComPtr<IDispatch>  pDocPropertyDispatch   = NULL;

	HRESULT		hr = pDoc->get_BuiltInDocumentProperties(&pDocPropertyDispatch);

	if(SUCCEEDED(hr)&&pDocPropertyDispatch)

	{

		if(HasDocumentProperties(pDocPropertyDispatch))

			retStatus= INSPECT_HAVE;

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectCustomProperty(Word::_Document *pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

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

			retStatus= INSPECT_HAVE;

		

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectDocProperty(Word::_Document *pDoc)

{

	ODHDSTATUS builtinStatus,customStatus,retStatus=INSPECT_NONE;

	builtinStatus=InspectBuiltinProperty(pDoc);

	customStatus=InspectCustomProperty(pDoc);

	if(INSPECT_NONE==builtinStatus&&INSPECT_NONE==customStatus)

		retStatus=INSPECT_NONE;

	else

	{

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_DOCPROP,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCPROP].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCPROP].m_itemFound);

		if(INSPECT_FAILED==builtinStatus||INSPECT_FAILED==customStatus)

			retStatus=INSPECT_FAILED;

		else

			retStatus=INSPECT_HAVE;

	}

	return retStatus;

}



BOOL CODHDWordInspector::IsValidPropertyName(LPCWSTR strPropName)

{

	int iIndex=0;



	for(iIndex=0;iIndex<sizeof(gs_PropertiesNames_Word)/sizeof(LPCWSTR);iIndex++)

	{

		if(0 == _wcsicmp(strPropName, gs_PropertiesNames_Word[iIndex]))

			return TRUE;

	}

	return FALSE;

    /*if(0 == _wcsicmp(strPropName, L"Title"))

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

	*/

    return FALSE;

}

ODHDSTATUS CODHDWordInspector::InspectEmailHeader(Word::_Document *pDoc)

{

	HRESULT			hr   = S_OK;

	ODHDSTATUS		retStatus=INSPECT_NONE;

	CComPtr<Office::IMsoEnvelopeVB>	pEmailHeader=NULL;

	CComPtr<IDispatch>	pMailItem=NULL;

	BSTR			bstrIntrodution=NULL;

	VARIANT			varResult;

	hr=pDoc->get_MailEnvelope(&pEmailHeader);

	if(SUCCEEDED(hr)&&pEmailHeader)

	{

		hr=pEmailHeader->get_Introduction(&bstrIntrodution);

		hr=pEmailHeader->get_Item(&pMailItem);

		if(0)

		{//for test

			BSTR bstrIntro = ::SysAllocString(L"introduce from inspect");

			hr=pEmailHeader->put_Introduction(bstrIntro);

			if(SUCCEEDED(hr))

				pDoc->Save();

			::SysFreeString(bstrIntro);

		}

		if(SUCCEEDED(hr)&&pMailItem)

		{

			hr=DispatchCallWraper(pMailItem,DISPATCH_METHOD|DISPATCH_PROPERTYGET,&varResult,L"Subject",0);

			::SysFreeString( bstrIntrodution );

		}

	}

	return retStatus;



}

ODHDSTATUS CODHDWordInspector::InspectRoutingSlip(Word::_Document *pDoc)

{

	HRESULT			hr   = S_OK;

	VARIANT_BOOL	bHasRoutingSlip = FALSE;

	ODHDSTATUS		retStatus=INSPECT_NONE;

	hr = pDoc->get_HasRoutingSlip(&bHasRoutingSlip);

	if(SUCCEEDED(hr) && bHasRoutingSlip)

    {

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_DOCPROP,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCPROP].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCPROP].m_itemFound);

		retStatus= INSPECT_HAVE;

    }

	return retStatus;



}



ODHDSTATUS	CODHDWordInspector::InspectDocSrvProp(Word::_Document *pDoc)

{

	HRESULT			hr   = S_OK;

	ODHDSTATUS	 retStatus=INSPECT_NONE;



	CComPtr<Office::DocumentLibraryVersions> pDocLibVersion = 0;

	hr = pDoc->get_DocumentLibraryVersions(&pDocLibVersion);

	if(SUCCEEDED(hr) && pDocLibVersion)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_SRVPROP,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_SRVPROP].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_SRVPROP].m_itemFound);



	}

	return retStatus;

}



ODHDSTATUS	CODHDWordInspector::InspectSrvPolicy(Word::_Document *pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	return retStatus;

}



ODHDSTATUS	CODHDWordInspector::InspectContentType(Word::_Document* pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectDataLink(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;



	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectTemplate(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	CComPtr<IDispatch>  pDocPropertyDispatch   = NULL;

	HRESULT		hr = pDoc->get_BuiltInDocumentProperties(&pDocPropertyDispatch);

	if(SUCCEEDED(hr)&&pDocPropertyDispatch)

	{

		WCHAR       wzTemplateName[MAX_PATH];  

		memset(wzTemplateName, 0, sizeof(wzTemplateName));

		if(GetPropertyValueByName(pDocPropertyDispatch, L"Template", wzTemplateName, MAX_PATH)!=FALSE)

		{

			if(0!=_wcsicmp(wzTemplateName, L"Normal.dot")&&0!=_wcsicmp(wzTemplateName, L"Normal"))

			{

				retStatus=INSPECT_HAVE;

				RecordInspectResult(ODHDCAT_WORD_PROP,

								ODHDITEM_WORD_TEMPLATE,

								INSPECT_HAVE,

								gs_ItemTitle_Word[ODHDITEM_WORD_TEMPLATE].m_itemTitle,

								gs_ItemTitle_Word[ODHDITEM_WORD_TEMPLATE].m_itemFound);

			}

		}

	}

	return retStatus;

}

BOOL	CODHDWordInspector::CheckOneHeaderFooter(Word::wordHeaderFooter*pHeaderFooter,Word::_Document* pDoc)

{

	HRESULT		hr   = S_OK;

	CComPtr<Word::Range> pRange=NULL;

	long	lStart=0,lEnd=0;



	hr=pHeaderFooter->get_Range(&pRange);

	hr=pRange->get_Start(&lStart);

	hr=pRange->get_End(&lEnd);

	if(lStart==lEnd||lStart==lEnd-1)

		return FALSE;

	else

		return TRUE;

}

long	CODHDWordInspector::CheckHeadersFooters(Word::wordHeadersFooters*pHeadersFooters,Word::_Document* pDoc)

{

	HRESULT		hr   = S_OK;

	VARIANT_BOOL	bExists=FALSE;

	CComPtr<Word::wordHeaderFooter> pHeaderFooter=NULL;

	long nCount=0;

	WdHeaderFooterIndex indexArr[]={wdHeaderFooterPrimary,wdHeaderFooterFirstPage,wdHeaderFooterEvenPages};

	int index=0;

	for(;index<sizeof(indexArr)/sizeof(WdHeaderFooterIndex);index++)

	{

		hr=pHeadersFooters->Item(indexArr[index],&pHeaderFooter);

		if(SUCCEEDED(hr)&&pHeaderFooter)

		{

			hr=pHeaderFooter->get_Exists(&bExists);

			if(SUCCEEDED(hr)&&bExists)

			{

				if(CheckOneHeaderFooter(pHeaderFooter,pDoc)==TRUE)

					nCount++;

			}

		}

	}

	return nCount;

}

ODHDSTATUS	CODHDWordInspector::InspectHeaderFooter(Word::_Document *pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	CComPtr<Word::Sections>	pSections=NULL;

	CComPtr<Word::Section>	pSection=NULL;

	CComPtr<Word::wordHeadersFooters>	pHeaders=NULL,pFooters=NULL;

	long			nCount=0,iIndex=0;

	long	nHeaderCount=0,nFooterCount=0;

	long	iIndexHeader=0,iIndexFooter=0;

	

	hr=pDoc->get_Sections(&pSections);

	if(SUCCEEDED(hr)&&pSections)

	{

		hr=pSections->get_Count(&nCount);

		if(SUCCEEDED(hr)&&nCount)

		{

			for(;iIndex<nCount;iIndex++)

			{

				nHeaderCount=0;nFooterCount=0;

				hr=pSections->Item(iIndex+1,&pSection);

				if(SUCCEEDED(hr)&&pSection)

				{

					//Header

					hr=pSection->get_Headers(&pHeaders);

					if(SUCCEEDED(hr)&&pHeaders)

					{

						nHeaderCount=CheckHeadersFooters(pHeaders,pDoc);

					}



					//Footer

					hr=pSection->get_Footers(&pFooters);

					if(SUCCEEDED(hr)&&pFooters)

					{

						nFooterCount=CheckHeadersFooters(pFooters,pDoc);

					}


				}//one section

			}//for sections

		}

	}

	if(nHeaderCount)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_HEADFOOT,

							ODHDITEM_WORD_DOCHEADER,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCHEADER].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_DOCHEADER].m_itemFound,

							nHeaderCount);

	}

	if(nFooterCount)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_HEADFOOT,

							ODHDITEM_WORD_FOOTER,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_FOOTER].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_FOOTER].m_itemFound,

							nFooterCount);

	}





	return retStatus;

}

ODHDSTATUS CODHDWordInspector::InspectHiddenText(Word::_Document *pDoc)

{

	ODHDSTATUS		retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	VARIANT_BOOL	bHiddenText=0,bHiddenTextOrig=0;

	long	lLenNoHiddenText=0,lLenHiddenText=0;



	CComPtr<Word::Range>	pRange=NULL;

	CComPtr<Word::TextRetrievalMode> pTextMode=NULL;

	

	hr=pDoc->get_Content(&pRange);

	if(SUCCEEDED(hr)&&pRange)

	{

		hr=pRange->get_TextRetrievalMode(&pTextMode);

		if(SUCCEEDED(hr)&&pTextMode)

		{

			BSTR bstrText;

			hr=pTextMode->get_IncludeHiddenText(&bHiddenTextOrig);	//use to save the orignal setting

			bHiddenText=0;

			hr=pTextMode->put_IncludeHiddenText(bHiddenText);	//change the setting to let it not hiddentext

			pRange->get_Text(&bstrText);

			lLenNoHiddenText=::SysStringLen(bstrText);

			::SysFreeString(bstrText);



			bHiddenText=-1;

			hr=pTextMode->put_IncludeHiddenText(bHiddenText);	//to recover the orignal setting

			pRange->get_Text(&bstrText);

			lLenHiddenText=::SysStringLen(bstrText);

			::SysFreeString(bstrText);



			hr=pTextMode->put_IncludeHiddenText(bHiddenTextOrig);

			//pDoc->Save();	//so no need to save 

		}

	}

	if(lLenHiddenText!=lLenNoHiddenText)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_TEXT,

							ODHDITEM_WORD_HIDDENTEXT,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_HIDDENTEXT].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_HIDDENTEXT].m_itemFound);

	}

	return retStatus;



}



ODHDSTATUS	CODHDWordInspector::InspectSend4Review(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	



	return retStatus;

}

ODHDSTATUS CODHDWordInspector::InspectUserName(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	BSTR		bstrUserName=NULL;

	long		lLenUserName=0;

	m_pApp->get_UserName(&bstrUserName);

	lLenUserName=::SysStringLen(bstrUserName);

	::SysFreeString(bstrUserName);

	if(lLenUserName)

	{

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_USERNAME,

							INSPECT_HAVE,

							gs_ItemTitle_Word[ODHDITEM_WORD_USERNAME].m_itemTitle,

							gs_ItemTitle_Word[ODHDITEM_WORD_USERNAME].m_itemFound);

		retStatus=INSPECT_HAVE;

	}

	return retStatus;

}

ODHDSTATUS	CODHDWordInspector::InspectCustomXML(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;



	return retStatus;

}

BOOL CODHDWordInspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)

{

	BOOL    bRet = TRUE;

    HRESULT hr   = S_OK;

	int		nStep=MAX_HDRDLG_PROGRESS;

	m_strTempFileName=strTempFileName;

	m_strSrcFileName =strSrcFileName;

	

	m_pApp=NULL;

	GetWordApp();

	if(m_pApp==NULL)

		return FALSE;



	VARIANT_BOOL	bHasRoutingSlip = FALSE;

    VARIANT_BOOL    varVisible = FALSE;

    CComPtr<Word::Documents>    spDocuments  = NULL;

    CComPtr<Word::_Document>	spDocument   = NULL;



	hr = m_pApp->get_Documents(&spDocuments);



	if(SUCCEEDED(hr) && spDocuments)

    {

		CComVariant FileName(strTempFileName.c_str());

        CComVariant varZero((short)0);

        CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

        CComVariant varTrue((short)TRUE);

        CComVariant varFalse((short)FALSE);

		CComVariant varSaveChanges(wdSaveChanges);  // WdSaveOptions::wdDoNotSaveChanges

		CComVariant varOrigFmt(wdOriginalDocumentFormat);      // WdOriginalFormat::wdOriginalDocumentFormat

		CComVariant varDontSaveChanges(wdDoNotSaveChanges);  // WdSaveOptions::wdDoNotSaveChanges

		_beginthread(CODHDUtilities::ShowPasswordWindows, 0, NULL);

        hr = spDocuments->Open( &FileName,      // File

            &varFalse,      // ConfirmConversions

            &varFalse,      // ReadOnly

            &varFalse,      // AddToRecentFiles

            &covOptional,   // PasswordDocument

            &covOptional,   // PasswordTemplate

            &varFalse,      // Revert

            &covOptional,   // WritePasswordDocument

            &covOptional,   // WritePasswordTemplate

            &covOptional,   // Format

            &covOptional,   // Encoding

            &varFalse,      // Visible

            &covOptional,	// OpenAndRepair

            &varZero,       // DocumentDirection

            &covOptional,   // NoEncodingDialog

            &covOptional,   // XMLTransform

            &spDocument);

        if(SUCCEEDED(hr) && spDocument)

        {

            m_pApp->put_Visible(varVisible);

            m_pApp->put_ShowWindowsInTaskbar(varVisible);



            WdProtectionType	wdProetctType = wdNoProtection;

            hr = spDocument->get_ProtectionType(&wdProetctType);

            DP((L"NoProtection = %s (Function %s)\n", (wdNoProtection==wdProetctType)?L"True":L"False", SUCCEEDED(hr)?L"OK":L"Fail"));

			

			wdProetctType=wdNoProtection;

            if(wdNoProtection==wdProetctType)

            {

                DP((L"Start word check!\n"));

				SetProgTitle(ODHDCAT_WORD_COMMENTS);

                InspectComments(spDocument);

				InspectRevision(spDocument);

				InspectVersion(spDocument);

				MakeStep(3);nStep-=3;



				SetProgTitle(ODHDCAT_WORD_PROP);

				InspectDocProperty(spDocument);

				//InspectEmailHeader(spDocument);

				InspectRoutingSlip(spDocument);

				//InspectSend4Review(spDocument);

				InspectDocSrvProp(spDocument);

				//InspectSrvPolicy(spDocument);

				//InspectContentType(spDocument);

				//InspectDataLink(spDocument);



				//InspectUserName(spDocument);

				

				InspectTemplate(spDocument);

				MakeStep(4);nStep-=4;

				

				SetProgTitle(ODHDCAT_WORD_HEADFOOT);

				InspectHeaderFooter(spDocument);

				MakeStep(1);nStep-=1;





				SetProgTitle(ODHDCAT_WORD_TEXT);

				InspectHiddenText(spDocument);

				MakeStep(1);nStep-=1;

				//InspectCustomXML(spDocument);

				m_pApp->put_Visible(varVisible);

                m_pApp->put_ShowWindowsInTaskbar(varVisible);



            }

            else

            {

                DP((L"There is a password!\n"));

            }

			hr = spDocument->Close(&varDontSaveChanges, &varOrigFmt, &varFalse);

        }

        else

        {

			std::wstring strMsg = L"";

			CODHDUtilities::ReplaceFileName(g_wstrCantOpenFile, strSrcFileName, strMsg);



            m_pApp->put_Visible(varVisible);

            m_pApp->put_ShowWindowsInTaskbar(varVisible);

			MessageBox(m_progDlg->m_hWnd, strMsg.c_str(), L"Hidden Data Removal", MB_OK | MB_ICONWARNING);

			bRet=FALSE;

        }

		//CComVariant varOrigFmtTmp(2);

		//hr=spDocuments->Close(&varDontSaveChanges, &varOrigFmt, &varFalse);

        //hr=spDocuments->Release();

        //spDocuments = NULL;

    }

    else

    {

        DP((L"Fail to get documents obj!\n"));

		bRet=FALSE;

    }

	if(nStep > 0)

		MakeStep(nStep);



    DP((L"End word inspect!\n"));



    return bRet;

}



void CODHDWordInspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)

{

	CODHDCategory *pCat=NULL;

	CODHDItem *		pItem=NULL;

	assert(m_pCatManager);

	pCat=m_pCatManager->GetHDCategory(odhdCat);

	switch(odhdCat)

	{

	case ODHDCAT_EXCEL_HEADFOOT:

		pCat->GetItemsResult(strResult);

		if(pCat->GetCatStatus()==INSPECT_HAVE)

			strResult+=L"\nHeaders and footers may include shapes such as watermarks.";

		break;

	default:

		CODHDInspector::GetResult(odhdCat,strResult);

		break;

	}

}



BOOL CODHDWordInspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)

{

	BOOL bFiltered = FALSE;



	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_WORD_COMMENTS && !(dwNeedCat & WordHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_WORD_PROP && !(dwNeedCat & WordHDCat_PROP))

			|| (odhdCat == ODHDCAT_WORD_HEADFOOT && !(dwNeedCat & WordHDCat_HEADFOOT))

			|| (odhdCat == ODHDCAT_WORD_TEXT && !(dwNeedCat & WordHDCat_TEXT)))

		{

			bFiltered = TRUE;

		}



		if (!bFiltered && odhdStatus == INSPECT_HAVE)

		{

			switch (odhdCat)

			{

			case ODHDCAT_WORD_COMMENTS:

				m_pHDRFile->m_dwFoundCatType |= WordHDCat_COMMENTS;

				break;

			case ODHDCAT_WORD_PROP:

				m_pHDRFile->m_dwFoundCatType |= WordHDCat_PROP;

				break;

			case ODHDCAT_WORD_HEADFOOT:

				m_pHDRFile->m_dwFoundCatType |= WordHDCat_HEADFOOT;

				break;

			case ODHDCAT_WORD_TEXT:

				m_pHDRFile->m_dwFoundCatType |= WordHDCat_TEXT;

				break;

			default:

				break;

			}

		}

	}



	return bFiltered;

}



BOOL CODHDWordInspector::GetFilter(ODHDCATEGORY odhdCat)

{

	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_WORD_COMMENTS && !(dwNeedCat & WordHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_WORD_PROP && !(dwNeedCat & WordHDCat_PROP))

			|| (odhdCat == ODHDCAT_WORD_HEADFOOT && !(dwNeedCat & WordHDCat_HEADFOOT))

			|| (odhdCat == ODHDCAT_WORD_TEXT && !(dwNeedCat & WordHDCat_TEXT)))

		{

			return TRUE;

		}

	}



	return FALSE;

}

