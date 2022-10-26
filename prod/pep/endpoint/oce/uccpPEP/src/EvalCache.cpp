#include "EvalCache.h"
extern CELog g_log;

#define CEVAL_CACHE_HINT 500	//	0.500 second

CEvalCache::CEvalCache(void)
{
	InitializeCriticalSection(&m_cs);
}

CEvalCache::~CEvalCache(void)
{
	DeleteCriticalSection(&m_cs);
}

CEvalCache* CEvalCache::GetInstance()
{
	static CEvalCache ins;
	return &ins;
}

//	we only cache "allow" enforcement
void CEvalCache::commit(const wstring& eval_key)
{
	EnterCriticalSection(&m_cs);

	DWORD dwCurrentTime = GetTickCount();


	//	traverse and erase the one that expires
	map<wstring, DWORD>::iterator  iter;
	for(iter = m_eval_cache.begin(); iter != m_eval_cache.end(); )
	{
		if ((dwCurrentTime - iter->second) > CEVAL_CACHE_HINT)
		{
			//	expired
			g_log.Log(CELOG_DEBUG, L"[CEvalCache] expired [%s]\n", iter->first.c_str());
			iter = m_eval_cache.erase(iter);
			//iter = m_eval_cache.begin();
			continue;
		}

		iter++;
	}

	//	commit eval_key and current time
	m_eval_cache[eval_key] = dwCurrentTime;
	g_log.Log(CELOG_DEBUG, L"[CEvalCache] commit [%s]\n", eval_key.c_str());

	LeaveCriticalSection(&m_cs);
}

//	if return false, means there is no cache for eval_key, otherwise return true
bool CEvalCache::query(const wstring& eval_key)
{
	bool bFind = false;

	EnterCriticalSection(&m_cs);

	map<wstring, DWORD>::iterator  iter;
	iter = m_eval_cache.find(eval_key);
	if (iter != m_eval_cache.end())
	{
		DWORD dwCurrentTime = GetTickCount();
		if ( (dwCurrentTime - iter->second) > CEVAL_CACHE_HINT )
		{
			//	expired, erase and will return false
			g_log.Log(CELOG_DEBUG, L"[CEvalCache] find expired cache [%s]\n", iter->first.c_str());
			m_eval_cache.erase(iter);
			bFind = false;
		}
		else
		{
			//	find and valid
			g_log.Log(CELOG_DEBUG, L"[CEvalCache] find valid cache [%s]\n", iter->first.c_str());
			bFind = true;
		}
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"[CEvalCache] find no cache for [%s]\n", eval_key.c_str());
	}

	LeaveCriticalSection(&m_cs);

	return bFind;
}
