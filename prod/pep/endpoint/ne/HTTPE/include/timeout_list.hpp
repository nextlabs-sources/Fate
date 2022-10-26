#ifndef _TIMEOUT_LIST_H_
#define _TIMEOUT_LIST_H_

#include <string>
#include <list>
#define TIMEOUT_DEFAULT				(5 * 60 * 1000)

struct struTimeOutItem
{
	std::wstring strKey;
	DWORD   dwEntryTime;
	std::wstring strValue;
};

class CTimeoutList
{
public:
	CTimeoutList()
	{
		m_dwTimeout = TIMEOUT_DEFAULT;
		m_list.clear();
		::InitializeCriticalSection(&m_csList);
	}
	CTimeoutList(DWORD dwTimeOut)
	{
		m_dwTimeout = dwTimeOut;
		m_list.clear();
		::InitializeCriticalSection(&m_csList);
	}
	~CTimeoutList()
	{
		::DeleteCriticalSection(&m_csList);
	}

	void AddItem(const std::wstring & strKey, const std::wstring & strValue = L"")
	{
		DeleteItem(strKey);//remove the item first.

		struTimeOutItem item;
		item.strKey = strKey;
		item.dwEntryTime = GetTickCount();
		item.strValue = strValue;

		EnterCriticalSection(&m_csList);
		m_list.push_back(item);//add the new item at back.
		LeaveCriticalSection(&m_csList);
	}

	BOOL FindItem(const std::wstring & strKey)
	{
		BOOL bSuccess = FALSE;
		DWORD dwCurTick = GetTickCount();

		EnterCriticalSection(&m_csList);

		for(std::list<struTimeOutItem>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{
			//remove all the items which are expired.
			if ( dwCurTick - (*itr).dwEntryTime > m_dwTimeout)
			{
				itr = m_list.erase(itr);
				continue;
			}
			
			if(_wcsicmp((*itr).strKey.c_str(), strKey.c_str()) == 0)
			{
				bSuccess = TRUE;
				break;
			}

			itr++;
		}
		LeaveCriticalSection(&m_csList);

		return bSuccess;
	}

	BOOL FindItem(const std::wstring & strKey, std::wstring& strValue)
	{
		BOOL bSuccess = FALSE;
		DWORD dwCurTick = GetTickCount();

		EnterCriticalSection(&m_csList);
		
		for(std::list<struTimeOutItem>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{
			//remove all the items which are expired.
			if ( dwCurTick - (*itr).dwEntryTime > m_dwTimeout)
			{
				itr = m_list.erase(itr);
				continue;
			}

			if(_wcsicmp((*itr).strKey.c_str(), strKey.c_str()) == 0)
			{
				bSuccess = TRUE;
				strValue = (*itr).strValue;
				break;
			}

			itr++;
		}
		LeaveCriticalSection(&m_csList);

		return bSuccess;
	}

	BOOL ContainKey(const std::wstring & strKey, std::wstring& strOriginalKey, std::wstring& strValue, bool bDelete = false)
	{
		BOOL bSuccess = FALSE;
		DWORD dwCurTick = GetTickCount();

		EnterCriticalSection(&m_csList);

		for(std::list<struTimeOutItem>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{
			//remove all the items which are expired.
			if ( dwCurTick - (*itr).dwEntryTime > m_dwTimeout)
			{
				itr = m_list.erase(itr);
				continue;
			}

			if((*itr).strKey.find(strKey) != string::npos)
			{
				bSuccess = TRUE;
				strOriginalKey = (*itr).strKey;
				strValue = (*itr).strValue;
				if(bDelete)
				{
					m_list.erase(itr);
				}
				break;
			}

			itr++;
		}
		LeaveCriticalSection(&m_csList);

		return bSuccess;
	}

	void DeleteItem(const std::wstring & strKey)
	{
		DWORD dwCurTick = GetTickCount();

		EnterCriticalSection(&m_csList);

		for(std::list<struTimeOutItem>::iterator itr = m_list.begin(); itr != m_list.end(); )
		{
			//remove all the items which are expired.
			if ( dwCurTick - (*itr).dwEntryTime > m_dwTimeout)
			{
				itr = m_list.erase(itr);
				continue;
			}

			if(_wcsicmp((*itr).strKey.c_str(), strKey.c_str()) == 0)//remove the item if it was found.
			{
				m_list.erase(itr);
				break;
			}
			itr++;
		}

		LeaveCriticalSection(&m_csList);
	}

	bool GetLastItem(wstring& strKey, wstring& strValue)
	{
		EnterCriticalSection(&m_csList);
		if(m_list.size() > 0)
		{
			strKey = m_list.back().strKey;
			strValue = m_list.back().strValue;
			LeaveCriticalSection(&m_csList);
			return true;
		}
		LeaveCriticalSection(&m_csList);
		return false;
	}
protected:
	DWORD m_dwTimeout;
	std::list<struTimeOutItem> m_list;
	CRITICAL_SECTION m_csList;
};



#endif