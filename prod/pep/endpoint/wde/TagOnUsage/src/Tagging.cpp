#include "StdAfx.h"
#include "Tagging.h"
#include "resattrmgr.h"
#include "PromptDlg.h"

#define   NXTLBS_INDEX_TAG				L"NXTLBS_TAGS"
#define   NXTLBS_INDEX_SEPARATOR		L";"


typedef int (*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
typedef int (*AllocAttributesType)(ResourceAttributes **attrs);
typedef int (*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*GetAttributeCountType)(const ResourceAttributes *attrs);
typedef void (*FreeAttributesType)(ResourceAttributes *attrs);
typedef void (*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
typedef void (*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
typedef int (*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

static CreateAttributeManagerType lfCreateAttributeManager = NULL;
static AllocAttributesType lfAllocAttributes = NULL;
static ReadResourceAttributesWType lfReadResourceAttributesW = NULL;
static GetAttributeCountType lfGetAttributeCount = NULL;
static FreeAttributesType lfFreeAttributes = NULL;
static CloseAttributeManagerType lfCloseAttributeManager = NULL;
static AddAttributeWType lfAddAttributeW = NULL;
static GetAttributeNameType lfGetAttributeName = NULL;
static GetAttributeValueType lfGetAttributeValue = NULL;
static WriteResourceAttributesWType lfWriteResourceAttributesW = NULL;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

static bool InitResattr()
{
	static bool bInit = false;
	if(!bInit)
	{
		std::wstring strCommonPath = GetCommonComponentsDir();
		if(strCommonPath.empty())
		{
			g_log.Log(CELOG_DEBUG, L"can't find common libraries path, check if you have installed common libraries please.\r\n");
			return false;
		}
		else
		{
			SetDllDirectoryW(strCommonPath.c_str());
			g_log.Log(CELOG_DEBUG, L"Common library path: %s\r\n", strCommonPath.c_str());
			
#ifdef _WIN64
			std::wstring strLib = strCommonPath + L"\\resattrlib.dll";
			std::wstring strMgr = strCommonPath + L"\\resattrmgr.dll";
#else
			std::wstring strLib = strCommonPath + L"\\resattrlib32.dll";
			std::wstring strMgr = strCommonPath + L"\\resattrmgr32.dll";
#endif

			HMODULE hModLib = (HMODULE)LoadLibraryW(strLib.c_str());
			HMODULE hModMgr = (HMODULE)LoadLibraryW(strMgr.c_str());

			if( !hModLib || !hModMgr)
			{
				g_log.Log(CELOG_DEBUG, L"Can't load %s, %s\r\n", strLib.c_str(), strMgr.c_str());
				return false;
			}

			lfCreateAttributeManager = (CreateAttributeManagerType)GetProcAddress(hModMgr, "CreateAttributeManager");
			lfAllocAttributes = (AllocAttributesType)GetProcAddress(hModLib, "AllocAttributes");
			lfReadResourceAttributesW = (ReadResourceAttributesWType)GetProcAddress(hModMgr, "ReadResourceAttributesW");
			lfGetAttributeCount = (GetAttributeCountType)GetProcAddress(hModLib, "GetAttributeCount");
			lfFreeAttributes = (FreeAttributesType)GetProcAddress(hModLib, "FreeAttributes");
			lfCloseAttributeManager = (CloseAttributeManagerType)GetProcAddress(hModMgr, "CloseAttributeManager");
			lfAddAttributeW = (AddAttributeWType)GetProcAddress(hModLib, "AddAttributeW");
			lfGetAttributeName = (GetAttributeNameType)GetProcAddress(hModLib, "GetAttributeName");
			lfGetAttributeValue = (GetAttributeValueType)GetProcAddress(hModLib, "GetAttributeValue");
			lfWriteResourceAttributesW = (WriteResourceAttributesWType)GetProcAddress(hModMgr, "WriteResourceAttributesW");

			if( !(lfCreateAttributeManager && lfAllocAttributes &&
				lfReadResourceAttributesW && lfGetAttributeCount &&
				lfFreeAttributes && lfCloseAttributeManager && lfAddAttributeW &&
				lfGetAttributeName && lfGetAttributeValue &&
				lfWriteResourceAttributesW) )
			{
				g_log.Log(CELOG_DEBUG, L"failed to get resattrlib/resattrmgr functions\r\n");
				return false;
			}

			bInit = true;
			return true;
		}
		
	}

	return bInit;
}


void ReadTagThread(LPVOID pParam )
{
	if(pParam == NULL)
		return;

	LPTAGGINGPARAM p = (LPTAGGINGPARAM)pParam;
	p->nResult = CTagging::GetAllTags(p->strFilePath.c_str(), p->mapTags);

	if(p->pDlg)
	{
		p->pDlg->SetEndFlag(TRUE);
	}

	return;
}

CTagging::CTagging(void)
{
}

CTagging::~CTagging(void)
{
}

BOOL CTagging::AddTag(LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags, HWND hParentWnd)
{
	if(!lpszFilePath)
		return FALSE;

	LPTAGGINGPARAM pParam = new TAGGINGPARAM;
	if(!pParam)
		return FALSE;

	pParam->mapTags = mapTags;
	pParam->pDlg = NULL;
	pParam->strFilePath = lpszFilePath;
	pParam->nResult = 0;

	BOOL bRet = FALSE;

        //for office2k7 or PDF files, we need to pop up a "waiting" dialog, since the tagging will take some time.
	if(NeedProcessDlg(lpszFilePath) && hParentWnd != NULL)
	{
		CPromptDlg dlg;
		dlg.SetEndFlag(FALSE);
		dlg.SetPathInfo(lpszFilePath);

		pParam->pDlg = &dlg;	
		HANDLE hThread = (HANDLE)_beginthread(TaggingThread, 0, pParam);
		dlg.DoModal(hParentWnd);
		WaitForSingleObject(hThread, INFINITE);
	}
	else
	{
		TaggingThread(pParam);
	}

	if(pParam->nResult)
		bRet = TRUE;

	if(pParam)
	{
		delete pParam;
		pParam = NULL;
	}

	return bRet;
}

BOOL CTagging::ReadTags(LPCWSTR lpszFilePath, OUT std::map<std::wstring, std::wstring>& mapTags, HWND hParentWnd)
{
	LPTAGGINGPARAM pParam = new TAGGINGPARAM;
	if(!pParam)
		return FALSE;

	pParam->pDlg = NULL;
	pParam->strFilePath = lpszFilePath;
	pParam->nResult = 0;

	BOOL bRet = FALSE;

	//for office2k7 or PDF files, we need to pop up a "waiting" dialog, since the tagging will take some time.
	if(NeedProcessDlg(lpszFilePath) && hParentWnd != NULL)
	{
		CPromptDlg dlg;
		dlg.SetEndFlag(FALSE);
		dlg.SetPathInfo(lpszFilePath);

		pParam->pDlg = &dlg;	
		HANDLE hThread = (HANDLE)_beginthread(ReadTagThread, 0, pParam);
		dlg.DoModal(hParentWnd);
		WaitForSingleObject(hThread, INFINITE);
	}
	else
	{
		ReadTagThread(pParam);
	}

	if(pParam->nResult)
		bRet = TRUE;

	mapTags = pParam->mapTags;

	if(pParam)
	{
		delete pParam;
		pParam = NULL;
	}

	return bRet;
}

BOOL CTagging::GetAllTags(LPCWSTR lpszFilePath, OUT std::map<std::wstring, std::wstring>& mapTags)
{
	if(!lpszFilePath || !InitResattr())
		return FALSE;

	ResourceAttributeManager *mgr = NULL;
	lfCreateAttributeManager(&mgr);

	ResourceAttributes *attrs;
	lfAllocAttributes(&attrs);

	if(!mgr || !attrs)
		return FALSE;

	BOOL bRet = FALSE;
	if(attrs)
	{
		int nRet = lfReadResourceAttributesW(mgr, lpszFilePath, attrs);

		if(!nRet)
		{//Tag error
			;
		}
		else
		{
			int size = lfGetAttributeCount(attrs);

			for (int i = 0; i < size; ++i)
			{
				WCHAR *tagName = (WCHAR *)lfGetAttributeName(attrs, i);
				WCHAR *tagValue = (WCHAR *)lfGetAttributeValue(attrs, i);

				if(tagName && tagValue)
				{
					mapTags[tagName] = tagValue;
					bRet = TRUE;

				}
			}
		}

	}
	
	lfFreeAttributes(attrs);
	lfCloseAttributeManager(mgr);
	return bRet;
}

void CTagging::UpdateIndexTag(LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags)
{
	if(!lpszFilePath )
	{
		return;
	}

	std::wstring strIndexValue(L"");
	std::map<std::wstring, std::wstring> mapAllTags;
	if(GetAllTags(lpszFilePath, mapAllTags))
	{
		std::map<std::wstring, std::wstring>::iterator itr;
		
		//try to find the value of index tag.
		for(itr = mapAllTags.begin(); itr != mapAllTags.end(); itr++)
		{
			if(_wcsicmp((*itr).first.c_str(), NXTLBS_INDEX_TAG) == 0)
			{
				strIndexValue = (*itr).second;
				break;
			}
		}
	}

	//check if the tag name exists in the "index tag". add it if it doesn't exist.
	std::map<std::wstring, std::wstring>::iterator itr2;
	for(itr2 = mapTags.begin(); itr2 != mapTags.end(); itr2++)
	{ 
		std::wstring strTagNameTemp = (*itr2).first + NXTLBS_INDEX_SEPARATOR;
		std::transform(strTagNameTemp.begin(), strTagNameTemp.end(), strTagNameTemp.begin(), towlower);

		std::wstring strTemp = strIndexValue;
		std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), towlower);
		if(strTemp.find(strTagNameTemp.c_str()) == std::wstring::npos)
		{
			strIndexValue += ((*itr2).first + NXTLBS_INDEX_SEPARATOR );
		}
	}

	mapTags[NXTLBS_INDEX_TAG] = strIndexValue;
}

void TaggingThread(LPVOID pParam )
{
	if(!pParam || !InitResattr())
		return;

	LPTAGGINGPARAM pTaggingParam = (LPTAGGINGPARAM)pParam;


	CTagging::UpdateIndexTag(pTaggingParam->strFilePath.c_str(), pTaggingParam->mapTags);//Update the index. it will add the new "Index" tag in mapTags.

	ResourceAttributeManager *mgr = NULL;
	lfCreateAttributeManager(&mgr);

	if(!mgr)
		return ;

	ResourceAttributes *attrs;
	lfAllocAttributes(&attrs);

	int nRet = 0;
	if(attrs)
	{
		std::map<std::wstring, std::wstring>::iterator itr;
		for(itr = pTaggingParam->mapTags.begin(); itr != pTaggingParam->mapTags.end(); itr++)
		{
			lfAddAttributeW(attrs, (*itr).first.c_str(), (*itr).second.c_str());
		}

		nRet = lfWriteResourceAttributesW(mgr, pTaggingParam->strFilePath.c_str(), attrs);

		lfFreeAttributes(attrs);
	}

	if(mgr)
	{
		lfCloseAttributeManager(mgr);
		mgr = NULL;
	}

	pTaggingParam->nResult = nRet;

	if(pTaggingParam->pDlg)
	{
		pTaggingParam->pDlg->SetEndFlag(TRUE);
	}
	return;
}