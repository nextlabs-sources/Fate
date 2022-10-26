#include "stdafx.h"
#include "filetag.h"
#include "resattrmgr.h"
#include "Utils.h"
#include "resource.h"



CFileTag::CFileTag()
{
	m_pMgr = NULL;
	m_hWnd = NULL;
}

DWORD CFileTag::AddTag(LPCWSTR pszTagName, LPCWSTR pszTagValue, LPCWSTR pszFile)
{
	if(!pszTagName || !pszTagValue || !pszFile)
		return FALSE;

	DWORD dwErrorID = 0;
	if(!CheckFileTaggingError(pszFile, m_hWnd, dwErrorID))
	{
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), m_pMgr->GetResString(dwErrorID), m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
		}
		return dwErrorID;
	}

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);

	if(!mgr)
		return IDS_NOT_ENOUGH_MEMORY;

	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	if(attrs)
	{
		AddAttributeW(attrs, pszTagName, pszTagValue);
			
		DP((L"Add tag start: Time: %s, Tag name: %s, tag value: %s, file name: %s\r\n", FormatCurrentTime().c_str(), pszTagName, pszTagValue, pszFile));
		DWORD dwOldTime = ::GetTickCount();
		int nRet = WriteResourceAttributes(mgr, pszFile, attrs);
		DWORD dwTime = ::GetTickCount() - dwOldTime;
		if(nRet)
		{
			DP((L"Add tag end: time:%s, Tag name: %s, tag value: %s, file name: %s\r\n", FormatCurrentTime().c_str(), pszTagName, pszTagValue, pszFile));
			if(m_pMgr)
			{
				PABase::ATTRIBUTELIST pa_listAttr;
				PABase::ATTRIBUTE pa_attr;
				pa_attr.strKey = m_pMgr->GetResString(IDS_TAG_TIME);
				wchar_t szTime[100] = {0};
				_snwprintf_s(szTime, 100, _TRUNCATE, L"%d ms", dwTime);
				pa_attr.strValue = std::wstring(szTime);
				pa_listAttr.push_back(pa_attr);
				std::wstring szIdentifier(L"Identifier");
				m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_ADDTAG), pa_listAttr);
			}
		}
		else
		{
			DP((L"Add tag failed. FileName: %s", pszFile));
			dwErrorID = IDS_UNKNOWN_ERROR;
			if(m_pMgr)
			{
				PABase::ATTRIBUTELIST pa_listAttr;
				std::wstring szIdentifier(L"Identifier");
				m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
			}
		}

		FreeAttributes(attrs);
	}

	if(mgr)
	{
		CloseAttributeManager(mgr);
		mgr = NULL;
	}
//	DP((L"Add tag. Tag name: %s, tag value: %s, file name: %s", pszTagName, pszTagValue, pszFile));
	return dwErrorID;
}

DWORD CFileTag::AddTag(std::list<smart_ptr<FILETAG>>* pListTags, LPCWSTR pszFileName)
{
	if(!pszFileName)
		return IDS_FILE_NOT_EXIST;

	DWORD dwErrorID = 0;
	if(!CheckFileTaggingError(pszFileName, m_hWnd, dwErrorID))
	{
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			m_pMgr->AddLog(L"Identifer", m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), m_pMgr->GetResString(dwErrorID), m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
		}
		return dwErrorID;
	}

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);
	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	std::list<smart_ptr<FILETAG>>::iterator f_itr;
	for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
	{//
		smart_ptr<FILETAG> spTag = *f_itr;
		if(spTag->listValues.size() < 1)
			continue;
		std::wstring strTagValue = *(spTag->listValues.begin());
		AddAttributeW(attrs, spTag->strTagName.c_str(), strTagValue.c_str());
	}

	DP((L"Add tag start: Time: %s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));
	DWORD dwOldTime = ::GetTickCount();
	int nRet = WriteResourceAttributes(mgr, pszFileName, attrs);
	DWORD dwTime = ::GetTickCount() - dwOldTime;
	if(nRet)
	{
		DP((L"Add tag end: time:%s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			PABase::ATTRIBUTE pa_attr;
			pa_attr.strKey = m_pMgr->GetResString(IDS_TAG_TIME);
			wchar_t szTime[100] = {0};
			_snwprintf_s(szTime, 100, _TRUNCATE, L"%d ms", dwTime);
			pa_attr.strValue = std::wstring(szTime);
			pa_listAttr.push_back(pa_attr);
			for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
			{//
				smart_ptr<FILETAG> spTag = *f_itr;
				if(spTag->listValues.size() < 1)
					continue;
				
				PABase::ATTRIBUTE pa_attr2;
				pa_attr2.strKey = spTag->strTagName;
				pa_attr2.strValue = *(spTag->listValues.begin());

				pa_listAttr.push_back(pa_attr2);

			}
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_ADDTAG), pa_listAttr);
		}
	}
	else
	{
		DP((L"Add tag failed. FileName: %s\r\n", pszFileName));
		dwErrorID = IDS_UNKNOWN_ERROR;
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
		}
	}

	FreeAttributes(attrs);

	if(mgr)
	{
		CloseAttributeManager(mgr);
		mgr = NULL;
	}
	return dwErrorID;
}

DWORD CFileTag::AddTag(std::list<smart_ptr<FILETAG_PAIR>>* pListTags, LPCWSTR pszFileName)
{
	if(!pszFileName)
		return IDS_FILE_NOT_EXIST;

	DWORD dwErrorID = 0;
	if(!CheckFileTaggingError(pszFileName, m_hWnd, dwErrorID))
	{
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			m_pMgr->AddLog(L"identifer", m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), m_pMgr->GetResString(dwErrorID), m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
		}
		return dwErrorID;
	}

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);
	ResourceAttributes *attrs = NULL;
	AllocAttributes(&attrs);

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	std::list<smart_ptr<FILETAG_PAIR>>::iterator f_itr;
	for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
	{//
		smart_ptr<FILETAG_PAIR> spTag = *f_itr;
	
		AddAttributeW(attrs, spTag->strTagName.c_str(), spTag->strTagValue.c_str());
	}

	DP((L"Add tag start: Time: %s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));

	DWORD dwOldTime = ::GetTickCount();
	int nRet = WriteResourceAttributes(mgr, pszFileName, attrs);
	DWORD dwTime = ::GetTickCount() - dwOldTime;

	if(nRet)
	{
		DP((L"Add tag end: time:%s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			PABase::ATTRIBUTE pa_attr;
			pa_attr.strKey = m_pMgr->GetResString(IDS_TAG_TIME);
			wchar_t szTime[100] = {0};
			_snwprintf_s(szTime, 100, _TRUNCATE,  L"%d ms", dwTime);
			pa_attr.strValue = std::wstring(szTime);
			pa_listAttr.push_back(pa_attr);
			for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
			{//
				smart_ptr<FILETAG_PAIR> spTag = *f_itr;

				PABase::ATTRIBUTE pa_attr2;
				pa_attr2.strKey = spTag->strTagName;
				pa_attr2.strValue = spTag->strTagValue;

				pa_listAttr.push_back(pa_attr2);

			}
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_ADDTAG), pa_listAttr);
		}
	}
	else
	{
		DP((L"Add tag failed. FileName: %s\r\n", pszFileName));
		dwErrorID = IDS_UNKNOWN_ERROR;
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);
		}
	}

	FreeAttributes(attrs);

	if(mgr)
	{
		CloseAttributeManager(mgr);
		mgr = NULL;
	}
	return dwErrorID;
}

DWORD CFileTag::RemoveTag(LPCWSTR pszTagName, LPCWSTR pszFile)
{
	if(!pszTagName || !pszFile)
	{
		return FALSE;
	}

	DWORD dwErrorID;
	if(!CheckFileTaggingError(pszFile, m_hWnd, dwErrorID, 1))
	{
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), m_pMgr->GetResString(dwErrorID), m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVE_FAIL), pa_listAttr);
		}
		return dwErrorID;
	}

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);

	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	AddAttributeW(attrs, pszTagName, L"");

	DP((L"Remove tag start: Time: %s, tag name:%s, file name: %s\r\n", FormatCurrentTime().c_str(), pszTagName, pszFile));
	DWORD dwOldTime = ::GetTickCount();
	int nRet = RemoveResourceAttributesW(mgr, pszFile, attrs);
	DWORD dwTime = ::GetTickCount() - dwOldTime;

	if(nRet)
	{
		DP((L"Remove tag end: time:%s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFile));
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			PABase::ATTRIBUTE pa_attr;
			pa_attr.strKey = m_pMgr->GetResString(IDS_TAG_TIME);
			wchar_t szTime[100] = {0};
			_snwprintf_s(szTime, 100, _TRUNCATE, L"%d ms", dwTime);
			pa_attr.strValue = std::wstring(szTime);
			pa_listAttr.push_back(pa_attr);
			pa_attr.strKey = pszTagName;
			pa_listAttr.push_back(pa_attr);
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVETAG), pa_listAttr);
		}
	}
	else
	{
		DP((L"Remove tag failed. FileName: %s\r\n", pszFile));
		dwErrorID = IDS_UNKNOWN_ERROR;

		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVE_FAIL), pa_listAttr);
		}
	}
	
	FreeAttributes(attrs);
	CloseAttributeManager(mgr);

	return 0;
}

DWORD CFileTag::RemoveTag(std::list<std::wstring>* pListTags, LPCWSTR pszFileName)
{
	if(!pszFileName)
		return IDS_FILE_NOT_EXIST;

	DWORD dwErrorID = 0;
	if(!CheckFileTaggingError(pszFileName, m_hWnd, dwErrorID))
	{
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			m_pMgr->AddLog(L"identifer", m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), m_pMgr->GetResString(dwErrorID), m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVE_FAIL), pa_listAttr);
		}
		return dwErrorID;
	}


	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);
	ResourceAttributes *attrs = NULL;
	AllocAttributes(&attrs);

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	std::list<std::wstring>::iterator f_itr;
	for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
	{//
		std::wstring strTagName = *f_itr;

		AddAttributeW(attrs, strTagName.c_str(), L"");
	}


	DP((L"Remove tag start: Time: %s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));
	DWORD dwOldTime = ::GetTickCount();
	
	int nRet = RemoveResourceAttributesW(mgr, pszFileName, attrs);
	DWORD dwTime = ::GetTickCount() - dwOldTime;

	if(nRet)
	{
		DP((L"Remove tag end: time:%s, file name: %s\r\n", FormatCurrentTime().c_str(), pszFileName));
		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			PABase::ATTRIBUTE pa_attr;
			pa_attr.strKey = m_pMgr->GetResString(IDS_TAG_TIME);
			wchar_t szTime[100] = {0};
			_snwprintf_s(szTime, 100, _TRUNCATE, L"%d ms", dwTime);
			pa_attr.strValue = std::wstring(szTime);
			pa_listAttr.push_back(pa_attr);
			for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
			{//
				std::wstring strTagName = *f_itr;
				PABase::ATTRIBUTE pa_attr2;
				pa_attr2.strKey = strTagName;
				pa_listAttr.push_back(pa_attr2);
			}
			
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVETAG), pa_listAttr);
		}
	}
	else
	{
		DP((L"Remove tag failed. FileName: %s\r\n", pszFileName));
		dwErrorID = IDS_UNKNOWN_ERROR;

		if(m_pMgr)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"Identifier");
			m_pMgr->AddLog(szIdentifier, m_pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), L"Options", m_pMgr->GetResString(IDS_FILETAG_DESCRIPTION), m_pMgr->GetResString(IDS_FILETAG_REMOVE_FAIL), pa_listAttr);
		}
	}

	FreeAttributes(attrs);

	if(mgr)
	{
		CloseAttributeManager(mgr);
		mgr = NULL;
	}
	return dwErrorID;
}

DWORD CFileTag::GetTagValueByName(LPCWSTR pszTagName, LPCWSTR pszFile, OUT std::wstring& strTagValue)
{
	if(!pszTagName || !pszFile)
		return IDS_FILE_NOT_EXIST;

	DWORD dwErrorID = 0;

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);

	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	if(attrs)
	{
		int nRet = ReadResourceAttributesW(mgr, pszFile, attrs);

		if(!nRet)
		{
			dwErrorID = IDS_READTAGS_ERROR;
		}
		else
		{
			int size = GetAttributeCount(attrs);

			for (int i = 0; i < size; ++i)
			{
				WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
				WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

				if(0 == _wcsicmp(tagName, pszTagName))
				{
					strTagValue = std::wstring(tagValue);
					break;
				}
			}
		}
		
	}

	FreeAttributes(attrs);
	CloseAttributeManager(mgr);

	return dwErrorID;

}

DWORD CFileTag::GetAllTags(LPCWSTR pszFile, std::list<smart_ptr<FILETAG_PAIR>>* pList)
{
	if(!pList || !pszFile)
		return IDS_FILE_NOT_EXIST;

	DWORD dwErrorID = 0;

	ResourceAttributeManager *mgr = NULL;
	CreateAttributeManager(&mgr);

	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	std::wstring strValue(L"");

	if(!mgr || !attrs)
		return IDS_NOT_ENOUGH_MEMORY;

	if(attrs)
	{
		int nRet = ReadResourceAttributesW(mgr, pszFile, attrs);

		if(!nRet)
		{
			dwErrorID = IDS_READTAGS_ERROR;
		}
		else
		{
			int size = GetAttributeCount(attrs);

			for (int i = 0; i < size; ++i)
			{
				WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
				WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

				smart_ptr<FILETAG_PAIR> spPair(new FILETAG_PAIR);

				spPair->strTagName = std::wstring(tagName);
				spPair->strTagValue = std::wstring(tagValue);
				pList->push_back(spPair);
			}
		}
		
	}

	FreeAttributes(attrs);
	CloseAttributeManager(mgr);

	return dwErrorID;
}

BOOL CFileTag::UpdateIndexTag(std::list<smart_ptr<FILETAG_PAIR>>* pListTags, LPCWSTR pszFileName, BOOL bAdd /* = TRUE */)
{
	if(!pszFileName || !pListTags)
		return FALSE;

	std::list<smart_ptr<FILETAG_PAIR>>::iterator f_itr;
	std::list<std::wstring> listTags;
	listTags.clear();
	for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
	{
		smart_ptr<FILETAG_PAIR> spTag = *f_itr;
		listTags.push_back(spTag->strTagName);
	}

	return UpdateIndexTag(&listTags, pszFileName, bAdd);
	
}

BOOL CFileTag::UpdateIndexTag(std::list<std::wstring>* pListTags, LPCWSTR pszFileName, BOOL bAdd /* = TRUE */)
{
	if(!pszFileName || !pListTags)
		return FALSE;

	DP((L"Update Index Tag for file: %s, bAdd: %d", pszFileName, bAdd));

	std::list<smart_ptr<FILETAG_PAIR>> listTags;
	listTags.clear();
	GetAllTags(pszFileName, &listTags);// Get the existing tags

	std::wstring strValue;
	BOOL bIndexTag = GetIndexTag(&listTags, pszFileName, strValue);// Get Index Tag from the existing tags

	std::wstring strExistingTags(L"");
	if(bIndexTag)
	{
		strExistingTags = strValue;
	}

	std::list<std::wstring> listNXTTags;
	listNXTTags.clear();
	if(strExistingTags.length() > 1)
		Split_NXT_Tags(strExistingTags.c_str(), listNXTTags);//split the content of "Index Tag"

	std::list<std::wstring>::iterator n_itr;
	if(bAdd)
	{
		std::list<std::wstring>::iterator f_itr;
		std::wstring strTags = strExistingTags;
		for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)
		{
			std::wstring strTagName = *f_itr;

			if(!TagExists(strTagName.c_str(), listNXTTags, n_itr))
			{
				strTags.append(strTagName);
				strTags.append(g_szSeparator_IndexTag);
			}
		}

		
		return AddTag(NXTLBS_INDEX_TAG, strTags.c_str(), pszFileName);
	}
	else
	{
		std::list<std::wstring>::iterator f_itr;

		for(f_itr = pListTags->begin(); f_itr != pListTags->end(); f_itr++)//remove the tag names in "Index Tag" for all tags which will be romoved. Kevin Zhou2008-9-21
		{
			std::wstring strTagName = *f_itr;

			if(TagExists(strTagName.c_str(), listNXTTags, n_itr))
			{
				if(n_itr != listNXTTags.end() )
				{
					DP((L"Remove tag, %s\r\n", strTagName.c_str()));
					listNXTTags.erase(n_itr);
				}
			}
		}

		if(listNXTTags.empty())//Remove the "Index Tag" if its value is empty
			return RemoveTag(NXTLBS_INDEX_TAG, pszFileName);

		std::wstring strTags(L"");
		std::list<std::wstring>::iterator itr2;
		for(itr2 = listNXTTags.begin(); itr2 != listNXTTags.end(); itr2++)
		{
			strTags.append(*itr2);
			strTags.append(g_szSeparator_IndexTag);
		}
		
		return AddTag(NXTLBS_INDEX_TAG, strTags.c_str(), pszFileName);
	}
	return FALSE;
}