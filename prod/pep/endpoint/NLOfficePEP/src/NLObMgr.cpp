#include "stdafx.h"
#include "utils.h"
#include "NLOfficePEP_Comm.h"
#include "obligations.h"
#include "TalkWithSCE.h"
#include "dllmain.h"
#include "NLObMgr.h"

static const wchar_t* g_szBKSaveAsText = L"This action is not permitted. Please use Windows explorer to perform the action.";
static const wchar_t* g_szBKSendText   = L"This action is not permitted. You cannot send/convert/insert NextLabs encrypted file.";
static const wchar_t* g_szDcoument = L"Document";
static const wchar_t* g_szWorkbook = L"Workbook";
static const wchar_t* g_szPresentation = L"Presentation";

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLOBMGR)
//////////////////////////////////////////////////////////////////////////

#define NLNEEDWRITESETAG_IMMEDIATLY false


//
// NLLogic Flag
//



class CNLLogicFlag
{
	// logic tag cache
	typedef struct tagLogicFlagCache
	{
		pair<bool, wstring> pairSaveAsFlag;	// is save as action and save the source file
		bool bButtonSave;	// used to flag button save event. word open in IE, click save button or ctrl+s, ribbon is save but COM event is copy
		bool bIEOpen;
		bool bUserClassifyCustomTags;		// used for user change the custom tags
		bool bIsUserSave;	// for save action we only process office saved by user. this flag add for bug24370, only used for word now.
		bool bIsPPTUserCancelClsoe;	// for PPT close action used in hook ShowWindow. PPT no VARIANT_BOOL* bCancel to deny close action 
		tagLogicFlagCache() : pairSaveAsFlag(false, L""), bButtonSave(false), bIEOpen(false),
			bUserClassifyCustomTags(false), bIsUserSave(false), bIsPPTUserCancelClsoe(false)
		{}
	}LogicFlagCache;
public:
	CNLLogicFlag(void) {};
	~CNLLogicFlag(void) {};

public:
	// close flag for PPT
	void NLSetPPTUserCancelCloseFlag(_In_ const wstring& wstrFilePath, _In_ const bool bIsPPTUserCancelClsoe)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
		stuLogicFlag.bIsPPTUserCancelClsoe = bIsPPTUserCancelClsoe;
	}
	bool NLGetPPTUserCancelCloseFlag(_In_ const wstring& wstrFilePath)
	{
		bool bIsPPTUserCancelClsoe = false;
		readLock(m_rwMutexLogicFlagCache);

		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			bIsPPTUserCancelClsoe = stuLogicFlag.bIsPPTUserCancelClsoe;
		}
		return bIsPPTUserCancelClsoe;
	}

	// save as flag
	void NLSetSaveAsFlag(_In_ const wstring& wstrDesFilePath, _In_ const bool bSaveAsFlag, _In_ const wstring& wstrSrcFilePath)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrDesFilePath];
		stuLogicFlag.pairSaveAsFlag.first = bSaveAsFlag;
		stuLogicFlag.pairSaveAsFlag.second = wstrSrcFilePath;
	}
	bool NLGetSaveAsFlag(_In_ const wstring& wstrDesFilePath, _Out_ wstring& wstrSrcFilePath)
	{
		wstrSrcFilePath.clear();
		bool bRet = false;
		readLock(m_rwMutexLogicFlagCache);

		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrDesFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			if (stuLogicFlag.pairSaveAsFlag.first)
			{
				bRet = true;
				wstrSrcFilePath = stuLogicFlag.pairSaveAsFlag.second;
			}
		}
		return bRet;
	}

	// button save flag
	void NLSetButtonSaveFlag(_In_ const wstring& wstrFilePath, _In_ const bool& bButtonSave)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
		stuLogicFlag.bButtonSave = bButtonSave;
	}
	bool NLGetButtonSaveFlag(_In_ const wstring& wstrFilePath)
	{
		bool bButtonSave = false;
		readLock(m_rwMutexLogicFlagCache);
		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			bButtonSave = stuLogicFlag.bButtonSave;
		}
		return bButtonSave;
	}

	// Open in IE, file path cache
	// office open  in IE, file path cache
	void NLSetIEFilePathCache(_In_ const IDispatch* pDoc, _In_ const wstring& wstrIEFilePath)
	{
		writeLock(m_rwMutexIEFilePathCache);
		m_mapIEFilePath[pDoc] = wstrIEFilePath;
	}
	wstring NLGetIEFilePathCache(_In_ const IDispatch* pDoc)
	{
		wstring wstrIEFilePath;
		readLock(m_rwMutexIEFilePathCache);
		map<const IDispatch*, wstring>::iterator itr = m_mapIEFilePath.find(pDoc);
		if (itr != m_mapIEFilePath.end())
		{
			wstrIEFilePath = itr->second;
		}
		return wstrIEFilePath;
	}

	// user classify custom tag flag
	void NLSetClassifyCustomTagsFlag(_In_ const wstring& wstrFilePath, _In_ const bool& bUserClassifyCustomTags)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
		stuLogicFlag.bUserClassifyCustomTags = bUserClassifyCustomTags;
	}
	bool NLGetClassifyCustomTagsFlag(_In_ const wstring& wstrFilePath)
	{
		bool bUserClassifyCustomTagsFlag = false;
		readLock(m_rwMutexLogicFlagCache);
		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			bUserClassifyCustomTagsFlag = stuLogicFlag.bUserClassifyCustomTags;
		}
		return bUserClassifyCustomTagsFlag;
	}

	// office open in IE flag
	void NLSetIEOpenFlag(_In_ const wstring& wstrFilePath, _In_ const bool& bIEOpenFlag)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
		stuLogicFlag.bIEOpen = bIEOpenFlag;
	}
	bool NLGetIEOpenFlag(_In_ const wstring& wstrFilePath)
	{
		bool bIEOpenFlag = false;
		readLock(m_rwMutexLogicFlagCache);
		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			bIEOpenFlag = stuLogicFlag.bIEOpen;
		}
		return bIEOpenFlag;
	}

	// edit action triggered by user save operation
	void NLSetUserSaveFlag(_In_ const wstring& wstrFilePath, _In_ const bool& bIsUserSave)
	{
		writeLock(m_rwMutexLogicFlagCache);
		LogicFlagCache& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
		stuLogicFlag.bIsUserSave = bIsUserSave;
	}
	bool NLGetUserSaveFlag(_In_ const wstring& wstrFilePath)
	{
		bool bIsUserSave = false;
		readLock(m_rwMutexLogicFlagCache);
		map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
		if (itr != m_mapLogicFlagCache.end())
		{
			LogicFlagCache& stuLogicFlag = itr->second;
			bIsUserSave = stuLogicFlag.bIsUserSave;
		}
		return bIsUserSave;
	}

	void NLClearFileCache(_In_ const wstring& wstrFilePath)
	{
		{
			// clear logic cache
			writeLock(m_rwMutexLogicFlagCache);
			map<wstring, LogicFlagCache>::iterator itr = m_mapLogicFlagCache.find(wstrFilePath);
			if (itr != m_mapLogicFlagCache.end())
			{
				m_mapLogicFlagCache.erase(itr);
			}
		}

		{
			// clear IE file path cache
			writeLock(m_rwMutexIEFilePathCache);
			map<const IDispatch*, wstring>::iterator itrIE = m_mapIEFilePath.begin();
			for (; itrIE != m_mapIEFilePath.end();)
			{
				if (wstrFilePath == itrIE->second)
				{
					m_mapIEFilePath.erase(itrIE++);
				}
				else
					itrIE++;
			}
		}
	}

	void NLClearCache()
	{
		{
			// clear logic cache
			writeLock(m_rwMutexLogicFlagCache);
			m_mapLogicFlagCache.clear();
		}

		{
			// clear IE file path cache
			writeLock(m_rwMutexIEFilePathCache);
			m_mapIEFilePath.clear();
		}
	}
private:
	rwmutex  m_rwMutexLogicFlagCache;
	map<wstring, LogicFlagCache> m_mapLogicFlagCache;

	/*
	*	this cache is used for office opening in IE, the key is the active pDoc.
	* Office open in IE: the active pDoc is the same, but we can not get the file path by the active pDoc
	*	In this case we only can get the right file path at open action and then save the path into cache.
	*	After open action we can get the file path from cache.
	*	But here we need a key to flag which file path you want and I use the active pDoc as the key.
	*/
	rwmutex  m_rwMutexIEFilePathCache;
	map<const IDispatch*, wstring> m_mapIEFilePath;
};

class CNLData
{
public:
	CNLData(void)
	{
		InitializeCriticalSection(&m_csMapFileCacheData);
		InitializeCriticalSection(&m_csCurActiveFilePath);
		InitializeCriticalSection(&m_csPPTPrintActiveFilePath);
		InitializeCriticalSection(&m_csHyperLinkFilePath);
		InitializeCriticalSection(&m_csGoldenTag);
	}
	~CNLData(void)
	{
		DeleteCriticalSection(&m_csMapFileCacheData);
		DeleteCriticalSection(&m_csCurActiveFilePath);
		DeleteCriticalSection(&m_csPPTPrintActiveFilePath);
		DeleteCriticalSection(&m_csHyperLinkFilePath);
		DeleteCriticalSection(&m_csGoldenTag);
	}

public:
	// Current runtime status
	void NLSetCurRibbonEvent(_In_ const wstring& wstrFilePath, _In_ const STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent);
	bool NLGetCurRibbonEvent(_In_ const wstring& wstrFilePath, _Out_ STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent);

	void NLSetCurComNotify(_In_ const wstring& wstrFilePath, _In_ const NotifyEvent& stuCurComNotify);
	bool NLGetCurComNotify(_In_ const wstring& wstrFilePath, _Out_ NotifyEvent& stuCurComNotify);

	// Close flag
	void NLSetCloseFlag(_In_ const wstring& wstrFilePath, _In_ const bool bIfNeedClose);
	bool NLGetCloseFlag(_In_ const wstring& wstrFilePath);

	// Encrypt flag
	/*
	*\ Brief: this two group flag used for encryption.
	*					NLSetEncryptRequirementFlag/NLGetEncryptRequirementFlag: set this flag tell us if we need encrypt the file
	*					NLSetFileEncryptFlag/NLGetFileEncryptFlag: this flag will tell us if the current file is encrypted or not
	*/
	void NLSetEncryptRequirementFlag(_In_ const wstring& wstrFilePath, _In_ const bool bNeedEncrypt);
	bool NLGetEncryptRequirementFlag(_In_ const wstring& wstrFilePath);

	void NLSetEncryptFileFlag(_In_ const wstring& wstrFilePath, _In_ const bool bIsEncryptFile);
	bool NLGetEncryptFileFlag(_In_ const wstring& wstrFilePath);

	// File path & pDoc
	void NLSetRealsePDocFlag(_In_ const wstring& wstrFilePath, _In_ const bool bNeedRealse); // this function used for some special case, like FixBug24688
	void NLSetFilePDocCache(_In_ const wstring& wstrFilePath, _In_ IDispatch* pDoc);
	CComPtr<IDispatch> NLGetFilePDocCache(_In_ const wstring& wstrFilePath);

	// File tags cache	
	void NLSetFileTagCache(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecTagPair);
	bool NLGetFileTagCache(_In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring, wstring>>& vecTagPair);

	// Golden tags back
	void NLSetGoldenTagCache(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldeTags);

	// note we will get the golden tags and clear erase it from the golden tag cache
	bool NLGetGoldenTagCache(_In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring, wstring>>& vecGoldeTags);
	void NLGetGoldenTagCache(_Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags);

	// Clear cache
	void NLClearFileCache(_In_ const wstring& wstrFilePath);
	void NLClearCache(void);

	// clear golden tag cache
	void NLClearGoldenTagFileCache(_In_ const wstring& wstrFilePath);
	void NLClearGoldenTagCache(void);

	// Current active file path
	void NLSetCurActiveFilePath(_In_ const wstring& wstrFilePath);
	wstring NLGetCurActiveFilePath();

	// For Bug24122 PPT right click to do print action, assist for current active file path
	void NLSetPPTPrintActiveFilePath(_In_ const wstring& wstrPPTPrintActiveFilePath);
	wstring NLGetPPTPrintActiveFilePath();

	// No need handle flag
	void NLSetNoNeedHandleFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bNoNeedHandle);
	bool NLGetNoNeedHandleFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction);

	// Action startup flag
	void NLSetActionStartUpFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bActionStartUp, _In_opt_ const ActionFlag* pstuOfficeActionFlag = NULL);
	bool NLGetActionStartUpFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _Out_opt_ ActionFlag* pstuOfficeActionFlag = NULL);

	// Hyperlink file path cache
	void NLSetLinkFilePath(_In_ const wstring& wstrHyperLinkFilePath);
	wstring NLGetLinkFilePath();

private:
	void NLInitializeCacheData();

private:
	CRITICAL_SECTION m_csMapFileCacheData;
	map<wstring, STUNLOFFICE_CACHEDATA> m_mapFileCacheData;

	CRITICAL_SECTION m_csGoldenTag;
	map<wstring, STUNLOFFICE_GOLDENTAG> m_mapGoldenTag;

	CRITICAL_SECTION m_csCurActiveFilePath;
	wstring m_wstrCurActiveFilePath;

	// For bug24122, PPT right click to do print action sometimes we can not get the current active file path, this is a assist cache
	CRITICAL_SECTION m_csPPTPrintActiveFilePath;
	wstring m_wstrPPTPrintActiveFilePath;

	CRITICAL_SECTION m_csHyperLinkFilePath;
	wstring m_wstrHyperLinkFilePath;
};


void CNLData::NLInitializeCacheData()
{
	NLClearCache();
}

// Current runtime status
void CNLData::NLSetCurRibbonEvent(_In_ const wstring& wstrFilePath, _In_ const STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.stuCurRibbonEvent = stuCurRibbonEvent;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetCurRibbonEvent(_In_ const wstring& wstrFilePath, _Out_ STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent)
{
	bool bRet = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		stuCurRibbonEvent = stuCacheData.stuCurRibbonEvent;
		bRet = true;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bRet;
}

void CNLData::NLSetCurComNotify(_In_ const wstring& wstrFilePath, _In_ const NotifyEvent& stuCurComNotify)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.stuCurComNotify = stuCurComNotify;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetCurComNotify(_In_ const wstring& wstrFilePath, _Out_ NotifyEvent& stuCurComNotify)
{
	bool bRet = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		stuCurComNotify = stuCacheData.stuCurComNotify;
		bRet = true;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bRet;
}

// Close flag
void CNLData::NLSetCloseFlag(_In_ const wstring& wstrFilePath, _In_ const bool bIfNeedClose)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.bNeedClose = bIfNeedClose;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetCloseFlag(_In_ const wstring& wstrFilePath)
{
	bool bIfNeedClose = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		bIfNeedClose = stuCacheData.bNeedClose;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bIfNeedClose;
}

// Encrypt flag
void CNLData::NLSetEncryptRequirementFlag(_In_ const wstring& wstrFilePath, _In_ const bool bNeedEncrypt)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.bNeedEncrypt = bNeedEncrypt;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetEncryptRequirementFlag(_In_ const wstring& wstrFilePath)
{
	bool bNeedEncrypt = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		bNeedEncrypt = stuCacheData.bNeedEncrypt;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bNeedEncrypt;
}

void CNLData::NLSetEncryptFileFlag(_In_ const wstring& wstrFilePath, _In_ const bool bIsEncryptFile)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.bIsEncryptFile = bIsEncryptFile;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetEncryptFileFlag(_In_ const wstring& wstrFilePath)
{
	bool bIsEncryptFile = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		bIsEncryptFile = stuCacheData.bIsEncryptFile;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bIsEncryptFile;
}

void CNLData::NLSetRealsePDocFlag(_In_ const wstring& wstrFilePath, _In_ const bool bNeedRealse)
{
	EnterCriticalSection(&m_csMapFileCacheData);

	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.bNeedReleaseDoc = bNeedRealse;

	LeaveCriticalSection(&m_csMapFileCacheData);
}

void CNLData::NLSetFilePDocCache(_In_ const wstring& wstrFilePath, _In_ IDispatch* pDoc)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	if (stuCacheData.pDoc != pDoc)
	{
		NLPRINT_DEBUGLOG(L"====Note: pDoc change: file:[%s], old pDoc:[0x%x], new pDoc:[0x%x] \n",
			wstrFilePath.c_str(), stuCacheData.pDoc, pDoc);
		if (stuCacheData.bNeedReleaseDoc)
		{
			NULL != stuCacheData.pDoc ? stuCacheData.pDoc->Release() : NULL;
			stuCacheData.pDoc = pDoc;
			NULL != stuCacheData.pDoc ? stuCacheData.pDoc->AddRef() : NULL;
		}
		else
		{
			stuCacheData.pDoc = pDoc;
		}
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
}

CComPtr<IDispatch> CNLData::NLGetFilePDocCache(_In_ const wstring& wstrFilePath)
{
	IDispatch* pDoc = NULL;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		pDoc = stuCacheData.pDoc;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return pDoc;
}

// File tags cache
void CNLData::NLSetFileTagCache(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecTagPair)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.vecTagPair = vecTagPair;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetFileTagCache(_In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring, wstring>>& vecTagPair)
{
	bool bRet = false;
	vecTagPair.clear();
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		vecTagPair = stuCacheData.vecTagPair;
		bRet = true;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bRet;
}

void CNLData::NLSetGoldenTagCache(_In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring, wstring>>& vecGoldeTags)
{
	EnterCriticalSection(&m_csGoldenTag);
	STUNLOFFICE_GOLDENTAG& stuGoldenTag = m_mapGoldenTag[wstrFilePath];
	stuGoldenTag.vecGoldeTags = vecGoldeTags;
	LeaveCriticalSection(&m_csGoldenTag);
}

bool CNLData::NLGetGoldenTagCache(_In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring, wstring>>& vecGoldeTags)
{
	NLCELOG_ENTER
		bool bRet = false;
	EnterCriticalSection(&m_csGoldenTag);
	vecGoldeTags.clear();
	map<wstring, STUNLOFFICE_GOLDENTAG>::iterator itr = m_mapGoldenTag.find(wstrFilePath);
	if (itr != m_mapGoldenTag.end())
	{
		STUNLOFFICE_GOLDENTAG& stuGoldenTag = itr->second;
		vecGoldeTags = stuGoldenTag.vecGoldeTags;
		bRet = true;
	}
	LeaveCriticalSection(&m_csGoldenTag);
	NLCELOG_RETURN_VAL(bRet)
}

void CNLData::NLGetGoldenTagCache(_Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags)
{
	EnterCriticalSection(&m_csGoldenTag);
	mapGoldenTags = m_mapGoldenTag;
	LeaveCriticalSection(&m_csGoldenTag);
}

// Clear cache
void CNLData::NLClearFileCache(_In_ const wstring& wstrFilePath)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		m_mapFileCacheData.erase(itr);
	}
	NLSetCurActiveFilePath(L"");
	LeaveCriticalSection(&m_csMapFileCacheData);
}

void CNLData::NLClearCache(void)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	m_mapFileCacheData.clear();
	LeaveCriticalSection(&m_csMapFileCacheData);

	NLSetCurActiveFilePath(L"");
	NLSetLinkFilePath(L"");
}

void CNLData::NLClearGoldenTagFileCache(_In_ const wstring& wstrFilePath)
{
	EnterCriticalSection(&m_csGoldenTag);
	map<wstring, STUNLOFFICE_GOLDENTAG>::iterator itr = m_mapGoldenTag.find(wstrFilePath);
	if (itr != m_mapGoldenTag.end())
	{
		m_mapGoldenTag.erase(itr);
	}
	LeaveCriticalSection(&m_csGoldenTag);
}

void CNLData::NLClearGoldenTagCache(void)
{
	EnterCriticalSection(&m_csGoldenTag);
	m_mapGoldenTag.clear();
	LeaveCriticalSection(&m_csGoldenTag);
}

void CNLData::NLSetCurActiveFilePath(_In_ const wstring& wstrFilePath)
{
	EnterCriticalSection(&m_csCurActiveFilePath);
	if (wstrFilePath != m_wstrCurActiveFilePath)
	{
		m_wstrCurActiveFilePath = wstrFilePath;
	}
	LeaveCriticalSection(&m_csCurActiveFilePath);
}

wstring CNLData::NLGetCurActiveFilePath()
{
	wstring wstrFilePath;
	EnterCriticalSection(&m_csCurActiveFilePath);
	wstrFilePath = m_wstrCurActiveFilePath;
	LeaveCriticalSection(&m_csCurActiveFilePath);
	return wstrFilePath;
}

void CNLData::NLSetPPTPrintActiveFilePath(_In_ const wstring& wstrPPTPrintActiveFilePath)
{
	EnterCriticalSection(&m_csPPTPrintActiveFilePath);
	if (wstrPPTPrintActiveFilePath != m_wstrPPTPrintActiveFilePath)
	{
		m_wstrPPTPrintActiveFilePath = wstrPPTPrintActiveFilePath;
	}
	LeaveCriticalSection(&m_csPPTPrintActiveFilePath);
}

wstring CNLData::NLGetPPTPrintActiveFilePath()
{
	wstring wstrPPTPrintActiveFilePath;
	EnterCriticalSection(&m_csPPTPrintActiveFilePath);
	wstrPPTPrintActiveFilePath = m_wstrPPTPrintActiveFilePath;
	LeaveCriticalSection(&m_csPPTPrintActiveFilePath);
	return wstrPPTPrintActiveFilePath;
}

void CNLData::NLSetNoNeedHandleFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bNoNeedHandle)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.mapNoNeedHandleFlag[emAction] = bNoNeedHandle;
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetNoNeedHandleFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction)
{
	bool bNoNeedHandle = false;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		bNoNeedHandle = stuCacheData.mapNoNeedHandleFlag[emAction];
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bNoNeedHandle;
}

// Action startup flag, like: we set this flag at process module and check it in hook module
void CNLData::NLSetActionStartUpFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bActionStartUp, _In_opt_ const ActionFlag* pstuOfficeActionFlag)
{
	EnterCriticalSection(&m_csMapFileCacheData);
	STUNLOFFICE_CACHEDATA& stuCacheData = m_mapFileCacheData[wstrFilePath];
	stuCacheData.mapActionStartupFlag[emAction].first = bActionStartUp;
	if (NULL != pstuOfficeActionFlag)
	{
		stuCacheData.mapActionStartupFlag[emAction].second = *pstuOfficeActionFlag;
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
}

bool CNLData::NLGetActionStartUpFlag(_In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _Out_opt_ ActionFlag* pstuOfficeActionFlag)
{
	bool bActionStartUp = false;
	ActionFlag stuOfficeActionFlag;
	EnterCriticalSection(&m_csMapFileCacheData);
	map<wstring, STUNLOFFICE_CACHEDATA>::iterator itr = m_mapFileCacheData.find(wstrFilePath);
	if (itr != m_mapFileCacheData.end())
	{
		STUNLOFFICE_CACHEDATA& stuCacheData = itr->second;
		bActionStartUp = stuCacheData.mapActionStartupFlag[emAction].first;
		if (NULL != pstuOfficeActionFlag)
		{
			*pstuOfficeActionFlag = stuCacheData.mapActionStartupFlag[emAction].second;
		}
	}
	LeaveCriticalSection(&m_csMapFileCacheData);
	return bActionStartUp;
}

void CNLData::NLSetLinkFilePath(_In_ const wstring& wstrHyperLinkFilePath)
{
	EnterCriticalSection(&m_csHyperLinkFilePath);
	m_wstrHyperLinkFilePath = wstrHyperLinkFilePath;
	LeaveCriticalSection(&m_csHyperLinkFilePath);
}

wstring CNLData::NLGetLinkFilePath()
{
	wstring wstrHyperLinkFilePath;
	EnterCriticalSection(&m_csHyperLinkFilePath);
	wstrHyperLinkFilePath = m_wstrHyperLinkFilePath;
	LeaveCriticalSection(&m_csHyperLinkFilePath);
	return wstrHyperLinkFilePath;
}




static const DWORD g_dwLocalFilePreopenEvaResultValidTimeMsDefault = 5 * 1000;
static const DWORD g_dwNetFilePreopenEvaResultValidTimeMsDefault = 30 * 1000;
CNLObMgr::CNLObMgr(void)
	:m_dwLocalFilePreopenEvaResultValidTimeMs(g_dwLocalFilePreopenEvaResultValidTimeMsDefault)
	,m_dwNetFilePreopenEvaResultValidTimeMs(g_dwNetFilePreopenEvaResultValidTimeMsDefault)
{
	InitializeCriticalSection(&m_csObMgrInitialize);
	InitializeCriticalSection(&m_csGoldenTag);
	NLInitMapBetweenActionAndObligation();
	InitializeCriticalSection(&m_csmap);
	InitializeCriticalSection(&m_csPreOpenEvaActionResult);
	
	m_pLogicFlag = new CNLLogicFlag;
	m_pNLData = new CNLData;
}

CNLObMgr::~CNLObMgr(void)
{
	DeleteCriticalSection(&m_csObMgrInitialize);
	DeleteCriticalSection(&m_csGoldenTag);
	DeleteCriticalSection(&m_csmap);
	DeleteCriticalSection(&m_csPreOpenEvaActionResult);
	
	delete m_pLogicFlag;
	delete m_pNLData;
}

CNLObMgr& CNLObMgr::NLGetInstance()
{
	static CNLObMgr theObMgrIns;
	return theObMgrIns;
}

ActionResult CNLObMgr::NLGetEvaluationResult( 
	_In_ const WCHAR* pwchSource, 
	_In_ const OfficeAction& emAction,
	_In_opt_ const WCHAR* pwchDest,	
	_In_ CENoiseLevel_t noise_level, 
	_In_ const wstring wstrapp_attr_value )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pwchSource=%ls, emAction=%d, pwchDest=%ls, noise_level=%d, wstrapp_attr_value=%ls \n", print_long_string(pwchSource),emAction, print_long_string(pwchDest), noise_level, wstrapp_attr_value.c_str());
	ActionResult emResult = kRtFailed;
	// 1. check the parameter, for insert action we don't known the insert file IDispatch point and tags.
	if ( NULL == pwchSource )
	{
		NLPRINT_DEBUGLOG( L"Local variables are: emResult=%d \n", emResult );
		return emResult;
	}
	
	// check the obMgr initialize
	bool bInitInner = false;
	if ( !NLGetObMgrInitializeFlag( pwchSource ) )
	{
		bInitInner = NLInitializeObMgr( pwchSource, NULL );
		if ( !bInitInner )
		{
			NLPRINT_DEBUGLOG( L"Local variables are: bInitInner=%ls, emResult=%d \n", bInitInner?L"TRUE":L"FALSE",emResult );
			return emResult;
		}
	}

	vector<pair<wstring,wstring>> vecTags;
	NLReadTag( pwchSource, vecTags );
	
	nextlabs::Obligations obs;
	emResult = CNxtSDK::DoEvaluation( pwchSource, emAction, obs, pwchDest, &vecTags,	noise_level, wstrapp_attr_value, true );

	if ( bInitInner )
	{
		NLPRINT_DEBUGLOG( L" --- inner initialize, revert this flag false. the file:[%s] failed, action:[%d]\n", pwchSource, emAction );
		NLSetObMgrInitializeFlag( pwchSource, False );
	}
	
	return emResult;
}
ActionResult CNLObMgr:: NLGetEvaluationResult( _In_ vector<wstring>& vecwstrSource, _In_ const OfficeAction& emAction,
													_In_opt_ const WCHAR* pwchDest,	_In_ CENoiseLevel_t noise_level,_In_ const wstring wstrapp_attr_value )
{
	ActionResult emAllow = kRtPCAllow;
	if(vecwstrSource.empty())	return kRtPCAllow;
	// get current file path
	wstring strDst = NLGetCurActiveFilePath();
	if(strDst.empty())	return emAllow;

	EMNLOFFICE_ENCPERMISSION dwEncStatus = emNLEncPermissionDoNothing;
	nextlabs::Obligations obs;
	for(vector<wstring>::iterator it = vecwstrSource.begin();it != vecwstrSource.end();)
	{
		ActionResult emRes = kRtUnknown;
		wstring& strSrcFile = (*it);
		if(dwEncStatus == emNLEncPermissionDoNothing )		
			dwEncStatus =  NLGetSEPermissopnForCurAction(strSrcFile.c_str(),strDst.c_str(),emAction);

		if (dwEncStatus  == emNLEncPermissionNeedDeny)
		{
			// erase it
			it = vecwstrSource.erase(it);
			continue ;
		}
		emRes = CNxtSDK::DoEvaluation( strSrcFile.c_str(), emAction, obs, pwchDest);
		if (emRes == kRtPCDeny)
		{
			it = vecwstrSource.erase(it);
			continue ;
		}
		it++;
	}
	if (vecwstrSource.empty())	emAllow = kRtPCDeny;
	return emAllow;

}

#define MSG_OFFICEPROTECTEDVIEDENIEDBYCOPYACTION L"The file cannot be opened in Protected View due to your usage rights.\nPlease disable Office Protected View feature to access this file."

ProcessResult CNLObMgr::NLProcessActionProtectedView( const wstring& wstrPath, IDispatch* pDoc )
{ONLY_DEBUG
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_OPEN, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );	
		
#pragma chMSG( "Here we can optimization: read tags by file path and the parameter pDoc may be no used" )
	vector<std::pair<wstring,wstring>> vecSourcePair;
	NLReadTag( pDoc, vecSourcePair);
	NLPRINT_TAGPAIRLOG( vecSourcePair, wstrPath.c_str(), L"Process protect view" );

	nextlabs::Obligations obs;
	ActionResult emOfficeResult = CNxtSDK::DoEvaluation( wstrPath.c_str(), kOA_OPEN, obs, NULL, &vecSourcePair );

	if ( kRtPCDeny == emOfficeResult )	
	{
		stuResult.kPolicyStat = kPSDeny, stuResult.line = __LINE__;
	}
	else
	{
        // according to Tonny said, since can't capture the PASTE action in protected view, so do evaluation of paste when OPEN in protected view,
        // if the paste action is deny, a dialog will be popped up to prompt user to disable protected view to access file.
        nextlabs::Obligations obs1;
        emOfficeResult = CNxtSDK::DoEvaluation( wstrPath.c_str(), kOA_PASTE, obs1, NULL, &vecSourcePair );
        if ( kRtPCDeny == emOfficeResult )
        {
            stuResult.kPolicyStat = kPSDeny, stuResult.line = __LINE__;
            MessageBox( GetActiveWindow(), MSG_OFFICEPROTECTEDVIEDENIEDBYCOPYACTION, L"Nextlabs", MB_OK );
        }
        else
        {
            NLVIEWOVERLAYINSTANCE.DoViewOverlay( obs, wstrPath.c_str() );
        }
        
	}
	
	return stuResult;
}

ProcessResult CNLObMgr::NLProcessActionCommon( 
	_In_opt_ IDispatch* pDoc,
	_In_ const wchar_t* pwchSrcFilePath,
	_In_opt_ const wchar_t* pwchDesFilePath,
	_In_ const OfficeAction& emAction)
{
	NLPRINT_DEBUGLOG( 
		L" The Parameters are: pDoc=%p, pwchSrcFilePath=%ls, pwchDesFilePath=%ls, emAction=%d \n", 
		pDoc, 
		print_long_string(pwchSrcFilePath), 
		print_long_string(pwchDesFilePath),
		emAction);
	ProcessResult stuResult(  kFSSuccess, kPSAllow, emAction, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	
	// 1. check the parameter, for insert action we don't known the insert file IDispatch point and tags.
	if ( NULL == pwchSrcFilePath )
	{
		NLPRINT_DEBUGLOG( L"the parameter first is error \n" );
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	if ( ( kOA_COPY == emAction && NULL == pwchDesFilePath )	|| 
		 ( kOA_COPY != emAction && NULL != pwchDesFilePath ) 
		)
	{
		NLPRINT_DEBUGLOG( L"the parameter second is error \n" );
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// check the obMgr initialize
	bool bInitInner = false;
	if ( !NLGetObMgrInitializeFlag( pwchSrcFilePath ) )
	{
		bInitInner = NLInitializeObMgr( pwchSrcFilePath, pDoc );
		if ( !bInitInner )
		{
			NLPRINT_DEBUGLOG( L" --- Seriours: inner initialize the file:[%s] failed, action:[%d]\n", pwchSrcFilePath, emAction );
			stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
			return stuResult;
		}
	}
	
	vector<pair<wstring, wstring>> vecSrcTag;

	if (kOA_INSERT != emAction)
	{
		NLReadTag( pwchSrcFilePath, vecSrcTag );
	}
	
	// debug
	{
		NLPRINT_DEBUGLOG( L"source file path:[%s], destination path:[%s], action:[%d] \n", pwchSrcFilePath,
			NULL == pwchDesFilePath ? L"NULL" : pwchDesFilePath, emAction );
		NLPRINT_TAGPAIRLOG( vecSrcTag, L"SOURCE FILE TAGS" );
	}
	
	EMNLOFFICE_ENCPERMISSION emEncStatus = NLGetSEPermissopnForCurAction( pwchSrcFilePath, pwchDesFilePath, emAction );
	if ( emNLEncPermissionNeedDeny == emEncStatus )
	{
		stuResult.kPolicyStat = kPSDeny, stuResult.line = __LINE__;
	} 
	else
	{
		// query policy, do evaluation
		nextlabs::Obligations obs;
		ActionResult emOfficeResult = CNxtSDK::DoEvaluation( pwchSrcFilePath, emAction, obs, pwchDesFilePath, &vecSrcTag );	
		if ( kRtPCDeny == emOfficeResult )	
		{
			stuResult.kPolicyStat = kPSDeny, stuResult.line = __LINE__;
		}
		else
		{
			if ( NLIsOfficeFile( pwchSrcFilePath ) )
			{
				// parse the obligations. we only support obligations for office file
				if ( !NLDoObligationCommon( pDoc, obs, pwchSrcFilePath, pwchDesFilePath, emAction, vecSrcTag, NULL ) )
				{
					stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
				}
			}
			else
			{
				NLVIEWOVERLAYINSTANCE.ClearCreatewindowCache();
			}
		}
	}

	if ( bInitInner )
	{
		NLPRINT_DEBUGLOG( L" --- inner initialize, revert this flag false. the file:[%s] failed, action:[%d]\n", pwchSrcFilePath, emAction );
		NLSetObMgrInitializeFlag( pwchSrcFilePath, False );
	}

	NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d],function:[%s],line:[%d] \n", stuResult.kFuncStat, stuResult.kPolicyStat,
				stuResult.kAction, stuResult.fname.c_str(), stuResult.line);
	return stuResult;
}

bool CNLObMgr::NLDoObligationCommon( 
	_In_opt_	IDispatch*					pdoc,
	_In_		nextlabs::Obligations&	obs,
	_In_		const wchar_t* const		srcpath,
	_In_opt_	const wchar_t* const	dstpath,
	_In_		const OfficeAction&							action,
	_In_		const vector<pair<wstring, wstring>>&	srctags,
	_In_opt_	const OverLayInfo*						poverlayinfo)
{
	NLCELOG_ENTER
	bool bNeedEncrypt = false;
	vector<pair<wstring,wstring>> vecObligationTags;
	bool bRet =  NLParseObligation( obs, srcpath, dstpath, action, srctags, NULL, bNeedEncrypt, vecObligationTags );

	if ( !bRet )
	{
		NLPRINT_DEBUGLOG( L"parse the obligation failed, please check \n" );
		NLCELOG_RETURN_VAL( bRet )
	}
	{
		// debug
		NLPRINT_DEBUGLOG( 
			L"source file path:[%s], destination path:[%s], action:[%d], pDoc:[0x%x] \n", 
			srcpath,
			NULL == dstpath ? L"NULL" : dstpath, 
			action, 
			NULL == pdoc ? 0 : pdoc );
		NLPRINT_TAGPAIRLOG( vecObligationTags, L"OBLIGATION TAGS" );
	}
	
	// encrypt the file
	// 1. if source file is encrypt file
	bool bIsEncryptedFile = NLIsEncryptedFile( srcpath );
	// 2. if it is save as to DRM folder or replace an exist SE file, Fixbug24116
	bool bSaveAsToDRMFolder = ( kOA_COPY == action && NLIsEncryptedFile( dstpath ) );
	// 3. set encrypt requirement flag
	bool bHasEncryptRequirement = bNeedEncrypt || bIsEncryptedFile || bSaveAsToDRMFolder;
	
	NLPRINT_DEBUGLOG( L"---- bHasEncryptRequirement:[%s] --- \n", bHasEncryptRequirement?L"true":L"false" );
	/*
		1. SE PPTX file in desktop, if we encrypt temp file at COM edit notify, 
		   sometimes edit and save the file, it will failed and pop up an message box( Probability of 20% ).
		2. For new solution, we encrypt the file at "BJReplaceFile" by hook "ReplaceFile" for Word/Excel/PPT edit action and  Word/Excel copy action
	*/
	if ( bHasEncryptRequirement )	// save as: if the source file is the encrypt file we must encrypt the destination file
	{
		if ( kOA_COPY == action )
		{
			// we only allow encrypt the local file
			if ( IsLocalDriver( dstpath ) )
			{
				// Word/Excel we can encrypt the file and synchronize the golden tags throw hook "BJReplaceFile", PPT we can not do it
				if ( pep::isPPTFile( dstpath ) )
				{
					// save as action encrypt the destination temp file, save as the original file it is save work flow
					bool bForSave = boost::algorithm::iequals( dstpath, srcpath );
					if ( bForSave )
					{
						NLPRINT_DEBUGLOG( L"this is PPT save as the original file \n" );
						NLSetEncryptRequirementFlag( dstpath, bHasEncryptRequirement );
					} 
					else
					{
						bRet = NLEncryptTempFile(pep::appType() , dstpath, bForSave);
					}
				}
				else
				{
					NLSetEncryptRequirementFlag( dstpath, bHasEncryptRequirement );
					NLPRINT_DEBUGLOG( L"there is the encryption requirement Copy action, action:[%d] \n", action );
				}
			}
			else
			{
				// not allow this action
				MessageBoxW( GetActiveWindow(), g_szBKSaveAsText, L"NextLabs", MB_OK );
				bRet = false;
			}
		}
		else
		{
			NLSetEncryptRequirementFlag( srcpath, bHasEncryptRequirement );
			NLPRINT_DEBUGLOG( L"there is the encryption requirement, action:[%d] \n", action );
		}
	}
	
	/*
	*	1. SE tag or custom, are both need remove all file tags and then add new tags
	*	2. For SE copy action we must synchronize the source file path tag first
	*/
	vector<pair<wstring,wstring>> vecAllFileTags = srctags;
	if ( !vecObligationTags.empty() )
	{
		NLReplaceTagsByTagNameEx( vecAllFileTags, vecObligationTags, true );
	}
	NLPRINT_TAGPAIRLOG( vecAllFileTags, L"All file tags", L"Test end" );

	// write tags, here we need write tags before we encrypt the file.
	// write the obligation tags into cache
	if ( bRet && !vecAllFileTags.empty() )
	{
		// if SE file edit without obligation tags the old golden will last and we need write it into file again
		if ( bHasEncryptRequirement )
		{
			// encrypt file, this function will write the tags into cache
			bRet = NLWriteTag( kOA_COPY==action ? dstpath : srcpath, vecAllFileTags, true, false, true );
		} 
		else
		{
			if ( !vecObligationTags.empty() )
			{
				if ( NULL == pdoc )
				{
					NLPRINT_DEBUGLOG( L"Note: the pDoc is NULL, when we do obligation, please check! \n" );
					bRet = NLWriteTag( srcpath, vecAllFileTags, true );
				} 
				else
				{
					// for save & save as action the tag will be saved in the default work flow
					bRet = NLWriteTag( pdoc, vecAllFileTags, true, false, false );
				}
			}
		}
	}
	
	if ( !bRet )
	{
		NLPRINT_DEBUGLOG( L"!!!!Error:write tags failed. pDoc=[0x%x], source file[%s], destination file:[%s], action:[%s] \n", pdoc, srcpath,
			NULL == dstpath ? L"NULL" : dstpath, NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum(action).c_str() );	
	}
	NLPRINT_DEBUGLOG( L"Local variables are: bNeedEncrypt=%ls, bRet=%ls, bIsEncryptedFile=%ls, bHasEncryptRequirement=%ls \n", bNeedEncrypt?L"TRUE":L"FALSE",bRet?L"TRUE":L"FALSE",bIsEncryptedFile?L"TRUE":L"FALSE",bHasEncryptRequirement?L"TRUE":L"FALSE" );
	NLCELOG_RETURN_VAL( bRet )
}

/*
* do all obligations what the office pep support: classification, auto file tagging, overlay and encryption( just return if there is the encryption requirement )
*/
bool CNLObMgr::NLParseObligation( 
	_In_  nextlabs::Obligations&	obs,
	_In_  const wchar_t* const		pwchSourceFilePath,
	_In_opt_  const wchar_t* const		pwchDestinationFilePath,
	_In_  const OfficeAction&				emAction,
	_In_  const vector<pair<wstring, wstring>>& vecSrcTag,
	_In_opt_  const OverLayInfo*				pStuNLOverlayInfo,
	_Out_ bool&	                        bIsNeedEncrypt,
	_Out_ vector<pair<wstring, wstring>>&	vecObligationTag,
	_Inout_opt_ vector<pair<wstring, wstring>>*	pvecAllFileTags)
{
	NLCELOG_ENTER
	bool bRet = true;	

	// initialize
	bIsNeedEncrypt = false;
	vecObligationTag.clear();
	if ( NULL != pvecAllFileTags )
	{
		*pvecAllFileTags = vecSrcTag;
	}

	// 1. do classification, NLOBLIGATION_CLASSIFICATION    = 1
	if ( NLIsSupportTheObligation( emAction, kOblClassification ) && NLCheckIfObligationExist( obs, kOblClassification ) )
	{
//#pragma chMSG( "Now this version we don't support classification" )
		NLPRINT_DEBUGLOG( L"this version we don't support the classification" );
	}

	// 2. do auto file tagging, NLOBLIGATION_AUTOFILETAGGING = 2
	// 3. do interactive file tagging, NLOBLIGATION_INTERACTIVEFILETAGGING = 3
	if ( ( NLIsSupportTheObligation( emAction, kOblInteractiveFileTagging ) && NLCheckIfObligationExist( obs, kOblInteractiveFileTagging ) )
			|| 
		( NLIsSupportTheObligation( emAction, kOblAutoFileTagging ) && NLCheckIfObligationExist( obs, kOblAutoFileTagging ) ) )
	{
//#pragma chMSG( "File tagging module will do interactive and auto file tagging with one interface" )
		NLPRINT_DEBUGLOG( L"begin to do file tagging" );
		vector<pair<wstring,wstring>>	vecFileTaggingTag;
		bRet = NLFILETAGGINGINSTANCE.NLDoFileTagging( pwchSourceFilePath, pwchDestinationFilePath, obs, emAction, vecFileTaggingTag );
		vecObligationTag.insert( vecObligationTag.end(), vecFileTaggingTag.begin(), vecFileTaggingTag.end() );
	}

	// after add tags by classify or file tagging, we should query policy again
	if ( !vecObligationTag.empty() && NULL != pvecAllFileTags )
	{
		NLReplaceTagsByTagNameEx( *pvecAllFileTags, vecObligationTag, true );
		ActionResult emOfficeResult = CNxtSDK::DoEvaluation( pwchSourceFilePath, emAction, obs, pwchDestinationFilePath, pvecAllFileTags, CE_NOISE_LEVEL_APPLICATION );
		if ( kRtPCDeny == emOfficeResult )	
		{
//#pragma chMSG( "How do we process this case? do we need deny the current action ?" )
//			NLPRINT_DEBUGLOG( L" we first allow the currrent action but select a tag that need the the current action \n" );
			NLCELOG_RETURN_VAL( false )
		}
	}
	
	// 4. do view overlay, NLOBLIGATION_VIEWOVERLAY  = 4
	if ( NLIsSupportTheObligation( emAction, kOblViewOverlay ) && NLCheckIfObligationExist( obs, kOblViewOverlay ) )
	{
//#pragma chMSG( "do view overlay, now office pep only need path the window hand to it" )
//		NLPRINT_DEBUGLOG( L"begin to do view overlay \n" );
		NLVIEWOVERLAYINSTANCE.DoViewOverlay(obs,pwchSourceFilePath);
	}
	else
	{
		NLVIEWOVERLAYINSTANCE.ClearCreatewindowCache();
	}

	// 5. do print overlay, NLOBLIGATION_PRINTOVERLAY = 5
	if ( NLIsSupportTheObligation( emAction, kOblPrintOverlay ) && NLCheckIfObligationExist( obs, kOblPrintOverlay ) )
	{
//#pragma chMSG( "do print overlay" )
//		NLPRINT_DEBUGLOG( L"begin to do print overlay \n" );
		CNLPrintOverlay& thePrintOverlayIns = CNLPrintOverlay::GetInstance();
		thePrintOverlayIns.SetDoOverlayFlag(thePrintOverlayIns.SetOverlayData(obs,pwchSourceFilePath));

	}

	// 6. do SE encryption, NLOBLIGATION_SEENCRYPTION = 6
	if ( NLIsSupportTheObligation( emAction, kOblSEEncryption ) && NLCheckIfObligationExist( obs, kOblSEEncryption ) )
	{
//#pragma chMSG( "judge SE encryption: no encrypt flag set, not encrypt file and support SE obligation, we need judge it" )
//		NLPRINT_DEBUGLOG( L"begin to set encryption flag \n" );
		bIsNeedEncrypt = true;
	}
	
	NLPRINT_TAGPAIRLOG( vecSrcTag, L"source file tags" );
	NLPRINT_TAGPAIRLOG( vecObligationTag, L"obligation tags" );
	if ( NULL != pvecAllFileTags  )
	{
		NLPRINT_TAGPAIRLOG( *pvecAllFileTags, L"file all file tags" );
	}
	NLCELOG_RETURN_VAL( bRet )
}

//SE file do not allow send, insert and convert; SE file can not allow save as out of the local machine.
EMNLOFFICE_ENCPERMISSION CNLObMgr::NLGetSEPermissopnForCurAction( _In_  const wchar_t* const		pwchSourceFilePath,
																								_In_opt_  const wchar_t* const		pwchDestinationFilePath,
																								_In_  const OfficeAction&	emAction )
{NLCELOG_ENUM_ENTER( EMNLOFFICE_ENCPERMISSION )
	EMNLOFFICE_ENCPERMISSION emEncStatus = emNLEncPermissionUnKnwon;
	
	if ( NLIsEncryptedFile( pwchSourceFilePath ) )
	{
		if ( kOA_SEND == emAction || kOA_CONVERT == emAction || kOA_INSERT == emAction )
		{
			// the encrypt file we do not allow send
			MessageBoxW(GetActiveWindow(),g_szBKSendText, L"NextLabs",MB_OK);
			emEncStatus = emNLEncPermissionNeedDeny;

		} 
		else if ( kOA_COPY == emAction )
		{
			// only allow encrypt file save as local file
			if ( !IsLocalDriver( pwchDestinationFilePath ) )
			{
				MessageBoxW(GetActiveWindow(),g_szBKSaveAsText, L"NextLabs",MB_OK);
				emEncStatus = emNLEncPermissionNeedDeny;
			}
			else
			{
				emEncStatus = emNLEncPermissionNeedEncryption;
			}
		}
	}
	NLCELOG_ENUM_RETURN_VAL( emEncStatus )
}

bool CNLObMgr::NLSaveFileByPEP( _In_ IDispatch* pDoc )
{ONLY_DEBUG
	HRESULT hr = S_FALSE;
	wstring wstrFilePath; 
	if ( getDocumentPathEx( wstrFilePath, pDoc ) )
	{
		NLPRINT_DEBUGLOG( L"the file path:[%s]", wstrFilePath.c_str() );

		CNLObMgr& theObMgr = CNLObMgr::NLGetInstance();
		theObMgr.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true );

		// before save we should prepare file encrypt flag and golden tag cache
		if ( theObMgr.NLIsEncryptedFile( wstrFilePath ) )
		{
			// set file tag cache
			theObMgr.NLSetEncryptRequirementFlag( wstrFilePath, true );

			// update golden tag cache
			theObMgr.NLUpdateGoldenTagCache( wstrFilePath );
		}

		switch ( pep::appType() )
		{
		case kAppWord:  
			{
				Word::_DocumentPtr ptrDoc(pDoc);
				hr = ptrDoc->put_Saved( VARIANT_FALSE );
				ptrDoc->Save();
			}
			break;
		case kAppExcel:
			{
				Excel::_WorkbookPtr ptrDoc(pDoc);
				hr = ptrDoc->put_Saved( 0, VARIANT_FALSE );
				ptrDoc->Save();
			}
			break;
		case kAppPPT:
			{
				PPT::_PresentationPtr ptrDoc(pDoc);
				hr = ptrDoc->put_Saved( msoFalse );
				ptrDoc->Save();
			}
			break;
		default:
			break;
		}
		theObMgr.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, false );
	}
	return SUCCEEDED(hr);
}

void CNLObMgr::NLInitializeSynchronizeGoldenTags()
{
	m_theSESynchronizeGoldenTags.NLInitialize();
}

void CNLObMgr::NLUnInitializeSynchronizeGoldenTags()
{
	m_theSESynchronizeGoldenTags.NLUnInitialize();
}

void CNLObMgr::NLUpdateGoldenTagCache( _In_ const wstring& wstrFilePath )
{
	vector<pair<wstring,wstring>> vecTagPair;
	if ( NLReadTag( wstrFilePath, vecTagPair ) && !vecTagPair.empty() )
	{
		NLSetGoldenTagCache( wstrFilePath, vecTagPair );
		NLPRINT_TAGPAIRLOG( vecTagPair, wstrFilePath.c_str(), L"Update golden tag cache end" );
	}	
}

bool CNLObMgr::NLStartSynchronizeGoldenTags( _In_ const wstring& wstrFilePath, _In_ const bool bIOSynchronize, _In_ const wchar_t* pwchTempFilePath )
{NLCELOG_ENTER
	bool bStart = true;
	vector<pair<wstring,wstring>> vecGoldenTags;
	if ( NLGetGoldenTagCache( wstrFilePath, vecGoldenTags ) && !vecGoldenTags.empty() )
	{
		NLPRINT_TAGPAIRLOG( vecGoldenTags, L"golden tags" );
		bStart = m_theSESynchronizeGoldenTags.NLStartSynchronizeGoldenTags( NULL==pwchTempFilePath ? wstrFilePath : pwchTempFilePath, vecGoldenTags, bIOSynchronize );
		
		/* 
			Synchronize I/O to write golden tags success, we will clear the tag cache here
			For asynchronous I.O to write golden tags, we will clear the tag cache at the I/O work thread. 
		*/
		if ( bStart && bIOSynchronize )
		{
			NLClearGoldenTagFileCache( wstrFilePath );
		}	
	}
	NLCELOG_RETURN_VAL( bStart )
}

bool CNLObMgr::NLStartSynchronizeGoldenTags( _In_ const bool bIOSynchronize )
{NLCELOG_ENTER
	map<wstring, STUNLOFFICE_GOLDENTAG> mapGoldenTags;
	NLGetGoldenTagCache( mapGoldenTags );
	
	bool bStart = true;
	if ( !mapGoldenTags.empty() )
	{
		bStart = m_theSESynchronizeGoldenTags.NLStartSynchronizeGoldenTags( mapGoldenTags, bIOSynchronize );
		/* 
			Synchronize I/O to write golden tags success, we will clear the tag cache here
			For asynchronous I.O to write golden tags, we will clear the tag cache at the I/O work thread. 
		*/
		if ( bStart && bIOSynchronize )
		{
			NLClearGoldenTagCache( );
		}	
	}
	NLCELOG_RETURN_VAL( bStart )
}

bool CNLObMgr::NLEncryptTempFile( _In_ const AppType& emOfficeAppType, _In_ const wstring& wstrFilePath, _In_ const bool bForSave, _In_ const bool bUpdateGoldenTagCache )
{NLCELOG_ENTER
	bool bEncrypt = m_theNLEncrypt.EncryptTempFile( emOfficeAppType, wstrFilePath, bForSave );
	NLSetEncryptFileFlag( wstrFilePath, bEncrypt );

	if ( bUpdateGoldenTagCache )
	{
		NLPRINT_DEBUGLOG( L"Emcrypt temp file and update the golden tag cache \n" );
		NLUpdateGoldenTagCache( wstrFilePath );
	}
	NLCELOG_RETURN_VAL( bEncrypt )
}

bool CNLObMgr::NLEncryptTempFile( _In_ const AppType& emOfficeAppType, _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTags, _In_ const bool bForSave )
{NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: emOfficeAppType=%d, wstrFilePath=%ls, vecFileTags=%p, bForSave=%ls \n", emOfficeAppType,wstrFilePath.c_str(),&vecFileTags,bForSave?L"TRUE":L"FALSE");
	bool bEncrypt = m_theNLEncrypt.EncryptTempFile( emOfficeAppType, wstrFilePath, bForSave );

	// here we should back the golden tags
	if ( bEncrypt )
	{
		NLPRINT_DEBUGLOG( L"this is encrypt file and we add the tags into folden tags cache now \n" );
		NLSynchroSETagCache( wstrFilePath, vecFileTags );
		NLPRINT_TAGPAIRLOG( vecFileTags, wstrFilePath.c_str() );
	}
	else
	{
		NLPRINT_DEBUGLOG( L" encrypt file:[%s] failed \n", wstrFilePath.c_str() );
	}
	NLSetEncryptFileFlag( wstrFilePath, bEncrypt );
	NLCELOG_RETURN_VAL( bEncrypt )
}

bool CNLObMgr::NLEncryptFile( _In_ const wstring& wstrFilePath )
{NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls \n", wstrFilePath.c_str());
	bool bEncryptFile = NLIsEncryptedFile( wstrFilePath );

	if ( !bEncryptFile )
	{
		bEncryptFile = m_theNLEncrypt.EncryptFile( wstrFilePath ); 
	}
	NLSetEncryptFileFlag( wstrFilePath, bEncryptFile );
	NLCELOG_RETURN_VAL( bEncryptFile )
}

bool CNLObMgr::NLEncryptFile( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTags )
{NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, vecFileTags=%p \n", wstrFilePath.c_str(),&vecFileTags);
	bool bEncryptFile = NLIsEncryptedFile( wstrFilePath );

	if ( !bEncryptFile )
	{
		bEncryptFile = m_theNLEncrypt.EncryptFile( wstrFilePath ); 
	}

	if ( bEncryptFile )
	{
		NLPRINT_DEBUGLOG( L"this is encrypt file and we add the tags into folden tags cache now \n" );
		NLSynchroSETagCache( wstrFilePath, vecFileTags );
		NLPRINT_TAGPAIRLOG( vecFileTags, wstrFilePath.c_str() );
	}
	NLSetEncryptFileFlag( wstrFilePath, bEncryptFile );
	NLCELOG_RETURN_VAL( bEncryptFile )
}

bool CNLObMgr::NLIsEncryptedFile( _In_ const wstring& wstrFilePath )
{
	// for bug24140: using save as to replace the original one
	return m_theNLEncrypt.IsEncryptedFile( wstrFilePath );
}

void CNLObMgr::NLSetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath, _In_ const ActionResult& emActionResult )
{
    DWORD dwCurTickCount = GetTickCount();
    EnterCriticalSection(&m_csPreOpenEvaActionResult);
    NLPRINT_DEBUGLOGEX( true, L"--Set preopen eva action result:[%s]=[0x%x], TickCount:[%u]\n", wstrFilePath.c_str(), emActionResult, dwCurTickCount );
    m_mapPreOpenEvaActionResult[wstrFilePath] = std::pair<ActionResult, DWORD>(emActionResult, dwCurTickCount);
    LeaveCriticalSection(&m_csPreOpenEvaActionResult);
}

ActionResult CNLObMgr::NLGetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath, _In_ const bool kbAutoCheckFromPC, _In_ const ActionResult& kemDefaultActionResultIfFailed) throw()
{
    ActionResult emPreOpenEvaActionResult = NLInnerGetPreOpenEvaActionResult(wstrFilePath);
    if ((kRtUnknown == emPreOpenEvaActionResult) && kbAutoCheckFromPC)
    {
        emPreOpenEvaActionResult = NLGetEvaluationResult(wstrFilePath.c_str(), kOA_OPEN, L"");
        if ((kRtPCDeny != emPreOpenEvaActionResult) && (kRtPCAllow != emPreOpenEvaActionResult))
        {
            // PPT ~ temp file cannot success evaluation and deny
            NLPRINT_DEBUGLOG(L"The file:[%s] preopen evaluation result is:[0x%x], this is a unexpected result, default we remark the result as PCAllow\n", wstrFilePath.c_str(), emPreOpenEvaActionResult);
            emPreOpenEvaActionResult = kemDefaultActionResultIfFailed;
        }
        NLSetPreOpenEvaActionResult(wstrFilePath, emPreOpenEvaActionResult);
    }
    return emPreOpenEvaActionResult;
}

ActionResult CNLObMgr::NLInnerGetPreOpenEvaActionResult( _In_ const wstring& wstrFilePath ) const throw()
{
    ActionResult emActionResultRet = kRtUnknown;

    DWORD dwCurTickCount = GetTickCount();

    EnterCriticalSection(&m_csPreOpenEvaActionResult);
    map<wstring, std::pair<ActionResult, DWORD> >::const_iterator itr = m_mapPreOpenEvaActionResult.find( wstrFilePath );
    if ( itr != m_mapPreOpenEvaActionResult.end() )
    {
        const std::pair<ActionResult, DWORD>& pairActionResult = itr->second;
        if (IsValidPreopEvaActionResult(itr->first, pairActionResult, dwCurTickCount))
        {
            // Valid
            emActionResultRet = pairActionResult.first;
        }
        else
        {
            // Invalid, the data will be neatened in open event or office access deny dialog close event
        }
    }
    LeaveCriticalSection(&m_csPreOpenEvaActionResult);

    NLPRINT_DEBUGLOGEX( true, L"--get preopen eva action result:[%s]=[0x%x] \n", wstrFilePath.c_str(), emActionResultRet );
    return emActionResultRet;
}

bool CNLObMgr::IsValidPreopEvaActionResult(_In_ const std::wstring& wstrFilePath, _In_ const std::pair<ActionResult, DWORD>& pairActionResult, _In_ const DWORD kdwCurTick) const throw()
{
    bool bValid = false;
    if (IsLocalDriver(wstrFilePath))
    {
        if ((m_dwLocalFilePreopenEvaResultValidTimeMs + pairActionResult.second) > kdwCurTick)
        {
            bValid = true;
        }
    }
    else
    {
        if ((m_dwNetFilePreopenEvaResultValidTimeMs + pairActionResult.second) > kdwCurTick)
        {
            bValid = true;
        }
    }
    return bValid;
}

bool CNLObMgr::IsContainsPreOpenSpecifyEvaActionResult(_In_ const ActionResult& kemActionResultIn) const throw()
{
    DWORD dwCurTickCount = GetTickCount();

    bool bContainsEvaDenyResult = false;
    EnterCriticalSection(&m_csPreOpenEvaActionResult);
    for (map<wstring, std::pair<ActionResult, DWORD> >::const_iterator itr = m_mapPreOpenEvaActionResult.begin(); itr != m_mapPreOpenEvaActionResult.end(); ++itr)
    {
        const std::pair<ActionResult, DWORD>& pairActionResult = itr->second;
        if (IsValidPreopEvaActionResult(itr->first, pairActionResult, dwCurTickCount))
        {
            // Valid, keep
            if (pairActionResult.first == kemActionResultIn)
            {
                bContainsEvaDenyResult = true;
                break;
            }
        }
        else
        {
            // Invalid, continue check
        }
    }
    LeaveCriticalSection(&m_csPreOpenEvaActionResult);
    return bContainsEvaDenyResult;
}

// Tag cache initialize flag
void CNLObMgr::NLSetObMgrInitializeFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bInitialize )
{
	EnterCriticalSection(&m_csObMgrInitialize);
	NLPRINT_DEBUGLOGEX( true, L"--Set initialize flag:[%s]=[%s] \n", wstrFilePath.c_str(), bInitialize ? L"true" : L"false" );
	m_mapObMgrInitialize[wstrFilePath] = bInitialize;
	LeaveCriticalSection(&m_csObMgrInitialize);
}

void CNLObMgr::NLRevertObMgrInitializeFlag()
{
	EnterCriticalSection(&m_csObMgrInitialize);
	map<wstring, bool>::iterator itr = m_mapObMgrInitialize.begin();
	for ( ; itr != m_mapObMgrInitialize.end(); itr++ )
	{
		itr->second = false;
	}
	LeaveCriticalSection(&m_csObMgrInitialize);
}

bool CNLObMgr::NLGetObMgrInitializeFlag( _In_ const wstring& wstrFilePath )
{
	bool bInit = false;
	EnterCriticalSection(&m_csObMgrInitialize);
	map<wstring, bool>::iterator itr = m_mapObMgrInitialize.find( wstrFilePath );
	if ( itr != m_mapObMgrInitialize.end() )
	{
		bInit = itr->second;
	}
	LeaveCriticalSection(&m_csObMgrInitialize);

	NLPRINT_DEBUGLOGEX( true, L"Get initialize flag:[%s]=[%s] \n", wstrFilePath.c_str(), bInit ? L"true" : L"false" );
	return bInit;
}

bool CNLObMgr::NLInitializeObMgr( _In_ const wstring& wstrFilePath, _In_opt_ IDispatch* pDoc, _In_ const bool bGoldenTagFlag )
{
	NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L"the parameters: wstrFilePath=[%s], pDoc=[%p], bGoldenTagFlag=[%d] \n", wstrFilePath.c_str(), pDoc, bGoldenTagFlag );
	// check parameter
	if ( wstrFilePath.empty() )
	{
		NLCELOG_RETURN_VAL( false ) 
	}
	
	bool bInit = true;
	
	// for template file double click a new file opened with tags from the template file
	// 1. initialize encrypt file cache
	bool bIsEncryptFile = m_theNLEncrypt.IsEncryptedFile( wstrFilePath );
	NLSetEncryptFileFlag( wstrFilePath, bIsEncryptFile );
		
	// 2. initialize tag cache
	/*
	*	for .xltx file, in open action we just can get the file short name without file path.
	* eg: c:\kaka\test.xltx --> we only can get test as the file name
	*/
	vector<pair<wstring,wstring>> vecFileTagPair;
	if ( bIsEncryptFile || NULL == pDoc )
	{
		bInit = NLInnerUpdateTags( wstrFilePath, true, vecFileTagPair );
	} 
	else
	{
		bInit = NLInnerUpdateTags( pDoc, true, vecFileTagPair );
	}
	
	NLPRINT_TAGPAIRLOG( vecFileTagPair, L"File Tags" );
	// here we need check if the file golden tags has been in cache
	if ( bIsEncryptFile && bGoldenTagFlag )
	{
		// Save as, need set the golden tags into the file cache
		vector<pair<wstring,wstring>> vecGoldenTag;
		if ( NLGetGoldenTagCache( wstrFilePath, vecGoldenTag ) && !vecGoldenTag.empty() )
		{
			NLPRINT_TAGPAIRLOG( vecGoldenTag, L"Golden tags" );
			NLReplaceTagsByTagNameEx( vecFileTagPair, vecGoldenTag, true );
		}

		// save the tag into the tag cache
		NLPRINT_DEBUGLOG( L"this is SE file and we get the old golden tags into the file tag cache \n" );
		NLSetFileTagCache( wstrFilePath, vecFileTagPair );
	}
	NLPRINT_TAGPAIRLOG( vecFileTagPair, wstrFilePath.c_str() );
	NLPRINT_DEBUGLOG( L" out inner update tags:[%s], file path:[%s], encrypt flag:[%s] \n", bInit?L"true":L"false", wstrFilePath.c_str(), bIsEncryptFile?L"true":L"false" );

	// 3. set ObMgr initialize flag
	NLSetObMgrInitializeFlag( wstrFilePath, bInit );
	NLCELOG_RETURN_VAL( bInit )
}

// synchronize the file custom tags to golden tags
bool CNLObMgr::NLSynchroniseFileTags( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecGoldenTags )
{
	/*
		About synchronize SE golden tags
		1. we can write golden tags into if we can get file write access.
		2. we can write the golden tags when the office process exit.
		3. For Word/Excel/PPT edit action we can write the golden tags in the function "BJReplaceFile" throw hook API "ReplaceFile".
		4. For Word/Excel copy action we also can write the golden tags in the function "BJReplaceFile" throw hook API "ReplaceFile".
		5. For PPT copy action we can synchronize the golden tags immediately. we cache them and synchronize them when the process exit.

		About cache synchronize
		1. we synchronize the file tags between file and tag cache when we write tags into file or into the golden tag cache
		2. we just write the golden tags into the file, so here no need to update the file tags cache when we write the tags  
	*/
	bool bWriteSuccess = NLInnerWriteTag( wstrFilePath, vecGoldenTags, false, false );
	if ( bWriteSuccess )
	{
		// clear the golden tag cache after it success synchronized
		NLClearGoldenTagFileCache( wstrFilePath );
	}
	return bWriteSuccess;
}

// Read tags
bool CNLObMgr::NLReadTag( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair )
{
	vecFileTagPair.clear();
	
	bool bRet = false;
	if ( NLGetObMgrInitializeFlag( wstrFilePath ) )
	{	
		bRet = NLInnerReadTag( wstrFilePath, vecFileTagPair, true );	// template file double click a new document opened
	}
	else
	{
		NLPRINT_DEBUGLOG( L"!!!!Error ---- un- intialize ---, please check! \n" );
	}
	return bRet;
}

bool CNLObMgr::NLReadTag( _In_ IDispatch* pDoc, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair )
{
	wstring wstrFilePath;
	if ( getDocumentPathEx( wstrFilePath, pDoc ) )
	{
		NLPRINT_DEBUGLOG( L"the document file path is:[%s] \n", wstrFilePath.c_str() );
		return NLReadTag( wstrFilePath, vecFileTagPair );
	}
	return false;
}

// Write tags
bool CNLObMgr::NLWriteTag( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ const bool bIsAllFileTag, _In_ bool bIfNeedErrHanding, _In_ const bool bFourceGoldenTag )
{
	NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, vecFileTagPair=%p, bIsAllFileTag=%ls, bIfNeedErrHanding=%ls \n", wstrFilePath.c_str(),&vecFileTagPair,bIsAllFileTag?L"TRUE":L"FALSE",bIfNeedErrHanding?L"TRUE":L"FALSE");
	bool bRet = false;
	vector<pair<wstring,wstring>> vecAllFileTags;
	if ( !bIsAllFileTag )
	{
		if ( NLReadTag( wstrFilePath, vecAllFileTags ) && !vecFileTagPair.empty() )
		{
			NLReplaceTagsByTagNameEx( vecAllFileTags, vecFileTagPair, true );
		}
	}
	
	const vector<pair<wstring,wstring>>& vecTempAllFileTags = bIsAllFileTag ? vecFileTagPair : vecAllFileTags;
	if ( bFourceGoldenTag || NLIsEncryptedFile( wstrFilePath ) )
	{
		// update tags that in the cache
		NLSynchroSETagCache( wstrFilePath, vecTempAllFileTags );
		NLPRINT_TAGPAIRLOG( vecTempAllFileTags, wstrFilePath.c_str(), L"Se File Tag end" );
		bRet = true;
	}
	else
	{
		NLPRINT_DEBUGLOG( L"--- !!!!!! this is normal file, may be this is an wrong invoke \n" );
		bRet = NLInnerWriteTag( wstrFilePath, vecTempAllFileTags, true, bIfNeedErrHanding );
	}
	NLCELOG_RETURN_VAL( bRet )
}

bool CNLObMgr::NLWriteTag( _In_ IDispatch* pDoc, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ const bool bIsAllFileTag, _In_ bool bIfNeedErrHanding, _In_ const bool bIfNeedSave )
{
	NLCELOG_ENTER	
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, vecFileTagPair=%p, bIsAllFileTag=%ls, bIfNeedErrHanding=%ls \n", pDoc,&vecFileTagPair,bIsAllFileTag?L"TRUE":L"FALSE",bIfNeedErrHanding?L"TRUE":L"FALSE");
	// here we can check if the file is encrypt file we should use the file path to add tags
	bool bWriteTags = false;
	wstring wstrFilePath;

	if ( getDocumentPathEx( wstrFilePath, pDoc ) )
	{
		NLPRINT_DEBUGLOG( L"the file path is:[%s] \n", wstrFilePath.c_str() );
		// 1. update tags that in the cache
		vector<pair<wstring,wstring>> vecAllFileTags;
		if ( !bIsAllFileTag )
		{
			// user pass all the file tags we no need to do this
			if ( NLReadTag( wstrFilePath, vecAllFileTags ) )
			{
				NLPRINT_TAGPAIRLOG( vecAllFileTags, L" File old Tags" );
				NLPRINT_TAGPAIRLOG( vecFileTagPair, L" File obligation Tag pair" );
				NLReplaceTagsByTagNameEx( vecAllFileTags, vecFileTagPair, true );
				NLPRINT_TAGPAIRLOG( vecAllFileTags, L" File new Tags	" );
			}
		}
		
		// begin to write file tags
		const vector<pair<wstring,wstring>>& vecTempAllFileTags = bIsAllFileTag ? vecFileTagPair : vecAllFileTags;
		// check if it is encrypt file
		if ( NLIsEncryptedFile( wstrFilePath ) )
		{
			NLPRINT_DEBUGLOG( L"this is encrypt file \n" );
			NLSynchroSETagCache( wstrFilePath, vecTempAllFileTags );	// tag cache, store all the current file tags
			bWriteTags = true;
		} 
		else
		{
			// for .xltx file we can get the full file path				
			// make sure no save event notify if we save the tags by ourself
			bIfNeedSave ? NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true ) : NULL;		
			bWriteTags = NLInnerWriteTag( pDoc, vecTempAllFileTags, true, bIfNeedErrHanding, bIfNeedSave);
			bIfNeedSave ? NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, false ) : NULL;
		}
	}
	NLCELOG_RETURN_VAL( bWriteTags )
}

void CNLObMgr::NLSynchroSETagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair )
{
	NLSetFileTagCache( wstrFilePath, vecFileTagPair );	// tag cache, store all the current file tags
	NLSetGoldenTagCache( wstrFilePath, vecFileTagPair );
	NLPRINT_TAGPAIRLOG( vecFileTagPair, wstrFilePath.c_str(), L"SE golden tags end" );
}

// Golden tags back
void CNLObMgr::NLSetGoldenTagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecGoldeTags )
{
	EnterCriticalSection(&m_csGoldenTag);
	m_pNLData->NLSetGoldenTagCache( wstrFilePath, vecGoldeTags );
	LeaveCriticalSection(&m_csGoldenTag);
}

bool CNLObMgr::NLGetGoldenTagCache( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecGoldeTags )
{
	NLCELOG_ENTER
	bool bRet = false;
	EnterCriticalSection(&m_csGoldenTag);
	if ( m_pNLData->NLGetGoldenTagCache( wstrFilePath, vecGoldeTags ) )
	{
		bRet = true;
	}
	LeaveCriticalSection(&m_csGoldenTag);
	NLCELOG_RETURN_VAL( bRet )
}

void CNLObMgr::NLGetGoldenTagCache( _Out_ map<wstring, STUNLOFFICE_GOLDENTAG>& mapGoldenTags )
{
	EnterCriticalSection(&m_csGoldenTag);
	m_pNLData->NLGetGoldenTagCache( mapGoldenTags );
	LeaveCriticalSection(&m_csGoldenTag);
}

void CNLObMgr::NLSetFileTagCache( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecTagPair )
{
	m_pNLData->NLSetFileTagCache( wstrFilePath, vecTagPair );
}

bool CNLObMgr::NLGetFileTagCache( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecTagPair )
{
	return m_pNLData->NLGetFileTagCache( wstrFilePath, vecTagPair );
}

bool CNLObMgr::NLInnerReadTag( _In_ const wstring& wstrFilePath, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bReadFromTagCache )
{
	if ( wstrFilePath.empty() )
	{
		return false;
	}

	bool bRet = false;
	if ( bReadFromTagCache )
	{
		bRet = m_pNLData->NLGetFileTagCache( wstrFilePath, vecFileTagPair );
	}
	else
	{
		bRet = m_theNxtTag.ReadTag( wstrFilePath, vecFileTagPair );
	}
	return bRet;
}

bool CNLObMgr::NLInnerReadTag( _In_ IDispatch* pDoc,         _Out_ vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bReadFromTagCache )
{
	if ( NULL == pDoc )
	{
		return false;
	}

	bool bRet = false;
	if ( bReadFromTagCache )
	{
		// get file path by pDoc
		wstring wstrFilePath; 
		if ( getDocumentPathEx( wstrFilePath, pDoc ) )
		{
			bRet = m_pNLData->NLGetFileTagCache( wstrFilePath, vecFileTagPair );
		}
	}
	else
	{
		bRet = m_theNxtTag.ReadTag( pDoc, vecFileTagPair );
		NLSplitTagValuesBySeprator( vecFileTagPair );
		NLPRINT_TAGPAIRLOG( vecFileTagPair, L"inner read tags by pDoc" );
	}
	return bRet;
}

bool CNLObMgr::NLInnerWriteTag( _In_ const wstring& wstrFilePath, _In_ const vector<pair<wstring,wstring>>& vecFileTagPair, _In_ bool bUpdateTagCache, _In_ bool bIfNeedErrHanding )
{
	NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, bUpdateTagCache=%ls, bIfNeedErrHanding=%ls \n", wstrFilePath.c_str(),bUpdateTagCache?L"TRUE":L"FALSE",bIfNeedErrHanding?L"TRUE":L"FALSE");
	if ( wstrFilePath.empty() )
	{
		NLCELOG_RETURN_VAL( false )
	}
	
	bool bRet = false;
	if ( bIfNeedErrHanding )
	{
#pragma chMSG( "Now this version not support the error handling" )
		NLPRINT_DEBUGLOG( L"No error handling support \n" );
		bRet = m_theNxtTag.AddTag( wstrFilePath, vecFileTagPair );
	} 
	else
	{
		bRet = m_theNxtTag.AddTag( wstrFilePath, vecFileTagPair );
	}

	if ( bRet && bUpdateTagCache )
	{
		NLSetFileTagCache( wstrFilePath, vecFileTagPair );	// tag cache, store all the current file tags
	}
	NLCELOG_RETURN_VAL( bRet )
}

bool CNLObMgr::NLInnerWriteTag( 
	_In_ IDispatch* pDoc,         
	_In_ const vector<pair<wstring,wstring>>& vecFileTagPair, 
	_In_ bool bUpdateTagCache, 
	_In_ bool bIfNeedErrHanding, 
	_In_ const bool bIfNeedSave )
{
	NLCELOG_ENTER
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, bUpdateTagCache=%ls, bIfNeedErrHanding=%ls \n", pDoc,bUpdateTagCache?L"TRUE":L"FALSE",bIfNeedErrHanding?L"TRUE":L"FALSE");
	if ( NULL == pDoc )
	{
		NLCELOG_RETURN_VAL( false )
	}
	
	bool bRet = false;
	if ( bIfNeedErrHanding )
	{
#pragma chMSG( "Now this version not support the error handling" )
		NLPRINT_DEBUGLOG( L"No error handling support \n" );
		bRet = m_theNxtTag.AddTag( pDoc, vecFileTagPair, bIfNeedSave );
	} 
	else
	{
		bRet = m_theNxtTag.AddTag( pDoc, vecFileTagPair, bIfNeedSave );
	}

	if ( bRet && bUpdateTagCache )
	{
		wstring wstrFilePath;
		if ( getDocumentPathEx( wstrFilePath, pDoc ) )
		{
			NLSetFileTagCache( wstrFilePath, vecFileTagPair );
			NLPRINT_TAGPAIRLOG( vecFileTagPair, L"Add tag cache succeed \n" );
		}
	}
	NLCELOG_RETURN_VAL( bRet )
}

/*	bRefreshTagCache:
*		true: clear the tag cache and then read tags from file to update the tag cache
*		false: clear the file tags and get the tags from tag cache and then write it into the file.
*/
bool CNLObMgr::NLInnerUpdateTags( _In_ const wstring& wstrFilePath, _In_ const bool& bRefreshTagCache, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair )
{
	bool bRet = false;
	if ( bRefreshTagCache )
	{
		if ( m_theNxtTag.ReadTag( wstrFilePath, vecFileTagPair ) )
		{
			NLSetFileTagCache( wstrFilePath, vecFileTagPair );
			bRet = true;
		}
	}
	else
	{
		if ( m_pNLData->NLGetFileTagCache( wstrFilePath, vecFileTagPair ) )
		{
			NLInnerWriteTag( wstrFilePath, vecFileTagPair, true, false );
		}
	}
	return bRet;
}

bool CNLObMgr::NLInnerUpdateTags( _In_ IDispatch* pDoc,         _In_ const bool& bRefreshTagCache, _Out_ vector<pair<wstring,wstring>>& vecFileTagPair )
{
	if ( NULL == pDoc )
	{
		return false;
	}

	wstring wstrFilePath;
	bool bRet = getDocumentPathEx( wstrFilePath, pDoc );
	NLPRINT_DEBUGLOG( L"FilePath:[%s] \n", wstrFilePath.c_str() );
	if ( !bRet )
	{
		NLPRINT_DEBUGLOG( L"Get file path failed \n" );
		return false;
	}
			
	if ( bRefreshTagCache )
	{
		if ( m_theNxtTag.ReadTag( pDoc, vecFileTagPair ) )
		{
			NLSetFileTagCache( wstrFilePath, vecFileTagPair );
			NLPRINT_TAGPAIRLOG( vecFileTagPair, L"inner update tags refresh tag cache" );
			bRet = true;
		}
	}
	else
	{
		if ( m_pNLData->NLGetFileTagCache( wstrFilePath, vecFileTagPair ) )
		{
			NLInnerWriteTag( pDoc, vecFileTagPair, true, false );
		}
	}
	return bRet;
}

void CNLObMgr::NLRemoveSameTags( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const bool& bIfNeedWholeMatch )
{
	vecOutTagPair.clear();
	vector<pair<wstring,wstring>>::const_iterator itrFisrt = vecTagPair.begin();
	for ( ; itrFisrt != vecTagPair.end(); itrFisrt++ )
	{
		bool bIsSame = false;
		vector<pair<wstring,wstring>>::const_iterator itrSecond = itrFisrt;
		for ( itrSecond++; itrSecond != vecTagPair.end(); itrSecond++ )
		{
			if ( itrFisrt->first == itrSecond->first )
			{
				bIsSame = bIfNeedWholeMatch ? ( itrFisrt->second == itrSecond->second ) : true;
				break;
			}
		}

		if ( !bIsSame )
		{
			vecOutTagPair.push_back( *itrFisrt );
		}
	}
}

void CNLObMgr::NLMergeTagsByTagName( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const wstring wstrSeparater )
{
	// init out tag pair
	vecOutTagPair.clear();
	vector<pair<wstring,wstring>>::const_iterator itrFisrt = vecTagPair.begin();
	for ( ; itrFisrt != vecTagPair.end(); itrFisrt++ )
	{
		if ( !itrFisrt->first.empty() && !itrFisrt->second.empty() )
		{
			vecOutTagPair.push_back( *itrFisrt );
			break;
		}
	}
	
	// begin merge tags
	itrFisrt = vecTagPair.begin();
	for ( ; itrFisrt != vecTagPair.end(); itrFisrt++ )
	{
		bool bReplace = false;
		vector<pair<wstring,wstring>>::iterator itrSecond = vecOutTagPair.begin();
		for ( ; itrSecond != vecOutTagPair.end(); itrSecond++ )
		{
			if ( !itrFisrt->first.empty() && !itrFisrt->second.empty() )
			{
				if ( itrFisrt->first == itrSecond->first )
				{
					bReplace = true;
					if ( itrFisrt->second != itrSecond->second )
					{
						// need merge
						itrSecond->second += wstrSeparater + itrFisrt->second;
						break;
					}
				}
			}
		}

		if ( !bReplace )
		{
			vecOutTagPair.push_back( *itrFisrt );
		}
	}
}

void CNLObMgr::NLReplaceTagsByTagName( _Out_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecSrcTagPair, _In_ const vector<pair<wstring,wstring>>& vecDesTagPair )
{
	
	vecOutTagPair = vecDesTagPair;

	vector<pair<wstring,wstring>>::const_iterator itrSrc = vecSrcTagPair.begin();
	for ( ; itrSrc != vecSrcTagPair.end(); itrSrc++ )
	{
		bool bIsSameTagName = false;
		vector<pair<wstring,wstring>>::const_iterator itrDes = vecDesTagPair.begin();
		for ( ; itrDes != vecDesTagPair.end(); itrDes++ )
		{
			if ( itrDes->first == itrSrc->first )
			{
				bIsSameTagName = true;
				break;
			}
		}

		if ( !bIsSameTagName )
		{
			vecOutTagPair.push_back( *itrSrc );
		}
	}
}

void CNLObMgr::NLReplaceTagsByTagNameEx( _Inout_ vector<pair<wstring,wstring>>& vecOutTagPair, _In_ const vector<pair<wstring,wstring>>& vecTagPair, _In_ const bool bNeedSplitOut )
{
	vector<pair<wstring,wstring>> vecTempTagPair;
	NLMergeTagsByTagName( vecTempTagPair, vecTagPair );
	
	vector<pair<wstring,wstring>> vecTempOutTagPair;
	NLMergeTagsByTagName( vecTempOutTagPair, vecOutTagPair );

	NLReplaceTagsByTagName( vecOutTagPair, vecTempOutTagPair, vecTempTagPair );

	if ( bNeedSplitOut )
	{
		NLSplitTagValuesBySeprator( vecOutTagPair );
	}
	NLPRINT_TAGPAIRLOG( vecOutTagPair, bNeedSplitOut ? L"Split true" : L"Split false" );
}

void CNLObMgr::NLSplitTagValuesBySeprator( _Inout_ vector<pair<wstring,wstring>>& vecTagPair, _In_ const wstring wstrSeparator )
{
	vector<pair<wstring,wstring>> vecTagPairOut;
	vector<pair<wstring,wstring>>::iterator itr = vecTagPair.begin();
	for ( ; itr != vecTagPair.end(); itr++ )
	{
		vector<wstring> vecTagValues;
		boost::algorithm::split( vecTagValues, itr->second, boost::algorithm::is_any_of( wstrSeparator ) );
		
		vector<wstring>::iterator itrValues = vecTagValues.begin();
		for ( ; itrValues != vecTagValues.end(); itrValues++ )
		{
			vecTagPairOut.push_back( pair<wstring,wstring>(itr->first, *itrValues) );
		}
	}
	vecTagPair = vecTagPairOut;
}

OblMap CNLObMgr::NLSetObligationFlag( const UINT32 unFlag )
{
	return NLOBLIGATIONTYPEINSTANCE.NLSetObligationFlag( unFlag );
}

void CNLObMgr::NLInitMapBetweenActionAndObligation()
{
/*
	NLOBLIGATION_UNKNOWN         = 0x00,
	NLOBLIGATION_CLASSIFICATION  = 0x01,
	NLOBLIGATION_AUTOFILETAGGING = 0x02,
	NLOBLIGATION_INTERACTIVEFILETAGGING = 0x04,
	NLOBLIGATION_VIEWOVERLAY  = 0x08,
	NLOBLIGATION_PRINTOVERLAY = 0x10,
	NLOBLIGATION_SEENCRYPTION = 0x20
*/
	// This is used for parse obligation, and the obligation must be queried by PEP not user.
	m_mapActionWithObligation[kOA_Unknown]   = NLSetObligationFlag( 0x00 );	// support: no obligation
	m_mapActionWithObligation[kOA_CLASSIFY]  = NLSetObligationFlag( 0x00 );	// support: no obligation

	m_mapActionWithObligation[kOA_OPEN]      = NLSetObligationFlag( static_cast<UINT32>( kOblViewOverlay ) );	// support: VIEWOVERLAY
	
	m_mapActionWithObligation[kOA_EDIT]      = NLSetObligationFlag( static_cast<UINT32>( kOblSEEncryption + kOblAutoFileTagging + kOblInteractiveFileTagging ) );	// support: SEENCRYPTION, FILETAGGING(AUTO+INTERACTIVE) 
	m_mapActionWithObligation[kOA_COPY]      = NLSetObligationFlag( static_cast<UINT32>( kOblSEEncryption + kOblAutoFileTagging + kOblInteractiveFileTagging ) );	// support: SEENCRYPTION, FILETAGGING(AUTO+INTERACTIVE) 
	
	m_mapActionWithObligation[kOA_PASTE]     = NLSetObligationFlag( 0x00 );	// support: no obligation
	
	m_mapActionWithObligation[kOA_PRINT]     = NLSetObligationFlag( static_cast<UINT32>( kOblPrintOverlay ) );	// support: PRINTOVERLAY
	
	m_mapActionWithObligation[kOA_SEND]      = NLSetObligationFlag( 0x00 );	// support: no obligation
	m_mapActionWithObligation[kOA_INSERT]    = NLSetObligationFlag( 0x00 );	// support: no obligation

	m_mapActionWithObligation[kOA_CONVERT]   = NLSetObligationFlag( static_cast<UINT32>( kOblAutoFileTagging + kOblInteractiveFileTagging ) );	// support: AUTO/INTERACTIVE FILETAGGING this convert only specific save as other type

	m_mapActionWithObligation[kOA_CHANGEATTRIBUTES] =	NLSetObligationFlag( 0x00 );	// support: no obligation
	m_mapActionWithObligation[kOA_CLOSE]     = NLSetObligationFlag( 0x00 );	// support: no obligation
}

bool CNLObMgr::NLIsSupportTheObligation( _In_ const OfficeAction emAction, _In_ const ObligationType emObligation )
{
	map<OfficeAction, OblMap>::iterator itrAc = m_mapActionWithObligation.find( emAction );
	if ( itrAc != m_mapActionWithObligation.end() )
	{
		map<ObligationType,bool>::iterator itrOb = (itrAc->second.m_mapNLDoObligaitonFlag).find( emObligation );
		if ( itrOb != (itrAc->second.m_mapNLDoObligaitonFlag).end() )
		{
			NLPRINT_DEBUGLOG( L" action:[%s], obligation:[%s], support:[%s] \n", NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum(emAction).c_str(),
				NLOBLIGATIONTYPEINSTANCE.NLGetStringObligationTypeByEnum(emObligation).c_str(), itrOb->second ?  L"Yes" : L"No" );
			return itrOb->second;
		}
	}

	NLPRINT_DEBUGLOG( L" !!!!!Error: action:[%s], obligation:[%s] \n", NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum(emAction).c_str(),
		NLOBLIGATIONTYPEINSTANCE.NLGetStringObligationTypeByEnum(emObligation).c_str() );
	return false;
}

bool CNLObMgr::NLCheckIfObligationExist( _In_ nextlabs::Obligations& obs, _In_ const ObligationType& emObligation )
{
	bool bRet = false;
	wstring wstrObligationName = NLOBLIGATIONTYPEINSTANCE.NLGetStringObligationTypeByEnum( emObligation );
	const list<nextlabs::Obligation>& listOb = obs.GetObligations();
	for ( list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
	{
		if( 0 == _wcsicmp( wstrObligationName.c_str(), (*itr).name.c_str() ) )
		{	
			NLPRINT_DEBUGLOG( L"Local variables are: bRet=%d, wstrObligationName=%ls \n", bRet?L"TRUE":L"FALSE",wstrObligationName.c_str() );
			return true;
		}
	}
	return bRet;
}

void CNLObMgr::NLSetRealsePDocFlag( _In_ const wstring& wstrFilePath, _In_ const bool bNeedRealse )
{
	m_pNLData->NLSetRealsePDocFlag( wstrFilePath, bNeedRealse );
}

void CNLObMgr::NLSetFilePDocCache( _In_ const wstring& wstrFilePath,  IDispatch* pDoc )
{
	m_pNLData->NLSetFilePDocCache( wstrFilePath, pDoc );
}

CComPtr<IDispatch> CNLObMgr::NLGetFilePDocCache( _In_ const wstring& wstrFilePath )
{
	return m_pNLData->NLGetFilePDocCache( wstrFilePath );
}

// Current runtime status
void CNLObMgr::NLSetCurRibbonEvent( _In_ const wstring& wstrFilePath, _In_ const STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent )
{
	m_pNLData->NLSetCurRibbonEvent( wstrFilePath, stuCurRibbonEvent );
}

bool CNLObMgr::NLGetCurRibbonEvent( _In_ const wstring& wstrFilePath, _Out_ STUNLOFFICE_RIBBONEVENT& stuCurRibbonEvent )
{
	return m_pNLData->NLGetCurRibbonEvent( wstrFilePath, stuCurRibbonEvent );
}

void CNLObMgr::NLSetCurComNotify( _In_ const wstring& wstrFilePath, _In_ const NotifyEvent& stuCurComNotify )
{
	m_pNLData->NLSetCurComNotify( wstrFilePath, stuCurComNotify );
}

bool CNLObMgr::NLGetCurComNotify( _In_ const wstring& wstrFilePath, _Out_ NotifyEvent& stuCurComNotify )
{
	return m_pNLData->NLGetCurComNotify( wstrFilePath, stuCurComNotify );
}

// Close flag
void CNLObMgr::NLSetCloseFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIfNeedClose )
{
	m_pNLData->NLSetCloseFlag( wstrFilePath, bIfNeedClose );
}

bool CNLObMgr::NLGetCloseFlag( _In_ const wstring& wstrFilePath )
{
	return m_pNLData->NLGetCloseFlag( wstrFilePath );
}

// Current active file
void CNLObMgr::NLSetCurActiveFilePath( _In_ const wstring& wstrFilePath )
{
	// check file path	
	if ( wstrFilePath.empty() || isOpenInIEFlag( wstrFilePath ) )
	{
		// set an IE flag is error, need fix
		NLPRINT_DEBUGLOG( L"!!!!!!!!wrong path to set into the active file path \n" );
	}
	else
	{
		if ( NLIsHttpPath( wstrFilePath ) )
		{
			wstring wstrNLUnionNetFilePath = wstrFilePath;
			NLConvertFilePathBySepcifiedSeparator( wstrNLUnionNetFilePath );

			m_pNLData->NLSetCurActiveFilePath( wstrNLUnionNetFilePath );
		}
		else
		{
			m_pNLData->NLSetCurActiveFilePath( wstrFilePath );
		}
	}
}

wstring CNLObMgr::NLGetCurActiveFilePath(  )
{
	return m_pNLData->NLGetCurActiveFilePath();
}

// for bug24122, used for PPT right click to do print
void CNLObMgr::NLSetPPTPrintActiveFilePath( _In_ const wstring& wstrPPTPrintActiveFilePath )
{
	m_pNLData->NLSetPPTPrintActiveFilePath( wstrPPTPrintActiveFilePath );
}

wstring CNLObMgr::NLGetPPTPrintActiveFilePath(  )
{
	return m_pNLData->NLGetPPTPrintActiveFilePath();
}

// no need handle flag
void CNLObMgr::NLSetNoNeedHandleFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _In_ const bool& bNoNeedHandle  )
{
	m_pNLData->NLSetNoNeedHandleFlag( wstrFilePath, emAction, bNoNeedHandle );
}

bool CNLObMgr::NLGetNoNeedHandleFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction )
{
	return m_pNLData->NLGetNoNeedHandleFlag( wstrFilePath, emAction );
}

void CNLObMgr::NLSetActionStartUpFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction,
																			_In_ const bool& bActionStartUp,  _In_opt_ const ActionFlag* pstuOfficeActionFlag )
{
	m_pNLData->NLSetActionStartUpFlag( wstrFilePath, emAction, bActionStartUp, pstuOfficeActionFlag );
}

bool CNLObMgr::NLGetActionStartUpFlag( _In_ const wstring& wstrFilePath, _In_ const OfficeAction& emAction, _Out_opt_ ActionFlag* pstuOfficeActionFlag )
{
	return m_pNLData->NLGetActionStartUpFlag( wstrFilePath, emAction, pstuOfficeActionFlag );
}

// Encryption requirement flag
void CNLObMgr::NLSetEncryptRequirementFlag( _In_ const wstring& wstrFilePath, _In_ const bool bNeedEncrypt )
{
	m_pNLData->NLSetEncryptRequirementFlag( wstrFilePath, bNeedEncrypt );
}

bool CNLObMgr::NLGetEncryptRequirementFlag( _In_ const wstring& wstrFilePath )
{
	return m_pNLData->NLGetEncryptRequirementFlag( wstrFilePath );
}

void CNLObMgr::NLSetEncryptFileFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIsEncryptFile )
{
	m_pNLData->NLSetEncryptFileFlag( wstrFilePath, bIsEncryptFile );
}

bool CNLObMgr::NLGetEncryptFileFlag( _In_ const wstring& wstrFilePath )
{
	return m_pNLData->NLGetEncryptFileFlag( wstrFilePath );
}

void CNLObMgr::NLSetLinkFilePath( _In_ const wstring& wstrHyperLinkFilePath )
{
	m_pNLData->NLSetLinkFilePath( wstrHyperLinkFilePath );
}

wstring CNLObMgr::NLGetLinkFilePath( )
{
	return m_pNLData->NLGetLinkFilePath();
}

void CNLObMgr::NLClearFileCache( _In_ const wstring& wstrFilePath )
{
	m_pNLData->NLClearFileCache( wstrFilePath );
	m_pLogicFlag->NLClearFileCache( wstrFilePath );
	NLSetObMgrInitializeFlag( wstrFilePath, false );
}

void CNLObMgr::NLClearCache( void )
{
	m_pNLData->NLClearCache();
	m_pLogicFlag->NLClearCache();
	NLRevertObMgrInitializeFlag();
}

// Clear golden tag cache
void CNLObMgr::NLClearGoldenTagFileCache( _In_ const wstring& wstrFilePath )
{
	m_pNLData->NLClearGoldenTagFileCache( wstrFilePath );
}

void CNLObMgr::NLClearGoldenTagCache( void )
{
	m_pNLData->NLClearGoldenTagCache();
}

void CNLObMgr::NLSetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _In_ const bool bSaveAsFlag, _In_ const wstring& wstrSrcFilePath )
{
	m_pLogicFlag->NLSetSaveAsFlag( wstrDesFilePath, bSaveAsFlag, wstrSrcFilePath );
}

bool CNLObMgr::NLGetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _Out_ wstring& wstrSrcFilePath )
{
	return  m_pLogicFlag->NLGetSaveAsFlag( wstrDesFilePath, wstrSrcFilePath );
}

// Open in IE, file path cache
void CNLObMgr::NLSetIEFilePathCache( _In_ const IDispatch* pDoc, _In_ const wstring& wstrIEFilePath )
{
	if ( NLIsHttpPath( wstrIEFilePath ) )
	{
		wstring wstrNLUnionNetFilePath = wstrIEFilePath;
		NLConvertFilePathBySepcifiedSeparator( wstrNLUnionNetFilePath );

		m_pLogicFlag->NLSetIEFilePathCache( pDoc, wstrNLUnionNetFilePath );
	}
	else
	{
		m_pLogicFlag->NLSetIEFilePathCache( pDoc, wstrIEFilePath );
	}
}

wstring CNLObMgr::NLGetIEFilePathCache( _In_ const IDispatch* pDoc )
{
	return m_pLogicFlag->NLGetIEFilePathCache( pDoc );
}

// office user classify custom flag
void CNLObMgr::NLSetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bUserClassifyCustomTags )
{
	m_pLogicFlag->NLSetClassifyCustomTagsFlag( wstrFilePath, bUserClassifyCustomTags );
}

bool CNLObMgr::NLGetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath )
{
	return m_pLogicFlag->NLGetClassifyCustomTagsFlag( wstrFilePath );
}

// Word open in IE flag
void CNLObMgr::NLSetIEOpenFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIEOpenFlag )
{
	m_pLogicFlag->NLSetIEOpenFlag( wstrFilePath, bIEOpenFlag );
}

bool CNLObMgr::NLGetIEOpenFlag( _In_ const wstring& wstrFilePath )
{
	return m_pLogicFlag->NLGetIEOpenFlag( wstrFilePath );
}

void CNLObMgr::SetPasteEvaResult(const wstring& strPath,bool bAllow)
{
	EnterCriticalSection(&m_csmap);
	map<wstring,bool>::iterator it = m_mapPasteResult.find(strPath);
	if(it == m_mapPasteResult.end())
	{
		m_mapPasteResult[strPath] = bAllow;
	}
	LeaveCriticalSection(&m_csmap);
}

bool CNLObMgr::GetPasteEvaResult(const wstring& strPath,bool& bAllow)
{
	bool bGet = false;
	EnterCriticalSection(&m_csmap);
	map<wstring,bool>::iterator it = m_mapPasteResult.find(strPath);
	if(it != m_mapPasteResult.end())
	{
		bAllow = (*it).second;
		bGet = true;
	}
	LeaveCriticalSection(&m_csmap);
	return bGet;
}

void CNLObMgr::NLSetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIsPPTUserCancelClsoe )
{
	m_pLogicFlag->NLSetPPTUserCancelCloseFlag( wstrFilePath, bIsPPTUserCancelClsoe );
}

bool CNLObMgr::NLGetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath )
{
	return m_pLogicFlag->NLGetPPTUserCancelCloseFlag( wstrFilePath );
}

void CNLObMgr::NLSetUserSaveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIsUserSave )
{
	m_pLogicFlag->NLSetUserSaveFlag( wstrFilePath, bIsUserSave );
}

bool CNLObMgr::NLGetUserSaveFlag( _In_ const wstring& wstrFilePath )
{
	return m_pLogicFlag->NLGetUserSaveFlag( wstrFilePath );
}
