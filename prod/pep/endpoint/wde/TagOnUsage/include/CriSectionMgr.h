#pragma once

class CCriSectionMgr
{
public:
	CCriSectionMgr(void);
	~CCriSectionMgr(void);
public:
	static CRITICAL_SECTION m_CriItemList;
	static CRITICAL_SECTION m_CriItemList_Shown;
};

