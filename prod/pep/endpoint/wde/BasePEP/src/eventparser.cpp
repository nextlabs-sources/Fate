#include "commonutils.hpp"
#include <winioctl.h>

#include <boost/algorithm/string.hpp>

#pragma warning(push)
#pragma warning(disable: 4512 4244 6011) 
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>  
#pragma warning(pop)
#include <boost/algorithm/string.hpp>

#include <ShlObj.h>

#include "eventparser.h"
#include "oleDragNDrop.h"
#include "contentstorage.h"

namespace  
{
    typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
    typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 
    boost::shared_mutex gMutex;
    boost::shared_mutex gOpenedFilesMutex;
	boost::shared_mutex gFilePolicyCacheMutex;
	boost::shared_mutex gPolicyCacheResultMutex;
    boost::shared_mutex gDropTargetsMutex;
}

namespace nextlabs
{
    BOOL CEventParser::IsCreateProcess(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
        LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
    {
        OutputDebugStringW(__FUNCTIONW__);
        UNREFERENCED_PARAMETER(lpApplicationName);
        UNREFERENCED_PARAMETER(lpCommandLine);
        UNREFERENCED_PARAMETER(lpProcessAttributes);
        UNREFERENCED_PARAMETER(lpThreadAttributes);
        UNREFERENCED_PARAMETER(bInheritHandles);
        UNREFERENCED_PARAMETER(dwCreationFlags);
        UNREFERENCED_PARAMETER(lpEnvironment);
        UNREFERENCED_PARAMETER(lpCurrentDirectory);
        UNREFERENCED_PARAMETER(lpStartupInfo);
        UNREFERENCED_PARAMETER(lpProcessInformation);

        return true;
    }

    BOOL CEventParser::IsCreateProcessToOpenFile(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
        LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, std::wstring& fileToOpenPath)
    {
        if (lpCommandLine != NULL)
        {
            LPWSTR *szArglist = NULL;
            int     nNumArgs = 0;
            szArglist = CommandLineToArgvW(lpCommandLine, &nNumArgs);
            if (szArglist != NULL)
            {
                if (nNumArgs > 1)
                {
                    for (int i = 1; i < nNumArgs; ++i)
                    {
                        if (szArglist[i] && ::PathFileExistsW(szArglist[i]))
                        {
                            fileToOpenPath = std::wstring(szArglist[i]);
                            LocalFree(szArglist);
                            szArglist = NULL;
                            return TRUE;
                        } 
                    }
                    
                }
                LocalFree(szArglist);
                szArglist = NULL;
            }  
        }  
        return FALSE;
    }

    BOOL CEventParser::IsNewFileAction(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem)
	{
		UNREFERENCED_PARAMETER(This);
		UNREFERENCED_PARAMETER(psiDestinationFolder);
		UNREFERENCED_PARAMETER(dwFileAttributes);
		UNREFERENCED_PARAMETER(pszName);
		UNREFERENCED_PARAMETER(pszTemplateName);
		UNREFERENCED_PARAMETER(pfopsItem);
		return TRUE;
	}

    BOOL CEventParser::IsNewFileAction(const std::wstring& strPath, const DWORD& dwCreationDisposition)
    {
        if (dwCreationDisposition == OPEN_EXISTING || dwCreationDisposition == TRUNCATE_EXISTING || PathFileExists(strPath.c_str()))
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsNewFileAction(const std::wstring& strPath, const ULONG& dwCreateDisposition, const ULONG& dwCreateOptions)
    {
		if (dwCreateOptions == FILE_OPEN || dwCreateOptions == 0)
		{
			return FALSE;
		}

		if (PathFileExists(strPath.c_str()))
		{
			return FALSE;
		}
		else
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_INVALID_NAME)
			{
				return FALSE;
			}
		}
		
		return TRUE;
    }

	BOOL CEventParser::IsEditFileAction(const std::wstring& strPath, const DWORD& dwDesiredAccess)
	{
		if (dwDesiredAccess & GENERIC_WRITE)
		{
			return TRUE;
		}

		return FALSE;
	}

    BOOL CEventParser::IsNewDirectory(const LPCWSTR lpPathName)
    {
        UNREFERENCED_PARAMETER(lpPathName);
        return TRUE;
    }

    BOOL CEventParser::IsNewDirectory(const LPCWSTR lpPathName, const ULONG createDisposition, const ULONG createOptions)
    {
        if (!(createOptions & FILE_DIRECTORY_FILE) || PathFileExists(lpPathName))
        {
            return FALSE;
        }
        return TRUE;
    }

    BOOL CEventParser::IsRenameAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
    {
		if (!lpExistingFileName || !lpNewFileName) 
		{
			return FALSE;
		}

        if (!nextlabs::utils::IsSameDirectory(lpExistingFileName, lpNewFileName))
        {
            return FALSE;
        }
        OutputDebugStringW(L"CEventParser::IsRenameAction----------------------same dir");
        OutputDebugStringW(lpExistingFileName);
        OutputDebugStringW(lpNewFileName);
        return TRUE;
    }

	BOOL CEventParser::IsRenameAction(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem)
	{
		UNREFERENCED_PARAMETER(This);
		UNREFERENCED_PARAMETER(psiDestinationFolder);
		UNREFERENCED_PARAMETER(pszNewName);
		UNREFERENCED_PARAMETER(pfopsItem);
		return TRUE;
	}

	BOOL CEventParser::IsRenameAction(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName)
	{
		UNREFERENCED_PARAMETER(This);
		UNREFERENCED_PARAMETER(pUnkItems);
		UNREFERENCED_PARAMETER(pszNewName);
		return TRUE;
	}

    BOOL CEventParser::IsDeleteAction(const IFileOperation* This, const IUnknown *punkItems)
    {
        UNREFERENCED_PARAMETER(This);
        UNREFERENCED_PARAMETER(punkItems);

        return TRUE;
    }

    BOOL CEventParser::IsDeleteAction(const IFileOperation* This, const IShellItem *psiItem, const IFileOperationProgressSink *pfopsItem)
    {
        UNREFERENCED_PARAMETER(This);
        UNREFERENCED_PARAMETER(psiItem);
        UNREFERENCED_PARAMETER(pfopsItem);

        return TRUE;
    }

    BOOL CEventParser::IsDeleteAction(const LPCWSTR lpFileName)
    {
		if (!lpFileName) 
		{
			return FALSE;
		}

        if (!nextlabs::utils::IsDestinationRecycleBin(lpFileName))
        {
            return FALSE;
        }
        return TRUE;
    }
    BOOL CEventParser::IsDeleteAction(const DWORD& dwFlagsAndAttributes)
    {
        if (!(dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE))
        {
            return FALSE;
        }
        return TRUE;
    }
    BOOL CEventParser::IsDeleteAction(ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG ShareAccess, ULONG OpenOptions)
    {
        UNREFERENCED_PARAMETER(ShareAccess);
        UNREFERENCED_PARAMETER(OpenOptions);

        if (!(DELETE & DesiredAccess) || !ObjectAttributes || !ObjectAttributes->ObjectName || !ObjectAttributes->ObjectName->Buffer)
        {
            return FALSE;
        }


        return TRUE;
    }

    BOOL CEventParser::IsDeleteAction(LPSHFILEOPSTRUCT lpFileOperation)
    {
        if (FO_DELETE != lpFileOperation->wFunc)
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsCopyAction(const IFileOperation* This, const IUnknown *punkItems, const IShellItem *psiDestinationFolder)
    {
        OutputDebugStringW(L"----access: CEventParser::IsCopyAction");
        UNREFERENCED_PARAMETER(This);
        UNREFERENCED_PARAMETER(punkItems);
        UNREFERENCED_PARAMETER(psiDestinationFolder);
        return TRUE;
    }

    BOOL CEventParser::IsCopyAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
    {
        if (!lpExistingFileName || !lpNewFileName) 
        {
            return FALSE;
        }

        std::wstring strNewFile = lpNewFileName;

        if (nextlabs::comhelper::DoesFileExist(strNewFile) && bFailIfExists) 
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsCopyAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
    {
        UNREFERENCED_PARAMETER(lpProgressRoutine);
        UNREFERENCED_PARAMETER(lpData);
        UNREFERENCED_PARAMETER(pbCancel);
        UNREFERENCED_PARAMETER(dwCopyFlags);

        if (!lpExistingFileName || !lpNewFileName) 
        {
            return FALSE;
        }

        std::wstring strNewFile = lpNewFileName;
        if (nextlabs::comhelper::DoesFileExist(strNewFile) && (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) )
        {
            OutputDebugStringW(L" -------copy failed DstFile have exist!-------------");
            return FALSE; 
        }
        
        return TRUE;
    }

    BOOL CEventParser::IsCopyAction(DWORD *pdwEffect)
    {
        if (*pdwEffect & DROPEFFECT_COPY)
        {
            return TRUE;
        }
        return FALSE;
    }

    BOOL CEventParser::IsMoveAction(const IFileOperation* This, const IUnknown *punkItems, const IShellItem *psiDestinationFolder)
    {
        OutputDebugStringW(L"----access: CEventParser::IsMoveAction");
        UNREFERENCED_PARAMETER(This);
        UNREFERENCED_PARAMETER(punkItems);
        UNREFERENCED_PARAMETER(psiDestinationFolder);
        return TRUE;
    }

    BOOL CEventParser::IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
    {
        if (!lpExistingFileName || !lpNewFileName) 
        {
            return FALSE;
        }

        std::wstring strNewFile = lpNewFileName;

        if (nextlabs::comhelper::DoesFileExist(strNewFile)) 
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
    {
        if (!lpExistingFileName || !lpNewFileName) 
        {
            return FALSE;
        }

        std::wstring strNewFile = lpNewFileName;

        if (nextlabs::comhelper::DoesFileExist(strNewFile)&& !(dwFlags & MOVEFILE_REPLACE_EXISTING)) 
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
    {
        UNREFERENCED_PARAMETER(lpProgressRoutine);
        UNREFERENCED_PARAMETER(lpData);

        if (!lpExistingFileName || !lpNewFileName) 
        {
            return FALSE;
        }

        std::wstring strNewFile = lpNewFileName;

        if (nextlabs::comhelper::DoesFileExist(strNewFile)&& !(dwFlags & MOVEFILE_REPLACE_EXISTING)) 
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL CEventParser::IsMoveAction(DWORD *pdwEffect)
    {
        if (*pdwEffect & DROPEFFECT_MOVE)
        {
            return TRUE;
        }
        return FALSE;
    }

    BOOL CEventParser::IsSetFileAttribute(LPCTSTR lpFileName)
    {
        return TRUE;
    }

    BOOL CEventParser::IsSetAttributeReadOnly(DWORD dwAttributes)
    {
        return (dwAttributes & FILE_ATTRIBUTE_READONLY);
    }

    BOOL CEventParser::IsSetAttributeHidden(DWORD dwAttributes)
    {
        return (dwAttributes & FILE_ATTRIBUTE_HIDDEN);
    }

    BOOL CEventParser::IsSetAttributeReadOnly(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation)
    {
        if (lpFileInfromation)
        {
            if (FileInformationClass == FileBasicInfo)
            {
                PFILE_BASIC_INFO pBasicInfo = static_cast<PFILE_BASIC_INFO>(lpFileInfromation);
                
                return (pBasicInfo->FileAttributes & FILE_ATTRIBUTE_READONLY);
            }
            
        }
        
        return FALSE;
    }
    BOOL CEventParser::IsSetAttributeHidden(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation)
    {
        if (lpFileInfromation)
        {
            if (FileInformationClass == FileBasicInfo)
            {
                PFILE_BASIC_INFO pBasicInfo = static_cast<PFILE_BASIC_INFO>(lpFileInfromation);

                return (pBasicInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN);
            }

        }
        return FALSE;
    }

    BOOL CEventParser::IsFileAttributeChange(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation)
    {

        if (lpFileInfromation)
        {
            if (FileInformationClass == FileBasicInfo)
            {
                PFILE_BASIC_INFO pBasicInfo = static_cast<PFILE_BASIC_INFO>(lpFileInfromation);
                if (pBasicInfo->FileAttributes == 0)
                {
                    return FALSE;
                }else
                {
                    return TRUE;
                }
                
            }

        }
        return FALSE;
    }

    BOOL CEventParser::IsSetAttributeEncrypt(LPCWSTR lpFileName)
    {
        if (lpFileName)
        {
            return TRUE;
        }
        return FALSE;
    }
    BOOL CEventParser::IsSetAttributeCompressed(HANDLE hDevice,DWORD dwIoControlCode, LPVOID lpInBuffer)
    {
        if (dwIoControlCode == FSCTL_SET_COMPRESSION && hDevice)
        {
            boost_share_lock lockRead(gMutex);
            std::map<HANDLE, std::wstring>::iterator itor = h2fileName_.find(hDevice);
            if (itor != h2fileName_.end())
            {
                std::wstring filePath = itor->second;
                USHORT *pOpt = (USHORT *)lpInBuffer;
               if (*pOpt == COMPRESSION_FORMAT_DEFAULT || *pOpt == COMPRESSION_FORMAT_LZNT1)
               {
                    return TRUE;
               }

            }
        }

        return FALSE;
    }

    BOOL CEventParser::IsSetAttributeUncompressed(HANDLE hDevice,DWORD dwIoControlCode, LPVOID lpInBuffer)
    {
        if (dwIoControlCode == FSCTL_SET_COMPRESSION && hDevice)
        {
            boost_share_lock lockRead(gMutex);
            std::map<HANDLE, std::wstring>::iterator itor = h2fileName_.find(hDevice);
            if (itor != h2fileName_.end())
            {
                std::wstring filePath = itor->second;
                USHORT *pOpt = (USHORT *)lpInBuffer;
                if ( *pOpt == COMPRESSION_FORMAT_NONE)
                {
                    return TRUE;

                }
            }
        }

        return FALSE;
    }
    BOOL CEventParser::IsSetAttributeUnencrypted(LPCWSTR lpFileName)
    {
        if (lpFileName)
        {
            return TRUE;
        }

        return FALSE;
    }

    BOOL CEventParser::IsSetFileSecurityAttribute(LPCWSTR pObjectName, SE_OBJECT_TYPE ObjectType)
    {
        if (pObjectName)
        {
            if (ObjectType == SE_FILE_OBJECT)
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL CEventParser::IsSetAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers)
    {
        if (lpFileName && pUsers)
        {
            return TRUE;
        }

        return FALSE;
        
    }

    BOOL CEventParser::isCopyContent(const UINT uFormat, std::wstring &srcFilePath)
    {
        BOOL bNeedHandle = FALSE;
        switch (uFormat)
        {
        case CF_TEXT:
        case CF_UNICODETEXT:
        case CF_OEMTEXT:
        case CF_DIB:
        case CF_ENHMETAFILE:
            bNeedHandle = TRUE;
            break;
        default:
            {
                WCHAR szFormat[256] = { 0 };
                if (0 != GetClipboardFormatNameW(uFormat, szFormat, 256))
                {
                    if (0 == wcscmp(szFormat, L"Rich Text Format"))
                    {
                        bNeedHandle = TRUE;
                    }
                }
            }
            break;
        }

        if (bNeedHandle)
        {
            std::wstring filePath = L"";
            if (GetCurrentDocumentPath(filePath))
            {
                if (StoreClipboardSrcFile(filePath) == TRUE)
                {
                    srcFilePath = filePath;
                    return TRUE;
                }
            } else
            {
                OutputDebugString(L"can not get content source filepath, do nothing");
                StoreClipboardSrcFile(L"");
                return FALSE;
            }
        }
        CleanClipboardSrcFile();
        return FALSE;
    }

    BOOL CEventParser::isCopyContent(const LPDATAOBJECT pDataObject, __out std::wstring& srcFilePath)
    {
        if (NULL == pDataObject)
        {
            return FALSE;
        }

        if (utils::isContentData(pDataObject))
        {
            std::wstring clipboardSrcFile = L"";
            if (FALSE == utils::GetOleContentClipboardDataSource(pDataObject, clipboardSrcFile, srcFilePath))
            {
                if(FALSE == GetCurrentDocumentPath(srcFilePath))
                {
                    OutputDebugString(L"Get current active document failed, do nothing");
                    StoreClipboardSrcFile(L"");
                    return FALSE;
                }
                StoreClipboardSrcFile(srcFilePath);
            }
            return TRUE;
        }

        return FALSE;
    }
    BOOL CEventParser::IsDragDropContent(const LPDATAOBJECT pDataObj, const LPDROPSOURCE pDropSource, const DWORD dwOkEffects, const LPDWORD pdwEffect, __out std::wstring& srcFilePath)
    {
        if (utils::isContentData(pDataObj))
        {
           if (FALSE == utils::GetOleContentClipboardDataSource(pDataObj, L"", srcFilePath))
           {
                if (FALSE == GetCurrentDocumentPath(srcFilePath))
                {
                    OutputDebugString(L"cat not get drag file path when starting draging");
                    return FALSE;
                }   
           }
           StoreDragDropContentFile(srcFilePath);
           return TRUE;
        }
        return FALSE;
    }

    BOOL CEventParser::isPasteContent(const UINT uFormat, __out std::wstring& dstFilePath, __out std::wstring& srcFilePath)
    {
        if (uFormat == CF_TEXT || 
            uFormat == CF_UNICODETEXT ||
            uFormat == CF_OEMTEXT ||
            uFormat == CF_DIB)
        {
            std::wstring clipboardSrcFilePath = L""; //clipboard content source filepath
            GetClipboardSrcFile(clipboardSrcFilePath);
            
            if(utils::GetClipboardContentDataSource(NULL, clipboardSrcFilePath, srcFilePath))
            {
                if (!boost::iequals(clipboardSrcFilePath, srcFilePath))
                {
                    StoreClipboardSrcFile(srcFilePath);
                }
                BOOL rt = GetCurrentDocumentPath(dstFilePath);
                if (rt == FALSE)
                {
                    OutputDebugString(L"get current active document path failed");
                }
                return TRUE;
            }
        }

        if (uFormat == CF_HDROP)
        {
            //TBD
            return FALSE;
        }
        return FALSE;
    }
    BOOL CEventParser::isPasteContent(const LPDATAOBJECT pDataObject, __out std::wstring &dstFilePath, __out std::wstring& srcFilePath)
    {
        if (pDataObject == NULL)
        {
            return FALSE;
        }
        if (TRUE == utils::isContentData(pDataObject))
        {
            std::wstring clipboardSrcFilePath = L"";
            GetClipboardSrcFile(clipboardSrcFilePath);
            if (utils::GetClipboardContentDataSource(pDataObject, clipboardSrcFilePath, srcFilePath))
            {
                if (!boost::iequals(clipboardSrcFilePath, srcFilePath))
                {
                    StoreClipboardSrcFile(srcFilePath);
                }
                if (GetCurrentDocumentPath(dstFilePath) == FALSE)
                {
                    OutputDebugString(L"can not get current active document path, do nothing");
                   /* return FALSE;*/
                } 
                return TRUE;
            } else
            {
                OutputDebugString(L"can not get srcFilepath, do nothing");
                return FALSE;
            }
        }
        return FALSE;
    }

    BOOL CEventParser::isDropForPasteContent(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::wstring& srcFilePath, __out std::wstring&dstFilePath)
    {
        if (FALSE == utils::isContentData(pDataObject))
        {
            return FALSE;
        }

        //Handle "copy content". the case is: drag some content and drop it.
        if (FALSE == utils::GetOleContentClipboardDataSource(pDataObject, L"", srcFilePath))
        {
            //we can't get content source; do nothing
            
            if (FALSE == GetDragDropContentFile(srcFilePath))
            {
                OutputDebugString(L"can not get content source file pathd when draging content, just do nothing");
                return FALSE;
            }
        }
        
        //we got content source; get context destination
        POINT point = {0, 0};
        ::GetCursorPos(&point);
        HWND hWnd = ::WindowFromPoint(point);
        if (NULL == hWnd)
        {
            OutputDebugString(L"can not get Hwnd from drop point do nothing");
            return FALSE;
        }

        while(true) 
        {
            HWND hParent = ::GetParent(hWnd);
            if(NULL == hParent)	
            {
                break;
            }
            hWnd = hParent;
        }

        WCHAR szTitle[MAX_PATH] = {0};
        GetWindowTextW(hWnd, szTitle, MAX_PATH);

        std::wstring title = szTitle;
        
        std::set<std::wstring> outFiles;
        std::set<std::wstring> openedFiles;
        {
            boost_share_lock lockRead(gOpenedFilesMutex);
            openedFiles = openedFiles_;
        }

        if (utils::GetFullFilePathFromTitle(title, outFiles, openedFiles)) 
        {
            //this have get all filepath that have the fileame of pTitleName. this may caused a bug when open two file have same name but in different
            //folder.
            dstFilePath = outFiles.begin()->c_str();
            return TRUE;	  
        } else
        {
			dstFilePath.clear();
			return TRUE;
        }
    }

    BOOL CEventParser::isDropForOpenFiles(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::list<std::wstring>& files)
    {
        if (TRUE == utils::isContentData(pDataObject))
        {
            return FALSE;
        }

        if (winClassAction.action != policyengine::WdeActionRead)
        {
            return FALSE;
        }

        //TBD

        return FALSE;
    }

    BOOL CEventParser::isDropForCopyFiles(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::list<std::wstring>& srcfilePaths, __out std::wstring &dstFilePath)
    {
        //get source files.
        if (FALSE == GetOLEDropFiles(pDataObject, srcfilePaths))
        {
            OutputDebugString(L"can not get droped files, do nothing");
            return FALSE;
        }

        if (winClassAction.action != policyengine::WdeActionCopy)
        {
            return FALSE;
        }

        //get current process name and ignore the explorer.exe
        TCHAR szPath[MAX_PATH] = {0};
        if (!::GetModuleFileName(NULL, szPath, MAX_PATH))
        {
            OutputDebugString(L"can not get current process name");
            return FALSE;
        };
        
        std::wstring currentExeName = L"";
        std::wstring currentProcessPath = szPath;
        size_t pos = currentProcessPath.rfind(L"\\");
        if (std::wstring::npos != pos && std::wstring::npos != pos + 1)
        {
            currentExeName = currentProcessPath.substr(pos + 1);
        } else
        {
            currentExeName = currentProcessPath;
        }

        if (boost::iequals(currentExeName, L"explorer.exe"))
        {
            OutputDebugString(L"do not handle explorer.exe, do nothing");
            return FALSE;
        }

        //get copy dst file path
        if (FALSE == GetCurrentDocumentPath(dstFilePath))
        {
            OutputDebugString(L"can not get current active docuement path, do nothing");
        }

        return TRUE;
    }


    BOOL CEventParser::IsFileOpen(LPCTSTR lpFileName, const DWORD dwDesiredAccess, const DWORD dwShareMode, const DWORD dwCreationDisposition, const DWORD dwFlagsAndAttributes)
    {                      
        if (lpFileName && ((dwCreationDisposition == OPEN_ALWAYS) || (dwCreationDisposition == OPEN_EXISTING)))
        {
            if (!PathIsDirectoryW(lpFileName) && PathFileExists(lpFileName))
            {
                return TRUE;
            }
            
        }
        
        return FALSE;
    }

    BOOL CEventParser::IsDirectoryOpen(LPCTSTR lpFileName, const DWORD dwDesiredAccess, const DWORD dwShareMode, const DWORD dwCreationDisposition, const DWORD dwFlagsAndAttributes)
    {
        if (lpFileName)
        {

            return PathIsDirectoryW(lpFileName);
        }

        return FALSE;
        
    }

    BOOL CEventParser::IsDirectoryOpen(LPCTSTR lpFileName)
    {
        if (lpFileName)
        {

            return PathIsDirectoryW(lpFileName);
        }

        return FALSE;
    }


    BOOL CEventParser::IsRegisterDragDrop(HWND hWnd, LPDROPTARGET pDropTarget, __out DNDWinClassAction& winClassAction)
    {
        OutputDebugString(L"Is Register Drag Drop parser");
        TCHAR szPath[MAX_PATH] = {0};
        if (!::GetModuleFileName(NULL, szPath, MAX_PATH))
        {
            OutputDebugString(L"can not get current process name");
            return FALSE;
        };
        OutputDebugString(szPath);
        std::wstring currentExeName = L"";
        std::wstring currentProcessPath = szPath;
        size_t pos = currentProcessPath.rfind(L"\\");
        if (std::wstring::npos != pos && std::wstring::npos != pos + 1)
        {
            currentExeName = currentProcessPath.substr(pos + 1);
        } else
        {
            currentExeName = currentProcessPath;
        }

        DNDWinClassAction *winClassActions = NULL;
        for (int i = 0, c = (sizeof(DRAG_N_DROP_APP_CLASS_ACTIONS)/sizeof(DNDAppWinClassAction)); i< c; i++)
        {
            if (boost::iequals(DRAG_N_DROP_APP_CLASS_ACTIONS[i].processName, currentExeName))
            {
                winClassActions = DRAG_N_DROP_APP_CLASS_ACTIONS[i].winClassActions;
                break;
            }
        }
        if (NULL == winClassActions)
        {
            OutputDebugString(currentExeName.c_str());
            OutputDebugString(L"this process do not handle drag/drop action");
            return FALSE;
        }

        // Then let's make sure we have something for this window
        if (boost::iequals(winClassActions->className, L"*"))
        {
            winClassAction = *winClassActions;
            OutputDebugString(L"Find Win Class Action");
        }
        else
        {
            // First let's retrieve the Window class name
            WCHAR winClassName[MAX_PATH] = {0};
            if (!GetClassNameW(hWnd, winClassName, MAX_PATH)) 
            {
                OutputDebugString(L"can not get Window class name do nothing");
                return FALSE;
            }
            while (!winClassActions->className.empty()) 
            { 
                if (boost::iequals(winClassName, winClassActions->className))
                {
                    winClassAction = *winClassActions;
                    OutputDebugString(L"Find Win Class Action");
                    break;
                }
                winClassActions++;
            }
        }

        if (policyengine::WdeActionMax != winClassActions->action) {
            // That is a window we want to intercept drag and drop actions, Register proxy target instead
            return TRUE;
        }
        OutputDebugString(L"Is Register Drag Drop failed");
        return FALSE;
    }

    BOOL CEventParser::IsPictureFileType(IShellItem *pShellItem,  std::wstring& filepath)
    {
        if(FAILED(nextlabs::comhelper::GetPathByShellItem(filepath, pShellItem)))
        {
            return FALSE;
        }

		if (filepath.empty())
		{
			return FALSE;
		}

		SFGAOF psfgaoAttribs = 0;
		pShellItem->GetAttributes(SFGAO_FOLDER, &psfgaoAttribs);
		if (psfgaoAttribs & SFGAO_FOLDER)
		{
			return FALSE;
		}

        std::wstring fileExtension = ::PathFindExtension(filepath.c_str());
        boost::algorithm::to_lower(fileExtension);
        if (boost::algorithm::iends_with(fileExtension, L".jpg")  ||
            boost::algorithm::iends_with(fileExtension, L".jpeg") ||
            boost::algorithm::iends_with(fileExtension, L".tif")  ||
            boost::algorithm::iends_with(fileExtension, L".tiff") ||
            boost::algorithm::iends_with(fileExtension, L".gif")  ||
            boost::algorithm::iends_with(fileExtension, L".bmp")  ||
            boost::algorithm::iends_with(fileExtension, L".dib")  ||
            boost::algorithm::iends_with(fileExtension, L".png")  ||
            boost::algorithm::iends_with(fileExtension, L".wdp"))
        {
            return TRUE;
        }

        return FALSE;
    }

    void CEventParser::StoreFileHandleWithPath(HANDLE& hFileHandle, LPCWSTR lpFileName)
    {
        if (lpFileName && hFileHandle)
        {    
            boost_unique_lock lockWrite(gMutex);
            h2fileName_[hFileHandle] = std::wstring(lpFileName);
        }
    }

    void CEventParser::RemoveFilePathInfoByHandle(HANDLE& hFileHandle)
    {
        boost_unique_lock lockWrite(gMutex);
        std::map<HANDLE, std::wstring>::iterator itor = h2fileName_.find(hFileHandle);
        if (itor != h2fileName_.end())
        {
            h2fileName_.erase(itor);
        }
    }

    std::wstring CEventParser::GetFilePathByHandle(HANDLE& hFileHandle)
    {
        boost_share_lock lockRead(gMutex);
        std::map<HANDLE, std::wstring>::iterator itor = h2fileName_.find(hFileHandle);
        std::wstring filePath;
        if (itor != h2fileName_.end())
        {
            filePath = itor->second;

        }

        return filePath;
    }

    BOOL CEventParser::IsHandleMapContantsFile(const std::wstring& filePath)
    {
        boost_share_lock lockRead(gMutex);
        for(std::map<HANDLE, std::wstring>::const_iterator iter = h2fileName_.begin() ; iter != h2fileName_.end() ; ++iter)
        {
            /* Each entry is basically a std::pair<HANDLE,std::wstring> */
            std::wstring wstr = iter->second;
            if (_wcsicmp(wstr.c_str(), filePath.c_str()))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    void CEventParser::AddOpenedFile(const std::wstring& filePath)
    {
        boost_unique_lock lockWrite(gOpenedFilesMutex);
        openedFiles_.insert(filePath);
    }

    void CEventParser::RemoveOpenedFile(const std::wstring& filePath)
    {
        boost_unique_lock lockWrite(gOpenedFilesMutex);
        openedFiles_.erase(filePath);
    }

    void CEventParser::StoreFilePolicy(HANDLE& hFileHandle, BOOL bWriteAllowed)
    {
        boost_unique_lock lockWrite(gFilePolicyCacheMutex);
        h2filePolicy_[hFileHandle] = bWriteAllowed;
    }

    void CEventParser::RemoveFilePolicy(HANDLE& hFileHandle)
    {
        boost_unique_lock lockWrite(gFilePolicyCacheMutex);
        std::map<HANDLE, BOOL>::iterator itor = h2filePolicy_.find(hFileHandle);
        if (itor != h2filePolicy_.end())
        {
            h2filePolicy_.erase(itor);
        }
    }

    BOOL CEventParser::GetFilePolicy(HANDLE& hFileHandle, BOOL& bWriteAllowed)
    {
        boost_share_lock lockRead(gFilePolicyCacheMutex);
        std::map<HANDLE, BOOL>::iterator itor = h2filePolicy_.find(hFileHandle);
        if (itor == h2filePolicy_.end())
        {
            return FALSE;
        }
        else
        {
            bWriteAllowed = itor->second;
            return TRUE;
        }
    }

	void CEventParser::StorePolicyCacheResult(const std::wstring& filePath, BOOL bAllowed)
	{
		strPolicyCacheResult policyCacheResult;
		policyCacheResult.bAllow = bAllowed;
		policyCacheResult.dwTime = GetTickCount();

		boost_unique_lock lockWrite(gPolicyCacheResultMutex);
		h2PolicyCacheResult_[filePath] = policyCacheResult;
	}

	BOOL CEventParser::GetPolicyCacheResult(const std::wstring& filePath, BOOL& bAllowed)
	{
		boost_share_lock lockRead(gPolicyCacheResultMutex);
		std::map<std::wstring, strPolicyCacheResult>::iterator itor = h2PolicyCacheResult_.find(filePath);
		if (itor == h2PolicyCacheResult_.end())
		{
			return FALSE;
		}
		else
		{
			if (GetTickCount() - itor->second.dwTime > 2000)
			{
				return FALSE;
			}

			bAllowed = itor->second.bAllow;
			return TRUE;
		}
	}

    BOOL CEventParser::StoreClipboardSrcFile(const std::wstring& filePath)
    {
        CContextStorage storage = CContextStorage();
        bool rt = storage.StoreClipboardInfo(filePath);
        if (rt == false)
        {
            OutputDebugString(L"store clipboardInfo into ContextStorage failed");
            return FALSE;
        }
        return TRUE;
    }

    BOOL CEventParser::GetClipboardSrcFile(__out std::wstring& filePath)
    {
        CContextStorage storage = CContextStorage();
        bool rt = storage.GetClipboardInfo(filePath);
        if (rt == false)
        {
            OutputDebugString(L"Get clipboardInfo into ContextStorage failed");
            return FALSE;
        }
        return TRUE;
    }

    BOOL CEventParser::CleanClipboardSrcFile()
    {
        CContextStorage storage = CContextStorage();
        bool rt = storage.StoreClipboardInfo(L"");
        if (rt == false)
        {
            OutputDebugString(L"clean clipboardInfo failed");
            return FALSE;
        }
        return TRUE;
    }

    BOOL CEventParser::StoreDragDropContentFile(const std::wstring& filePath)
    {
        CContextStorage storage = CContextStorage();
        bool rt = storage.StoreDragDropContentFileInfo(filePath);
        if (rt == false)
        {
            OutputDebugString(L"store dragdrop content filePath into ContextStorage failed");
            return FALSE;
        }
        return TRUE;
    }

    BOOL CEventParser::GetDragDropContentFile(__out std::wstring& filePath)
    {
        CContextStorage storage = CContextStorage();
        bool rt = storage.GetDragDropContentFileInfo(filePath);
        if (rt == false)
        {
            OutputDebugString(L"Getstore dragdrop content filePath into ContextStorage failed");
            return FALSE;
        }
        return TRUE;
    }

    void CEventParser::addDropTargetProxy(const HWND& hwnd, DropTargetProxy *pProxy)
    {
        boost_unique_lock lockWrite(gDropTargetsMutex);
        droptargets_[hwnd] = pProxy;
    }

    void CEventParser::removeDropTargetProxy(const HWND& hwnd)
    {
        boost_unique_lock lockWrite(gDropTargetsMutex);
        std::map<HWND, DropTargetProxy*>::iterator itor = droptargets_.find(hwnd);
        if (itor != droptargets_.end())
        {
            droptargets_.erase(itor);
        }
    }

    DropTargetProxy* CEventParser::GetDropTargetProxy(const HWND& hwnd)
    {
        DropTargetProxy *proxy = NULL;

        boost_share_lock lockRead(gDropTargetsMutex);
        std::map<HWND, DropTargetProxy *>::iterator itor = droptargets_.find(hwnd);
        if (itor != droptargets_.end())
        {
            proxy = itor->second; 
        }
        return proxy;
    }

    void CEventParser::RevokeAllDropTargets()
    {   
        boost_unique_lock lockwrite(gDropTargetsMutex);

        for (std::map<HWND, DropTargetProxy *>::iterator itor = droptargets_.begin(); itor != droptargets_.end(); ++itor)
        {
            if (SUCCEEDED(itor->first))
            {
                ::RevokeDragDrop(itor->first);
            }
        }
        droptargets_.clear();
    }

    BOOL CEventParser::GetCurrentDocumentPath(__out std::wstring& filePath, BOOL bFullPath, BOOL *pPathReliable, const std::wstring& titleName)
    {
        // We cannot be reliable in the generic case
        if (NULL != pPathReliable) 
        {
            *pPathReliable = FALSE;
        }

        if (!titleName.empty())
        {
            std::set<std::wstring> outFiles;
            std::set<std::wstring> openedFiles;
            {
                boost_share_lock lockRead(gOpenedFilesMutex);
                openedFiles = openedFiles_;
            }
            if (utils::GetFullFilePathFromTitle(titleName, outFiles, openedFiles)) 
            {
                //this have get all filepath that have the fileame of pTitleName. this may caused a bug when open two file have same name but in different
                //folder.
                filePath = outFiles.begin()->c_str();
                return TRUE;	  
            }
        }

        HWND hFWnd = GetCurrentDocumentWindow();
        if (NULL == hFWnd) {
            return FALSE;
        }

        WCHAR pszTitle[MAX_PATH] = {0};
        GetWindowText(hFWnd, pszTitle, _countof(pszTitle)); 

        std::wstring title = pszTitle;

        std::set<std::wstring> outFiles;
        std::set<std::wstring> openedFiles;
        {
            boost_share_lock lockRead(gOpenedFilesMutex);
            openedFiles = openedFiles_;
        }
        if (!utils::GetFullFilePathFromTitle(title, outFiles, openedFiles))
        {
            return FALSE;
        }
        filePath = outFiles.begin()->c_str();
        return TRUE;
    }

    HWND CEventParser::GetCurrentDocumentWindow()
    {
        HWND hfgWnd = ::GetForegroundWindow();
        if (hfgWnd)
        {
            DWORD processId;
            ::GetWindowThreadProcessId (hfgWnd, &processId);
            if (processId != ::GetCurrentProcessId ())
            {            
                EnumThreadWindows(::GetCurrentThreadId(), EnumWndProc, (LPARAM)&hfgWnd);
            }
        }
        return hfgWnd;
    }

    BOOL CEventParser::GetOLEDropFiles(IDataObject *pDataObject, std::list<std::wstring>& files)
    {
        STGMEDIUM medium;
        FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        HDROP hdrop;

        if(NULL == pDataObject) 
        {
            return FALSE;
        }
        BOOL bSucceed = FALSE;
        if (SUCCEEDED((pDataObject)->GetData(&fe, &medium))) 
        {
            hdrop=(HDROP)GlobalLock(medium. hGlobal);
            if(hdrop != NULL) 
            {
                if(utils::GetFilesFromHDROP(hdrop, files)) 
                {
                    bSucceed = TRUE;
                }
                GlobalUnlock (medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }
        //if(!bSucceed) {
        //    //When the folder is transfered over RPC, 
        //    //data is in ShellIDListArray format
        //    OleGetFilesFromShellIDListArray(pDataObject, files);
        //    if(files.size() > 0) 
        //        bSucceed=TRUE;
        //}
        return bSucceed;
    }

    BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam) 
    {
        HWND *phWnd = (HWND*)lParam;
        WCHAR buf [1024] = {0};
        if (::GetParent (hwnd) == NULL && ::GetWindowText(hwnd, buf, 1024) != 0)
        {
            *phWnd = hwnd;
            return FALSE;
        }
        return TRUE;
    }

#pragma region CExplorerParser
     BOOL CExplorerParser::IsSetFileAttribute(LPCTSTR lpFileName)
     {
         if (lpFileName)
         { 
             PWSTR outPath = NULL;
			 BOOL retVal = TRUE;
             HRESULT res = ::SHGetKnownFolderPath(FOLDERID_CDBurning, 0, NULL, &outPath);
             if(res == S_OK)
             {    
                 if (boost::algorithm::iequals(lpFileName, outPath))
                 {
                     retVal = FALSE;
                 }
             }
			 ::CoTaskMemFree(outPath);
			 return retVal;
         }
         return TRUE;
     }

     BOOL CExplorerParser::IsFileOpen(LPCTSTR lpFileName, const DWORD dwDesiredAccess, const DWORD dwShareMode, const DWORD dwCreationDisposition, const DWORD dwFlagsAndAttributes)
     {
         if (lpFileName && dwDesiredAccess == GENERIC_READ && dwCreationDisposition == OPEN_EXISTING && (dwShareMode & (FILE_SHARE_READ | FILE_SHARE_DELETE)))
         {
             if (!PathIsDirectoryW(lpFileName) && PathFileExistsW(lpFileName))
             {
                 return TRUE;
             }

         }

         return FALSE;
     }

     BOOL CExplorerParser::IsCreateProcessToOpenFile(LPCWSTR lpApplicationName,LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,  LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, std::wstring& filePath)
     {
         return FALSE;
     }
#pragma endregion

    void CNetworkEventParser::OnSuccessInternetConnect(HINTERNET hConnect, LPCWSTR lpszServerName, INTERNET_PORT nServerPort)
    {
        INETCONNMAP::iterator it = FindIterByKeySafe(hConnect);
        // uniLock
        boost::unique_lock<boost::shared_mutex> uniLock(mapMutex_);
        if (it != inetConnMap_.end())	// found
        {
            it->second->inc_ref();
        }
        else  // new one
        {
            inetConnMap_.insert(std::make_pair(hConnect, boost::shared_ptr<COpenedInternetHandle>(new COpenedInternetHandle(hConnect, lpszServerName, nServerPort))));
        }
    }

    void CNetworkEventParser::OnSuccessInternetCloseHandle(HINTERNET hConnect)
    {
        INETCONNMAP::iterator it = FindIterByKeySafe(hConnect);
        // uniLock
        boost::unique_lock<boost::shared_mutex> uniLock(mapMutex_);
        if ( it != inetConnMap_.end() && 0 == it->second->dec_ref()	 ) {
            inetConnMap_.erase(it);
        }
    }

    bool CNetworkEventParser::OnSuccessHttpOpenRequest(HINTERNET hConnect, LPCWSTR lpszObjectName ,std::wstring& outServerPath)
    {
        INETCONNMAP::iterator it = FindIterByKeySafe(hConnect);
        if (it == inetConnMap_.end())
        {
            return false;
        }
        std::wstring host;
        WORD         port = 0;

        host = it->second->get_server();
        port = it->second->get_port();
        outServerPath.clear();
        if (!host.empty() && 0 != port)
        {
            if (INTERNET_DEFAULT_HTTPS_PORT == port)
            {
                outServerPath = L"https://";
            }
            else
            {
                outServerPath = L"http://";
            }
            outServerPath += host;

            if (NULL != lpszObjectName && '\0' != lpszObjectName[0])
            {
                outServerPath += lpszObjectName;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

}  // ns nextlabs
