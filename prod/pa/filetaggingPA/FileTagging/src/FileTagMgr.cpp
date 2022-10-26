#include "StdAfx.h"
#include "FileTagMgr.h"
#include <Commctrl.h>
#include "utils.h"
#include <algorithm>
#include "FileTagViewDlg.h"
#include "PromptDlg.h"
#include "WorkThreads.h"
#include "nl_sysenc_lib.h"

#define  FILETAGGING_TAGNAME_MAX_LENGTH			1024
#define  FILETAGGING_TAGVALUE_MAX_LENGTH		4096

#define  FILETAGGING_SOURCE_VALUE				0x01
#define  FILETAGGING_DESTINATION_VALUE			0x02
#define  ADD_HC_TAG_FAIL_BOCLK_STATUS		1
#define  ADD_HC_TAG_FAIL_CONTINUE_STATUS	2
#define  ADD_HC_TAG_SUCCESS_STATUS			3
#define  ADD_HC_TAG_USERCANCEL_STATUS	    4

#define	 OBLIGATION_ENCRYPTION_SYNCTAGS_NAME	L"ENCRYPTION_SYNCTAGS"
#define	 SYNC_TAGS_NAME							L"Sync Tags"

extern HINSTANCE g_hPafDLL;

wchar_t g_szSeparateTagValues[2] = {';','\0'};

/***************************************
bool IsFolder(const wchar_t* pPath)
return true if the pPath is a folder
*****************************************/
static bool IsFolder(const wchar_t* pPath )
{
	if(!pPath)
		return false;
	if(memcmp((void*)pPath,(void*)L"http",8) == 0)	
	{
		return false;
	}

	DWORD dw = ::GetFileAttributesW(pPath);

	if( dw & FILE_ATTRIBUTE_DIRECTORY )
		return true;
	else
		return false;
}

CFileTagMgr::CFileTagMgr(void)
{
	m_pFileTagPanel = NULL;

	m_listAutoItems.clear();
	m_listManualItems.clear();

	m_bLastPA = TRUE;

	m_hInstance = g_hInstance;

	wchar_t szBuffer[1001] = {0};
	
	LoadString(m_hInstance, IDS_AUTOMATIC_TAG_OBLIGATION, szBuffer, 100);
	m_strAutoTag_Obligation = std::wstring(szBuffer);

	memset(szBuffer, 0, sizeof(szBuffer));
	LoadString(m_hInstance, IDS_MANUAL_TAG_OBLIGATION, szBuffer, 100);
	m_strManualTag_Obligation = std::wstring(szBuffer);

	m_strHierarchicalClassify_Obligation = L"OE_HIERARCHICAL_CLASSIFICATION";

	memset(szBuffer, 0, sizeof(szBuffer));
	LoadString(m_hInstance, IDS_FILETAGGING_HINT, szBuffer, 1000);
	m_strDefaultHint = std::wstring(szBuffer);

	m_strLastButtonText = L"";

	m_FileTag.SetFileTagMgr(this);
	m_pLogFunc = NULL;


	m_fpSetNext_OKCallBack = NULL;
	m_fpCreateMainFrame = NULL;
	m_fpGetParentWindow = NULL;
	m_fpShowHSCDlg = NULL;
	m_fpReleaseBuffer = NULL;

	m_hWnd = NULL;//added by kevin zhou 2009-1-17

	m_pSrcTagPairs = NULL;	// added by Tonny for office pep tagging feature.
	m_pDstTagPairs = NULL;
}

CFileTagMgr::~CFileTagMgr(void)
{
	if(m_pFileTagPanel)
		delete m_pFileTagPanel;
	// Note, we don't need to release it at all.
	if(m_pSrcTagPairs != NULL)	m_pSrcTagPairs = NULL;
	if(m_pDstTagPairs != NULL)	m_pDstTagPairs = NULL;
}

HINSTANCE CFileTagMgr::GetCurrentInstance()
{
	return m_hInstance;
}

DWORD CFileTagMgr::ShowFileTagPanel()
{
	if(!m_pFileTagPanel || !m_hParentWnd || !m_hWnd)
		return IDCANCEL; 

	if(m_fpSetNext_OKCallBack == NULL)
	{
		if(g_hPafDLL)
		{
			m_fpSetNext_OKCallBack = (SETNEXT_OKCCALLBACK)GetProcAddress(g_hPafDLL, "SetNEXT_OKCallBack");
		}
	}
	if(m_fpSetNext_OKCallBack)
	{
		m_fpSetNext_OKCallBack( CFileTagPanel::OnClickOK , this,m_hParentWnd) ;
	}
	//SetNEXT_OKCallBack( CFileTagPanel::OnClickOK ,NULL) ;

	std::wstring strText = GetOKButtonText();


	
	DWORD dwRet = IDOK;
	if(m_fpCreateMainFrame == NULL)
	{
		if(g_hPafDLL)
		{
			m_fpCreateMainFrame = (CREATEMAINFRAME)GetProcAddress(g_hPafDLL, "CreateMainFrame");
		}
	}
	if(m_fpCreateMainFrame)
	{
		dwRet = m_fpCreateMainFrame(m_hWnd,TRUE , NULL, strText.c_str(), pafUI::BT_ENABLE, (HWND)this, NULL, NULL) ;//kevin 2008-12-23
	}
//	DWORD dwRet = CreateMainFrame(m_hWnd,TRUE , NULL, strText.c_str(), pafUI::BT_ENABLE, m_hParentWnd) ;
	
	return dwRet;
}
PABase::PA_STATUS CFileTagMgr::DoFileTaggingAssistant2(PABase::PA_PARAM& _iParam, const HWND _hParentWnd,
													  vector<pair<wstring,wstring>>* pSrcTagsPair,vector<pair<wstring,wstring>>* pDstTagsPair)
{
	m_pSrcTagPairs = pSrcTagsPair;
	m_pDstTagPairs = pDstTagsPair;
	return DoFileTaggingAssistant(_iParam,_hParentWnd);
}

PABase::PA_STATUS CFileTagMgr::DoFileTaggingAssistant(PABase::PA_PARAM& _iParam, const HWND _hParentWnd, bool bEncryptBeforeTag,vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo)
{
	if(!m_pFileTagPanel)
	{
		m_pFileTagPanel = new CFileTagPanel;
		m_pFileTagPanel->SetFileTagMgr(this);
	}
	if(!m_pFileTagPanel)
		return PA_ERROR;

	m_hParentWnd = _hParentWnd;


	m_FileTag.SetFileTagMgr(this);

	m_pLogFunc = _iParam.fLog;//kevin 2008-9-18
	m_lpLogCtx = _iParam.lpLogCtx;
	m_bLastPA = _iParam._bIsLastPA;
	m_strLastButtonText = _iParam._strLastButtonName;
	m_action = _iParam._action;//kevin 2008-10-23
	m_nIndexItem = 0;

	DP((L"PA_PARAM, lastPA:%d, lastButtonText:%s", m_bLastPA, m_strLastButtonText.c_str()));

	ParseParameters(_iParam, bEncryptBeforeTag);

	DisplayAllFileTaggingItems();//Display all the items after parse, can be remove in release version

	if(!DoAutomaticTagging())//do automatic tagging
	{
		if(m_pFileTagPanel)
		{
			if(m_pFileTagPanel->m_hWnd && ::IsWindow(m_pFileTagPanel->m_hWnd))
			{
				//	SendMessage(m_pFileTagPanel->m_hWnd, WM_CLOSE, 0, 0);
				m_pFileTagPanel->DestroyWindow();
			}


			delete m_pFileTagPanel;
			m_pFileTagPanel = NULL;
			m_hParentWnd = NULL;
			m_hWnd = NULL;
		}
		return PA_ERROR;
	}

	//do HSC tag
	OutputDebugStringW(L"Begin Do Hierarchical Structure of Classification file tag\n");
	int nHSCRet = PA_SUCCESS;
	if (PA_SUCCESS != (nHSCRet = DoHierarchicalStructureClassification(_hParentWnd,pVecHCInfo)))
	{
		OutputDebugStringW(L"DoFileTaggingAssistant end with DoHierarchicalStructureClassification\n");
		return nHSCRet;
	}
	
	//do manual tag
	DWORD dwRet = PA_SUCCESS;
	OutputDebugStringW(L"Begin Do manual file tag\n");
	if (!RemoveUnsupportFileTypeItem())
	{
		return PA_ERROR;
	}
		
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr;
	for(itr = m_listManualItems.begin(); itr != m_listManualItems.end(); itr++)
	{
		smart_ptr<FILETAG_ITEM> spItem = *itr;

		DWORD dwRetGetFileAttr = ::GetFileAttributesW(spItem->strFile.c_str());

		if((m_pSrcTagPairs == NULL && m_pDstTagPairs == NULL) && 0xFFFFFFFF == dwRetGetFileAttr)
		{//No this file
//			AddLog();
	//		DP((L"File not exist(DoFileTaggingAssistant()). %s", spItem->strFile.c_str()));
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"identifier");
			AddLog(szIdentifier, GetResString(IDS_FILETAG_DISPLAYNAME), GetResString(IDS_FILE_NOT_EXIST), GetResString(IDS_FILETAG_DESCRIPTION), GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);

			
			PopupMessageBox(IDS_FILE_NOT_EXIST, spItem->strFile.c_str(), m_hParentWnd);//FIX BUG292
			m_nIndexItem++;
		//	continue;
			dwRet = PA_SUCCESS;
			break;//kevin 2008-10-15
		}
		else
		{
			if(m_action == PABase::AT_MOVE && IsFolder(spItem->strFile.c_str()))//fix bug8415, Kevin Zhou 2009-1-20
			{
				DP((L"FileTagging, this is a folder. doesn't need to do tagging: %s\r\n", spItem->strFile.c_str()));
				dwRet = PA_SUCCESS;
				break;
			}

			//create tagging panel
			HWND hTempWnd = NULL;
			if(m_fpGetParentWindow == NULL)
			{
				if(g_hPafDLL)
				{
					m_fpGetParentWindow = (GETPARENTWINDOW)GetProcAddress(g_hPafDLL, "GetParentWindow");
				}
			}
			if(m_fpGetParentWindow)
			{
				m_fpGetParentWindow(hTempWnd,m_hParentWnd);
			}

			if(!hTempWnd)
			{
				DP((L"Failed to create temp parent window in PAF, try to use the window of current process.\r\n"));
				hTempWnd = m_hParentWnd;
			}
			if(!hTempWnd)
			{
				DP((L"the parent window of PA(PAF container) is emtpy\r\n"));
				return PA_ERROR;
			}

			m_hWnd = m_pFileTagPanel->Create(hTempWnd);

			if(!m_hWnd)
				return PA_ERROR;

			m_FileTag.SetParentWnd(m_hWnd);

			DP((L"window of current process:%d, parent window of PA(the handle of PAF):%d, FileTagging Panel Window: %d", m_hParentWnd, hTempWnd, m_hWnd));

			
			FILETAG_ITEM item;

			item.strFile = spItem->strFile;
			item.strHint = spItem->strHint;
			item.listTags = spItem->listTags;
			item.strDisplayFile = spItem->strDisplayFile;
			if (m_pFileTagPanel->PrepareItem(item))
			{
				dwRet = ShowFileTagPanel();
				if (dwRet==IDOK)
				{
					dwRet = PA_SUCCESS;
				}
				else if (dwRet==IDCANCEL)
				{
					dwRet = PA_USER_CANCEL;
				}
				else
				{
					dwRet = (DWORD)PA_ERROR;
				}
			}
		
			else
				dwRet = PA_SUCCESS;
			break;
		}
	}
	
	if(m_pFileTagPanel)
	{
		if(m_pFileTagPanel->m_hWnd && ::IsWindow(m_pFileTagPanel->m_hWnd))
		{
		//	SendMessage(m_pFileTagPanel->m_hWnd, WM_CLOSE, 0, 0);
			m_pFileTagPanel->DestroyWindow();
		}
		

		delete m_pFileTagPanel;
		m_pFileTagPanel = NULL;
		m_hParentWnd = NULL;
		m_hWnd = NULL;
	}

	if(dwRet != PA_SUCCESS)
	{
		PABase::ATTRIBUTELIST listAttrs;
		AddLog(L"identifer", GetResString(IDS_FILETAG_DISPLAYNAME), L"", GetResString(IDS_FILETAG_DESCRIPTION), GetResString(IDS_FILETAG_CANCELTAG), listAttrs);
	}

	DP((L"FileTagging End: %d\r\n", dwRet));
	return dwRet;
}

std::wstring CFileTagMgr::GetOKButtonText()
{
	wchar_t szBuffer[101] = {0};
	if(m_listManualItems.size() > 1 && m_nIndexItem < (int)m_listManualItems.size() - 1 && m_hParentWnd)
	{
		memset(szBuffer, 0, sizeof(szBuffer));
		LoadString(m_hInstance, IDS_NEXT, szBuffer, 100);
	}
	else if(m_hParentWnd)
	{
		memset(szBuffer, 0, sizeof(szBuffer));
		if(m_bLastPA)
		{
			if(m_strLastButtonText.length() > 0)
				memcpy(szBuffer, m_strLastButtonText.c_str(), (m_strLastButtonText.length() > 100?100:m_strLastButtonText.length()) * sizeof(wchar_t));
			else
				LoadString(m_hInstance, IDS_OK, szBuffer, 100);
		}
		else
			LoadString(m_hInstance, IDS_NEXT, szBuffer, 100);
	}

	return std::wstring(szBuffer);
}

BOOL CFileTagMgr::NextFileTaggingItem(smart_ptr<FILETAG_ITEM>& spItem, BOOL& bNextItem,CFileTagPanel*& pPanel)
{
	if(m_nIndexItem < (int)(m_listManualItems.size() - 1))
	{
		m_nIndexItem++;

		bNextItem = GetCurrentItem(spItem);
	}
	else
		bNextItem = FALSE;

	pPanel = m_pFileTagPanel;

	return true;
}

BOOL CFileTagMgr::GetCurrentItem(smart_ptr<FILETAG_ITEM>& spItem)
{
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr;
	int nIndex = 0;
	
	for(itr = m_listManualItems.begin(); itr != m_listManualItems.end(); itr++)
	{
		if(nIndex == m_nIndexItem)
		{
			spItem = *itr;
			return TRUE;
		}
		nIndex++;
	}
	return FALSE;
}

BOOL CFileTagMgr::DoCurrentManualTagging()
{
	if(!m_pFileTagPanel)
		return FALSE;

	BOOL bTag = TRUE;
	
	if(m_nIndexItem >= 0 && m_nIndexItem < (int)m_listManualItems.size())
	{
		smart_ptr<FILETAG_ITEM> spItem;
		if(GetCurrentItem(spItem))
		{
			std::list<smart_ptr<FILETAG_PAIR>> listValues;

			if(!m_pFileTagPanel->GetTagValues(&listValues))
				return FALSE;
			
			DP((L"Start to tag(manual) the FILE_ITEM, btTarget:%d, file: %s, temp file: %s \r\n", spItem->btTarget, spItem->strFile.c_str(), spItem->strTempFilePath.c_str()));
			if(m_action == PABase::AT_COPY || PABase::AT_SAVEAS == m_action )//Kevin 2008-10-24
			{
				if( spItem->btTarget & FILETAGGING_SOURCE_VALUE )//tag source file 
				{
#if 1	// add code for office pep , only want to get manu tag.
					if(m_pSrcTagPairs != NULL && m_pDstTagPairs != NULL)
					{
						// add for get manu tag from tag library panel
						std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;
						for(itr = listValues.begin(); itr != listValues.end(); itr++)
						{
							smart_ptr<FILETAG_PAIR> spTag = *itr;
							m_pSrcTagPairs->push_back(pair<wstring,wstring>(spTag->strTagName,spTag->strTagValue));
						}
						if(!m_pSrcTagPairs->empty())	bTag = TRUE;
					}
					else	bTag = AddTagEx(&listValues, spItem->strFile.c_str(), this);
#else 
					bTag = AddTagEx(&listValues, spItem->strFile.c_str(), this);
#endif
					
				}
				if( bTag && (spItem->btTarget & FILETAGGING_DESTINATION_VALUE) )
				{
#if 1	// add code for office pep , only want to get manu tag.
					if(m_pSrcTagPairs != NULL && m_pDstTagPairs != NULL)
					{
						// add for get manu tag from tag library panel
						std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;
						for(itr = listValues.begin(); itr != listValues.end(); itr++)
						{
							smart_ptr<FILETAG_PAIR> spTag = *itr;
							m_pDstTagPairs->push_back(pair<wstring,wstring>(spTag->strTagName,spTag->strTagValue));
						}
						if(!m_pDstTagPairs->empty())	bTag = TRUE;
					}
					else	bTag = AddTagEx(&listValues, spItem->strTempFilePath.c_str(), this);
#else 
					bTag = AddTagEx(&listValues, spItem->strTempFilePath.c_str(), this);
#endif
				}
			}
			else if(m_action == PABase::AT_SENDMAIL)//tag both original file and attachment file. kevin 2008-12-5
			{
				bTag = AddTagEx(&listValues, spItem->strFile.c_str(), this, spItem->strTagOnError.c_str(), spItem->strMessageForBlockAction.c_str());
				if(bTag && spItem->strDisplayFile != spItem->strFile)
					bTag = AddTagEx(&listValues, spItem->strDisplayFile.c_str(), this,spItem->strTagOnError.c_str(), spItem->strMessageForBlockAction.c_str());
			}
			else
			{

#if 1	// add code for office pep , only want to get manu tag.
				if(m_pSrcTagPairs != NULL && m_pDstTagPairs != NULL)
				{
					// add for get manu tag from tag library panel
					std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;
					for(itr = listValues.begin(); itr != listValues.end(); itr++)
					{
						smart_ptr<FILETAG_PAIR> spTag = *itr;
						m_pSrcTagPairs->push_back(pair<wstring,wstring>(spTag->strTagName,spTag->strTagValue));
					}
					if(!m_pSrcTagPairs->empty())	bTag = TRUE;
				}
				else	bTag = AddTagEx(&listValues, spItem->strFile.c_str(), this);
#else 
				bTag = AddTagEx(&listValues, spItem->strFile.c_str(), this);
#endif
			}
		}
	}
	return bTag ;
}

int CFileTagMgr::ParseParameters(PABase::PA_PARAM &_iParam, bool bEncryptBeforeTag)
{
	m_listAutoItems.clear();
	m_listManualItems.clear();
	m_listHierarchicalClassification.clear();

	PABase::OBJECTINFOLIST& list = _iParam.objList;

	PABase::OBJECTINFOLIST::iterator itr;

	DP((L"Start ParseParameters, OBJECTINFO count: %d", _iParam.objList.size()));

	for(itr = list.begin(); itr != list.end(); itr++)
	{
		PABase::OBJECTINFO& infoObj = *itr;//An object means a file here

		std::wstring strFileName, strDisplayName;

		DP((L" [ParseParameters]PABase::OBJECTINFO: DisplayName=%s, DestName=%s, SrcName=%s, TempName=%s, RetName=%s",
			itr->strDisplayName.c_str(), itr->strDestName.c_str(), itr->strSrcName.c_str(), itr->strTempName.c_str(), itr->strRetName));

		if(infoObj.bFileNameChanged)//if file name has been changed.
			strFileName = infoObj.strRetName;
		else
			strFileName = infoObj.strSrcName;

		if(infoObj.strDisplayName.length() > 0)
			strDisplayName = infoObj.strDisplayName;
		else
			strDisplayName = strFileName;

		DP((L"DisplayName:%s, FileName:%s, obligation count:%d\r\n", strDisplayName.c_str(), strFileName.c_str(), infoObj.obList.size()));

		if(_iParam._action == PABase::AT_SENDMAIL )//for OE, PDF need reattach
		{
			if(infoObj.strSrcName.length() > 3 && 
				_wcsicmp(infoObj.strSrcName.substr(infoObj.strSrcName.length() - 3, 3).c_str(), L"pdf") == 0)
			{//need re-attach
				std::wstring strDir = infoObj.strTempName;
				PABase::OBLIGATION ob;
				if(GetTheFirstOBForTagging(infoObj.obList, ob) && CheckAndCreateRandTemp(strFileName, strDir))
				{
					std::wstring strName;
					if(GetFileNameFromPath(strDisplayName.c_str(), strName))//fix bug340
					{
						std::wstring strNewFileName = strDir + strName;
						if(CopyFileW(strFileName.c_str(), strNewFileName.c_str(), FALSE))
						{
							DP((L"Copy file(for PDF) successfully: srcfile: %s, new file: %s", strFileName.c_str(), strNewFileName.c_str()));
							/*
							Modified by chellee on 14/10/08 ;6:32
							mark Code:infoObj.strRetName = strNewFileName;
							*/
							::wcsncpy_s( infoObj.strRetName, MAX_PATH,strNewFileName.c_str(), _TRUNCATE ) ;
							//-----------------------------------------------------------------
							strFileName = strNewFileName;
							infoObj.bFileNameChanged = TRUE;	
						}
						else
							DP((L"CopyFile failed. src: %s, dest: %s\r\n", strFileName.c_str(), strNewFileName.c_str()));
					}
					else
						DP((L"GetFileNameFromPath failed: file: %s\r\n", strFileName.c_str()));
				}
				else
					DP((L"Create temp directory failed: %s, temp dir: %s\r\n", strFileName.c_str(), strDir.c_str()));
			}
		}

		std::wstring strNewFileName_COPYMOVE;
		if(_iParam._action == PABase::AT_COPY)//copy
		{
			PABase::OBLIGATION ob;
			if(GetTheFirstOBForTagging(infoObj.obList, ob))
			{
				std::wstring strDir = infoObj.strTempName;
				
				if(CheckAndCreateRandTemp(strFileName, strDir))
				{
					std::wstring strName;
					if(GetFileNameFromPath(strDisplayName.c_str(), strName))
					{
						strNewFileName_COPYMOVE = strDir + strName;
						BOOL bCopyRet = FALSE;
						if (SE_IsEncrypted(strFileName.c_str()))
						{
							bCopyRet = SE_CopyEncryptedFile(strFileName.c_str(), strNewFileName_COPYMOVE.c_str());
							DP((L"Copy with SE_CopyEncryptedFile, src: %s, dest: %s\n", strFileName.c_str(), strNewFileName_COPYMOVE.c_str()));
						}
						else
						{
							bCopyRet = CopyFileW(strFileName.c_str(), strNewFileName_COPYMOVE.c_str(), FALSE);
							
							if (bCopyRet && bEncryptBeforeTag)
							{
								std::set<std::wstring> FilterTagsName;

								GetFileterTagsName(infoObj.obList, FilterTagsName);

								std::list<smart_ptr<FILETAG_PAIR>> TagList;

								GetFileTag()->GetAllTags(strFileName.c_str(), &TagList);
								
								if (!FilterTagsName.empty())
								{
									for (std::list<smart_ptr<FILETAG_PAIR>>::const_iterator ci = TagList.begin(); ci != TagList.end();)
									{
										std::wstring TagName = (*ci)->strTagName;
										transform(TagName.begin(), TagName.end(), TagName.begin(), towupper);

										if (FilterTagsName.find(TagName) == FilterTagsName.end())
										{
											ci = TagList.erase(ci);
										}
										else
										{
											ci++;
										}
									}
								}
								
								if (SE_EncryptFileForce(strNewFileName_COPYMOVE.c_str()))
								{
									GetFileTag()->AddTag(&TagList, strNewFileName_COPYMOVE.c_str());
								}
							}
							
							DP((L"Copy with CopyFileW, src: %s, dest: %s\n", strFileName.c_str(), strNewFileName_COPYMOVE.c_str()));
						}
						
						if (bCopyRet)
						{
							DP((L"Copy file successfully: srcfile: %s, new file: %s, action:%d", strFileName.c_str(), strNewFileName_COPYMOVE.c_str(), _iParam._action));
						//	infoObj.strRetName = strNewFileName_COPYMOVE;
							::wcsncpy_s( infoObj.strRetName, MAX_PATH,strNewFileName_COPYMOVE.c_str(), _TRUNCATE ) ;
							infoObj.bFileNameChanged = TRUE;	
						}
					}
				}
			}
			else
				DP((L"No Tagging obligation, action:%d\r\n", _iParam._action));
			
		}
		else if ( PABase::AT_SAVEAS == _iParam._action )
		{
			strNewFileName_COPYMOVE = infoObj.strDestName;
		}


		//extract automatic-tag obligations, and push into m_listAutoItems.
		std::list<smart_ptr<FILETAG>> listFileTagItem_AutoTag;
		listFileTagItem_AutoTag.clear();
		std::map<std::wstring, std::wstring> mapProps;
		mapProps.clear();
		if(HasAutomaticTag_Obligation(infoObj, listFileTagItem_AutoTag, mapProps))
		{
			MergeAutomaticTag_Obligation(strFileName.c_str(), strDisplayName.c_str(), listFileTagItem_AutoTag, strNewFileName_COPYMOVE.c_str(), mapProps);
		}

		//extract manual-tag obligations, and push into m_listManulItems.
		std::list<smart_ptr<FILETAG>> listFileTagItem_ManulTag;
		listFileTagItem_ManulTag.clear();
		mapProps.clear();
		if(HasManualTag_Obligation(infoObj, listFileTagItem_ManulTag, mapProps))
		{
			MergeManualTag_Obligation(strFileName.c_str(), strDisplayName.c_str(), listFileTagItem_ManulTag, strNewFileName_COPYMOVE.c_str(), mapProps);
		}

		//extract Hierarchical Structure of Classification obligations,
		GetHSCTag_Obligation(infoObj, strFileName.c_str(), strDisplayName.c_str());
		
	}

	return 0;
}

BOOL CFileTagMgr::DoAutomaticTagging()
{
	//do automatic-tag obligations
	LPAUTOTHREADPARAM pParam = new AUTOTHREADPARAM;
	if(!pParam)
		return FALSE;
	pParam->pMgr = this;
	
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr;
#if 1
	if(m_pSrcTagPairs != NULL && m_pDstTagPairs != NULL)
	{
		// add for get auto tag from tag library panel
		for(itr = m_listAutoItems.begin(); itr != m_listAutoItems.end(); itr++)
		{
			smart_ptr<FILETAG_ITEM> spItem = *itr;
			
			std::list<smart_ptr<FILETAG>>::iterator itrt = spItem->listTags.begin();
			for(; itrt != spItem->listTags.end(); itrt++)
			{
				smart_ptr<FILETAG> spTag = *itrt;
				//if(spItem->btTarget & FILETAGGING_DESTINATION_VALUE)
				//{
					m_pDstTagPairs->push_back(pair<wstring,wstring>(spTag->strTagName,*(spTag->listValues.begin())));
				//}
				//else
				//{
					m_pSrcTagPairs->push_back(pair<wstring,wstring>(spTag->strTagName,*(spTag->listValues.begin())));
				//}
			}
		}
		if(!m_pDstTagPairs->empty() || !m_pSrcTagPairs->empty())	return TRUE;
	}
#endif
	
	BOOL bSuccess = TRUE;
	for(itr = m_listAutoItems.begin(); itr != m_listAutoItems.end(); itr++)
	{
		smart_ptr<FILETAG_ITEM> spItem = *itr;

		if(m_action == PABase::AT_MOVE && IsFolder(spItem->strFile.c_str()))//fix bug8415, Kevin Zhou 2009-1-20
		{
			DP((L"FileTagging, this is a folder. doesn't need to do tagging: %s\r\n", spItem->strFile.c_str()));
			bSuccess = FALSE;
			break;
		}

		if(m_action == PABase::AT_COPY || PABase::AT_SAVEAS == m_action )
		{
			if( spItem->btTarget & FILETAGGING_SOURCE_VALUE )//tag source file 
			{
				pParam->pszFileName = spItem->strFile.c_str();
				pParam->pList = &(spItem->listTags);
				if(!DoAutoTag(pParam, m_hParentWnd))
				{
					bSuccess = FALSE;
					break;
				}
			}
			if( spItem->btTarget & FILETAGGING_DESTINATION_VALUE )//tag target file 
			{
				pParam->pszFileName = spItem->strTempFilePath.c_str();
				pParam->pList = &(spItem->listTags);
				if(!DoAutoTag(pParam, m_hParentWnd))
				{
					bSuccess = FALSE;
					break;
				}
			}
			continue;
		}
		else if(m_action == PABase::AT_SENDMAIL)//tag both original file and attachment file. kevin 2008-12-5
		{
			pParam->pszFileName = spItem->strFile.c_str();
			pParam->pList = &(spItem->listTags);

			//check file type
			if (!FileTypeSupport(spItem->strFile.c_str()))
			{
				if (_wcsicmp(spItem->strTagOnUnSupportFileType.c_str(), ERROR_ACTION_BLOCK) == 0 )
				{
					DP((L"break in Auto tag file type error, file=%s\n", spItem->strFile.c_str()));
					ShowTagErrorBlockMessage(m_hParentWnd, spItem->strFile.c_str(), spItem->strMessageForBlockAction.c_str());
					bSuccess = FALSE;
					break;
				}	
				else
				{
					DP((L"Continue in Auto tag file type error, file=%s\n", spItem->strFile.c_str()));
					continue;
				}
			}

			//tag source or dest or both file tag by obligation
			bool bTaged=false;
			if( spItem->btTarget & FILETAGGING_DESTINATION_VALUE )//tag FILETAGGING_DESTINATION_VALUE file 
			{
				pParam->pszFileName = spItem->strFile.c_str();
				if (!DoAutoTag(pParam, m_hParentWnd, spItem->strTagOnUnSupportFileType.c_str(), spItem->strMessageForBlockAction.c_str()))
				{
					bSuccess = FALSE;
					break;
				}
				bTaged=true;
			}
			if( spItem->btTarget & FILETAGGING_SOURCE_VALUE )//tag FILETAGGING_SOURCE_VALUE file 
			{
				if(!(bTaged&&0 ==  _wcsicmp(spItem->strDisplayFile.c_str(), spItem->strFile.c_str())))
				{
					pParam->pszFileName = spItem->strDisplayFile.c_str();
					pParam->pList = &(spItem->listTags);
					if (!DoAutoTag(pParam, m_hParentWnd, spItem->strTagOnUnSupportFileType.c_str(), spItem->strMessageForBlockAction.c_str()))
					{
						bSuccess = FALSE;
						break;
					}
				}
			}
		}
		else
		{
			pParam->pszFileName = spItem->strFile.c_str();
			pParam->pList = &(spItem->listTags);

			if(!DoAutoTag(pParam, m_hParentWnd))
			{
				bSuccess = FALSE;
				break;
			}
		}
	}
	
	
	if(pParam)
		delete pParam;

	return bSuccess;
}

BOOL CFileTagMgr::HasAutomaticTag_Obligation(PABase::OBJECTINFO& obj, OUT std::list<smart_ptr<FILETAG>>& listFileTagItem, OUT std::map<std::wstring, std::wstring>& mapProps)
{
	PABase::OBLIGATIONLIST::iterator itr;

	BOOL bHasAuto_Obligation = FALSE;

	std::list<std::wstring> listTagName;
	listTagName.clear();
	byte btTarget = 0;
	std::wstring strDescription;
	strDescription.clear();
	for(itr = obj.obList.begin(); itr != obj.obList.end(); itr++)
	{
		PABase::OBLIGATION& ob = *itr;
		if(Is_IndicatedObligation(ob, m_strAutoTag_Obligation.c_str()))
		{
			std::wstring strTagName = GetTagNameInObligation(ob);
			wchar_t szBuffer[501] = {0};
			memcpy(szBuffer, strTagName.c_str(), strTagName.length() > 500? 500*sizeof(wchar_t):strTagName.length() * sizeof(wchar_t));
			_wcslwr_s(szBuffer, wcslen(szBuffer) + 1);

			if(listTagName.end() == std::find(listTagName.begin(), listTagName.end(), szBuffer))
			{
				smart_ptr<FILETAG> spTag(new FILETAG);
				spTag->strTagName = strTagName;
				GetTagValuesInObligation(ob, spTag->listValues);
				spTag->strTagType = m_strAutoTag_Obligation;
				listFileTagItem.push_back(spTag);

				listTagName.push_back(szBuffer);
				bHasAuto_Obligation = TRUE;
			}
			

			if(strDescription.length() <= 0)//use the description from first obligation. Kevin Zhou2008-9-12
				strDescription = GetDescriptionInObligation(ob);

			//Merge the "target" property of multiple obligations. Added by Kevin Zhou 2008-10-23 [begin]
			std::wstring strTarget = GetTargetInObligation(ob);
			if(_wcsicmp(strTarget.c_str(), GetResString(IDS_TARGET_OB_SOURCE).c_str()) == 0)
			{
				btTarget |= FILETAGGING_SOURCE_VALUE;
			}
			else if(_wcsicmp(strTarget.c_str(), GetResString(IDS_TARGET_OB_DESTINATION).c_str()) == 0)
			{
				btTarget |= FILETAGGING_DESTINATION_VALUE;
			}
			else if(_wcsicmp(strTarget.c_str(), GetResString(IDS_TARGET_OB_BOTH).c_str()) == 0)
			{
				// this only for OE ,WDE don't support both tag
				if(m_action==PABase::AT_SENDMAIL)
				{
					btTarget |= FILETAGGING_DESTINATION_VALUE|FILETAGGING_SOURCE_VALUE;
				}
			}
			//[end]

			mapProps[FILETAGGING_TAGONERROR] = GetValueFromObligation(ob, FILETAGGING_TAGONERROR);
			mapProps[FILETAGGING_TAGONUNSUPPORTTYPE] = GetValueFromObligation(ob, FILETAGGING_TAGONUNSUPPORTTYPE);
			mapProps[FILETAGGING_BLOCK_MESSAGE] = GetValueFromObligation(ob, FILETAGGING_BLOCK_MESSAGE);
		}
	}
	mapProps[FILETAGGING_DESCRIPTION_OB] = strDescription;
	wchar_t buffer[50] = {0};
	_snwprintf_s(buffer, 50, _TRUNCATE, L"%d", btTarget);
	mapProps[FILETAGGING_TARGET_OB] = std::wstring(buffer);

	return bHasAuto_Obligation;
}

void CFileTagMgr::GetHSCInfoFromObligation(PABase::OBLIGATION* pHSCObligation, HSC_ITEM& hscItem)
{
	//get obligatin values
	std::list<std::wstring> lstValues;
	std::list<PABase::ATTRIBUTE>::iterator itAttribute = pHSCObligation->attrList.begin();
	while (itAttribute != pHSCObligation->attrList.end())
	{
		lstValues.push_back(itAttribute->strValue);
		itAttribute++;
        NLPRINT_DEBUGVIEWLOG(L"%s", (lstValues.back()).c_str());
	}

    //get hsc infor
	std::list<std::wstring>::iterator itValue = lstValues.begin();
	while (itValue != lstValues.end())
	{
		if (_wcsicmp(itValue->c_str(), L"Title") == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strHint = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), L"Target") == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strTagTarget = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), FILETAGGING_TAGONERROR) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strTagOnError = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), FILETAGGING_TAGONUNSUPPORTTYPE) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strTagOnUnSupportFileType = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), FILETAGGING_BLOCK_MESSAGE) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strMessageForBlockAction = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), FILETAGGING_XML_FORMAT) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strClassificationXml = *itValue;
			}
		}
        else if (_wcsicmp(itValue->c_str(), OBCHECKMODIFIEDTIMENAME) == 0)
        {
            // Check file last modify time and record time interval
            itValue++;
            if (itValue != lstValues.end())
            {
                if (_wcsicmp(itValue->c_str(), OBCHECKMODIFIEDTIMEVALUE) == 0)
                {
                    hscItem.dwLastModifyTimeInterval = g_kdwLastModifyTimeInterval;
                    if (!GetFileTimeByType(hscItem.strDisplayFile, 1, &hscItem.stuFileTime))
                    {
                        hscItem.stuFileTime.dwHighDateTime = 0;
                        hscItem.stuFileTime.dwLowDateTime = 0;
                    }
                }
            }
            DP((L"File:[%s]: TimeInter:%d, CurrentTime:[%u-%u]\n", hscItem.strDisplayFile.c_str(), hscItem.dwLastModifyTimeInterval, hscItem.stuFileTime.dwHighDateTime, hscItem.stuFileTime.dwLowDateTime));
        }
		else if (_wcsicmp(itValue->c_str(), FILETAGGING_LOGID) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strLogID = *itValue;
			}
		}
		else if (_wcsicmp(itValue->c_str(), OEACTIONFORHSC_NAME ) == 0)
		{
			itValue++;
			if (itValue != lstValues.end())
			{
				hscItem.strOEAction = *itValue;
				DP((L"testx get oe customer action for hsc:%s\n", hscItem.strOEAction.c_str() )); 
			}
		}

		itValue++;
	}
}

int CFileTagMgr::GetHSCTag_Obligation(IN PABase::OBJECTINFO& obj, LPCWSTR pszFileName, LPCWSTR pszDisplayName)
{
	//get first HSC obligation
	PABase::OBLIGATION* pHSCObligation = NULL;
	PABase::OBLIGATIONLIST::iterator itObligation = obj.obList.begin();
	while (itObligation != obj.obList.end())
	{
		if (Is_IndicatedObligation(*itObligation, m_strHierarchicalClassify_Obligation.c_str()))
		{
			pHSCObligation = &(*itObligation);
		}
		itObligation++;
	}

	//get HSC obligation info
	if (NULL != pHSCObligation)
	{
		HSC_ITEM* pHscItem= new HSC_ITEM();
        pHscItem->strFile = pszFileName;
        pHscItem->strDisplayFile = pszDisplayName;
        pHscItem->dwLastModifyTimeInterval = 0;
        pHscItem->stuFileTime.dwHighDateTime = 0;
        pHscItem->stuFileTime.dwHighDateTime = 0;
		pHscItem->strLogID = L"";
        GetHSCInfoFromObligation(pHSCObligation, *pHscItem);

		if (!pHscItem->strClassificationXml.empty())
		{
			m_listHierarchicalClassification.push_back(smart_ptr<HSC_ITEM>(pHscItem));
		}
	}

	return 1;
	
}

void CFileTagMgr::MergeAutomaticTag_Obligation(LPCWSTR pszFileName, LPCWSTR pszDisplayName, std::list<smart_ptr<FILETAG>>& listFileTagItem, LPCWSTR pszTempFile, std::map<std::wstring, std::wstring>& mapProps)
{
	if(!pszFileName || !pszDisplayName)
		return;

	smart_ptr<FILETAG_ITEM> spExistItem;
	BOOL bExists = GetFileTagItemByFileName(pszFileName, m_listAutoItems, spExistItem);

	byte btTarget = 0;
	std::wstring strTarget = mapProps[FILETAGGING_TARGET_OB];
	btTarget = (byte)_wtoi(strTarget.c_str());
	std::wstring strDescription = mapProps[FILETAGGING_DESCRIPTION_OB];
	if(bExists)
	{
		spExistItem->btTarget |= btTarget;//kevin 2008-10-23
		std::list<smart_ptr<FILETAG>>::iterator f_itr;
		for(f_itr = listFileTagItem.begin(); f_itr != listFileTagItem.end(); f_itr++)//all items which needs to be merged into m_listAutoItems
		{
			smart_ptr<FILETAG> sp_FileTagItem = *f_itr;//need to be merged

			std::list<smart_ptr<FILETAG>>::iterator itr;
			BOOL bTagExists = FALSE;
			for(itr = spExistItem->listTags.begin(); itr != spExistItem->listTags.end(); itr++)
			{
				smart_ptr<FILETAG> sp_FileTag = *itr;// the existing tag 
				if(0 == _wcsicmp(sp_FileTag->strTagName.c_str(), sp_FileTagItem->strTagName.c_str()))//ignore if current tag name has existed 
				{
					bTagExists = TRUE;
					break;
				}

			}
			if(!bTagExists)
			{
				spExistItem->listTags.push_back(sp_FileTagItem);//add the tag into the existing file
				DP( (L"(Automatic Tag Obligation) File Exists: Add a new tag. File name: %s, tag name:%s\r\n", pszFileName, sp_FileTagItem->strTagName.c_str()));
			}
			else
				DP( (L"(Automatic Tag Obligation) File Exists: Duplicate tags. File name: %s, tag name:%s\r\n", pszFileName, sp_FileTagItem->strTagName.c_str()));

		}
	}
	else
	{
		smart_ptr<FILETAG_ITEM> spItem(new FILETAG_ITEM);
		spItem->strFile = pszFileName;
		spItem->strDisplayFile = pszDisplayName;
		spItem->strHint = strDescription;
		spItem->listTags = listFileTagItem;
		spItem->btTarget = btTarget;//kevin 2008-10-23
		if(pszTempFile)
			spItem->strTempFilePath = pszTempFile;

		spItem->strTagOnError = mapProps[FILETAGGING_TAGONERROR];
		spItem->strTagOnUnSupportFileType = mapProps[FILETAGGING_TAGONUNSUPPORTTYPE];
		spItem->strMessageForBlockAction = mapProps[FILETAGGING_BLOCK_MESSAGE];

		m_listAutoItems.push_back(spItem);
		DP( (L"(Automatic Tag Obligation) Add a new FILETAG_ITEM for automatic tag obligation. File name: %s\r\n", pszFileName));
	}
	
	
}


//used to parse manual-tag obligation
BOOL CFileTagMgr::HasManualTag_Obligation(PABase::OBJECTINFO& obj, OUT std::list<smart_ptr<FILETAG>>& listFileTagItem, OUT std::map<std::wstring, std::wstring>& mapProps)
{
	PABase::OBLIGATIONLIST::iterator itr;

	BOOL bHasManual_Obligation = FALSE;

	std::list<std::wstring> listTagName;
	listTagName.clear();

	byte btTarget = 0;
	std::wstring strDescription;
	strDescription.clear();
	for(itr = obj.obList.begin(); itr != obj.obList.end(); itr++)
	{
		PABase::OBLIGATION& ob = *itr;
		if(Is_IndicatedObligation(ob, m_strManualTag_Obligation.c_str()))
		{
			std::wstring strTagName = GetTagNameInObligation(ob);
			wchar_t szBuffer[501] = {0};
			memcpy(szBuffer, strTagName.c_str(), strTagName.length() > 500? 500*sizeof(wchar_t):strTagName.length() * sizeof(wchar_t));
			_wcslwr_s(szBuffer, wcslen(szBuffer) + 1);

			if(listTagName.end() == std::find(listTagName.begin(), listTagName.end(), szBuffer))
			{
				smart_ptr<FILETAG> spTag(new FILETAG);
				spTag->strTagName = strTagName;
				GetTagValuesInObligation(ob, spTag->listValues);
				spTag->strTagType = m_strManualTag_Obligation;
				listFileTagItem.push_back(spTag);

				listTagName.push_back(szBuffer);
				bHasManual_Obligation = TRUE;
			}

			if(strDescription.length() <= 0)//get the description from the first obligation. Kevin Zhou 2008-9-12
				strDescription = GetDescriptionInObligation(ob);

			//Merge the "target" property of multiple obligations. Added by Kevin Zhou 2008-10-23 [begin]
			std::wstring strTarget = GetTargetInObligation(ob);
			if(_wcsicmp(strTarget.c_str(), GetResString(IDS_TARGET_OB_SOURCE).c_str()) == 0)
			{
				btTarget |= FILETAGGING_SOURCE_VALUE;
			}
			else if(_wcsicmp(strTarget.c_str(), GetResString(IDS_TARGET_OB_DESTINATION).c_str()) == 0)
			{
				btTarget |= FILETAGGING_DESTINATION_VALUE;
			}
			//[end]

			mapProps[FILETAGGING_TAGONERROR] = GetValueFromObligation(ob, FILETAGGING_TAGONERROR);
			mapProps[FILETAGGING_TAGONUNSUPPORTTYPE] = GetValueFromObligation(ob, FILETAGGING_TAGONUNSUPPORTTYPE);
			mapProps[FILETAGGING_BLOCK_MESSAGE] = GetValueFromObligation(ob, FILETAGGING_BLOCK_MESSAGE);
		}
	}

	mapProps[FILETAGGING_DESCRIPTION_OB] = strDescription;
	wchar_t buffer[50] = {0};
	_snwprintf_s(buffer, 50, _TRUNCATE, L"%d", btTarget);
	mapProps[FILETAGGING_TARGET_OB] = std::wstring(buffer);

	return bHasManual_Obligation;
}

void CFileTagMgr::MergeManualTag_Obligation(LPCWSTR pszFileName, LPCWSTR pszDisplayName, std::list<smart_ptr<FILETAG>>& listFileTagItem, LPCWSTR pszTempFile, std::map<std::wstring, std::wstring>& mapProps)
{
	if(!pszFileName || !pszDisplayName)
		return;

	smart_ptr<FILETAG_ITEM> spExistItem;
	BOOL bExists = GetFileTagItemByFileName(pszFileName, m_listManualItems, spExistItem);

	byte btTarget = 0;
	std::wstring strTarget = mapProps[FILETAGGING_TARGET_OB];
	btTarget = (byte)_wtoi(strTarget.c_str());
	std::wstring strDescription = mapProps[FILETAGGING_DESCRIPTION_OB];

	if(bExists)
	{
		spExistItem->btTarget |= btTarget;//kevin 2008-10-23
		std::list<smart_ptr<FILETAG>>::iterator f_itr;
		for(f_itr = listFileTagItem.begin(); f_itr != listFileTagItem.end(); f_itr++)//all items which needs to be merged into m_listAutoItems
		{
			smart_ptr<FILETAG> sp_FileTagItem = *f_itr;//need to be merged

			std::list<smart_ptr<FILETAG>>::iterator itr;
			BOOL bTagExists = FALSE;
			for(itr = spExistItem->listTags.begin(); itr != spExistItem->listTags.end(); itr++)
			{
				smart_ptr<FILETAG> sp_FileTag = *itr;// the existing tag 
				if(0 == _wcsicmp(sp_FileTag->strTagName.c_str(), sp_FileTagItem->strTagName.c_str()))//ignore if current tag name has existed 
				{
					bTagExists = TRUE;
					break;
				}

			}
			if(!bTagExists)
			{
				spExistItem->listTags.push_back(sp_FileTagItem);//add the tag into the existing file
				DP( (L"(Manual Tag Obligation) File Exists: Add a new tag. File name: %s, tag name:%s\r\n", pszFileName, sp_FileTagItem->strTagName.c_str()));
			}
			else
				DP( (L"(Manual Tag Obligation) File Exists: Duplicate tags. File name: %s, tag name:%s\r\n", pszFileName, sp_FileTagItem->strTagName.c_str()));

		}
	}
	else
	{
		smart_ptr<FILETAG_ITEM> spItem(new FILETAG_ITEM);
		spItem->strFile = pszFileName;
		spItem->strDisplayFile = pszDisplayName;
		spItem->strHint = strDescription;
		spItem->listTags = listFileTagItem;
		spItem->btTarget = btTarget;//kevin 2008-10-23
		if(pszTempFile)
			spItem->strTempFilePath = pszTempFile;

		spItem->strTagOnError = mapProps[FILETAGGING_TAGONERROR];
		spItem->strTagOnUnSupportFileType = mapProps[FILETAGGING_TAGONUNSUPPORTTYPE];
		spItem->strMessageForBlockAction = mapProps[FILETAGGING_BLOCK_MESSAGE];

		m_listManualItems.push_back(spItem);
		DP( (L"(Manual Tag Obligation) Add a new FILETAG_ITEM for manual tag obligation. File name: %s\r\n", pszFileName));
	}


}

BOOL CFileTagMgr::GetTheFirstOBForTagging(PABase::OBLIGATIONLIST& listOB, OUT PABase::OBLIGATION& obTagging)
{
	PABase::OBLIGATIONLIST::iterator itr;

	for(itr = listOB.begin(); itr != listOB.end(); itr++)
	{
		PABase::OBLIGATION& ob = *itr;
		if(Is_IndicatedObligation(ob, m_strAutoTag_Obligation.c_str()) || Is_IndicatedObligation(ob, m_strManualTag_Obligation.c_str()))
		{
			obTagging = ob;
			return TRUE;
		}
	}
	return FALSE;
}


//common APIs for automatic/manual tag obligations
BOOL CFileTagMgr::Is_IndicatedObligation(IN PABase::OBLIGATION& ob, IN LPCWSTR pszOB)
{
	if(ob.strOBName.compare(pszOB) == 0)
		return TRUE;
	else
		return FALSE;
}

std::wstring CFileTagMgr::GetTagNameInObligation(IN PABase::OBLIGATION& ob)
{
	wchar_t szBuffer[101] = {0};
	LoadStringW(GetCurrentInstance(), IDS_TAGNAME_FROM_OB, szBuffer, 100);

	PABase::ATTRIBUTELIST::iterator itr;
	for(itr = ob.attrList.begin(); itr != ob.attrList.end(); itr++)
	{
		PABase::ATTRIBUTE& attr = *itr;
		
		if(_wcsicmp(attr.strValue.c_str(), szBuffer) == 0)
		{
			itr++;
			if(itr != ob.attrList.end())
			{
				std::wstring strTagName = (*itr).strValue;

				if(strTagName.length() > FILETAGGING_TAGNAME_MAX_LENGTH)
				{
					strTagName = strTagName.substr(0, FILETAGGING_TAGNAME_MAX_LENGTH);
				}
				return strTagName;
			}

			break;
			
		}
	}

	return L"No tag value";
}

void CFileTagMgr::GetTagValuesInObligation(IN PABase::OBLIGATION& ob, OUT std::list<std::wstring>& listValues)
{
	wchar_t szTagValues[101] = {0};
	LoadStringW(GetCurrentInstance(), IDS_TAGVALUES_FROM_OB, szTagValues, 100);
	wchar_t szTagValue[101] = {0};
	LoadStringW(GetCurrentInstance(), IDS_TAGVALUE_FROM_OB, szTagValue, 100);

	PABase::ATTRIBUTELIST::iterator itr;
	for(itr = ob.attrList.begin(); itr != ob.attrList.end(); itr++)
	{
		PABase::ATTRIBUTE& attr = *itr;
		
		if(_wcsicmp(attr.strValue.c_str(), szTagValue) == 0 || _wcsicmp(attr.strValue.c_str(), szTagValues) == 0)
		{
			itr++;
			if(itr != ob.attrList.end())
			{
				std::wstring strValues = (*itr).strValue;

				int nIndex = -1;
				while((nIndex = (int)strValues.find(g_szSeparateTagValues)) >= 0)
				{
					std::wstring strTagValue = strValues.substr(0, nIndex);
					if(strTagValue.length() > FILETAGGING_TAGVALUE_MAX_LENGTH)
					{
						strTagValue = strTagValue.substr(0, FILETAGGING_TAGVALUE_MAX_LENGTH);
					}
					if(strTagValue.length() > 0 )//bug462
						listValues.push_back(strTagValue);

					if(nIndex >= (int)(strValues.length() - 1))
					{
						strValues.clear();
						break;
					}
					else
						strValues = strValues.substr(nIndex + 1, strValues.length() - nIndex -1);
				}
				if(!strValues.empty())
				{
					if(strValues.length() > FILETAGGING_TAGVALUE_MAX_LENGTH)
					{
						strValues = strValues.substr(0, FILETAGGING_TAGVALUE_MAX_LENGTH);
					}
					listValues.push_back(strValues);
				}
				return;
			}
			

			break;

		}
	}

	
}

std::wstring CFileTagMgr::GetDescriptionInObligation(IN PABase::OBLIGATION& ob)
{
	wchar_t szDescription[1001] = {0};
	LoadStringW(GetCurrentInstance(), IDS_DESCRIPTION_OB, szDescription, 1000);

	PABase::ATTRIBUTELIST::iterator itr;
	for(itr = ob.attrList.begin(); itr != ob.attrList.end(); itr++)
	{
		
		PABase::ATTRIBUTE& attr = *itr;
//		DP((L"Attribute values: %s\r\n", attr.strValue.c_str()));
		if(_wcsicmp(attr.strValue.c_str(), szDescription) == 0)
		{
			itr++;
			if(itr != ob.attrList.end())
				return (*itr).strValue;

			break;
		}
	}
	return L"";
}

std::wstring CFileTagMgr::GetTargetInObligation(IN PABase::OBLIGATION& ob)
{
	wchar_t szTarget[1001] = {0};
	LoadStringW(GetCurrentInstance(), IDS_TARGET_OB, szTarget, 1000);

	PABase::ATTRIBUTELIST::iterator itr;
	for(itr = ob.attrList.begin(); itr != ob.attrList.end(); itr++)
	{

		PABase::ATTRIBUTE& attr = *itr;
		//		DP((L"Attribute values: %s\r\n", attr.strValue.c_str()));
		if(_wcsicmp(attr.strValue.c_str(), szTarget) == 0)
		{
			itr++;
			if(itr != ob.attrList.end())
				return (*itr).strValue;

			break;
		}
	}
	return L"";
}


BOOL CFileTagMgr::GetFileTagItemByFileName(IN LPCWSTR pszFileName, IN std::list<smart_ptr<FILETAG_ITEM>>& listItems, OUT smart_ptr<FILETAG_ITEM>& listFileItems)
{
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr;

	for(itr = listItems.begin(); itr != listItems.end(); itr++)//get the related FILETAG_ITEM if the indicated file exists in the m_listAutoItems already
	{
		smart_ptr<FILETAG_ITEM> spFileTag_Item = *itr;
		if( 0 == _wcsicmp(spFileTag_Item->strFile.c_str(), pszFileName) )
		{
			listFileItems = spFileTag_Item;
			return TRUE;
		}

	}
	return FALSE;
}

void CFileTagMgr::DisplayAllFileTaggingItems()
{
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr;

	for(itr = m_listAutoItems.begin(); itr != m_listAutoItems.end(); itr++)
	{
		smart_ptr<FILETAG_ITEM> spItem = *itr;

		DP((L"File name: %s\r\n",spItem->strFile.c_str()));

		std::list<smart_ptr<FILETAG>>::iterator f_itr;

		for(f_itr = spItem->listTags.begin(); f_itr != spItem->listTags.end(); f_itr++)
		{
			smart_ptr<FILETAG> spTagItem = *f_itr;

			DP((L"     Tag name: %s \r\n", spTagItem->strTagName.c_str()));
			DP((L"     Tag type: %s \r\n", spTagItem->strTagType.c_str()));
			DP((L"     Tag values:\r\n"));

			std::list<std::wstring>::iterator v_itr;
			for(v_itr = spTagItem->listValues.begin(); v_itr != spTagItem->listValues.end(); v_itr++)
			{
				std::wstring& value = *v_itr;
				DP((L"             %s\r\n", value.c_str()));
			}


		}
		
	}

	for(itr = m_listManualItems.begin(); itr != m_listManualItems.end(); itr++)
	{
		smart_ptr<FILETAG_ITEM> spItem = *itr;

		DP((L"File name: %s\r\n",spItem->strFile.c_str()));

		std::list<smart_ptr<FILETAG>>::iterator f_itr;

		for(f_itr = spItem->listTags.begin(); f_itr != spItem->listTags.end(); f_itr++)
		{
			smart_ptr<FILETAG> spTagItem = *f_itr;

			DP((L"     Tag name: %s \r\n", spTagItem->strTagName.c_str()));
			DP((L"     Tag type: %s \r\n", spTagItem->strTagType.c_str()));
			DP((L"     Tag values:\r\n"));

			std::list<std::wstring>::iterator v_itr;
			for(v_itr = spTagItem->listValues.begin(); v_itr != spTagItem->listValues.end(); v_itr++)
			{
				std::wstring& value = *v_itr;
				DP((L"              %s\r\n", value.c_str()));
			}


		}

	}
}

CFileTag* CFileTagMgr::GetFileTag()
{
	return &m_FileTag;
}

void CFileTagMgr::ShowViewResetTagsDlg(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return;

	
	CFileTagViewDlg dlg;
	dlg.SetFileTagMgr(this);
	dlg.SetFileName(pszFileName);
	dlg.DoModal();
}

HWND CFileTagMgr::GetFileTagPanelWnd()
{
	return m_hWnd;
}

void CFileTagMgr::AddLog( 
						 std::wstring wstrlogIdentifier,
						 std::wstring wstrAssistantName, 
						 std::wstring wstrAssistantOptions, 
						 std::wstring wstrAssistantDescription, 
						 std::wstring wstrAssistantUserActions, 
						 PABase::ATTRIBUTELIST &optAttributes)
{
	if(m_pLogFunc)
	{
		m_pLogFunc(m_lpLogCtx, wstrlogIdentifier, wstrAssistantName, wstrAssistantOptions, wstrAssistantDescription, wstrAssistantUserActions, optAttributes);
	}
	else
	{
		DP((L"No Log API\r\n"));
	}
}

std::wstring CFileTagMgr::GetResString(DWORD dwID)
{
	if(!m_hInstance)
		return L"";
	wchar_t szBuffer[1001] = {0};

	LoadString(m_hInstance, dwID, szBuffer, 1000);
	return szBuffer;
}

BOOL CFileTagMgr::GetFileterTagsName(PABase::OBLIGATIONLIST& listOB, OUT std::set<std::wstring>& FileterTagsName)
{
	FileterTagsName.clear();

	for(PABase::OBLIGATIONLIST::const_iterator ci = listOB.begin(); ci != listOB.end(); ci++)
	{
		if(0 == ci->strOBName.compare(OBLIGATION_ENCRYPTION_SYNCTAGS_NAME))
		{
 			for(PABase::ATTRIBUTELIST::const_iterator itr = ci->attrList.begin(); itr != ci->attrList.end(); itr++)
 			{
 				if(0 == _wcsicmp(itr->strValue.c_str(), SYNC_TAGS_NAME))
 				{
 					itr++;
					if(itr != ci->attrList.end() && !itr->strValue.empty())
					{	
						std::wstring::size_type iBegin = 0;

						while (TRUE)
						{
							std::wstring::size_type iEnd = itr->strValue.find(L';', iBegin);
							
							if (std::wstring::npos == iEnd)
							{
								std::wstring tagsName = itr->strValue.substr(iBegin);
								transform(tagsName.begin(), tagsName.end(), tagsName.begin(), towupper);

								FileterTagsName.insert(tagsName);
								break;
							}
							else if (itr->strValue.length() == iEnd + 1)
							{
								if (iEnd - iBegin > 0)
								{
									std::wstring tagsName = itr->strValue.substr(iBegin, iEnd - iBegin);
									transform(tagsName.begin(), tagsName.end(), tagsName.begin(), towupper);

									FileterTagsName.insert(tagsName);
								}
								
								break;					
							}

							if (iEnd - iBegin > 0)
							{
								std::wstring tagsName = itr->strValue.substr(iBegin, iEnd - iBegin);
								transform(tagsName.begin(), tagsName.end(), tagsName.begin(), towupper);

								FileterTagsName.insert(tagsName);
							}

							iBegin = iEnd + 1;
						}
					
						return TRUE;
					}

 					return FALSE;
 				}
 			}
		}
	}

	return FALSE;
}


BOOL CFileTagMgr::NeedDoHierarchicalStructureTag(smart_ptr<HSC_ITEM> pHscItem, std::list<smart_ptr<FILETAG_PAIR>>& lstTags)
{
	std::wstring strOEAction = pHscItem->strOEAction;
	if (_wcsicmp(strOEAction.c_str(), OEACTIONFORHSC_FROCEDOING)==0)
	{
		DP((L"testx Hierarchical classify force to do , src=%s\n", pHscItem->strDisplayFile.c_str())); 
		return TRUE;
	}
	else if (_wcsicmp(strOEAction.c_str(), OEACTIONFORHSC_FROCENOTDO)==0)
	{
		DP((L"testx  Hierarchical classify force not to do, src=%s\n", pHscItem->strDisplayFile.c_str())); 
		return FALSE;
	}
	else
	{
		//check last modify time
		std::wstring wstrNLLastModifyTime;
		std::list<smart_ptr<FILETAG_PAIR>>::iterator itTag = lstTags.begin();
		while (itTag != lstTags.end())
		{
			if (0 == _wcsicmp(((*itTag)->strTagName).c_str(), NLLASTMODIFYTIMETAGNAME)) // no need check wstrNLLastModifyTime is empty or not, just compare to get the last one
			{
				wstrNLLastModifyTime = (*itTag)->strTagValue;
				break;
			}
			itTag++;
		}

		// Check file last modify time and judge if the current file modified or not
	   bool bModified = IsTheFileModified(wstrNLLastModifyTime, pHscItem->stuFileTime, pHscItem->dwLastModifyTimeInterval);
	    DP((L"testx Hierarchical classify bModified=%d, src=%s\n", bModified, pHscItem->strDisplayFile.c_str())); 

	   return bModified;
	
	}
}

/*
*   in order to fix the bug of 35630, we need to find last item that need to popup HSC, means later item always done HSC already.
*   at the same time, we have loop to get all tags, so let's cache and don't want to read tag again
*/

int CFileTagMgr::GetTagsAndLastHSCPos(std::vector<std::list<smart_ptr<FILETAG_PAIR>>>& tagpair)
{
    std::list<smart_ptr<HSC_ITEM>>::const_iterator item = m_listHierarchicalClassification.begin();
    int nPos = 0;
    int nIndex = 0;
    for (; item != m_listHierarchicalClassification.end(); item++, nIndex++)
    {
        smart_ptr<HSC_ITEM> pHscItem = *item;

        // for non-supported file, just get HSC tag.
        bool bNeedRealTagOnFile = true;

        //check un-support file type
        if (!FileTypeSupport(pHscItem->strDisplayFile.c_str()))
        {
            bNeedRealTagOnFile = false;
        }

        std::wstring wstrFileTags;
        if (bNeedRealTagOnFile)
        {

            //get the exist file tags.
            std::list<smart_ptr<FILETAG_PAIR>> lstTags;
            m_FileTag.GetAllTags(pHscItem->strDisplayFile.c_str(), &lstTags);
            tagpair.push_back(lstTags);

            if (g_bLoadByOE && !NeedDoHierarchicalStructureTag(pHscItem , lstTags))
            {
                continue;
            }
        }
        else
        {
            std::list<smart_ptr<FILETAG_PAIR>> lstTags;
            tagpair.push_back(lstTags);
        }
        nPos = nIndex;
    }
    return nPos;
}

int CFileTagMgr::DoHierarchicalStructureClassification(HWND hParentWnd,vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo)
{
	//get function
	if (m_fpShowHSCDlg==NULL)
	{
		m_fpShowHSCDlg = (PAUI_FUN_ShowHierarchicalClassifyDlg)GetProcAddress(g_hPafDLL, "ShowHierarchicalClassifyDlg");
		if (m_fpShowHSCDlg==NULL)
		{
			return PA_ERROR;
		}	
	}
	
	if (m_fpReleaseBuffer==NULL)
	{
		m_fpReleaseBuffer = (PAUI_FUN_ReleaseBuffer)GetProcAddress(g_hPafDLL, "ReleaseBuffer");
		if (m_fpReleaseBuffer==NULL)
		{
			return PA_ERROR;
		}	
	}
	
	//
	std::list<smart_ptr<HSC_ITEM>>::iterator itHscItem = m_listHierarchicalClassification.begin();
	std::list<smart_ptr<HSC_ITEM>>::iterator itHscItemCheckLast = itHscItem;

    std::vector<std::list<smart_ptr<FILETAG_PAIR>>> tagpair;
    int nLastPos = GetTagsAndLastHSCPos(tagpair);
    int nIndex = 0;
    std::vector<std::list<smart_ptr<FILETAG_PAIR>>>::iterator tagpairiter = tagpair.begin();
    while (itHscItem != m_listHierarchicalClassification.end())
	{
		itHscItemCheckLast = itHscItem;
		bool bLastFile = (++itHscItemCheckLast == m_listHierarchicalClassification.end());
        if (nIndex++ == nLastPos) bLastFile = true;
		smart_ptr<HSC_ITEM> pHscItem = *itHscItem;

        // for non-supported file, just get HSC tag.
        bool bNeedRealTagOnFile = true;

		//check un-support file type
		if (!FileTypeSupport(pHscItem->strDisplayFile.c_str()))
		{
            bNeedRealTagOnFile = false;
		}
		    
        std::wstring wstrFileTags;
        if (bNeedRealTagOnFile)
        {   
		    //get the exist file tags.
            std::list<smart_ptr<FILETAG_PAIR>>& lstTags = *tagpairiter;
		    std::list<smart_ptr<FILETAG_PAIR>>::iterator itTag = lstTags.begin();
		    while (itTag != lstTags.end())
		    {
			    wstrFileTags += (*itTag)->strTagName;
			    wstrFileTags += pafUI::g_kSepTagVNameAndValue;
			    wstrFileTags += (*itTag)->strTagValue;
			    wstrFileTags += pafUI::g_kSepTags;
			    itTag++;
		    }
		    DP((L"Got exist file tags:%s\n", wstrFileTags.c_str()));
		
            // Check file last modify time and judge if the current file modified or not
            if (g_bLoadByOE && !NeedDoHierarchicalStructureTag(pHscItem, lstTags))
            {
                itHscItem++;
                tagpairiter++;
                continue;
            }
        }

		//show dialog
		wchar_t* pszAddTagResult = NULL;
		wchar_t* pszDelTagResult = NULL;
		UI_STATUS uiStatus = m_fpShowHSCDlg(hParentWnd, pHscItem->strDisplayFile.c_str(), bLastFile, pHscItem->strHint.c_str(), 
			pHscItem->strClassificationXml.c_str(), wstrFileTags.c_str(), &pszAddTagResult, &pszDelTagResult);
		if (uiStatus != IDOK)
		{	//user cancel	
            if (pszAddTagResult != NULL) m_fpReleaseBuffer(pszAddTagResult);
            if (pszDelTagResult != NULL) m_fpReleaseBuffer(pszDelTagResult);
			return PA_USER_CANCEL;
		}
		

		//tag to the file
        bool bTag = false;
        std::list<smart_ptr<FILETAG_PAIR>> lstFileTag;
        //format tag values
        if (pszAddTagResult != NULL)
        {
            FromatHierarchicalClassificationTag(pszAddTagResult, lstFileTag);
            //release buffer
            m_fpReleaseBuffer(pszAddTagResult);
            pszAddTagResult = NULL;

        }
        std::list<std::wstring> lstDelTag;
        if (pszDelTagResult != NULL)
        {
            GetDelTag(pszDelTagResult, lstDelTag);

            m_fpReleaseBuffer(pszDelTagResult);
            pszDelTagResult = NULL;
        }

        // If loag by OE, need check file last modify time and add current time as the file last modify time
        if (bNeedRealTagOnFile && (g_bLoadByOE || (!lstFileTag.empty())))
        {
            //delete tag
            DeleteOldHierarchicalStructureTag(pHscItem.get(), lstDelTag);

            //need tag to source file or dest file by obligation
            if (_wcsicmp(pHscItem->strTagTarget.c_str(), L"Source") == 0)
            {
                g_bLoadByOE ? AddNLFileLastModifyTime(lstFileTag, pHscItem->strDisplayFile) : NULL;
                bTag = AddTagEx(&lstFileTag, pHscItem->strDisplayFile.c_str(), this, pHscItem->strTagOnError.c_str(), pHscItem->strMessageForBlockAction.c_str());
            }
            else if (_wcsicmp(pHscItem->strTagTarget.c_str(), L"Destination") == 0)
            {
                g_bLoadByOE ? AddNLFileLastModifyTime(lstFileTag, pHscItem->strFile) : NULL;;
                bTag = AddTagEx(&lstFileTag, pHscItem->strFile.c_str(), this, pHscItem->strTagOnError.c_str(), pHscItem->strMessageForBlockAction.c_str());
            }
            else
            {
                g_bLoadByOE ? AddNLFileLastModifyTime(lstFileTag, pHscItem->strFile) : NULL;;
                bTag = AddTagEx(&lstFileTag, pHscItem->strFile.c_str(), this, pHscItem->strTagOnError.c_str(), pHscItem->strMessageForBlockAction.c_str());
                if (bTag && 0 != _wcsicmp(pHscItem->strDisplayFile.c_str(), pHscItem->strFile.c_str()))
                {
                    g_bLoadByOE ? AddNLFileLastModifyTime(lstFileTag, pHscItem->strDisplayFile) : NULL;;
                    bTag = AddTagEx(&lstFileTag, pHscItem->strDisplayFile.c_str(), this, pHscItem->strTagOnError.c_str(), pHscItem->strMessageForBlockAction.c_str());
                }
            }
        }

		std::tr1::shared_ptr<HCADDTAGINFO> Info(new HCADDTAGINFO);
        if (!bNeedRealTagOnFile)
        {
            Info->AddTagStatus = ADD_HC_TAG_FAIL_CONTINUE_STATUS; // only get tag.
        }
        else
        {
             if(!bTag)
		    {
			    if (_wcsicmp(pHscItem->strTagOnError.c_str(), L"block") == 0)
			    {
				    Info->AddTagStatus = ADD_HC_TAG_FAIL_BOCLK_STATUS;
				    return PA_USER_CANCEL;
			    }
			    else
			    {	//continue
				    DP((L"ignore the file tag error, continue to the next file:%s\n", pHscItem->strDisplayFile.c_str()));
				    Info->AddTagStatus = ADD_HC_TAG_FAIL_CONTINUE_STATUS;
			    }
		    }
		    else
		    {
			    Info->AddTagStatus = ADD_HC_TAG_SUCCESS_STATUS;
		    }
        }
		
		Info->strSrcPath =  pHscItem->strDisplayFile;
		Info->strDstPath =  pHscItem->strFile;
		std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;

		for(itr = lstFileTag.begin(); itr != lstFileTag.end(); itr++)
		{
			Info->vecTagInfo.push_back(pair<wstring,wstring>((*itr)->strTagName,(*itr)->strTagValue));
		}
		Info->strLogID = pHscItem->strLogID;

		(*pVecHCInfo).push_back(Info);
			
		itHscItem++;
        tagpairiter++;
	}

	return PA_SUCCESS;
	
}

BOOL CFileTagMgr::FileTypeSupport(LPCWSTR wszFileName)
{
	if (!g_bLoadByOE)// the file type check is only applied on OE
	{
		return TRUE;
	}
	
	const wchar_t* szSupportTypes[] = { L".docx", L".doc", L".dot", L".docm", L".dotx", L".dotm",
		                                L".xlsx", L".xls", L".xlsm", L".xlt", L".xltm", L".xltx",
		                                L".pptx", L".ppt", L".pot", L".potx", L".potm", L".potx", L".pptm",
		                                L".pdf",
		                               };

	if (NULL != wszFileName)
	{
		size_t nFileNameLen = wcslen(wszFileName);
		for (int i = 0; i < sizeof(szSupportTypes) / sizeof(szSupportTypes[0]); i++)
		{
			const wchar_t* pFileType = szSupportTypes[i];
			size_t nFileTypeLen = wcslen(pFileType);

			if ((nFileNameLen>nFileTypeLen) &&
				(_wcsicmp(wszFileName+nFileNameLen-nFileTypeLen, pFileType) == 0))
			{
				return TRUE;
			}
		}

	}

	return FALSE;
}

size_t CFileTagMgr::FromatHierarchicalClassificationTag(LPCWSTR wszTag, std::list<smart_ptr<FILETAG_PAIR>>& lstTag)
{
	const TCHAR chTagSeprator = L';';
	const TCHAR chTagValueSeprator = L'=';

	std::wstring wstrTag = wszTag;
	size_t nCurrentPos = 0;
	size_t nPosTagSeprator = wstrTag.find(chTagSeprator, nCurrentPos);

	while (nCurrentPos < wstrTag.size())
	{
		std::wstring wstrSubTag = nPosTagSeprator == std::wstring::npos ? wstrTag.substr(nCurrentPos) : wstrTag.substr(nCurrentPos, nPosTagSeprator - nCurrentPos);

		size_t nPosTagValue = wstrSubTag.find(chTagValueSeprator);
		if (nPosTagValue != std::wstring::npos)
		{
			std::wstring wstrTagName = wstrSubTag.substr(0, nPosTagValue);
			std::wstring wstrTagValue = wstrSubTag.substr(nPosTagValue + 1);

			std::list<smart_ptr<FILETAG_PAIR>>::iterator itFileTag = lstTag.begin();
			while (itFileTag != lstTag.end())
			{
				smart_ptr<FILETAG_PAIR> pFileTag = *itFileTag;
				if (pFileTag->strTagName == wstrTagName)
				{
					break;
				}
				itFileTag++;
			}

			if (itFileTag == lstTag.end())
			{
				FILETAG_PAIR* pFileTag = new FILETAG_PAIR();
				pFileTag->strTagName = wstrTagName;
				pFileTag->strTagValue = wstrTagValue;
				lstTag.push_back(smart_ptr<FILETAG_PAIR>(pFileTag));
			}
			else
			{
				smart_ptr<FILETAG_PAIR> pFileTag = *itFileTag;
				pFileTag->strTagValue += L"|";
				pFileTag->strTagValue += wstrTagValue;
			}	
		}
		
		nCurrentPos = nPosTagSeprator == std::wstring::npos ? wstrTag.size() : nPosTagSeprator + 1;
		nPosTagSeprator = wstrTag.find(chTagSeprator, nCurrentPos);
	}
	

	return lstTag.size();

}

std::wstring CFileTagMgr::GetValueFromObligation(PABase::OBLIGATION& ob, LPCWSTR pszKeyName)
{
	PABase::ATTRIBUTELIST::iterator itr;
	DP((L"[GetValueFromObligation]>>>>>>>>>>>>>>>>>\"%s\"\n", pszKeyName)); 
	for (itr = ob.attrList.begin(); itr != ob.attrList.end(); itr++)
	{
		PABase::ATTRIBUTE& attr = *itr;
		DP((L"[GetValueFromObligation]\"%s\"=", attr.strValue.c_str())); 
		if (_wcsicmp(attr.strValue.c_str(), pszKeyName) == 0)
		{
			itr++;
			if (itr != ob.attrList.end())
			{
				DP((L"[GetValueFromObligation]\"%s\"", (*itr).strValue.c_str())); 
				return (*itr).strValue;
			}
			break;
		}
	}
	return L"";
}

bool CFileTagMgr::RemoveUnsupportFileTypeItem()
{
	std::list<smart_ptr<FILETAG_ITEM>>::iterator itr = m_listManualItems.begin();
	while (itr != m_listManualItems.end())
	{
		smart_ptr<FILETAG_ITEM> pFileItem = *itr;
		if (!FileTypeSupport(pFileItem->strDisplayFile.c_str()))
		{
			if (_wcsicmp(pFileItem->strTagOnUnSupportFileType.c_str(), ERROR_ACTION_BLOCK)==0)
			{
				//show block message and return false
				ShowTagErrorBlockMessage(m_hParentWnd, pFileItem->strDisplayFile.c_str(), pFileItem->strMessageForBlockAction.c_str());
				return false;
			}
			else
			{
				//remove the item
				m_listManualItems.erase(itr);
				itr = m_listManualItems.begin();
				continue;
			}
		}

		itr++;
	}
	
	return true;
}

void CFileTagMgr::GetDelTag(LPCWSTR wszTag, std::list<std::wstring>& lstDelTag)
{
	std::wstring wstrDelTag = wszTag;

	size_t nCurrentPos = 0;
	size_t nPosTagSeprator = std::wstring::npos;
	while (nCurrentPos < wstrDelTag.size())
	{
		nPosTagSeprator = wstrDelTag.find(pafUI::g_kSepTags, nCurrentPos);
		std::wstring wstrTagName = (nPosTagSeprator == std::wstring::npos) ? wstrDelTag.substr(nCurrentPos) : wstrDelTag.substr(nCurrentPos, nPosTagSeprator - nCurrentPos);
		if (!wstrTagName.empty())
		{
			lstDelTag.push_back(wstrTagName);
		}

		nCurrentPos = (nPosTagSeprator == std::wstring::npos) ? wstrDelTag.size() : nPosTagSeprator + 1;
	}

}

BOOL CFileTagMgr::DeleteOldHierarchicalStructureTag(HSC_ITEM* pHscItem, std::list<std::wstring>& lstDelTag)
{
	if (lstDelTag.empty())
	{
		return TRUE;
	}

	BOOL bDelTag = FALSE;
	if (_wcsicmp(pHscItem->strTagTarget.c_str(), L"Source") == 0)
	{
		bDelTag = RemoveTagEx(&lstDelTag, pHscItem->strDisplayFile.c_str(), this);
	}
	else if (_wcsicmp(pHscItem->strTagTarget.c_str(), L"Destination") == 0)
	{
		bDelTag = RemoveTagEx(&lstDelTag, pHscItem->strFile.c_str(), this);
	}
	else
	{
		bDelTag = RemoveTagEx(&lstDelTag, pHscItem->strDisplayFile.c_str(), this);
		if (bDelTag && 0 != _wcsicmp(pHscItem->strDisplayFile.c_str(), pHscItem->strFile.c_str()))
		{
			bDelTag = RemoveTagEx(&lstDelTag, pHscItem->strFile.c_str(), this);
		}
	}

	DP((L"DeleteOldHierarchicalStructureTag result:%s.\n", bDelTag ? L"Success" : L"Failed"));
	return bDelTag;
}

PABase::PA_STATUS CFileTagMgr::DeleteFileLastModifyTimeTag(const wchar_t* wszFile)
{
	std::list<std::wstring> lstDelTag;
	lstDelTag.push_back(std::wstring(NLLASTMODIFYTIMETAGNAME));
	DP((L"testx DeleteFileLastModifyTimeTag:%s\n", wszFile));
	return RemoveTagEx(&lstDelTag, wszFile, this)? PA_SUCCESS : PA_ERROR;
}

PABase::PA_STATUS CFileTagMgr::DoTaggingOnFile(const wchar_t* wszFileName, const vector<pair<wstring,wstring>>* pVecTags)
{
	std::list<smart_ptr<FILETAG_PAIR>> lstTags;

	vector<pair<wstring,wstring>>::const_iterator itTag = pVecTags->begin();
	while (itTag != pVecTags->end())
	{
		smart_ptr<FILETAG_PAIR> spPair(new FILETAG_PAIR);

		spPair->strTagName = std::wstring(itTag->first.c_str());
		spPair->strTagValue = std::wstring(itTag->second.c_str());
  
		lstTags.push_back(spPair);
		itTag++;
	}
	

	return AddTagEx(&lstTags, wszFileName, this);
}