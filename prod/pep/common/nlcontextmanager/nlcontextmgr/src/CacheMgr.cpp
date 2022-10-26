#include "StdAfx.h"
#include "CacheMgr.h"
#include "CriticalSections.h"


CCacheMgr::CCacheMgr(void)
{
}

CCacheMgr::~CCacheMgr(void)
{
}

CCacheMgr* CCacheMgr::CreateInst()
{
	static CCacheMgr inst;
	return &inst;
}

void CCacheMgr::CacheData(CPackage* pack)
{
	EnterCriticalSection(&CCriticalSections::m_csMap);
	m_mapCache[pack->GetKey()] = pack->GetValue();
	LeaveCriticalSection(&CCriticalSections::m_csMap);
}

cache_value CCacheMgr::GetData(CPackage* pack, bool forcedelete)
{
	cache_value ret;
			
	EnterCriticalSection(&CCriticalSections::m_csMap);
	boost::unordered_map<cache_key, cache_value>::iterator itr = m_mapCache.find(pack->GetKey());

	if (itr != m_mapCache.end())
	{
		ret = (*itr).second;

		if (forcedelete)
		{
			m_mapCache.erase(itr);
		}
	}
	
	LeaveCriticalSection(&CCriticalSections::m_csMap);
		
	return ret;
}