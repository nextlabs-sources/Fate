#pragma once
#include "afxwin.h"

class CISBListBox :
	public CListBox
{
public:
	CISBListBox(void);
	~CISBListBox(void);
	int AddString(LPCTSTR lpszItem);
	int DeleteString(UINT nIndex);
	void ResetContent();
protected:
	int m_nMaxWidth;

};
