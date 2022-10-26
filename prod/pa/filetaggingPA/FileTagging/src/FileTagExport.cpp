#include "stdafx.h"
#include "FileTagExport.h"
#include "FileTagMgr.h"
#include "Utils.h"

extern HINSTANCE g_hPafDLL;

PABase::PA_STATUS WINAPI DoPolicyAssistant( PABase::PA_PARAM &_iParam, const HWND _hParentWnd, bool bEncryptBeforeTag)
{
	CFileTagMgr mgr;
	PABase::PA_STATUS paResult = mgr.DoFileTaggingAssistant(_iParam, _hParentWnd, bEncryptBeforeTag);
	return g_bLoadByOE ?  paResult : ((paResult==PA_SUCCESS) ? PA_SUCCESS : PA_ERROR);	

}
/*
*\brief: change the vector pair data to buf so that it can be safe pass to different module.
*/
static bool ConvVecStringPair2Buf(_In_ const vector<pair<wstring,wstring>>& srcTag,_Out_ wchar_t** szBuf,_Out_ DWORD& dwLen)
{
	vector<pair<wstring,wstring>>::const_iterator it = srcTag.begin();
	DWORD dwOrigLen = 0;
	for(;it != srcTag.end();it++)
	{
		dwOrigLen += (DWORD)(*it).first.length();
		dwOrigLen ++;
		dwOrigLen += (DWORD)(*it).second.length();
		dwOrigLen ++;
	}
	if(dwOrigLen == 0)	return false;
	wchar_t* szOrig = new wchar_t[dwOrigLen];
	if(szOrig == NULL )	return false;
	ZeroMemory(szOrig,dwOrigLen*sizeof(wchar_t));
	int nLen = 0;
	for(it = srcTag.begin();it != srcTag.end();it++)
	{
		wcsncpy_s(szOrig+nLen,dwOrigLen-nLen,(*it).first.c_str(), _TRUNCATE);
		nLen += (int)(*it).first.length();
		szOrig[nLen++]=L'\n';
		wcsncpy_s(szOrig+nLen,dwOrigLen-nLen,(*it).second.c_str(), _TRUNCATE);
		nLen += (int)(*it).second.length();
		if(nLen +1 == (int)dwOrigLen)	szOrig[nLen++]=L'\0';
		else szOrig[nLen++] = L'\n';
		assert(nLen <= (int)dwOrigLen);
	}
	*szBuf = szOrig;
	dwLen = dwOrigLen;
	return true;

}

PABase::PA_STATUS WINAPI DoPolicyAssistant_OE( PABase::PA_PARAM &_iParam, const HWND _hParentWnd, bool bEncryptBeforeTag,vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo)
{
	CFileTagMgr mgr;
	PABase::PA_STATUS paResult = mgr.DoFileTaggingAssistant(_iParam, _hParentWnd, bEncryptBeforeTag,pVecHCInfo);
	return g_bLoadByOE ?  paResult : ((paResult==PA_SUCCESS) ? PA_SUCCESS : PA_ERROR);	

}

PABase::PA_STATUS WINAPI DoPolicyAssistant2( PABase::PA_PARAM &_iParam, const HWND _hParentWnd, bool forceObligation,
										   _Out_ wchar_t** pSrcBuf,_Out_ DWORD& dwSrcLen,
										   _Out_ wchar_t** pDstBuf,_Out_ DWORD& dwDstLen)
{
	forceObligation;	// for warning;
	CFileTagMgr mgr;
	vector<pair<wstring,wstring>> srcTag,dstTag;
	PABase::PA_STATUS nret = mgr.DoFileTaggingAssistant2(_iParam, _hParentWnd,&srcTag,&dstTag);
	ConvVecStringPair2Buf(srcTag,pSrcBuf,dwSrcLen);
	ConvVecStringPair2Buf(dstTag,pDstBuf,dwDstLen);
	return nret;
}
void ReleaseTagBuf(const wchar_t* szBuf)
{
	if(szBuf != NULL)	delete[] szBuf;
}
void ShowResetPanel(LPCWSTR pszFile)
{
	if(!pszFile)
		return;

	CFileTagMgr mgr;
	mgr.ShowViewResetTagsDlg(pszFile);
}

PABase::PA_STATUS WINAPI DeleteLastModifyTimeTag(const wchar_t* wszFileName)
{
	if(!wszFileName)
		return PA_ERROR;
		
	CFileTagMgr mgr;
	return mgr.DeleteFileLastModifyTimeTag(wszFileName);
}


PABase::PA_STATUS WINAPI TaggingOnFile(const wchar_t* wszFileName, const vector<pair<wstring,wstring>>* pVecTags)
{

	if((!wszFileName) || (!pVecTags))
		return PA_ERROR;

	CFileTagMgr mgr;
	return mgr.DoTaggingOnFile(wszFileName, pVecTags);

}