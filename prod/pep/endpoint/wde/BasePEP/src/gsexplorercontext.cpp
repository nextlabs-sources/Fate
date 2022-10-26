#include "gsexplorercontext.h"

#include <windows.h>
#include <Shlwapi.h>
#include <Shellapi.h>

#pragma warning(push)
#pragma warning(disable: 6387 6011) 
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning(pop)

#include <commonutils.hpp> 

#pragma warning(push)
#pragma warning(disable: 4819)  // ignored character code page issue  
#include <madCHook.h>
#pragma warning(pop)

#include "eventhander.h"
#include "eventparser.h"

extern nextlabs::CRuntimeContext* gContext_;

namespace
{

#define BEFORECODEBLOCK_BOOL(rt) \
	{ \
	switch (rt) \
		{ \
	case kEventAllow: \
	break; \
	case kEventDeny: \
	return FALSE; \
	case kEventReturnDirectly: \
	return TRUE; \
	default: \
	break; \
		} \
	} 

#define BEFORECODEBLOCK_HANDLE(rt) \
	{ \
	switch (rt) \
	{ \
	case kEventAllow: \
	break; \
	case kEventDeny: \
	::SetLastError(ERROR_ACCESS_DENIED); \
	return INVALID_HANDLE_VALUE; \
	case kEventReturnDirectly: \
	return INVALID_HANDLE_VALUE; \
	default: \
	break; \
		} \
	}

	const unsigned int kCOMSetData = 7;

	typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
	typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 

	boost::shared_mutex gMutex; 
}

namespace nextlabs
{

std::map<LPVOID, LPVOID> CGSExplorerContext::mapCOMHooks_;

/************************************************************************/
/* CDllHostContext                                                */
/************************************************************************/
CGSExplorerContext::CGSExplorerContext():CGenericContext(), bBeginDeleteOnPaste(FALSE), bMoveFileDenied(FALSE) 
{
	WCHAR AppDataPath[MAX_PATH] = { 0 };
	SHGetSpecialFolderPathW(NULL, AppDataPath, CSIDL_APPDATA, FALSE);
	wstrTempPath = AppDataPath;
	wstrTempPath += L"\\GoodSync\\";
	wstrTempPathPrefix = L"\\\\?\\";
	wstrTempPathPrefix += AppDataPath;
	wstrTempPathPrefix += L"\\GoodSync\\";
}

HANDLE CGSExplorerContext::MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (!lpFileName || nextlabs::utils::CanIgnoreFile(lpFileName))
	{
		return Hooked_CreateFileW_Next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	if (boost::algorithm::starts_with(lpFileName, wstrTempPathPrefix) || boost::algorithm::starts_with(lpFileName, wstrTempPath))
	{
		return Hooked_CreateFileW_Next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	//
	// notify before
	//
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_HANDLE(OnBeforeCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, &obligations));
	// real
	//
	HANDLE hrt = Hooked_CreateFileW_Next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	//
	// after
	//
	EventResult ret = OnAfterCreateFileW(hrt, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, &obligations);
	switch (ret)
	{
	case kEventAllow:
		{
			if (INVALID_HANDLE_VALUE != hrt)
			{
				eventParser_->StoreFileHandleWithPath(hrt, lpFileName);
				eventParser_->AddOpenedFile(lpFileName);
			}
		}
		break;
	case kEventDeny:
		{
			return INVALID_HANDLE_VALUE;
		}
		break;
	default:
		break;
	}

	saveAsObligation->DoCreateFileW ( hrt, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );

	return hrt;
}

HRESULT CGSExplorerContext::MyOleGetClipboard(LPDATAOBJECT *ppDataObj)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	bBeginDeleteOnPaste = false;
	bMoveFileDenied = false;

	HRESULT hr = Hooked_OleGetClipboard_Next(ppDataObj);
	if (SUCCEEDED(hr))
	{
		EventResult result = OnAfterOleGetClipboard(ppDataObj);
		if (kEventAllow != result)
		{
			*ppDataObj = NULL;
			return E_FAIL;
		} 
		else
		{
			FORMATETC formatetc = { 0 };
			formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);
			formatetc.dwAspect = DVASPECT_CONTENT;
			formatetc.lindex = -1;
			formatetc.tymed = TYMED_HGLOBAL;

			STGMEDIUM medium = { 0 };

			if (SUCCEEDED((*ppDataObj)->GetData(&formatetc, &medium)))
			{
				int* pValue = (int*)GlobalLock(medium.hGlobal);
				if (pValue != NULL)
				{
					if (*pValue == DROPEFFECT_MOVE)
					{
						bBeginDeleteOnPaste = true;
					}
					GlobalUnlock(medium.hGlobal);
				}

				ReleaseStgMedium(&medium);
			}

			if (bBeginDeleteOnPaste)
			{
				HookIDataObjectSetData(*ppDataObj);
			}

			return hr;
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CGSExplorerContext::Hooked_SetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
	if (!gContext_)
	{
		return E_FAIL;
	}

	PF_COMSetData next_func = NULL;
	{
		LPVOID* pVTable = (*(LPVOID**)pThis);
		LPVOID pSetData = pVTable[kCOMSetData];

		boost_share_lock lockReader(gMutex);
		std::map<LPVOID, LPVOID>::iterator iter = mapCOMHooks_.find(pSetData);

		if(iter == mapCOMHooks_.end())
		{
			return E_FAIL;
		}

		next_func = (PF_COMSetData)(*iter).second;
	}

	if (hook_control.is_disabled())
	{
		return next_func(pThis, pformatetc, pmedium, fRelease);
	}

	return gContext_->MyCOMSetData(pThis, pformatetc, pmedium, fRelease, next_func);
}

BOOL CGSExplorerContext::HookIDataObjectSetData(LPDATAOBJECT pDataObj)
{
	LPVOID* pVTable = (*(LPVOID**)pDataObj);

	boost_unique_lock lockWriter(gMutex);
	{
		LPVOID pSetData = pVTable[kCOMSetData];

		if (mapCOMHooks_.find(pSetData) == mapCOMHooks_.end())
		{
			LPVOID* pnext_SetData = new LPVOID();
			
			if(HookCode((LPVOID)pSetData,(PVOID)CGSExplorerContext::Hooked_SetData,(LPVOID*)pnext_SetData) && *pnext_SetData)
			{
				mapCOMHooks_[pSetData] = *pnext_SetData;
			}
		}
	}

	return TRUE;
}

HRESULT CGSExplorerContext::MyCOMSetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease, PF_COMSetData nextFunc)
{
	bool bInDeleteOnPaste = bBeginDeleteOnPaste;
	bool bDenied = bMoveFileDenied;

	bBeginDeleteOnPaste = false;
	bMoveFileDenied = false;

	if (bInDeleteOnPaste && bDenied)
	{
		WCHAR wzFormat[MAX_PATH] = { 0 };
		GetClipboardFormatNameW(pformatetc->cfFormat, wzFormat, MAX_PATH);
		if(boost::algorithm::equals(wzFormat, CFSTR_PASTESUCCEEDED))
		{
			int* pValue = (int*)GlobalLock(pmedium->hGlobal);
			if (pValue != NULL)
			{
				if (*pValue == DROPEFFECT_MOVE)
				{
					GlobalUnlock(pmedium->hGlobal);
					return S_OK;
				}
				GlobalUnlock(pmedium->hGlobal);
			}
		}
	}

	return nextFunc(pThis, pformatetc, pmedium, fRelease);
}

BOOL CGSExplorerContext::MyMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);

	nextlabs::Obligations obligations;

	EventResult rt = OnBeforeMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations);

	switch (rt)
	{
	case kEventDeny:
		return FALSE;
	case kEventReturnDirectly:
		bMoveFileDenied = true;
		return TRUE;
	default:
		break;
	} 

	BOOL bRet = Hooked_MoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);

	if (bRet)
	{
		return OnAfterMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations);
	}
	return bRet;
}

EventResult CGSExplorerContext::OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
	LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData)
{
	return this->EventAfterCreateFile(hFile, lpFileName, pUserData);
}

EventResult CGSExplorerContext::OnAfterOleGetClipboard(LPDATAOBJECT *ppDataObj)
{
	std::wstring srcFilePath = L"";
	std::wstring dstFilePath = L"";
	if (eventParser_->isPasteContent(*ppDataObj, dstFilePath, srcFilePath))
	{
		return this->EventBeforePasteContent(srcFilePath, dstFilePath);
	}

	return kEventAllow;
}

EventResult CGSExplorerContext::OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
	LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData)
{
	if (eventParser_->IsDeleteAction(dwFlagsAndAttributes))
	{
		std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
		vecFiles.push_back(std::make_pair(lpFileName, ob));
		return this->EventBeforeDeleteFiles(vecFiles);
	}
	else if (eventParser_->IsNewFileAction(lpFileName, dwCreationDisposition))
	{
		return this->EventBeforeNewFile(lpFileName, pUserData);
	}
	else if (eventParser_->IsFileOpen(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes))
	{
		BOOL bAllowed = TRUE;
		if (eventParser_->GetPolicyCacheResult(lpFileName, bAllowed))
		{
			if (bAllowed)
			{
				return kEventAllow;
			}
			else
			{
				return kEventDeny;
			}
		}

		EventResult er = this->EventBeforeFileOpen(lpFileName, pUserData);

		if (er == kEventAllow)
		{
			eventParser_->StorePolicyCacheResult(lpFileName, true);
		}
		else
		{
			eventParser_->StorePolicyCacheResult(lpFileName, false);
		}

		return er;
	}
	else if (eventParser_->IsDirectoryOpen(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes))
	{
		BOOL bAllowed = TRUE;
		if (eventParser_->GetPolicyCacheResult(lpFileName, bAllowed))
		{
			if (bAllowed)
			{
				return kEventAllow;
			}
			else
			{
				return kEventDeny;
			}
		}

		EventResult er = this->EventBeforeFolderOpen(lpFileName, pUserData);

		if (er == kEventAllow)
		{
			eventParser_->StorePolicyCacheResult(lpFileName, true);
		}
		else
		{
			eventParser_->StorePolicyCacheResult(lpFileName, false);
		}

		return er;
	}


	return this->EventBeforeCreateFile(lpFileName, pUserData);
}

EventResult CGSExplorerContext::EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());

	//return kEventDeny;
	EventResult rt = handler->HandleDeleteAction(vecFiles);

	return rt;
}

EventResult CGSExplorerContext::EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());

	EventResult rt = handler->HandleNewFileAction(strPath, pUserData);

	return rt;
}

EventResult CGSExplorerContext::EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData)
{
	return kEventAllow;
}

EventResult CGSExplorerContext::EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData)
{
	return kEventAllow;
}

EventResult CGSExplorerContext::EventBeforePasteContent(const std::wstring& strFileSrc, std::wstring& strFileDst)
{
	boost::scoped_ptr<CEventHander> handle(new CEventHander());
	EventResult rt = handle->HandlePasteContentAction(strFileSrc, strFileDst);
	return rt;
}

EventResult CGSExplorerContext::OnAfterMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CGSExplorerContext::OnBeforeMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
	if (eventParser_->IsDeleteAction(lpNewFileName))
	{
		std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
		return this->EventBeforeDeleteFiles(vecFiles);
	}
	else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
	{
		std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
		vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
		EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
	}
	else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName, dwFlags))
	{
		std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
		nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
		EventResult er = this->EventBeforeMoveFiles(vecFileOP);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
	} 
	else
	{
		return kEventAllow;
	}
}

EventResult CGSExplorerContext::EventAfterMoveFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData)
{
	if (pUserData != NULL)
	{
		nextlabs::Obligations* pob = (nextlabs::Obligations*)pUserData;
		if (pob->IsObligationSet(WDE_OBLIGATION_ENCRYPT_NAME))
		{
			HandleEFSObligation(lpNewFileName);
		}
	}

	return kEventAllow;
}

BOOL CGSExplorerContext::HandleEFSObligation(LPCWSTR lpFileName)
{
	BOOL bRet = TRUE;

	DWORD le = GetLastError();
	if (Hooked_EncryptFileW_Next == NULL)
	{
		bRet = EncryptFileW(lpFileName);
	}
	else
	{
		bRet = Hooked_EncryptFileW_Next(lpFileName);
	}
	SetLastError(le);

	return bRet;
}

EventResult CGSExplorerContext::EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());

	EventResult rt = handler->HandleRenameAction(vecFiles, strNewName);

	return rt;
}

EventResult CGSExplorerContext::EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	EventResult rt = handler->HandleMoveAction(vecFileOp);

	return rt;
}

}  // ns nextlabs
