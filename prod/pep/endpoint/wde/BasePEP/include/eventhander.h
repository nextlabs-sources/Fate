#ifndef __WDE_EVENTHANDER_H__
#define  __WDE_EVENTHANDER_H__

#include "commonutils.hpp"
#include <boost/noncopyable.hpp>
#include <Commdlg.h>

namespace nextlabs
{
    class CEventHander : public boost::noncopyable 
    {
    public:
        virtual ~CEventHander(){}

		EventResult HandleNewFileAction(const std::wstring& fileName, PVOID pUserData);

		EventResult HandleEditFileAction(const std::wstring& fileName, PVOID pUserData);

        EventResult HandleNewDirectoryAction(const std::wstring& dirName, PVOID pUserData);

        EventResult HandleRenameAction(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName);

        EventResult HandleDeleteAction(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles);

        EventResult HandleCopyAction(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);

		EventResult HandleCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations);

        EventResult HandleMoveAction(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);

        EventResult HandleCopyContentAction(const std::wstring& wstrFilePath);

        EventResult HandlePasteContentAction(const std::wstring& wstrSrcFilePath, const std::wstring& wstrDstFilePath);
        
        EventResult HandleDropContentAction(const std::wstring& wstrSrcFilePath, const std::wstring& wstrDstFilePath);
		
        EventResult HandleWriteFileAction(const std::wstring& wstrFilePath);

        EventResult HandleOpenFileAction(const std::wstring& filePath, bool bIgnoreSomeDirectorys = true);

        EventResult HandleOpenFolderAction(const std::wstring& folderPath);

        EventResult HandleSetFileAttributeAction(const std::wstring& fileName);

        EventResult HandleSetFileSecurityAttributeAction(const std::wstring& fileName);

        EventResult HandleCreateProcess(const std::wstring& wstrAppPath);

        EventResult HandleSaveAsAction(LPOPENFILENAMEW lpofn, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr);

		EventResult HandleSaveAsAction(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr);

        EventResult HandleUploadAction(LPOPENFILENAMEW lpofn, const std::wstring& strDest);

		void		HandleHttpOpen(HINTERNET hRequest, const std::wstring& serverUrl);
		
    private:
		BOOL HasHttpInjectHeader(const std::wstring& serverUrl, std::wstring& key, std::wstring& val);
		EventResult _ImplHandleMoveCopyAction(nextlabs::policyengine::WdeAction action,const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);
    };
}  // ns nextlabs


#endif