#include "stdafx.h"

#include "odhd_word2k7.h"
#include <string>
using namespace std;

#ifndef WSO2K3



/*Author: Casablanca

Purpose: for bug119



InspectVersionsByStream: Inspect version information of a word document by stream.

Parameters:

std::wstring (pFileName): [in] Pointer to the Doc file name.

Return value: BOOL 

TRUE:  HAS Version information

FALSE: No V.I.

Remark: None. */



extern std::wstring g_wstrCantOpenFile;


BOOL InspectVersionsByStream(std::wstring pFileName)

{

	BOOL bRet = FALSE; 



	WCHAR wcEndStr[5];

	int nlen = int(pFileName.length()), nc = 0;

	while (nlen--)

	{

		nc++;

		wcEndStr[4-nc] = towlower(pFileName[nlen]);

		if (nc >= 4)//length of '.doc' or '.dot'

		{

			break;

		}

	}

	if (nc >= 4)

	{

		wcEndStr[4] = '\0';

		if (wcscmp(wcEndStr,L".doc"))

		{

			return bRet;

		}

	}



	HRESULT hr = E_FAIL; 

	CComPtr<IStorage> ptrRootStg = NULL; // root storage 



	hr = ::StgIsStorageFile( pFileName.c_str() ); 

	if( S_OK != hr ) 

	{ 

		DP( (L"InspectVersionsByStream: Not a StorageFile!") );

		return bRet; 

	}



	// open the Compound document 

	hr = ::StgOpenStorage( pFileName.c_str(), NULL, 

		STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 

		0, 0, &ptrRootStg ); 

	if( FAILED(hr)||NULL==ptrRootStg )

	{ 

		DP( (L"InspectVersionsByStream: Fail to open!") );

		return bRet;

	} 

	//////////////////////////////////////////////////////////////////////////



	CComPtr<IEnumSTATSTG> ptrEnum = NULL; 



	hr = ptrRootStg->EnumElements( 0, NULL, 0, &ptrEnum ); 

	if( FAILED(hr) ||NULL==ptrEnum) { 

		return bRet; 

	} 



	STATSTG StatStg = { 0 }; 



	bool bFlag = false;

	while( S_OK == hr&&!bFlag ) 

	{ 

		hr = ptrEnum->Next( 1, &StatStg, NULL ); 

		if( S_FALSE == hr ) 

		{ 

			continue; 

		} 



		switch(StatStg.type) 

		{ 

		case STGTY_STORAGE: // 是存储对象, "文件夹" 

			break; 

		case STGTY_STREAM:  // 是流, "文件" 

			{ 

				bFlag = true;

			} 

			break; 

		case STGTY_LOCKBYTES: 

			break; 

		case STGTY_PROPERTY: 

			break; 

		default: 

			break; 

		} 



		::CoTaskMemFree( StatStg.pwcsName ); // 释放名称所使用的内存 

	}



	if (bFlag)

	{

		DWORD dw_fcDop       = 0;//0x0192

		WORD  w_fWhichTblStm = 0;//0x000A

		WORD  w_fHasVersion  = 0;//0x019C



		CComPtr<IStream> spStreamWD = NULL, spStreamTable = NULL;

		LARGE_INTEGER   uli_offset;



		//get WordDocument stream which contains FIB

		hr = ptrRootStg->OpenStream(L"WordDocument", NULL, 

			STGM_READWRITE|STGM_SHARE_EXCLUSIVE, 0, &spStreamWD); 

		if(FAILED(hr)||spStreamWD==NULL)

			return bRet;

		//get fWhichTblStm which determin which table to read.

		memset( &uli_offset, 0, sizeof(LARGE_INTEGER) );

		uli_offset.HighPart = 0;

		uli_offset.LowPart  = 0x000A;

		hr = spStreamWD->Seek( uli_offset, STREAM_SEEK_SET, NULL );

		hr = spStreamWD->Read( (PVOID)&w_fWhichTblStm, 2, NULL );

		//get fcDop from FIB which indicates the offset of DOP in table stream.

		memset( &uli_offset, 0, sizeof(LARGE_INTEGER) );

		uli_offset.HighPart = 0;

		uli_offset.LowPart  = 0x0192;

		hr = spStreamWD->Seek( uli_offset, STREAM_SEEK_SET, NULL );

		hr = spStreamWD->Read( (PVOID)&dw_fcDop, 4, NULL );



		//get table stream

		std::wstring wcsTableName;

		w_fWhichTblStm = w_fWhichTblStm & 0x0200;

		if (w_fWhichTblStm == 0x0200)

		{

			wcsTableName = L"1Table";

		}

		else

		{

			wcsTableName = L"0Table";

		}



		hr = ptrRootStg->OpenStream( wcsTableName.c_str(), NULL, 

			STGM_READWRITE|STGM_SHARE_EXCLUSIVE, 0, &spStreamTable); 

		if(FAILED(hr)||spStreamTable==NULL)

			return bRet;



		//get fHasVersion which indicates if the doc contains versions.

		memset( &uli_offset, 0, sizeof(LARGE_INTEGER) );

		uli_offset.HighPart = 0;

		uli_offset.LowPart  = dw_fcDop + 0x019C;

		hr = spStreamTable->Seek( uli_offset, STREAM_SEEK_SET, NULL );

		hr = spStreamTable->Read( (PVOID)&w_fHasVersion, 2, NULL );



		w_fHasVersion = w_fHasVersion & 0x0001;

		if (w_fHasVersion == 0x0001)

		{

			bRet = TRUE;

		}

	}



	return bRet;

}

BOOL CODHDWord2K7Inspector::RemoveCategoryComments(Word::_Document*pDoc)

{

	BOOL			bRet=TRUE;

	HRESULT			hr=S_OK;

	if(m_HasComment==TRUE)

	{

		hr = pDoc->RemoveDocumentInformation(wdRDIComments);

		if(FAILED(hr))

		{

			DP((L"Word::RemoveData-- Fail to remove wdRDIComments"));

			bRet=FALSE;

		}

		if(bRet!=FALSE)

			m_HasComment=FALSE;

	}

	

#if 0

	hr=pDoc->DeleteAllInkAnnotations();

	if(FAILED(hr))

	{

		DP((L"Word::RemoveData-- Fail to remove DeleteAllInkAnnotations"));

		bRet=FALSE;

	}

#endif	

	if(m_HasRevision==TRUE)

	{

		hr=pDoc->put_TrackRevisions(FALSE);

		if(FAILED(hr))

		{

			DP((L"Word::RemoveData-- Fail to remove put_TrackRevisions"));

			bRet=FALSE;

		}

		

		hr=pDoc->AcceptAllRevisions();

		if(FAILED(hr))

		{

			DP((L"Word::RemoveData-- Fail to remove AcceptAllRevisions"));

			//bRet=FALSE;

		}

		

		hr=pDoc->RemoveDocumentInformation(wdRDIRevisions);

		if(FAILED(hr))

		{

			DP((L"Word::RemoveData-- Fail to remove RemoveDocumentInformation"));

			bRet=FALSE;

		}

		if(bRet!=FALSE)

			m_HasRevision=FALSE;

	}

	if(m_HasVersion==TRUE)

	{

		hr=pDoc->RemoveDocumentInformation(wdRDIVersions);

		if(FAILED(hr))

		{

			DP((L"Word::RemoveData-- Fail to remove RemoveDocumentInformation"));

			bRet=FALSE;

		}

		if(bRet!=FALSE)

			m_HasVersion=FALSE;



	}

	

	return bRet;

}



BOOL	CODHDWord2K7Inspector::SaveDocAfterRemove(Word::_Document*pDoc)

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

	if(bReadonly)

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





BOOL	CODHDWord2K7Inspector::RemoveCategoryDocProp(Word::_Document*pDoc)

{

	BOOL	bRet=TRUE;

	HRESULT	hr=S_OK;

	hr = pDoc->RemoveDocumentInformation(wdRDIDocumentProperties);			if(FAILED(hr))bRet=FALSE;

	hr = pDoc->RemoveDocumentInformation(wdRDIRemovePersonalInformation);	if(FAILED(hr))bRet=FALSE;

	hr = pDoc->RemoveDocumentInformation(wdRDITemplate);					if(FAILED(hr))bRet=FALSE;

	//hr = pDoc->RemoveDocumentInformation(wdRDIEmailHeader);					if(FAILED(hr))bRet=FALSE;

	hr = pDoc->RemoveDocumentInformation(wdRDIRoutingSlip);					if(FAILED(hr))bRet=FALSE;

	//hr = pDoc->RemoveDocumentInformation(wdRDISendForReview);				if(FAILED(hr))bRet=FALSE;

	//hr = pDoc->RemoveDocumentInformation(wdRDIDocumentWorkspace);			if(FAILED(hr))bRet=FALSE;

	//hr = pDoc->RemoveDocumentInformation(wdRDIDocumentServerProperties);	if(FAILED(hr))bRet=FALSE;

	hr = pDoc->RemoveDocumentInformation(wdRDIDocumentManagementPolicy);	if(FAILED(hr))bRet=FALSE;



	if(m_HasContentType==TRUE)

	{

		bRet=RemoveCategoryCustomXML(pDoc);

		hr = pDoc->RemoveDocumentInformation(wdRDIContentType);					if(FAILED(hr))bRet=FALSE;

	}

	

	// Remove template

	/*CComVariant varTemplateDotx(L"Normal.dotx");

	hr=pDoc->put_AttachedTemplate(&varTemplateDotx);

	if(FAILED(hr))

	{

		CComVariant varTemplateDotm(L"Normal.dotm");

		hr=pDoc->put_AttachedTemplate(&varTemplateDotm);

	}*/

	

	return bRet;

}



BOOL	CODHDWord2K7Inspector::RemoveCategoryHeaderFooter(Word::_Document*pDoc)

{

	BOOL		bRet=FALSE;

	HRESULT			hr   = S_OK;

	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	VARIANT_BOOL	oldTrackRevision,noTrackRevision=0;

	if(m_InspectorIndex2==0)

		return TRUE;

	hr=pDoc->get_TrackRevisions(&oldTrackRevision);

	if(SUCCEEDED(hr))

	{

		hr=pDoc->put_TrackRevisions(noTrackRevision);

	}

	hr = pDoc->get_DocumentInspectors(&spInspectors);

#ifdef WSO2013
	m_InspectorIndex2 = 3;
#endif 
	if(SUCCEEDED(hr) && spInspectors)

		bRet= RemoveByInspector(spInspectors,m_InspectorIndex2,ODHDCAT_WORD_HEADFOOT,ODHDITEM_WORD_DOCHEADER);

	pDoc->put_TrackRevisions(oldTrackRevision);

	return bRet;

}

BOOL	CODHDWord2K7Inspector::RemoveCategoryHiddenText(Word::_Document*pDoc)

{

	BOOL		bRet=FALSE;

	HRESULT			hr   = S_OK;

	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	if(m_InspectorIndex3==0)

		return TRUE;



	hr = pDoc->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		return RemoveByInspector(spInspectors,m_InspectorIndex3,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT);

	}

	return bRet;

}

BOOL	CODHDWord2K7Inspector::RemoveCategoryCustomXML(Word::_Document*pDoc)

{

	BOOL		bRet=FALSE;

	HRESULT			hr   = S_OK;

	if(m_InspectorIndex1==0)

		return TRUE;



	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pDoc->get_DocumentInspectors(&spInspectors);


#ifdef WSO2013
	m_InspectorIndex1 = 2;
#endif
	if(SUCCEEDED(hr) && spInspectors)

		return RemoveByInspector(spInspectors,m_InspectorIndex1,ODHDCAT_WORD_XML,ODHDITEM_WORD_CUSTOMXML);

	return bRet;

}

BOOL CODHDWord2K7Inspector::Remove(ODHDCATEGORY odhdCat,ODHDITEM odhdItem)

{
#ifdef WSO2013
	if (odhdCat >= ODHDCAT_COLLAPSE)
	{
		odhdCat = (ODHDCATEGORY)(odhdCat + 1);
	}
#endif
	BOOL	bRet=FALSE;

	HRESULT	hr=S_OK;



	CComPtr<Word::Documents>	pDocuments=NULL;

	CComPtr<Word::_Document>	pDoc   = NULL;

	VARIANT_BOOL		varVisible = 0;



	//MSOAppInstance::Instance()->ReleaseWordApp();

	//m_pApp=NULL;

	//GetWordApp();



	if(NULL==m_pApp)

	{

		SetCatStatus(odhdCat,INSPECT_FAILED);

		return bRet;

	}



	{

		CComBSTR	bstrAppName;

		hr=m_pApp->get_Name(&bstrAppName);

		if(hr==0x800706ba)//The RPC server is unavailable means Word process was killed

		{			

			MSOAppInstance::Instance()->ReleaseWordApp();

			m_pApp=NULL;

			GetWordApp();

			if(NULL==m_pApp)

			{

				SetCatStatus(odhdCat,INSPECT_FAILED);

				return bRet;

			}

		}

	}



	hr = m_pApp->get_Documents(&pDocuments);

	/*if(hr==0x800706ba||hr==0x800706be)

	{

		m_pApp=NULL;

		MSOAppInstance::Instance()->ReleaseWordApp();

		GetWordApp();

		if(NULL==m_pApp)

		{

			SetCatStatus(odhdCat,INSPECT_FAILED);

			return bRet;

		}

		hr = m_pApp->get_Documents(&pDocuments);

	}*/

	if(SUCCEEDED(hr)&&pDocuments)

	{
#ifdef WSO2013
		std::wstring wstrRegVal;
		BOOL bRegBakRes = FALSE;
		if (ODHDCAT_WORD_TEXT == odhdCat)
		{
			bRegBakRes = ProcessOutlookTempFolderinReg(1,wstrRegVal);
		}
#endif
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

					&covOptional,   // OpenAndRepair

					&varZero,       // DocumentDirection

					&covOptional,   // NoEncodingDialog

					&covOptional,   // XMLTransform

            &pDoc);

		

		pDocuments->get_Count(&lCount);



		m_pApp->put_Visible(varVisible);

		m_pApp->put_ShowWindowsInTaskbar(varVisible);

		if(SUCCEEDED(hr)&&pDoc)

		{

			m_pApp->put_Visible(varVisible);

			m_pApp->put_ShowWindowsInTaskbar(varVisible);

			VARIANT_BOOL    bReadonly  = FALSE;

			if( pDoc )

			{

				HRESULT hr = pDoc->get_ReadOnly( &bReadonly) ;



				if( SUCCEEDED( hr ) )

				{	

					DP((L"Read only:[%d]",bReadonly));

					if( bReadonly )

					{

						DP((L"Read only"));

						bRet = FALSE ;

					}

				}

			}

			if( bReadonly == FALSE )

			{

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

					{

						DP((L"Remove Comments[Word]:%d",bRet));

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

					bRet=RemoveCategoryHiddenText(pDoc);
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_TEXT;

					}

					break;

				case ODHDCAT_WORD_XML:

					bRet=RemoveCategoryCustomXML(pDoc);
					_beginthread(CODHDUtilities::ShowPromtWindows, 0, NULL);
					bRet= bRet && SaveDocAfterRemove(pDoc);
					if (bRet && m_pHDRFile != NULL)

					{

						m_pHDRFile->m_dwRemovedCatType |= WordHDCat_XML;

					}

					break;

				default:

					DP((L"Error Parameter for Hidden Data Category \n"));

					break;

				}



			}

			//pDocuments->Close(&varSaveChanges, &varOrigFmt, &varFalse);
#ifdef WSO2013
			if (bRegBakRes)
			{
				ProcessOutlookTempFolderinReg(2,wstrRegVal);
			}
#endif
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



ODHDSTATUS	CODHDWord2K7Inspector::InspectComments(Word::_Document *pDoc)

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

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_COMMENTS].m_itemTitle,

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_COMMENTS].m_itemFound,

								nCount);

			m_HasComment=TRUE;

			retStatus= INSPECT_HAVE;

		}

		

	}

	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectRevision(Word::_Document *pDoc)

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

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_REVISION].m_itemTitle,

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_REVISION].m_itemFound,

								nCount);

			m_HasRevision=TRUE;

			retStatus= INSPECT_HAVE;

		}

	}

	/*here want to detect revision in Header/Footers

	if(retStatus!=INSPECT_HAVE)

	{

		CComPtr<Word::wordHeadersFooters> pHeaderFooters=NULL;

		hr=pDoc->get_wordHeaderFooters()



	}*/

	

	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectVersion(Word::_Document *pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	HRESULT	hr=S_OK;

	CComPtr<Word::Versions> pVersions=NULL;

	long	nCount=0;

	if(m_HasVersion==TRUE)

	{

		RecordInspectResult(ODHDCAT_WORD_COMMENTS,

			ODHDITEM_WORD_VERSION,

			INSPECT_HAVE,

			gs_ItemTitle_Word2K7[ODHDITEM_WORD_VERSION].m_itemTitle,

			gs_ItemTitle_Word2K7[ODHDITEM_WORD_VERSION].m_itemFound,

			nCount);

		retStatus= INSPECT_HAVE;

	}

	else

	{

		hr=pDoc->get_Versions(&pVersions);

		if(SUCCEEDED(hr)&&pVersions)

		{

			hr=pVersions->get_Count(&nCount);

			if(SUCCEEDED(hr)&&nCount)

			{



				RecordInspectResult(ODHDCAT_WORD_COMMENTS,

					ODHDITEM_WORD_VERSION,

					INSPECT_HAVE,

					gs_ItemTitle_Word2K7[ODHDITEM_WORD_VERSION].m_itemTitle,

					gs_ItemTitle_Word2K7[ODHDITEM_WORD_VERSION].m_itemFound,

					nCount);

				m_HasVersion=TRUE;

				retStatus= INSPECT_HAVE;

			}



		}

	}

	

	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectBuiltinProperty(Word::_Document *pDoc)

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

ODHDSTATUS	CODHDWord2K7Inspector::InspectCustomProperty(Word::_Document *pDoc)

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

ODHDSTATUS	CODHDWord2K7Inspector::InspectDocProperty(Word::_Document *pDoc)

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

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_DOCPROP].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_DOCPROP].m_itemFound);

		if(INSPECT_FAILED==builtinStatus||INSPECT_FAILED==customStatus)

			retStatus=INSPECT_FAILED;

		else

			retStatus=INSPECT_HAVE;

	}

	return retStatus;

}



BOOL CODHDWord2K7Inspector::IsValidPropertyName(LPCWSTR strPropName)

{

	int iIndex=0;

	static LPCWSTR ls_PropertiesNames_Word2K7[]={

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

	for(iIndex=0;iIndex<sizeof(ls_PropertiesNames_Word2K7)/sizeof(LPCWSTR);iIndex++)

	{

		if(0 == _wcsicmp(strPropName, ls_PropertiesNames_Word2K7[iIndex]))

			return TRUE;

	}

    return FALSE;

}

ODHDSTATUS CODHDWord2K7Inspector::InspectEmailHeader(Word::_Document *pDoc)

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

ODHDSTATUS CODHDWord2K7Inspector::InspectRoutingSlip(Word::_Document *pDoc)

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

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_ROUTESLIP].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_ROUTESLIP].m_itemFound);

		retStatus= INSPECT_HAVE;

    }

	return retStatus;



}



ODHDSTATUS	CODHDWord2K7Inspector::InspectDocSrvProp(Word::_Document *pDoc)

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

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_SRVPROP].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_SRVPROP].m_itemFound);



	}

	return retStatus;

}



ODHDSTATUS	CODHDWord2K7Inspector::InspectSrvPolicy(Word::_Document *pDoc)

{

	HRESULT			hr   = S_OK;

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	CComPtr<Office::ServerPolicy> spServerPolicy = 0;

	hr = pDoc->get_ServerPolicy(&spServerPolicy);

	if(SUCCEEDED(hr) && spServerPolicy)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_MANAGEPOLICY,

							INSPECT_HAVE,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_MANAGEPOLICY].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_MANAGEPOLICY].m_itemFound);

	}



	return retStatus;

}



ODHDSTATUS	CODHDWord2K7Inspector::InspectContentType(Word::_Document* pDoc)

{

	HRESULT			hr   = S_OK;

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	CComPtr<Office::MetaProperties> spMetaProperties = 0;

	hr = pDoc->get_ContentTypeProperties(&spMetaProperties);

	if(SUCCEEDED(hr) && spMetaProperties)

	{

		retStatus=INSPECT_HAVE;

		RecordInspectResult(ODHDCAT_WORD_PROP,

							ODHDITEM_WORD_CONTENTTYPE,

							INSPECT_HAVE,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_CONTENTTYPE].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_CONTENTTYPE].m_itemFound);

		m_HasContentType=TRUE;

	}



	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectDataLink(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;



	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectTemplate(Word::_Document*pDoc)

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

			if(0!=_wcsicmp(wzTemplateName, L"Normal.dotm")&&

				0!=_wcsicmp(wzTemplateName, L"Normal.dotx")&&

				0!=_wcsicmp(wzTemplateName, L"Normal.dot")&&

				0!=_wcsicmp(wzTemplateName, L"Normal")&&

				0!=_wcsicmp(wzTemplateName, L""))

			{

				retStatus=INSPECT_HAVE;

				RecordInspectResult(ODHDCAT_WORD_PROP,

								ODHDITEM_WORD_TEMPLATE,

								INSPECT_HAVE,

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_TEMPLATE].m_itemTitle,

								gs_ItemTitle_Word2K7[ODHDITEM_WORD_TEMPLATE].m_itemFound);

			}

#if 0	

			CComVariant attachedTemplate;



			hr=pDoc->get_AttachedTemplate(&attachedTemplate);

			if(SUCCEEDED(hr)&&attachedTemplate.vt==VT_DISPATCH&&attachedTemplate.pdispVal)

			{

				CComPtr<IDispatch>	pTemplate=attachedTemplate.pdispVal;

				CComVariant	varResult;

				hr=DispatchCallWraper(pTemplate,DISPATCH_PROPERTYGET, &varResult, L"Name", 0);

				hr=DispatchCallWraper(pTemplate,DISPATCH_PROPERTYGET, &varResult, L"FullName", 0);

				hr=DispatchCallWraper(pTemplate,DISPATCH_PROPERTYGET, &varResult, L"Type", 0);

				hr=DispatchCallWraper(pTemplate,DISPATCH_PROPERTYGET,&varResult,L"BuiltInDocumentProperties",0);

				if(SUCCEEDED(hr))

				{

					HasDocumentProperties(varResult.pdispVal);



	

				}

			}

#endif

		}

	}

	return retStatus;

}



ODHDSTATUS	CODHDWord2K7Inspector::InspectHeaderFooter(Word::_Document *pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pDoc->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{
		int nIndex = 2;

		if (m_IsOffice2013 || m_IsOffice2016)	nIndex = 3;

    	if(InspectorExist(spInspectors,nIndex,_T("Headers, Footers, and Watermarks")))

		{

			m_InspectorIndex2=nIndex;

			return InspectByInspector(spInspectors,ODHDCAT_WORD_HEADFOOT,ODHDITEM_WORD_DOCHEADER,&m_InspectorFail2);

		}

		else

		{

			if(InspectorExist(spInspectors,1,_T("Headers, Footers, and Watermarks")))

			{

				m_InspectorIndex2=1;

				return InspectByInspector(spInspectors,1,ODHDCAT_WORD_HEADFOOT,ODHDITEM_WORD_DOCHEADER,&m_InspectorFail2);

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



ODHDSTATUS CODHDWord2K7Inspector::InspectHiddenText(Word::_Document *pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pDoc->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		if((m_IsOffice2013==TRUE || m_IsOffice2016)&&InspectorExist(spInspectors,5,_T("Hidden Text")))
		{
			m_InspectorIndex3=5;

			return InspectByInspector(spInspectors,5,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

		}
		else if(m_IsOffice2010==TRUE&&InspectorExist(spInspectors,4,_T("Hidden Text")))
		{

			m_InspectorIndex3=4;

			return InspectByInspector(spInspectors,4,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

			//return InspectByInspector(spInspectors,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

		}

		else

		{

			if(InspectorExist(spInspectors,3,_T("Hidden Text")))

			{

				m_InspectorIndex3=3;

				return InspectByInspector(spInspectors,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

			}

			else

			{

				if(InspectorExist(spInspectors,2,_T("Hidden Text")))

				{

					m_InspectorIndex3=2;

					return InspectByInspector(spInspectors,2,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

				}

				else

				{

					if(InspectorExist(spInspectors,1,_T("Hidden Text")))

					{

						m_InspectorIndex3=1;

						return InspectByInspector(spInspectors,2,ODHDCAT_WORD_TEXT,ODHDITEM_WORD_HIDDENTEXT,&m_InspectorFail3);

					}

					else

					{

						m_InspectorIndex3=0;

						retStatus=INSPECT_NONE;

					}

				}

			}

		}

	}

	return retStatus;



}

ODHDSTATUS	CODHDWord2K7Inspector::InspectSend4Review(Word::_Document*pDoc)

{

	ODHDSTATUS	retStatus=INSPECT_NONE;

	



	return retStatus;

}

ODHDSTATUS CODHDWord2K7Inspector::InspectUserName(Word::_Document*pDoc)

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

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_USERNAME].m_itemTitle,

							gs_ItemTitle_Word2K7[ODHDITEM_WORD_USERNAME].m_itemFound);

		retStatus=INSPECT_HAVE;

	}

	return retStatus;

}

ODHDSTATUS	CODHDWord2K7Inspector::InspectCustomXML(Word::_Document*pDoc)

{

	ODHDSTATUS	 retStatus=INSPECT_NONE;

	HRESULT			hr   = S_OK;

	CComPtr<Office::DocumentInspectors> spInspectors=NULL;

	hr = pDoc->get_DocumentInspectors(&spInspectors);

	if(SUCCEEDED(hr) && spInspectors)

	{

		int nIndex = 1;

		if(m_IsOffice2013 || m_IsOffice2016)
			nIndex = 2;
	
		
		BOOL bSuccess = FALSE;
		if (m_IsOffice2016)
		{
			bSuccess = InspectorExist(spInspectors,nIndex,_T("Custom &XML Data"));
			if (!bSuccess)
			{
				bSuccess = InspectorExist(spInspectors,nIndex,_T("Custom XML Data"));
			}
			
		}
		else
		{
			bSuccess = InspectorExist(spInspectors,nIndex,_T("Custom XML"));
		}
		
		
		if(bSuccess)

		{

			m_InspectorIndex1=1;

			return InspectByInspector(spInspectors,ODHDCAT_WORD_XML,ODHDITEM_WORD_CUSTOMXML,&m_InspectorFail1);

		}

		else

		{

			m_InspectorIndex1=0;

			retStatus=INSPECT_NONE;

		}

	}

	return retStatus;



}

/*

static BOOL IsWordMainWindow(HWND hwnd)

{

	TCHAR tchClassName[128],tchTitle[128];

	if(GetClassName(hwnd,tchClassName,sizeof(tchClassName)/sizeof(TCHAR)))

	{

		if(_tcsncmp(tchClassName,_T("OpusApp"),7)==0)

		{

			if(GetWindowText(hwnd,tchTitle,sizeof(tchTitle)/sizeof(TCHAR)))

			{

				if(_tcsncmp(tchTitle,_T("Microsoft Word"),14)==0)

				{

					return TRUE;

				}

			}

		}

	}

	return FALSE;

}



static BOOL CALLBACK EnumWindowsProc4Word2K7(HWND hwnd, 

							  LPARAM lParam 

							  )

{

	DWORD dwThreadID,dwProcID=0;

	CODHDWord2K7Inspector* pWord2K7Inspector=(CODHDWord2K7Inspector*)lParam;

	dwThreadID=GetWindowThreadProcessId(hwnd,&dwProcID);

	if(pWord2K7Inspector->GetPID()==dwProcID)

	{

		TCHAR tchClassName[128];

		if(GetClassName(hwnd,tchClassName,sizeof(tchClassName)/sizeof(TCHAR)))

		{

			if(_tcsncmp(tchClassName,_T("bosa_sdm_Microsoft Office Word 12.0"),35)==0)

			{

				TCHAR tchWndTitle[128];

				if(GetWindowText(hwnd,tchWndTitle,sizeof(tchWndTitle)/sizeof(TCHAR)))

				{

					if(_tcsncmp(tchWndTitle,_T("Password"),8)==0)

					{//Here ,find the handle of the password window

						HWND hwndWord=GetParent(hwnd);

						if(hwndWord&&IsWordMainWindow(hwndWord))

						{

							pWord2K7Inspector->SetPasswordFlag(TRUE);

							WNDPROC wndprocPasswd=(WNDPROC)GetWindowLong(hwnd,GWL_WNDPROC);



							return FALSE;

							LONG wndExStyle=GetWindowLong(hwnd,GWL_EXSTYLE);

							wndExStyle|=WS_EX_TOPMOST;

							SetWindowLong(hwnd,GWL_EXSTYLE,wndExStyle);

							SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE); 

						}

					}

				}

			}

		}



	}

	return TRUE;



}

static DWORD CheckDialogOfWord2K7(LPVOID lpVoid)

{

	DWORD dwPID=0;

	CODHDWord2K7Inspector* pWord2K7Inspector=(CODHDWord2K7Inspector*)lpVoid;

	Sleep(2000);

	dwPID=pWord2K7Inspector->GetPID();

	if(dwPID!=0)

	{

		if(EnumWindows(&EnumWindowsProc4Word2K7,(LPARAM)lpVoid))

		{

			DP((L"EnumWindows succeeded!"));

		}

	}



	return 0L;

}*/
#ifdef WSO2013
//op=1, backup org value and write a temp value
//op=2, restore  old value
BOOL CODHDWord2K7Inspector::ProcessOutlookTempFolderinReg(const int op, std::wstring& wstrRegValBackup)
{
#define REG_LENGTH 1024
#define OUTLOOK_SECURE_REGISTRY_NAME L"OutlookSecureTempFolder"
#define OUTLOOK_SECURE_REGISTRY_PATH L"Software\\Microsoft\\Office\\15.0\\Outlook\\Security"
	BOOL bRet = FALSE;
	DWORD dwType = REG_SZ;
	HKEY hKeyOtlSec = NULL;
	WCHAR   wzValue[REG_LENGTH] = {0};
	DWORD dwValueLen = REG_LENGTH*sizeof(WCHAR);

	wchar_t szValTempcbData[REG_LENGTH] = {0};

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, OUTLOOK_SECURE_REGISTRY_PATH, 0, KEY_ALL_ACCESS, &hKeyOtlSec);
	if (lResult == ERROR_SUCCESS && hKeyOtlSec != NULL)
	{
		switch(op)
		{
		case 1: 
			lResult = RegQueryValueExW(hKeyOtlSec, OUTLOOK_SECURE_REGISTRY_NAME, 0, &dwType, (LPBYTE)wzValue, &dwValueLen);
			if (ERROR_SUCCESS == lResult && dwValueLen != 0)
			{
				wstrRegValBackup = wzValue;
				swprintf_s(szValTempcbData,REG_LENGTH,L"%sTempValueForHDR", wzValue);
				lResult = RegSetValueExW(hKeyOtlSec, OUTLOOK_SECURE_REGISTRY_NAME, 0, REG_SZ,(const BYTE*)szValTempcbData, sizeof(WCHAR) * (wcslen(szValTempcbData)+1));
				if (lResult == ERROR_SUCCESS)
					bRet = TRUE;
			}
			break;
		case 2:
			lResult = RegSetValueExW(hKeyOtlSec, OUTLOOK_SECURE_REGISTRY_NAME, 0, REG_SZ,(const BYTE*)(wstrRegValBackup.c_str()), sizeof(WCHAR) * (wcslen(wstrRegValBackup.c_str())+1));
			if (lResult == ERROR_SUCCESS)
			{
				bRet = TRUE;
			}
			break;
		}
	
		RegCloseKey(hKeyOtlSec);
	}

	return bRet;
}
#endif
BOOL CODHDWord2K7Inspector::Inspect(std::wstring strSrcFileName,std::wstring strTempFileName)

{

	BOOL    bRet = TRUE;

    HRESULT hr   = S_OK;

	int		nStep=MAX_HDRDLG_PROGRESS;

	m_strTempFileName=strTempFileName;

	m_strSrcFileName =strSrcFileName;

	m_HasVersion = InspectVersionsByStream( strTempFileName );

	try

	{

		m_pApp=NULL;

		GetWordApp();

		if(m_pApp==NULL)

			return FALSE;



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

			//DWORD dwTID=0;

			//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckDialogOfWord2K7, this, 0, &dwTID);

			_beginthread(CODHDUtilities::ShowPasswordWindows, 0, NULL);
			
#ifdef WSO2013
			std::wstring wstrRegBackup;
			BOOL bOpRegRes = ProcessOutlookTempFolderinReg(1, wstrRegBackup);
#endif

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

					&covOptional,   // OpenAndRepair

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

					InspectSrvPolicy(spDocument);

					InspectContentType(spDocument);

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

					InspectCustomXML(spDocument);

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

#ifdef WSO2013
			if (bOpRegRes)
			{
				ProcessOutlookTempFolderinReg(2, wstrRegBackup);
			}
#endif
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

	}

	catch (_com_error e)

    {

        DP((L"[CheckTopInspector Exception] msg:: %s\n", e.ErrorMessage()));

    }



	if(nStep > 0)

		MakeStep(nStep);



    DP((L"End word inspect!\n"));



    return bRet;

}



//void CODHDWord2K7Inspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)

//{

//	CODHDInspector::GetResult(odhdCat,strResult);

//}



void CODHDWord2K7Inspector::GetResult(ODHDCATEGORY odhdCat,std::wstring & strResult)

{

	switch(odhdCat)

	{

	case ODHDCAT_WORD_XML:

		if(m_InspectorFail1==1)

		{

			strResult=L"This document is protected. To do HDR for this document, remove document protection, and re-attach this document.";

			return;

		}

	case ODHDCAT_WORD_HEADFOOT:

		if(m_InspectorFail2==1)

		{

			strResult=L"This document is protected. To do HDR for this document, remove document protection, and re-attach this document.";

			return;

		}

	case ODHDCAT_WORD_TEXT:

		if(m_InspectorFail3==1)

		{

			strResult=L"This document is protected. To do HDR for this document, remove document protection, and re-attach this document.";

			return;

		}

	default:

		CODHDInspector::GetResult(odhdCat,strResult);

		return;

	}

	return;

}



BOOL CODHDWord2K7Inspector::FilterByCategory(ODHDCATEGORY odhdCat, ODHDSTATUS odhdStatus)

{

	BOOL bFiltered = FALSE;



	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_WORD_COMMENTS && !(dwNeedCat & WordHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_WORD_PROP && !(dwNeedCat & WordHDCat_PROP))

			|| (odhdCat == ODHDCAT_WORD_XML && !(dwNeedCat & WordHDCat_XML))

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

			case ODHDCAT_WORD_XML:

				m_pHDRFile->m_dwFoundCatType |= WordHDCat_XML;

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



BOOL CODHDWord2K7Inspector::GetFilter(ODHDCATEGORY odhdCat)

{

	if (m_pHDRFile != NULL)

	{

		DWORD dwNeedCat = m_pHDRFile->m_dwNeedDetectCatType;

		if (dwNeedCat == 0

			|| (odhdCat == ODHDCAT_WORD_COMMENTS && !(dwNeedCat & WordHDCat_COMMENTS))

			|| (odhdCat == ODHDCAT_WORD_PROP && !(dwNeedCat & WordHDCat_PROP))

			|| (odhdCat == ODHDCAT_WORD_XML && !(dwNeedCat & WordHDCat_XML))

			|| (odhdCat == ODHDCAT_WORD_HEADFOOT && !(dwNeedCat & WordHDCat_HEADFOOT))

			|| (odhdCat == ODHDCAT_WORD_TEXT && !(dwNeedCat & WordHDCat_TEXT)))

		{

			return TRUE;

		}

	}



	return FALSE;

}



#endif // WSO2K7

