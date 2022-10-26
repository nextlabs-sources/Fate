#include "StdAfx.h"
#include "NotifyMgrImp.h"
#include "Actions.h"
#include "RegUtility.h"

typedef struct MonthMap 
{
	wstring strInput;
	wstring strNumber;
}MONTHMAP;

const static MONTHMAP g_szMonth[] = {
	wstring(L"Jan"), wstring(L"1"), \
	wstring(L"Feb"), wstring(L"2"), \
	wstring(L"Mar"), wstring(L"3"), \
	wstring(L"Apr"), wstring(L"4"), \
	wstring(L"May"), wstring(L"5"), \
	wstring(L"Jun"), wstring(L"6"), \
	wstring(L"Jul"), wstring(L"7"), \
	wstring(L"Aug"), wstring(L"8"), \
	wstring(L"Sep"), wstring(L"9"), \
	wstring(L"Oct"), wstring(L"10"), \
	wstring(L"Nov"), wstring(L"11"), \
	wstring(L"Dec"), wstring(L"12"), \
};

CNotifyMgrImp::HourSwitch CNotifyMgrImp::s_arrayHourSwitch[] = {
	wstring(L"00"), wstring(L"12"), wstring(L"am"), \
	wstring(L"01"), wstring(L"1"), wstring(L"am"),	\
	wstring(L"02"), wstring(L"2"), wstring(L"am"), \
	wstring(L"03"), wstring(L"3"), wstring(L"am"), \
	wstring(L"04"), wstring(L"4"), wstring(L"am"), \
	wstring(L"05"), wstring(L"5"), wstring(L"am"),	\
	wstring(L"06"), wstring(L"6"), wstring(L"am"), \
	wstring(L"07"), wstring(L"7"), wstring(L"am"),	\
	wstring(L"08"), wstring(L"8"), wstring(L"am"), \
	wstring(L"09"), wstring(L"9"), wstring(L"am"),	\
	wstring(L"10"), wstring(L"10"), wstring(L"am"), \
	wstring(L"11"), wstring(L"11"), wstring(L"am"),	\
	wstring(L"12"), wstring(L"12"), wstring(L"pm"), \
	wstring(L"13"), wstring(L"1"), wstring(L"pm"),	\
	wstring(L"14"), wstring(L"2"), wstring(L"pm"), \
	wstring(L"15"), wstring(L"3"), wstring(L"pm"),	\
	wstring(L"16"), wstring(L"4"), wstring(L"pm"), \
	wstring(L"17"), wstring(L"5"), wstring(L"pm"),	\
	wstring(L"18"), wstring(L"6"), wstring(L"pm"), \
	wstring(L"19"), wstring(L"7"), wstring(L"pm"),	\
	wstring(L"20"), wstring(L"8"), wstring(L"pm"), \
	wstring(L"21"), wstring(L"9"), wstring(L"pm"),	\
	wstring(L"22"), wstring(L"10"), wstring(L"pm"), \
	wstring(L"23"), wstring(L"11"), wstring(L"pm"),	\
};

CNotifyMgrImp::CNotifyMgrImp(void)
{
	m_bNotifyMgrInited = FALSE;
	m_dwNotifyHistoryMax = 0;

	::InitializeCriticalSection(&m_csList);
}

CNotifyMgrImp::~CNotifyMgrImp(void)
{
	MUTEX mutex(&m_Mutex);
	for (DWORD i = 0; i < m_NotificationArray.size(); i++)
	{
		delete m_NotificationArray[i];
		m_NotificationArray[i] = NULL;
	}

	::DeleteCriticalSection(&m_csList);
}

BOOL CNotifyMgrImp::SetNotifyDisplayLever(NOTIFY_DIS_LEVEL eLevel)
{
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in SetNotifyDisplayLever\n");
		return FALSE;
	}

	//	opened, set level value
	LONG rstatus = RegSetValueEx(
		hEDPManager,
		EDPM_REG_NOTIFY_LEVEL,
		0,
		REG_DWORD,
		(BYTE*)&eLevel,
		sizeof(eLevel));
	if (rstatus)
	{
		g_log.Log(CELOG_DEBUG, L"no permission to set NotifyLevel in registry\n");
		//	error
		CRegUtility::CloseEDPMKey(&hEDPManager);
		return FALSE;
	}

	//	set Ok, close reg key handle and return true
	CRegUtility::CloseEDPMKey(&hEDPManager);
	return TRUE;	
}

BOOL CNotifyMgrImp::GetNotifyDisplayLever(NOTIFY_DIS_LEVEL& eLevel)
{
	//	open EDP Manager key
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		//	failed
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in GetNotifyDisplayLever\n");
		return FALSE;
	}

	//	opened, query key value
	DWORD dwLevel = 0;
	DWORD dwSize = sizeof(dwLevel);

	LONG rstatus = RegQueryValueExW(hEDPManager,EDPM_REG_NOTIFY_LEVEL,NULL,NULL,(LPBYTE)&dwLevel,&dwSize);
	if( rstatus != ERROR_SUCCESS )
	{
		//	query failed
		g_log.Log(CELOG_DEBUG, L"no NotifyLevel in registry, or no permission, we use default value - all\n");
		eLevel = E_ALL;
	}
	else
	{
		eLevel = (NOTIFY_DIS_LEVEL)dwLevel;
	}


	//	close key
	CRegUtility::CloseEDPMKey(&hEDPManager);
	
	return TRUE;
}

BOOL CNotifyMgrImp::SetNotifyDisplayDuration(NOTIFY_DIS_DURATION eDuration)
{
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		//	failed
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in SetNotifyDisplayDuration\n");

		return FALSE;
	}

	//	opened, set level value
	LONG rstatus = RegSetValueEx(
		hEDPManager,
		EDPM_REG_NOTIFY_DURATION,
		0,
		REG_DWORD,
		(BYTE*)&eDuration,
		sizeof(eDuration));
	if (rstatus)
	{
		//	error
		g_log.Log(CELOG_DEBUG, L"no permission to set NotifyDuration in registry\n");

		CRegUtility::CloseEDPMKey(&hEDPManager);
		return FALSE;
	}

	//	set Ok, close reg key handle and return true
	CRegUtility::CloseEDPMKey(&hEDPManager);
	return TRUE;	
}


BOOL CNotifyMgrImp::GetNotifyDisplayDuration(NOTIFY_DIS_DURATION& eDuration)
{
	//	open key \\Software\\Nextlabs\\EDP Manager
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in GetNotifyDisplayDuration\n");
		return FALSE;
	}

	//	opened, query key value
	DWORD dwLevel = 0;
	DWORD dwSize = sizeof(dwLevel);

	LONG rstatus = RegQueryValueExW(hEDPManager,EDPM_REG_NOTIFY_DURATION,NULL,NULL,(LPBYTE)&dwLevel,&dwSize);
	if( rstatus != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"no NotifyDuration in registry, or no permission, we use default 30 seconds\n");
		eDuration = E_30_SECONDS;
	}
	else
	{
		eDuration = (NOTIFY_DIS_DURATION)dwLevel;
	}


	//	close key
	CRegUtility::CloseEDPMKey(&hEDPManager);

	return TRUE;
}

BOOL CNotifyMgrImp::FormatTimeStr(wstring& strTime, OUT wstring& strFormated)
{
	g_log.Log(CELOG_DEBUG, L"Try to format the time from PC: %s\n", strTime.c_str());

	//	get month and date
	wstring::size_type npos = strTime.find(L" ");
	wstring::size_type npos_2 = strTime.find(L" ", npos + 1);
	wstring::size_type npos_3 = strTime.find(L" ", npos_2 + 1);

	if(npos == wstring::npos || npos_2 == wstring::npos || npos_3 == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring month = strTime.substr(npos + 1, npos_2 - npos - 1);
	wstring date = strTime.substr(npos_2 + 1, npos_3 - npos_2 - 1);
			
			//	check if date is "04" -> "4"
// 			if (date[0] == L'0')
// 			{
// 				date = date.substr(1, 1);
// 			}

	wstring monthDate;
	wstring numMonth(L"");
	
	for(int m = 0; m < _countof(g_szMonth); m++)
	{
		if(_wcsicmp(g_szMonth[m].strInput.c_str(), month.c_str()) == 0)
		{
			numMonth = g_szMonth[m].strNumber;
			break;
		}
	}
	if(numMonth.length() > 0)
	{
		monthDate = numMonth + L"/" + date;
	}
	else
	{
		monthDate = month + wstring(L" ") + date;
	}
			
			//	get year
	npos = strTime.rfind(L" ");
	if(npos == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring year = strTime.substr(npos + 1, strTime.length() - npos - 1);

	if(year.length() == 4)
	{
		year = year.substr(2, 2);
	}
			
			//	get hour/minutes/seconds, it is 16:20:25
	npos = strTime.find(L" ", npos_3 + 1);
	if(npos == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring hourMinSeconds = strTime.substr(npos_3 + 1, npos - npos_3 - 1);
			
			//	get hour
			npos = hourMinSeconds.find(L":");
			npos_2 = hourMinSeconds.find(L":", npos + 1);

	if(npos == wstring::npos || npos_2 == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

			wstring hour = hourMinSeconds.substr(0, npos);
			
			g_log.Log(CELOG_DEBUG, L"24 hour is %s\n", hour.c_str());

			//	switch hour from 24 hour to 12 hour
			//	and determine if it is AM or PM
			wstring strAMPM;
			for (DWORD j = 0; j < sizeof(s_arrayHourSwitch)/sizeof(HourSwitch); j++)
			{
				if ( s_arrayHourSwitch[j].str24Hour == hour )
				{
					hour = s_arrayHourSwitch[j].str12tHour;
					strAMPM = s_arrayHourSwitch[j].strAMPM;
					break;
				}
			}
			g_log.Log(CELOG_DEBUG, L"12 hour is %s, %s\n", hour.c_str(), strAMPM.c_str());

			//	get minutes and seconds
			wstring minSeconds = hourMinSeconds.substr(npos + 1, hourMinSeconds.length() - npos - 1);

			//	target format is "11/7/10 4:20pm"
	strFormated = monthDate + wstring(L"/") + year + wstring(L"  ") + hour + wstring(L":") + minSeconds + wstring(L" ") + strAMPM;
	return TRUE;
}

BOOL CNotifyMgrImp::AddNotify(NOTIFY_INFO& notify)
{
	//	parse notify and add notify info into array
	//	this should keep sync with get from array using mutex
	MUTEX mutex(&m_Mutex);


	//	init notifymgr first
	//	we always call it, it will only init once
	if (!m_bNotifyMgrInited)
	{
		NotifyMgrInit();
	}

	NotificationInfo * pInfo = NULL;


	//	create a NotificationInfo object for each message
	//	and add each object into vector.
	for (int i = 3; i < 7; i++)
	{
		//Always add first message. For the rest, only add if present
		if (i == 3 || wcslen(notify.params [i]) != 0)
		{
			//	i could be from 3 to 6, when code comes here, there is a new notify.
			pInfo = new NotificationInfo;

			//	parse time
			//	original one is: Monday May 27 16:20:25 CST 2010
			wstring time = notify.params[0];

			CNotifyMgrImp::FormatTimeStr(time, pInfo->time);
			
			//	parse enforcement
			pInfo->enforcement = DENY; // default to DENY
			if (notify.params[1][0] == L'D')
				pInfo->enforcement = DENY;
			else if (notify.params[1][0] == L'A')
				pInfo->enforcement = ALLOW;

			//	parse action
			pInfo->action = &(notify.params[1][1]);

			//	parse source resource title
			pInfo->file = notify.params[2];

			//	parse notification message string
			pInfo->message = notify.params[i];

			::EnterCriticalSection(&m_csList);
			//	check if history array is full
			if(m_dwNotifyHistoryMax == m_NotificationArray.size() )
			{
				//	yes, it is full, pop up one
				NotificationInfo *last_pInfo = m_NotificationArray.at(m_dwNotifyHistoryMax - 1 );
				m_NotificationArray.pop_back();
				delete last_pInfo;
			}
			//	insert new one
			m_NotificationArray.insert(m_NotificationArray.begin(), pInfo);

			::LeaveCriticalSection(&m_csList);
		}
	}

	
	return TRUE;
}


void CNotifyMgrImp::NotifyMgrInit()
{
	if (m_bNotifyMgrInited)
	{
		return;
	}

	//	read m_dwNotifyHistoryMax from registry
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in NotifyMgrInit\n");
		return;
	}
	
	//	opened, query key value
	DWORD dwNumber = 0;
	DWORD dwSize = sizeof(dwNumber);

	LONG rstatus = RegQueryValueExW(hEDPManager,EDPM_REG_NOTIFY_MAX,NULL,NULL,(LPBYTE)&dwNumber,&dwSize);
	if( rstatus != ERROR_SUCCESS )
	{
		//	query failed

		//	set default notify history max value
		m_dwNotifyHistoryMax = 300;		

		//	create one reg
		rstatus = RegSetValueEx(
			hEDPManager,
			EDPM_REG_NOTIFY_MAX,
			0,
			REG_DWORD,
			(BYTE*)&m_dwNotifyHistoryMax,
			sizeof(m_dwNotifyHistoryMax));
		if (rstatus != ERROR_SUCCESS)
		{
			g_log.Log(CELOG_DEBUG, L"create MaxNotifyCache failed in registry\n");
		}

		//	close handle
		CRegUtility::CloseEDPMKey(&hEDPManager);

		g_log.Log(CELOG_DEBUG, L"no MaxNotifyCache in registry, we use default 300\n");
	}
	else
	{
		//	set max notify history
		m_dwNotifyHistoryMax = dwNumber;
	}


	//	close key
	CRegUtility::CloseEDPMKey(&hEDPManager);

	m_bNotifyMgrInited = TRUE;
	return;
}

BOOL CNotifyMgrImp::GetNotifyHistory(NotifyInfo* pNotifies, int nCount)
{
	MUTEX mutex(&m_Mutex);

	int i = 0;
	NotificationVector::reverse_iterator riter;

	::EnterCriticalSection(&m_csList);
	for(riter = m_NotificationArray.rbegin(); riter != m_NotificationArray.rend() && i < nCount; riter++)
	{
		memset(&(pNotifies[i]), 0, sizeof(NotifyInfo));
		wcsncpy_s(pNotifies[i].action, sizeof(pNotifies[i].action)/sizeof(wchar_t), (*riter)->action.c_str(), _TRUNCATE);
		wcsncpy_s(pNotifies[i].file, sizeof(pNotifies[i].file)/sizeof(wchar_t), (*riter)->file.c_str(), _TRUNCATE);
		wcsncpy_s(pNotifies[i].message, sizeof(pNotifies[i].message)/sizeof(wchar_t), (*riter)->message.c_str(), _TRUNCATE);
		wcsncpy_s(pNotifies[i].time, sizeof(pNotifies[i].time)/sizeof(wchar_t), (*riter)->time.c_str(), _TRUNCATE);

		pNotifies[i].enforcement = (*riter)->enforcement;

		i++;
	}

	::LeaveCriticalSection(&m_csList);
	return TRUE;
}

int CNotifyMgrImp::GetNotifyCount()
{
	::EnterCriticalSection(&m_csList);
	unsigned uCount = (unsigned)m_NotificationArray.size();
	::LeaveCriticalSection(&m_csList);
	return (int)uCount;
}

/*

not used

*/
BOOL CNotifyMgrImp::ParseNotify(NOTIFY_INFO& notify)
{

	//Always add first message. For the rest, only add if present
	if (wcslen(notify.params [3]) != 0)
	{
		//	i could be from 3 to 6, when code comes here, there is a new notify.
		NotificationInfo* pInfo = new NotificationInfo;

		//	parse time
		//	original one is: Monday May 27 16:20:25 CST 2010
		wstring time = notify.params[0];

		//	get month and date
		wstring::size_type npos = time.find(L" ");
		wstring::size_type npos_2 = time.find(L" ", npos + 1);
		wstring::size_type npos_3 = time.find(L" ", npos_2 + 1);

		wstring month = time.substr(npos + 1, npos_2 - npos - 1);
		wstring date = time.substr(npos_2 + 1, npos_3 - npos_2 - 1);

		//	check if date is "04" -> "4"
		if (date[0] == L'0')
		{
			date = date.substr(1, 1);
		}

		wstring monthDate = month + wstring(L" ") + date;

		//	get year
		npos = time.rfind(L" ");
		wstring year = time.substr(npos + 1, time.length() - npos - 1);

		//	get hour/minutes/seconds, it is 16:20:25
		npos = time.find(L" ", npos_3 + 1);
		wstring hourMinSeconds = time.substr(npos_3 + 1, npos - npos_3 - 1);
		
		//	change it to 4:20PM..................
		
		//	get hour
		npos = hourMinSeconds.find(L":");
		npos_2 = hourMinSeconds.find(L":", npos + 1);
		wstring hour = hourMinSeconds.substr(0, npos);

		g_log.Log(CELOG_DEBUG, L"24 hour is %s\n", hour.c_str());

		//	switch hour from 24 hour to 12 hour
		//	and determine if it is AM or PM
		wstring strAMPM;
		for (DWORD i = 0; i < sizeof(s_arrayHourSwitch)/sizeof(HourSwitch); i++)
		{
			if ( s_arrayHourSwitch[i].str24Hour == hour )
			{
				hour = s_arrayHourSwitch[i].str12tHour;
				strAMPM = s_arrayHourSwitch[i].strAMPM;
				break;
		}
		}
		g_log.Log(CELOG_DEBUG, L"12 hour is %s, %s\n", hour.c_str(), strAMPM.c_str());

		//	get minutes and seconds
		wstring minSeconds = hourMinSeconds.substr(npos + 1, hourMinSeconds.length() - npos - 1);

		//	target format is "May 27, 2010  4:20PM"
		pInfo->time = monthDate + wstring(L", ") + year + wstring(L"  ") + hour + wstring(L":") + minSeconds + strAMPM;

		//	parse enforcement
		pInfo->enforcement = DENY; // default to DENY
		if (notify.params[1][0] == L'D')
			pInfo->enforcement = DENY;
		else if (notify.params[1][0] == L'A')
			pInfo->enforcement = ALLOW;

		//	parse action
		pInfo->action = &(notify.params[1][1]);

		//	parse source resource title
		pInfo->file = notify.params[2];

		//	parse notification message string
		pInfo->message = notify.params[3];
	}

	return TRUE;
}

