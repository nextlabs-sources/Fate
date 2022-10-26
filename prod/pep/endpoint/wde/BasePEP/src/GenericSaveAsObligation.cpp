#include "GenericSaveAsObligation.h"

#include <boost\algorithm\string.hpp>

namespace nextlabs
{

//////////////////////////////////////////////////////////////////////////
//GenericSaveAsObligation

GenericSaveAsObligation::CreateAttributeManagerType GenericSaveAsObligation::lfCreateAttributeManager = NULL;
GenericSaveAsObligation::AllocAttributesType GenericSaveAsObligation::lfAllocAttributes = NULL;
GenericSaveAsObligation::ReadResourceAttributesWType GenericSaveAsObligation::lfReadResourceAttributesW = NULL;
GenericSaveAsObligation::GetAttributeCountType GenericSaveAsObligation::lfGetAttributeCount = NULL;
GenericSaveAsObligation::FreeAttributesType GenericSaveAsObligation::lfFreeAttributes = NULL;
GenericSaveAsObligation::CloseAttributeManagerType GenericSaveAsObligation::lfCloseAttributeManager = NULL;
GenericSaveAsObligation::AddAttributeWType GenericSaveAsObligation::lfAddAttributeW = NULL;
GenericSaveAsObligation::GetAttributeNameType GenericSaveAsObligation::lfGetAttributeName = NULL;
GenericSaveAsObligation::GetAttributeValueType GenericSaveAsObligation::lfGetAttributeValue = NULL;
GenericSaveAsObligation::WriteResourceAttributesWType GenericSaveAsObligation::lfWriteResourceAttributesW = NULL;

GenericSaveAsObligation::GenericSaveAsObligation ( ) : bValid ( FALSE ), dwThreadID ( 0 )
{
	InitializeCriticalSection ( &CriticalSection );
}

GenericSaveAsObligation::~GenericSaveAsObligation ( )
{
	DeleteCriticalSection ( &CriticalSection );
}

GenericSaveAsObligation* GenericSaveAsObligation::GetInstance ()
{
	WCHAR ProcessPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, ProcessPath, MAX_PATH);
	
	if (boost::algorithm::iends_with(ProcessPath, L"\\explorer.exe") || boost::algorithm::iends_with(ProcessPath, L"\\explorer.exe.mui"))
	{
		return new NullSaveAsObligation;
	}
	else if (boost::algorithm::iends_with(ProcessPath, L"\\mspaint.exe") || boost::algorithm::iends_with(ProcessPath, L"\\mspaint.exe.mui"))
	{
		return new MSPaintSaveAsObligation;
	}
	else if (boost::algorithm::iends_with(ProcessPath, L"\\chrome.exe"))
	{
		return new ChromeSaveAsObligation;
	}
	else
	{
		return new CommonSaveAsObligation;
	}
}

BOOL GenericSaveAsObligation::SetValid ( BOOL bStatus )
{
	EnterCriticalSection ( &CriticalSection );

	bValid = bStatus;
	
	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

bool GenericSaveAsObligation::InitResattr()
{
	static bool bInit = false;
	if(!bInit)
	{
		std::wstring strCommonPath = utils::GetCommonComponentsDir();
		if(strCommonPath.empty())
		{
			return false;
		}
		else
		{
			SetDllDirectoryW(strCommonPath.c_str());

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
				return false;
			}

			bInit = true;
			return true;
		}
	}

	return bInit;
}

BOOL GenericSaveAsObligation::GetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags )
{
	if ( NULL == lpszFilePath || !InitResattr ( ) )
	{
		return FALSE;
	}

	mapTags.clear ( );

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

BOOL GenericSaveAsObligation::SetFileTags ( LPCWSTR lpszFilePath, std::map<std::wstring, std::wstring>& mapTags )
{
	if ( NULL == lpszFilePath || !InitResattr ( ) )
	{
		return FALSE;
	}

	ResourceAttributeManager *mgr = NULL;
	lfCreateAttributeManager(&mgr);

	if(!mgr)
		return FALSE;

	ResourceAttributes *attrs;
	lfAllocAttributes(&attrs);

	int nRet = 0;
	if(attrs)
	{
		std::map<std::wstring, std::wstring>::iterator itr;
		for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
		{
			lfAddAttributeW(attrs, (*itr).first.c_str(), (*itr).second.c_str());
		}

		nRet = lfWriteResourceAttributesW(mgr, lpszFilePath, attrs);

		lfFreeAttributes(attrs);
	}

	if(mgr)
	{
		lfCloseAttributeManager(mgr);
		mgr = NULL;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//NullSaveAsObligation

NullSaveAsObligation::NullSaveAsObligation ( )
{

}

NullSaveAsObligation::~NullSaveAsObligation ( )
{

}

BOOL NullSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn )
{
	return TRUE;
}

BOOL NullSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog )
{
	return TRUE;
}

BOOL NullSaveAsObligation::DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	return TRUE;
}

BOOL NullSaveAsObligation::DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	return TRUE;
}

BOOL NullSaveAsObligation::DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	return TRUE;
}

BOOL NullSaveAsObligation::DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CommonSaveAsObligation

CommonSaveAsObligation::CommonSaveAsObligation ( )
{

}

CommonSaveAsObligation::~CommonSaveAsObligation ( )
{

}

BOOL CommonSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn )
{
	if ( NULL == lpofn )
	{
		return FALSE;
	}
	
	DWORD ThreadID = GetCurrentThreadId ( );

	EnterCriticalSection ( &CriticalSection );

	GetFileTags ( SrcFileName.c_str ( ), SourceFileTags );

	SourceFile = SrcFileName;
	DestinationFile = DstFileName;
	this->obs = obs;
	bValid = TRUE;
	dwThreadID = ThreadID;

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

BOOL CommonSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog )
{
	if ( NULL == pIFileSaveDialog )
	{
		return FALSE;
	}
	
	DWORD ThreadID = GetCurrentThreadId ( );

	EnterCriticalSection ( &CriticalSection );

	GetFileTags ( SrcFileName.c_str ( ), SourceFileTags );
	
	SourceFile = SrcFileName;
	DestinationFile = DstFileName;
	this->obs = obs;
	bValid = TRUE;
	dwThreadID = ThreadID;

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

BOOL CommonSaveAsObligation::DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	if ( OPEN_ALWAYS == dwCreationDisposition || CREATE_ALWAYS == dwCreationDisposition )
	{
		EnterCriticalSection ( &CriticalSection );

		if ( bValid && dwThreadID == GetCurrentThreadId ( ) )
		{
			bValid = FALSE;

			WCHAR FileName[MAX_PATH] = { 0 };
			utils::BJAnsiToUnicode ( lpFileName, FileName, MAX_PATH );

			if ( INVALID_HANDLE_VALUE != hFile && 0 == _wcsicmp ( DestinationFile.c_str ( ), FileName ) )
			{
				CloseHandle(hFile);

				SetFileTags ( FileName, SourceFileTags );

				nextlabs::policyassistant::PAHelper(PABase::AT_SAVEAS, obs, SourceFile, DestinationFile, ERROR_ACCESS_DENIED, true);

				hFile = CreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes, OPEN_ALWAYS,dwFlagsAndAttributes,hTemplateFile);
			}
		}

		LeaveCriticalSection ( &CriticalSection );
	}

	return TRUE;
}

BOOL CommonSaveAsObligation::DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{\
	if ( OPEN_ALWAYS == dwCreationDisposition || CREATE_ALWAYS == dwCreationDisposition )
	{
		EnterCriticalSection ( &CriticalSection );

		if ( bValid && dwThreadID == GetCurrentThreadId ( ) )
		{
			bValid = FALSE;
		
			if ( INVALID_HANDLE_VALUE != hFile && 0 == _wcsicmp ( DestinationFile.c_str ( ), lpFileName ) )
			{
				CloseHandle(hFile);

				SetFileTags ( lpFileName, SourceFileTags );

				nextlabs::policyassistant::PAHelper(PABase::AT_SAVEAS, obs, SourceFile, DestinationFile, ERROR_ACCESS_DENIED, true);
								
				hFile = CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes, OPEN_ALWAYS,dwFlagsAndAttributes,hTemplateFile);
			}
		}

		LeaveCriticalSection ( &CriticalSection );
	}

	return TRUE;
}

BOOL CommonSaveAsObligation::DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	return TRUE;
}

BOOL CommonSaveAsObligation::DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//MSPaintSaveAsObligation

MSPaintSaveAsObligation::MSPaintSaveAsObligation ( ) : OSVersion ( 0 ), bCreateFile ( FALSE ), bCopyFile ( FALSE )
{
	OSVersion = 1;
}

MSPaintSaveAsObligation::~MSPaintSaveAsObligation ( )
{

}

BOOL MSPaintSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, const LPOPENFILENAMEW lpofn )
{
	if ( 0 == OSVersion )
	{
		return CommonSaveAsObligation::Prepare ( SrcFileName, DstFileName, obs, lpofn );
	}

	if ( NULL == lpofn )
	{
		return FALSE;
	}

	DWORD ThreadID = GetCurrentThreadId ( );

	EnterCriticalSection ( &CriticalSection );

	GetFileTags ( SrcFileName.c_str ( ), SourceFileTags );

	SourceFile = SrcFileName;
	DestinationFile = DstFileName;
	this->obs = obs;
	bValid = TRUE;
	dwThreadID = ThreadID;

	switch ( lpofn->nFilterIndex )
	{
	case 1:
	case 2:
	case 3:
	case 4:
		   {
			   bCreateFile = TRUE;
			   bCopyFile = FALSE;
			   break;
		   }
	default:
		{
			bCreateFile = FALSE;
			bCopyFile = TRUE;
			break;
		}
	}

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

BOOL MSPaintSaveAsObligation::Prepare ( const std::wstring& SrcFileName, const std::wstring& DstFileName, const nextlabs::Obligations& obs, void* pIFileSaveDialog )
{
	if ( 0 == OSVersion )
	{
		return CommonSaveAsObligation::Prepare ( SrcFileName, DstFileName, obs, pIFileSaveDialog );
	}

	if ( NULL == pIFileSaveDialog )
	{
		return FALSE;
	}

	DWORD ThreadID = GetCurrentThreadId ( );

	EnterCriticalSection ( &CriticalSection );

	GetFileTags ( SrcFileName.c_str ( ), SourceFileTags );

	SourceFile = SrcFileName;
	DestinationFile = DstFileName;
	this->obs = obs;
	bValid = TRUE;
	dwThreadID = ThreadID;

	IFileSaveDialog* p = (IFileSaveDialog*)pIFileSaveDialog;

	UINT uIndex = 1;
	p->GetFileTypeIndex(&uIndex);

	switch ( uIndex )
	{
	case 1:
	case 2:
	case 3:
	case 4:
		{
			bCreateFile = TRUE;
			bCopyFile = FALSE;
			break;
		}
	default:
		{
			bCreateFile = FALSE;
			bCopyFile = TRUE;
			break;
		}
	}

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

BOOL MSPaintSaveAsObligation::DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	if ( 0 == OSVersion )
	{
		return CommonSaveAsObligation::DoCreateFileW ( hFile, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	}

	if ( OPEN_ALWAYS == dwCreationDisposition || CREATE_ALWAYS == dwCreationDisposition )
	{
		EnterCriticalSection ( &CriticalSection );

		if ( bValid && bCreateFile && dwThreadID == GetCurrentThreadId ( ) )
		{
			bValid = FALSE;
		
			if ( INVALID_HANDLE_VALUE!= hFile && 0 == _wcsicmp ( DestinationFile.c_str ( ), lpFileName ) )
			{
				CloseHandle(hFile);

				SetFileTags ( lpFileName, SourceFileTags );

				nextlabs::policyassistant::PAHelper(PABase::AT_SAVEAS, obs, SourceFile, DestinationFile, ERROR_ACCESS_DENIED, true);
				
				hFile = CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes, OPEN_ALWAYS,dwFlagsAndAttributes,hTemplateFile);
			}
		}

		LeaveCriticalSection ( &CriticalSection );
	}

	return TRUE;
}

BOOL MSPaintSaveAsObligation::DoCopyFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	if ( 0 == OSVersion )
	{
		return CommonSaveAsObligation::DoCopyFileW ( bRet, lpExistingFileName, lpNewFileName );
	}

	EnterCriticalSection ( &CriticalSection );

	if ( bValid && bCopyFile && dwThreadID == GetCurrentThreadId ( ) )
	{
		bValid = FALSE;
		
		if ( NULL != lpNewFileName && bRet )
		{
			if ( 0 == _wcsicmp ( DestinationFile.c_str ( ), lpNewFileName ) )
			{
				SetFileTags ( lpNewFileName, SourceFileTags );

				nextlabs::policyassistant::PAHelper(PABase::AT_SAVEAS, obs, SourceFile, DestinationFile, ERROR_ACCESS_DENIED, true);
			}
		}
	}

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//ChromeSaveAsObligation

ChromeSaveAsObligation::ChromeSaveAsObligation ( )
{

}

ChromeSaveAsObligation::~ChromeSaveAsObligation ( )
{
	
}

BOOL ChromeSaveAsObligation::DoCreateFileA ( HANDLE& hFile, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	return TRUE;
}

BOOL ChromeSaveAsObligation::DoCreateFileW ( HANDLE& hFile, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	return TRUE;
}

BOOL ChromeSaveAsObligation::DoMoveFileW ( BOOL bRet, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName )
{
	EnterCriticalSection ( &CriticalSection );

	if ( bValid )
	{
		bValid = FALSE;

		if ( NULL != lpNewFileName && bRet )
		{
			if ( 0 == _wcsicmp ( DestinationFile.c_str ( ), lpNewFileName ) )
			{
				SetFileTags ( lpNewFileName, SourceFileTags );

				nextlabs::policyassistant::PAHelper(PABase::AT_SAVEAS, obs, SourceFile, DestinationFile, ERROR_ACCESS_DENIED, true);
			}
		}
	}

	LeaveCriticalSection ( &CriticalSection );

	return TRUE;
}

}