#include "StdAfx.h"
#include "celog.h"
#include "NLLogicFlag.h"
////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLLOGICFLAG)
//////////////////////////////////////////////////////////////////////////

typedef boost::shared_lock<rwmutex> readLock; 
typedef boost::unique_lock<rwmutex> writeLock; 

CNLLogicFlag::CNLLogicFlag(void)
{
}

CNLLogicFlag::~CNLLogicFlag(void)
{
}

void CNLLogicFlag::NLSetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath, _In_ const bool bIsPPTUserCancelClsoe )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
	stuLogicFlag.bIsPPTUserCancelClsoe = bIsPPTUserCancelClsoe;
}

bool CNLLogicFlag::NLGetPPTUserCancelCloseFlag( _In_ const wstring& wstrFilePath )
{
	bool bIsPPTUserCancelClsoe = false;
	readLock( m_rwMutexLogicFlagCache );

	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		bIsPPTUserCancelClsoe = stuLogicFlag.bIsPPTUserCancelClsoe;
	}
	return bIsPPTUserCancelClsoe;
}

void CNLLogicFlag::NLSetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _In_ const bool bSaveAsFlag, _In_ const wstring& wstrSrcFilePath )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrDesFilePath];
	stuLogicFlag.pairSaveAsFlag.first = bSaveAsFlag;
	stuLogicFlag.pairSaveAsFlag.second = wstrSrcFilePath;
}

bool CNLLogicFlag::NLGetSaveAsFlag( _In_ const wstring& wstrDesFilePath, _Out_ wstring& wstrSrcFilePath )
{
	wstrSrcFilePath.clear();
	bool bRet = false;
	readLock( m_rwMutexLogicFlagCache );
	
	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrDesFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		if ( stuLogicFlag.pairSaveAsFlag.first )
		{
			bRet = true;
			wstrSrcFilePath = stuLogicFlag.pairSaveAsFlag.second;
		}
	}
	return bRet;
}

void CNLLogicFlag::NLSetButtonSaveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bButtonSave )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
	stuLogicFlag.bButtonSave = bButtonSave;
}

bool CNLLogicFlag::NLGetButtonSaveFlag( _In_ const wstring& wstrFilePath )
{
	bool bButtonSave = false;
	readLock( m_rwMutexLogicFlagCache );
	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		bButtonSave = stuLogicFlag.bButtonSave;
	}
	return bButtonSave;
}

void CNLLogicFlag::NLSetIEOpenFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIEOpenFlag )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
	stuLogicFlag.bIEOpen = bIEOpenFlag;
}

bool CNLLogicFlag::NLGetIEOpenFlag( _In_ const wstring& wstrFilePath )
{
	bool bIEOpenFlag = false;
	readLock( m_rwMutexLogicFlagCache );
	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		bIEOpenFlag = stuLogicFlag.bIEOpen;
	}
	return bIEOpenFlag;
}

void CNLLogicFlag::NLSetUserSaveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bIsUserSave )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
	stuLogicFlag.bIsUserSave = bIsUserSave;
}

bool CNLLogicFlag::NLGetUserSaveFlag( _In_ const wstring& wstrFilePath )
{
	bool bIsUserSave = false;
	readLock( m_rwMutexLogicFlagCache );
	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		bIsUserSave = stuLogicFlag.bIsUserSave;
	}
	return bIsUserSave;
}

void CNLLogicFlag::NLSetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bUserClassifyCustomTagsFlag )
{
	writeLock( m_rwMutexLogicFlagCache );
	STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = m_mapLogicFlagCache[wstrFilePath];
	stuLogicFlag.bUserClassifyCustomTags = bUserClassifyCustomTagsFlag;
}

bool CNLLogicFlag::NLGetClassifyCustomTagsFlag( _In_ const wstring& wstrFilePath )
{
	bool bUserClassifyCustomTagsFlag = false;
	readLock( m_rwMutexLogicFlagCache );
	map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
	if ( itr != m_mapLogicFlagCache.end() )
	{
		STUNLOFFICE_LOGICFLAGCACHE& stuLogicFlag = itr->second;
		bUserClassifyCustomTagsFlag = stuLogicFlag.bUserClassifyCustomTags;
	}
	return bUserClassifyCustomTagsFlag;
}

// office open  in IE, file path cache
void CNLLogicFlag::NLSetIEFilePathCache( _In_ const IDispatch* pDoc, _In_ const wstring& wstrIEFilePath )
{
	writeLock( m_rwMutexIEFilePathCache );
	m_mapIEFilePath[pDoc] = wstrIEFilePath;
}

wstring CNLLogicFlag::NLGetIEFilePathCache( _In_ const IDispatch* pDoc )
{
	wstring wstrIEFilePath;
	readLock( m_rwMutexIEFilePathCache );
	map<const IDispatch*, wstring>::iterator itr = m_mapIEFilePath.find( pDoc );
	if ( itr != m_mapIEFilePath.end() )
	{
		wstrIEFilePath = itr->second;
	}
	return wstrIEFilePath;
}

void CNLLogicFlag::NLClearFileCache( _In_ const wstring& wstrFilePath )
{
	{
		// clear logic cache
		writeLock( m_rwMutexLogicFlagCache );
		map<wstring, STUNLOFFICE_LOGICFLAGCACHE>::iterator itr = m_mapLogicFlagCache.find( wstrFilePath );
		if ( itr != m_mapLogicFlagCache.end() )
		{
			m_mapLogicFlagCache.erase( itr );
		}
	}

	{
		// clear IE file path cache
		writeLock( m_rwMutexIEFilePathCache );
		map<const IDispatch*, wstring>::iterator itrIE = m_mapIEFilePath.begin();
		for ( ; itrIE != m_mapIEFilePath.end();)
		{
			if ( wstrFilePath == itrIE->second )
			{
				m_mapIEFilePath.erase( itrIE++ );
			}
			else
				itrIE++;
		}
	}
}

void CNLLogicFlag::NLClearCache( )
{
	{
		// clear logic cache
		writeLock( m_rwMutexLogicFlagCache );
		m_mapLogicFlagCache.clear();
	}

	{
		// clear IE file path cache
		writeLock( m_rwMutexIEFilePathCache );
		m_mapIEFilePath.clear();
	}
}
