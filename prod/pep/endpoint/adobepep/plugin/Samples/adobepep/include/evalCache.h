#pragma once

#include <string>
#include <vector>
using namespace std;

#define TIMEOUT_VALUE_FOR_CACHE_EDIT (1*60*1000)	//	1 minute
#define TIMEOUT_VALUE_FOR_CACHE (5*1000)	//	5 seconds

class CEvalCache
{
public:
	static CEvalCache* getInstance()
	{
		static CEvalCache ins;
		return &ins;
	}
	/*

	this function will query cache result

	output param value:
	bFound: true means find cache
	bAllow: only when bFound is true, it has meaning, true means cache value is allow
	*/
	void query(const string& src, const string& dest, const string& action, bool& bFound, bool& bAllow,DWORD dwtimeout=TIMEOUT_VALUE_FOR_CACHE)
	{
		EnterCriticalSection(&m_cache_lock);
		vector<EVAL_ITEM>::iterator it=m_cache.begin();
		for (; it!=m_cache.end(); it++)
		{
			if ( (it->action==action) &&
				(it->dst==dest) &&
				(it->src==src) )
			{
				DWORD ticket=::GetTickCount();
				if ( ( ticket - it->dwticket ) > dwtimeout )
				{
					m_cache.erase(it);
				}
				else
				{
					bFound=true;
					bAllow=it->ballow;
					it->dwticket=ticket;
				}
				break;
			}
		}
		LeaveCriticalSection(&m_cache_lock);
	}

	/*
	
	this function will cache query result
	
	*/
	void cache(const string& src, const string& dest, const string& action, bool bAllow)
	{
		EnterCriticalSection(&m_cache_lock);
		bool bfound=false;
		for (DWORD i=0; i<m_cache.size(); i++)
		{
			if ( (m_cache[i].action==action) &&
				(m_cache[i].dst==dest) &&
				(m_cache[i].src==src) )
			{
				bfound=true;
				m_cache[i].dwticket=::GetTickCount();
				m_cache[i].ballow=bAllow;
				break;
			}
		}
		if (bfound==false)
		{
			EVAL_ITEM item;
			item.action=action;
			item.src=src;
			item.dst=dest;
			item.ballow=bAllow;
			item.dwticket=::GetTickCount();
			m_cache.push_back(item);
		}
		LeaveCriticalSection(&m_cache_lock);
	}
private:
	CEvalCache()
	{
		InitializeCriticalSection(&m_cache_lock);
	}
	~CEvalCache()
	{
		DeleteCriticalSection(&m_cache_lock);
	}

	CRITICAL_SECTION m_cache_lock;
	typedef struct 
	{
		string src;
		string dst;
		string action;
		bool ballow;
		DWORD dwticket;
	}EVAL_ITEM;
	vector<EVAL_ITEM> m_cache;
};
