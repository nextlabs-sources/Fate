#include "genericcontext.h"

#include "eventhander.h"

namespace nextlabs
{
    void CGenericContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
    {
        table[kHMDeleteFileW] = true;
        table[kHMCreateFileMappintW] = true;
        table[kHMNtCreateFile] = true;
        table[kHMFindFirstFileExW] = true;
		table[kHMNtOpenFile] = true;
        table[kHMNtClose] = true;
		table[kHMMoveFileW] = true;
		table[kHMMoveFileExW] = true;
        table[kHMMoveFileWithProgressW] = true;
        table[kHMKernelBaseMoveFileExW] = true;
        table[kHMKernelBaseMoveFileWithProgressW] = true;
        table[kHMCopyFileW] = true;  
        table[kHMCopyFileExW] = true; 
        table[kHMPrivCopyFileExW] = true; 
        table[kHMKernelBasePrivCopyFileExW] = true;
        table[kHMMoveFileW] = true;
        table[kHMMoveFileExW] = true;
        table[kHMMoveFileWithProgressW] = true;
        table[kHMSetFileAttributesW] = true;
        table[kHMGetFileAttributesW] = true;
        table[kHMDeviceIoControl] = true;
        table[kHMEncryptFileW] = true;
        table[kHMCreateFileW] = true;
        table[kHMCreateDirectoryW] = true;
        table[kHMCloseHandle] = true;
        table[kHMDecryptFileW] = true;
        table[kHMCreateProcessW] = true;
        table[kHMAddUsersToEncryptedFile] = true;
        table[kHMSetNamedSecurityInfoW] = true;

		table[kHMWriteFile] = true;
		table[kHMWriteFileEx] = true;

        table[kHMSetClipboardData] = true;
        table[kHMGetClipboardData] = true;
        table[kHMOleSetClipboard] = true;
        table[kHMOleGetClipboard] = true;

        table[kHMDoDragDrop] = true;
        table[kHMRegisterDragDrop] = true;
        table[kHMRevokeDragDrop] = true;

        table[kHMSetFileInformationByHandle] = true;

        table[kHMCoCreateInstance] = true;
		table[kHMCOMCopyItems] = true;

		table[kHMCOMShow] = true;

		// for internet 
		//table[kHMInternetConnectA] = true;
		table[kHMInternetConnectW] = true;
		table[kHMInternetCloseHandle] = true;
		//table[kHMHttpOpenRequestA] = true;
		table[kHMHttpOpenRequestW] = true;

		table[kHMGetSaveFileNameW] = true;

		table[kHMBitBlt] = true;
		table[kHMMaskBlt] = true;
		table[kHMPlgBlt] = true;
		table[kHMStretchBlt] = true;
		table[kHMPrintWindow] = true;
		table[kHMCreateDCA] = true;
		table[kHMCreateDCW] = true;
		table[kHMDeleteDC] = true;
		table[kHMGetDC] = true;
		table[kHMGetDCEx] = true;
		table[kHMGetWindowDC] = true;
		table[kHMReleaseDC] = true;

        table[kHMNtSetSecurityObject] = true;
    }

    EventResult CGenericContext::EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());

        EventResult rt = handler->HandleNewFileAction(strPath, pUserData);

        return rt;
    }

	EventResult CGenericContext::EventBeforeEditFile(const std::wstring& strPath, PVOID pUserData)
	{
		boost::scoped_ptr<CEventHander> handler(new CEventHander());

		EventResult rt = handler->HandleEditFileAction(strPath, pUserData);
		return rt;
	}

    EventResult CGenericContext::EventBeforeNewDirectory(const std::wstring& strPath, PVOID pUserData)
    {
        //OutputDebugStringW(__FUNCTIONW__);
        boost::scoped_ptr<CEventHander> handler(new CEventHander());

        EventResult rt = handler->HandleNewDirectoryAction(strPath, pUserData);

        return rt;
    }

	EventResult CGenericContext::EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName)
	{
		boost::scoped_ptr<CEventHander> handler(new CEventHander());

		EventResult rt = handler->HandleRenameAction(vecFiles, strNewName);

		return rt;
	}
    
    EventResult CGenericContext::EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        
		//return kEventDeny;
		EventResult rt = handler->HandleDeleteAction(vecFiles);

        return rt;
    }

 //   EventResult CGenericContext::EventBeforeSetHiddenAttributeAction(const std::wstring& deviceName)
 //   {
 //       OutputDebugStringW(L"CGenericContext::EventBeforeSetHiddenAttributeAction");
 ///*       OutputDebugStringW(deviceName.c_str());
 //            if (boost::algorithm::icontains(deviceName.c_str(), L"CannotHidden"))
 //       {
 //       OutputDebugStringW(L"Can not hidden");
 //       return kEventDeny;
 //       }*/
 //       return kEventAllow;
 //   }
 //   EventResult CGenericContext::EventBeforeSetReadOnlyAttributeAction(const std::wstring& deviceName)
 //   {
 //       OutputDebugStringW(L"CGenericContext::EventBeforeSetReadOnlyAttributeAction");
 //       OutputDebugStringW(deviceName.c_str());
 ///*       if (boost::algorithm::icontains(deviceName.c_str(), L"CannotReadOnly"))
 //       {
 //           OutputDebugStringW(L"Can not readonly");
 //           return kEventDeny;
 //       }*/
 //       return kEventAllow;
 //   }
    EventResult CGenericContext::EventBeforeSetAttributeAction(const std::wstring& deviceName)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleSetFileAttributeAction(deviceName);
    }

    EventResult CGenericContext::EventBeforeSetSecurityAttributeAction(const std::wstring& fileName)
    {
        boost::scoped_ptr<CEventHander> handle(new CEventHander());
        return handle->HandleSetFileSecurityAttributeAction(fileName);
    }

    EventResult CGenericContext::EventBeforeSetEncryptAttributeAction(const std::wstring& deviceName)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleSetFileAttributeAction(deviceName);
    }
    EventResult CGenericContext::EventBeforeSetUnencryptedAttributeAction(const std::wstring& deviceName)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleSetFileAttributeAction(deviceName);
    }

    EventResult CGenericContext::EventBeforeSetCompressionAttributeAction(const std::wstring& deviceName)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleSetFileAttributeAction(deviceName);
    }
    EventResult CGenericContext::EventBeforeSetUncompressionAttributeAction(const std::wstring& deviceName)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleSetFileAttributeAction(deviceName);
    }

    EventResult CGenericContext::EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleOpenFileAction(filePath);
    
    }

    EventResult CGenericContext::EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        return handler->HandleOpenFolderAction(folderPath);
    }

	EventResult CGenericContext::EventBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations)
	{
		boost::scoped_ptr<CEventHander> handler(new CEventHander());
		EventResult rt = handler->HandleCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists, obligations);

		return rt;
	}

    EventResult CGenericContext::EventBeforeCopyFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        EventResult rt = handler->HandleCopyAction(vecFileOp);

        return rt;
    }

    EventResult CGenericContext::EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
        //OutputDebugStringW(L"-----CGenericContext::EventBeforeMoveFiles-------");
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        EventResult rt = handler->HandleMoveAction(vecFileOp);

        return rt;
    }

    EventResult CGenericContext::EventBeforeCopyContent(const std::wstring& strFileSrc)
    {
        //OutputDebugString(L"CGenericContext::EventBeforeCopyContent");
        //OutputDebugString(strFileSrc.c_str());
        boost::scoped_ptr<CEventHander> handle(new CEventHander());
        EventResult rt = handle->HandleCopyContentAction(strFileSrc);
        return rt;
    }

    EventResult CGenericContext::EventBeforePasteContent(const std::wstring& strFileSrc, std::wstring& strFileDst)
    {
        //OutputDebugString(L"CGenericContext::EventBeforePasteContent");
        //OutputDebugString(strFileSrc.c_str());
        //OutputDebugString(strFileDst.c_str());
        boost::scoped_ptr<CEventHander> handle(new CEventHander());
        EventResult rt = handle->HandlePasteContentAction(strFileSrc, strFileDst);
        return rt;
    }

    EventResult CGenericContext::EventBeforeDropContent(const std::wstring& strFilesrc, const std::wstring& strFileDst)
    {
        //OutputDebugString(L"CGenericContext::EventBeforeDropContent");
        //OutputDebugString(strFilesrc.c_str());
        //OutputDebugString(strFileDst.c_str());
        boost::scoped_ptr<CEventHander> handle(new CEventHander());
        EventResult rt = handle->HandleDropContentAction(strFilesrc, strFileDst);
        return rt;
    }

	EventResult CGenericContext::EventBeforeWriteFile(const std::wstring& strFile)
	{
		boost::scoped_ptr<CEventHander> handle(new CEventHander());
		return handle->HandleWriteFileAction(strFile);
	}

	EventResult CGenericContext::EventBeforeSetEndOfFile(const std::wstring& strFile)
	{
		return kEventAllow;
	}

    EventResult CGenericContext::EventBeforeCreateProcess(const std::wstring& wstrAppPath)
    {
        //OutputDebugStringW(__FUNCTIONW__);
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        EventResult rt = handler->HandleCreateProcess(wstrAppPath);

        return rt;
    }

	EventResult	CGenericContext::EventAfterSaveAs(LPOPENFILENAMEW lpofn, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		boost::scoped_ptr<CEventHander> handle(new CEventHander());
		return handle->HandleSaveAsAction(lpofn, strSource, pComSaveAsStr);
	}

	EventResult	CGenericContext::EventAfterSaveAs(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		boost::scoped_ptr<CEventHander> handle(new CEventHander());
		return handle->HandleSaveAsAction(pThis, hwndOwner, strSource, pComSaveAsStr);
	}
	
	void CGenericContext::EventOpenHttpServer(HINTERNET hRequest, const std::wstring& serverUrl)
	{
		//::OutputDebugStringW(__FUNCTIONW__);
		//::OutputDebugStringW(serverUrl.c_str());
		boost::scoped_ptr<CEventHander> handle(new CEventHander());
		handle->HandleHttpOpen(hRequest, serverUrl);
		return;
	}

}  // ns nextlabs
