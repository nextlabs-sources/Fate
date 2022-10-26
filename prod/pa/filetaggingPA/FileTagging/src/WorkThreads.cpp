#include "stdafx.h"
#include "WorkThreads.h"
#include "Utils.h"
#include "PromptDlg.h"
#include "FileTagViewDlg.h"


unsigned __stdcall DoAutomaticTaggingThread( void* pArguments )
{
	if(!pArguments)
		return 0;

	AUTOTHREADPARAM* pParam = (AUTOTHREADPARAM*)pArguments;

	BOOL bSuccess = TRUE;
	DWORD dwErrorID = pParam->pMgr->GetFileTag()->AddTag(pParam->pList, pParam->pszFileName);
	pParam->dwErrorID = dwErrorID;
	if(dwErrorID > 0)
	{
		bSuccess = FALSE;
		if(pParam->pDlg)
		{
			pParam->pDlg->SetEndFlag(TRUE);
			Sleep(100);
		}
	//	PopupMessageBox(dwErrorID, pParam->pszFileName, pParam->pMgr->GetFileTag()->GetParentWnd());
	}
	else
	{
		
		std::list<smart_ptr<FILETAG>>::iterator itr_tag;
		std::list<smart_ptr<FILETAG_PAIR>>		listAutoTags;
		listAutoTags.clear();
		for(itr_tag = pParam->pList->begin(); itr_tag != pParam->pList->end(); itr_tag++)
		{
			smart_ptr<FILETAG> tag = *itr_tag;
			if(tag->listValues.size() < 1)
				continue;

			smart_ptr<FILETAG_PAIR> pair(new FILETAG_PAIR);
			pair->strTagName = tag->strTagName;
			pair->strTagValue = *(tag->listValues.begin());
			listAutoTags.push_back(pair);
		}
		pParam->pMgr->GetFileTag()->UpdateIndexTag(&listAutoTags, pParam->pszFileName, TRUE);
		
	}

	pParam->bSucceed = bSuccess;
	if(pParam->pDlg)
		pParam->pDlg->SetEndFlag(TRUE);
	
	return 0;
}

unsigned __stdcall DoManualTaggingThread( void* pArguments )
{
	if(!pArguments)
		return 0;

	LPMANUALTHREADPARAM pParam = (LPMANUALTHREADPARAM)pArguments;

	BOOL bTag = TRUE;
	DWORD dwErrorID = pParam->pMgr->GetFileTag()->AddTag(pParam->pList, pParam->pszFileName);
	pParam->dwErrorID = dwErrorID;
	if(dwErrorID > 0)
	{
		bTag = FALSE;
		if(pParam->pDlg)
		{
			pParam->pDlg->SetEndFlag(TRUE);
			//		Sleep(500);
		}
//		PopupMessageBox(dwErrorID, pParam->pszFileName, pParam->pMgr->GetFileTag()->GetParentWnd());
	}
	else
	{//Update Index Tag
		pParam->pMgr->GetFileTag()->UpdateIndexTag(pParam->pList, pParam->pszFileName, TRUE);
	}
	
	if(pParam->pDlg)
		pParam->pDlg->SetEndFlag(TRUE);
	pParam->bSucceed = bTag;
	
	return 0;
}

unsigned __stdcall DoRemoveTagsThread( void* pArguments )
{
	if(!pArguments)
		return 0;

	LPREMOVETAGTHREADPARAM pParam = (LPREMOVETAGTHREADPARAM)pArguments;

	BOOL bRemove = TRUE;
	DWORD dwErrorID = pParam->pMgr->GetFileTag()->RemoveTag(pParam->pList, pParam->pszFileName);
	pParam->dwErrorID = dwErrorID;
	if(dwErrorID > 0)
	{
		bRemove = FALSE;
		if(pParam->pDlg)
		{
			pParam->pDlg->SetEndFlag(TRUE);
			Sleep(100);
		}
	//	PopupMessageBox(dwErrorID, pParam->pszFileName, pParam->pMgr->GetFileTag()->GetParentWnd());
	}
	else
	{
		pParam->pMgr->GetFileTag()->UpdateIndexTag(pParam->pList, pParam->pszFileName, FALSE);
	}

	if(pParam->pDlg)
		pParam->pDlg->SetEndFlag(TRUE);
	pParam->bSucceed = bRemove;
	
	return 0;

}

unsigned __stdcall GetTagValueByNameThread(void* pArguments )
{
	if(!pArguments)
		return 0;

	LPGET_TAGVALUE_BYNAME_THREAD_PARAM pParam = (LPGET_TAGVALUE_BYNAME_THREAD_PARAM)pArguments;
	if(!pParam->pMgr)
		return 0;

	//Set the default value of the tag here. 
	int nIndex = 0;
	std::list<smart_ptr<FILETAG>>::iterator itr;
	for(itr = pParam->pItem->listTags.begin(); itr != pParam->pItem->listTags.end(); itr++)
	{
		smart_ptr<FILETAG> spTag = *itr;

		std::wstring strTagValue;
		// pParam->pItem->strFile.c_str() == NULL
//		pParam->dwErrorID = pParam->pMgr->GetFileTag()->GetTagValueByName(spTag->strTagName.c_str(), pParam->pItem->strFile.c_str(), strTagValue);
		
		// kim code 2012-11-21 
		pParam->dwErrorID = pParam->pMgr->GetFileTag()->GetTagValueByName( spTag->strTagName.c_str(), pParam->pItem->strDisplayFile.c_str(), strTagValue );

		if(0 == pParam->dwErrorID)
		{
			pParam->pMgr->m_pFileTagPanel->m_lvTags.SetComboBoxValue( nIndex, strTagValue.c_str() ); // set default value
			pParam->bSuccess = TRUE;
		}
		else
		{
			pParam->bSuccess = FALSE;
			break;
		}

		nIndex++;
	}
	if(pParam->pDlg)
		pParam->pDlg->SetEndFlag(TRUE);

	return 0;
}

unsigned __stdcall GetAllTagValuesThread(void* pArguments )
{
	if(!pArguments)
		return 0;

	LPGETALLTAGVALUESTHREADPARAM pParam = (LPGETALLTAGVALUESTHREADPARAM)pArguments;


	pParam->dwErrorID = pParam->pMgr->GetFileTag()->GetAllTags(pParam->pszFileName, pParam->pList);

	if(pParam->dwErrorID)
	{
		pParam->bSuccess = FALSE;
	}
	else
		pParam->bSuccess = TRUE;

	if(pParam->pDlg)
		pParam->pDlg->SetEndFlag(TRUE);
	return 0;

}
