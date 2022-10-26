// stdafx.cpp : source file that includes just the standard includes
// Obligation.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


bool CheckEmailValiable(const std::wstring str)
{

	size_t nAt = str.find(L'@');
	size_t nPoint = str.find_last_of(L'.');
	if(nAt == std::wstring::npos || nPoint  == std::wstring::npos)	return false;
	return true;
}

void GetQuarantinePath(/*IN*/ULONGLONG lTime,/*IN*/const CString& strOrigPath,/*OUT*/std::wstring& strQuarPath)
{
	// get file title
	int nstart=0;
	nstart = strOrigPath.ReverseFind('\\');
	strQuarPath = g_BaseArgument.wstrQuarantineDir;
	strQuarPath += strOrigPath.Right(strOrigPath.GetLength()-nstart);
	/*
	nend = strOrigPath.Find('.');
	CString strTitle = strOrigPath.Mid(nstart,nend-nstart);
	CString strExtName = strOrigPath.Right(strOrigPath.GetLength()-nend);
	CString strTime;
	strTime.Format(L"_%u",lTime);
	strTitle += strTime;
	strTitle += strExtName;
	strQuarPath = g_BaseArgument.wstrQuarantineDir;
	strQuarPath += strTitle;
	*/
}