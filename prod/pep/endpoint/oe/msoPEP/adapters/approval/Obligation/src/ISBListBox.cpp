#include "StdAfx.h"
#include "ISBListBox.h"

CISBListBox::CISBListBox(void)
{
	m_nMaxWidth=0;
}

CISBListBox::~CISBListBox(void)
{
}
static BOOL IsLocalDrive(LPCWSTR wzPath)
{
	if(wcslen(wzPath) < 2)
		return FALSE;
	if(L':'!=wzPath[1] || wzPath[0] < L'A' ||  (wzPath[0] > L'Z' && wzPath[0] < L'a') || wzPath[0] > L'z')
		return FALSE;

	return TRUE;
}

#define NETMAP_HEAD	L"\\Device\\LanmanRedirector\\;"
static BOOL IsMapped(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
	BOOL bIsMapped = FALSE;
	if(IsLocalDrive(wzPath))
	{
		WCHAR wzDriver[3]; wzDriver[0] = wzPath[0]; wzDriver[1]=L':'; wzDriver[2] = 0;
		WCHAR wzRealDriver[1024+1];	memset(wzRealDriver, 0, sizeof(wzRealDriver));
		int nLeft = 0;
		if(0 < QueryDosDeviceW(wzDriver, wzRealDriver, 1024))
		{
			if(0 == wcsncmp(wzRealDriver, L"\\??\\", 4))
			{
				bIsMapped = TRUE;
				wcsncpy_s(wzRealPath, cch, wzRealDriver+4, _TRUNCATE);
				nLeft = cch - (int)wcslen(wzRealPath);
				wcsncat_s(wzRealPath, cch, wzPath+2, _TRUNCATE);
			}
			else if(0 == _wcsnicmp(wzRealDriver, NETMAP_HEAD, (int)wcslen(NETMAP_HEAD)))
			{
				bIsMapped = TRUE;
				WCHAR* pStart = wcsstr(wzRealDriver+(int)wcslen(NETMAP_HEAD), L"\\");
				if(pStart)//((int)wcslen(wzRealDriver) >  (int)wcslen(NETMAP_HEAD)+18)
				{
					wcsncpy_s(wzRealPath, cch, L"\\", _TRUNCATE);
					wcsncpy_s(wzRealPath+1, cch+1, pStart, _TRUNCATE);//wzRealDriver+(int)wcslen(NETMAP_HEAD)+18, cch);

					// Now we get the remote path here
					nLeft = cch - (int)wcslen(wzRealPath);
					wcsncat_s(wzRealPath, cch, wzPath+2, _TRUNCATE);
				}
				else
				{
					wcsncpy_s(wzRealPath, cch, wzPath, _TRUNCATE);
				}

			}
		}
	}

	return bIsMapped;
}

static void GetRealPath(LPCWSTR wzPath, LPWSTR wzRealPath, int cch)
{
    WCHAR wzTempPath[1025] = {0};

	wcsncpy_s(wzTempPath, 1024, wzPath, _TRUNCATE);
	while(IsMapped(wzTempPath, wzRealPath, cch))
	{
		wcsncpy_s(wzTempPath, 1024, wzRealPath, _TRUNCATE);
	}
	wcsncpy_s(wzRealPath, cch, wzTempPath, _TRUNCATE);
}
int CISBListBox::AddString(LPCTSTR lpszItem)
{
	WCHAR wzItem[1025];memset(wzItem,0,sizeof(wzItem));
	GetRealPath(lpszItem,wzItem,sizeof(wzItem)/sizeof(WCHAR));

	int nRet=CListBox::AddString(wzItem);
	//int nRet=(int)::SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM)wzItem);
	SCROLLINFO scrollInfo;
	memset(&scrollInfo,0,sizeof(scrollInfo));
	scrollInfo.cbSize=sizeof(scrollInfo);
	scrollInfo.fMask=SIF_ALL;
	GetScrollInfo(SB_VERT,&scrollInfo,SIF_ALL);

	int nScrollWidth=0;
	if(GetCount()>1&&((int)scrollInfo.nMax>=(int)scrollInfo.nPage))
	{
		nScrollWidth=GetSystemMetrics(SM_CXVSCROLL);
	}

	SIZE sSize;
	CClientDC myDC(this);
	
	CFont* pListBoxFont=GetFont();
	if(pListBoxFont!=NULL)
	{
		CFont* pOldFont=myDC.SelectObject(pListBoxFont);
		GetTextExtentPoint32(myDC.m_hDC,wzItem,(int)wcslen(wzItem),&sSize);
		
		if(sSize.cx>m_nMaxWidth)
		{
			RECT lRect;
			GetWindowRect(&lRect);

			int nBoxWidth=lRect.right-lRect.left;
			if(sSize.cx>nBoxWidth)
			{
				ShowScrollBar(SB_HORZ,TRUE);
				SetHorizontalExtent(sSize.cx);
			}
			m_nMaxWidth=max(m_nMaxWidth,(int)sSize.cx);
		}

		
		myDC.SelectObject(pOldFont);
	}

	return nRet;
}
int CISBListBox::DeleteString(UINT nIndex)
{
	RECT lRect;
	GetWindowRect(&lRect);

	int nRet=CListBox::DeleteString(nIndex);
	int nBoxWidth=lRect.right-lRect.left;
	m_nMaxWidth=nBoxWidth;

	SIZE sSize;
	CClientDC myDC(this);

	int i;
	WCHAR wzEntry[1024];
	for(i=0;i<GetCount();i++)
	{
		GetText(i,wzEntry);
		GetTextExtentPoint32(myDC.m_hDC,wzEntry,(int)wcslen(wzEntry),&sSize);
		m_nMaxWidth=max(m_nMaxWidth,(int)sSize.cx);
	}
	if(m_nMaxWidth>nBoxWidth)
	{
		ShowScrollBar(SB_HORZ,TRUE);
		SetHorizontalExtent(m_nMaxWidth);
	}
	else
	{
		ShowScrollBar(SB_HORZ,FALSE);
	}
	return nRet;
}
void CISBListBox::ResetContent()
{
	CListBox::ResetContent();
	m_nMaxWidth=0;
	SetHorizontalExtent(0);
}
