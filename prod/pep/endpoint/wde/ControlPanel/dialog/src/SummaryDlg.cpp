#include "StdAfx.h"
#include <assert.h>
#include "SummaryDlg.h"
#include "NL_ListView.h"
#include "EDPMgrUtilities.h"
#include "dsipc.h"
#include "IPCProxy.h"

#define NL_EDPM_TYPE_MAX		281
#define NL_EDPM_VERSION_MAX		90
#define NL_EDPM_DATE_MAX		90

const TCHAR *  IPC_CM_REQUEST_HANDLER   = _T("com.bluejungle.destiny.agent.controlmanager.CMRequestHandler");
const TCHAR *  GET_AGENT_INFO = _T("getAgentInfo");

BOOL g_bAscending = FALSE;
int g_nColSort = -1;

static   int   CALLBACK MyCompareProc(LPARAM   lParam1,   LPARAM   lParam2,   LPARAM   lParamSort) 
{ 
	if(g_nColSort == -1)
	{
		return 0;
	}

	HWND hWnd = (HWND)lParamSort;
	wchar_t buf1[1024] = {0};
	wchar_t buf2[1024] = {0};
	ListView_GetItemText(hWnd, lParam1, g_nColSort, buf1, 1024);
	ListView_GetItemText(hWnd, lParam2, g_nColSort, buf2, 1024);	
	
	if(g_bAscending)
	{
		return wcscmp(buf1, buf2);
	}
	else
	{
		return wcscmp(buf2, buf1);
	}
	
} 

void RetrieveLastUpdatedBundleTime(void* pParam)
{
	if(!pParam)
	{
		return;
	}

	CSummaryDlg* pDlg = (CSummaryDlg*)pParam;
	if(pDlg)
	{
		pDlg->ShowLastUpdatedBundleTime();
	}

	return;
}

CSummaryDlg::CSummaryDlg(void)
{
	m_hBold = CreateFontW(g_nFontSizeBig,
							0,
							0,
							0,
							FW_NORMAL,
							0,
							0,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);

	m_hNormal = CreateFontW(g_nFontSize,
							0,
							0,
							0,
							FW_NORMAL,
							0,
							0,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);


	m_mapMonth[L"Jan"] = wstring(L"1");
	m_mapMonth[L"Feb"] = wstring(L"2");
	m_mapMonth[L"Mar"] = wstring(L"3");
	m_mapMonth[L"Apr"] = wstring(L"4");
	m_mapMonth[L"May"] = wstring(L"5");
	m_mapMonth[L"Jun"] = wstring(L"6");
	m_mapMonth[L"Jul"] = wstring(L"7");
	m_mapMonth[L"Aug"] = wstring(L"8");
	m_mapMonth[L"Sep"] = wstring(L"9");
	m_mapMonth[L"Oct"] = wstring(L"10");
	m_mapMonth[L"Nov"] = wstring(L"11");
	m_mapMonth[L"Dec"] = wstring(L"12");

	m_hGetBundleTimeThread = 0;

}

CSummaryDlg::~CSummaryDlg(void)
{
	if(m_hBold)
	{
		DeleteObject(m_hBold);
		m_hBold = NULL;
	}

	if(m_hNormal)
	{
		DeleteObject(m_hNormal);
		m_hNormal = NULL;
	}
}

void CSummaryDlg::ShowLastUpdatedBundleTime()
{
	//	get last updated bundle time
	wstring lastUpdateTime;
	
//	if (GetLastUpdateBundle(lastUpdateTime))
	if(GetBundleLastModifiedTime(lastUpdateTime))
	{
		//	yes, we get it
		m_lastUpdateTime = lastUpdateTime;
	}
	else
	{
		lastUpdateTime = m_lastUpdateTime;
	}

	::SendMessage(m_hWnd, WM_SHOWLASTUPDATEDBUNDLE, (WPARAM)lastUpdateTime.c_str(), 0);
}

LRESULT CSummaryDlg::OnShowLastUpdatedBundle(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(wParam)
	{
		LPCWSTR lpszText = (LPCWSTR)wParam;
		SetDlgItemText(IDC_STATIC_LAST_BUNDLE_TIME, lpszText);
	}

	return 0;
}

LRESULT CSummaryDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	//	fill data for installed component
	DWORD dwStart = GetTickCount();
	FillData();

	g_log.Log(CELOG_DEBUG, L"Get installed components used: %d ms\r\n", GetTickCount() - dwStart);

	dwStart = GetTickCount();
//	m_hGetBundleTimeThread = (HANDLE)_beginthread(RetrieveLastUpdatedBundleTime, 0, this);
	RetrieveLastUpdatedBundleTime(this);//we don't need to create a thread since we are reading the "last updated policy" from the "last modified time" of bundle.exe, this is very quick.

	g_log.Log(CELOG_DEBUG, L"Get \"last updated bundle\" used: %d ms\r\n", GetTickCount() - dwStart);
	
	return TRUE;
}


void CSummaryDlg::FillData()
{
	m_compnentList.SubclassWindow(GetDlgItem(IDC_LIST_INSTALLED_COMPONENT));
	m_compnentList.ForceMeasureItemMessage();
	m_compnentList.SetBkColor(g_clrHighlight);
	m_compnentList.SetFont(m_hNormal);
	m_compnentList.SetHeaderFont(m_hNormal);

	//	set column into list view control
	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;    
	lvColumn.cchTextMax = 0;
	lvColumn.pszText = L"Type";
	lvColumn.cx = NL_EDPM_TYPE_MAX;
	m_compnentList.InsertColumn(0, &lvColumn);

	lvColumn.pszText = L"Version";
	lvColumn.cx = NL_EDPM_VERSION_MAX;
	m_compnentList.InsertColumn(1, &lvColumn);

	lvColumn.pszText = L"Last Updated";
	lvColumn.cx = NL_EDPM_DATE_MAX;
	m_compnentList.InsertColumn(2, &lvColumn);

	CEDPMUtilities& edpmgr_utility = CEDPMUtilities::GetInstance();

	vector<pair<wstring, wstring>> v_version;
	vector<pair<wstring, wstring>> v_lastDate;

	edpmgr_utility.GetComponentsVersion(v_version);
	edpmgr_utility.GetComponentsLastUpdatedDate(v_lastDate);
	
	assert(v_version.size() == v_lastDate.size());

	for (DWORD i = 0; i < v_version.size(); i++)
	{
		//	insert a new item
		LVITEM lvItem;
		memset (&lvItem, 0, sizeof (lvItem));
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.cchTextMax = (int)v_version[i].first.length();
		lvItem.pszText = (LPWSTR)v_version[i].first.c_str();    //	this is component type, such as "Desktop Enforcer"
		lvItem.lParam = i;
		m_compnentList.InsertItem(&lvItem);

		//	set text of other column(sub item) in this item
		m_compnentList.SetItemText(i, 1, (LPWSTR)v_version[i].second.c_str());	//	this is component version, such as "4.5.0.0"

		
		wstring newDate;
		if (i < v_lastDate.size())
		{
			newDate = v_lastDate[i].second;
		}
		
		m_compnentList.SetItemText(i, 2, (LPWSTR)newDate.c_str());	//	this is component version, such as "5/28/10"
		//	we finish one item, continue
	}

	//for test start
// 	for (int j = 0; j < 8; j++)
// 	{
// 		LVITEM lvItem;
// 		memset (&lvItem, 0, sizeof (lvItem));
// 		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
// 		lvItem.iItem = j + 4;
// 		lvItem.cchTextMax = 5;
// 		wchar_t buf[100] = {0};
// 		swprintf_s(buf, L"hello%d", j);
// 		lvItem.pszText = (LPWSTR)buf;    
// 		lvItem.lParam = lvItem.iItem;
// 		m_compnentList.InsertItem(&lvItem);
// 	}
	//end
	//	we finish all items
	return;
}
LRESULT CSummaryDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	m_compnentList.UnsubclassWindow();

	return 0;
}



LRESULT CSummaryDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CWindow parent = GetParent();
	SendMessage(parent.m_hWnd, WM_COMMAND, IDOK, NULL);
	return 0;
}


BOOL CSummaryDlg::GetLastUpdateBundle(wstring& strLastUpdateTime)
{
	DWORD dwStart = GetTickCount();

	g_log.Log(CELOG_DEBUG, L"begin to GetLastUpdateBundle\n");

	BOOL bAgentRunning = FALSE;
	WCHAR lastUpdateTime [256] = {0};
	IPCProxy* pIPCProxy = new IPCProxy ();
	if (pIPCProxy != NULL)
	{
		if (pIPCProxy->Init(IPC_CM_REQUEST_HANDLER))
		{
			IPCREQUEST request, response;

			memset (&request, 0, sizeof (IPCREQUEST));
			request.ulSize = sizeof (IPCREQUEST);

			memset (&response, 0, sizeof (IPCREQUEST));
			response.ulSize = sizeof (IPCREQUEST);

			wcsncpy_s (request.methodName, 64, GET_AGENT_INFO, _TRUNCATE);
			if (pIPCProxy->Invoke (&request, &response))
			{
				bAgentRunning = TRUE;
				wcsncpy_s (lastUpdateTime, sizeof(lastUpdateTime)/sizeof(WCHAR),
					response.params[0], _TRUNCATE);
			}
			else
			{
				//	log it, error happen when call to invoke
				g_log.Log(CELOG_ERR, L"pIPCProxy->Invoke fail, so, can't get last bundle update time\n");
			}
		}
		else
		{
			//	log it, error happen when call to init
			g_log.Log(CELOG_ERR, L"pIPCProxy->init fail, so, can't get last bundle update time\n");
		}
		delete (pIPCProxy);
	}

	g_log.Log(CELOG_DEBUG, L"Call API to get last updated bundle time, elapsed: %d ms\n", GetTickCount() -dwStart);

	if (bAgentRunning && wcslen(lastUpdateTime) > 0)
	{
		g_log.Log(CELOG_DEBUG, L"Bundle updated time: %s\n", lastUpdateTime);
		//	original format is --	Jun 8, 2010 9:56:59 AM
		//	target format is	--	7/8/10  9:56AM

		//	get month
		wstring time = wstring(lastUpdateTime);
		wstring::size_type pos = time.find(L' ');

		if(pos == wstring::npos)
		{
			strLastUpdateTime = wstring(lastUpdateTime);
			return TRUE;
		}

		wstring strMonth = time.substr(0, pos);
		wstring strTargetMonth = m_mapMonth[strMonth];
		
		//	get date
		wstring::size_type pos2 = time.find(L',', pos + 1);

		if(pos2 == wstring::npos)
		{
			strLastUpdateTime = wstring(lastUpdateTime);
			return TRUE;
		}

		wstring strTargetDate = time.substr(pos + 1, pos2 - pos - 1);


		//	get year
		wstring strTargetYear = time.substr(pos2 + 4, 2);

		//	get hour+minutes
		pos = time.rfind(L':');

		if(pos == wstring::npos)
		{
			strLastUpdateTime = wstring(lastUpdateTime);
			return TRUE;
		}

		wstring strTargetHourMinutes = time.substr(pos2 + 7, pos - pos2 - 7);

		//	get AM/PM
		pos = time.rfind(L' ');

		if(pos == wstring::npos)
		{
			strLastUpdateTime = wstring(lastUpdateTime);
			return TRUE;
		}

		wstring strTargetAMPM = time.substr(pos + 1, 2);

		//	compose them
		strLastUpdateTime = strTargetMonth + wstring(L"/") + strTargetDate + wstring(L"/") + strTargetYear + wstring(L"   ") + strTargetHourMinutes + strTargetAMPM;
		return TRUE;
	}

	return (FALSE);
}

BOOL CSummaryDlg::GetBundleLastModifiedTime(wstring& lastUpdateTime)
{
	wstring strPCDir;
	if(edp_manager::CCommonUtilities::GetPCInstallPath(strPCDir))
	{
		wstring strBundlePath = strPCDir + L"\\bundle.bin";

		g_log.Log(CELOG_DEBUG, L"EDPManager::Try to get last modified time of file: %s\n", strBundlePath.c_str());

		wchar_t szLastModifiedTime[100] = {0};
		
		if(getFileLastModifiedTime(strBundlePath.c_str(), szLastModifiedTime, 100))
		{
			lastUpdateTime = szLastModifiedTime;

			g_log.Log(CELOG_DEBUG, L"EDPManager::last modified time: %s, file: %s\n", lastUpdateTime.c_str(), strBundlePath.c_str());

			return TRUE;
		}
	}

	return FALSE;
}

LRESULT CSummaryDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	//Draw line
	HWND hHeartBeat = GetDlgItem(IDC_HEARTBEAT);
	if(hHeartBeat)
	{
		RECT rc;
		::GetWindowRect(hHeartBeat, &rc);
		ScreenToClient(&rc);

		RECT rc2;
		::GetWindowRect(GetDlgItem(IDC_LIST_INSTALLED_COMPONENT), &rc2);
		ScreenToClient(&rc2);

		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(123, 166, 198));
		HPEN hOld = (HPEN)SelectObject(hdc, hPen);
		MoveToEx(hdc, rc.left + 60,rc.top + 8, NULL);
		LineTo(hdc, rc2.right, rc.top + 8);
		SelectObject(hdc, hOld);
	}

	::EndPaint(m_hWnd, &ps); 

	return 0;
}

LRESULT CSummaryDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;
	HWND hWnd = (HWND)lParam;
	
	SetBkMode(dc, TRANSPARENT);
	if(hWnd == GetDlgItem(IDC_INSTALLEDCOMPONENTS) )
	{
		SetTextColor(dc, RGB(0, 51, 153));
		SelectObject(dc, m_hBold);
	}
	else
	{
		SelectObject(dc, m_hNormal);
	}

	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}


	return   (LRESULT)hbr; 

}

LRESULT CSummaryDlg::OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}
	return   (LRESULT)hbr; 
}

LRESULT CSummaryDlg::OnCtlColorBtn(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
}
LRESULT CSummaryDlg::OnLvnColumnclickListInstalledComponent(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	if(pNMLV)
	{
		if(m_mapSort.find(pNMLV->iSubItem) == m_mapSort.end())
		{
			m_mapSort[pNMLV->iSubItem] = 1;//ascending
			g_bAscending = TRUE;
		}
		else
		{
			if ( m_mapSort[pNMLV->iSubItem] == 1)
			{
				//Sort by descending if the previous sorting is ascending
				g_bAscending = FALSE;
				m_mapSort[pNMLV->iSubItem] = 0;
			}
			else
			{
				g_bAscending = TRUE;
				m_mapSort[pNMLV->iSubItem] = 1;
			}
		}

		g_nColSort = pNMLV->iSubItem;

		//Update the lParam before sorting since we used lParam as the index
		int nCount = m_compnentList.GetItemCount();
		for(int i = 0; i < nCount; i++)
		{
			LVITEM item;
			memset(&item, 0, sizeof(LVITEM));
			item.iItem = i;
			item.mask = LVIF_PARAM;
			item.lParam = i;
			m_compnentList.SetItem(&item);
		}

		ListView_SortItems(m_compnentList.m_hWnd, MyCompareProc, m_compnentList.m_hWnd);
		
	}
	return 0;
}
