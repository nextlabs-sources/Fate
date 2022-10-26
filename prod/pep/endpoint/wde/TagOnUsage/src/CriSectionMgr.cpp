#include "StdAfx.h"
#include "CriSectionMgr.h"

CRITICAL_SECTION CCriSectionMgr::m_CriItemList;
CRITICAL_SECTION CCriSectionMgr::m_CriItemList_Shown;

CCriSectionMgr::CCriSectionMgr(void)
{
	::InitializeCriticalSection(&m_CriItemList);
	::InitializeCriticalSection(&m_CriItemList_Shown);
}

CCriSectionMgr::~CCriSectionMgr(void)
{
	::DeleteCriticalSection(&m_CriItemList);
	::DeleteCriticalSection(&m_CriItemList_Shown);
}

