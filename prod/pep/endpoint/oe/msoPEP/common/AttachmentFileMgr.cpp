
/** precompile header and resource header files */
#include "stdafx.h"

/** current class declare header files */
#include "AttachmentFileMgr.h"

/** C system header files */

/** C++ system header files */

/** Platform header files */

/** Third part library header files */
/** boost */
// #include "boost/thread.hpp"

/** Other project header files */

/** Current project header files */
#include "Hook.h"
#include "CommonTools.h"
#include "log.h"



CAttachmentFileMgr::CAttachmentFileMgr() : m_lInitFlag(0), m_lUseDragDropFlag(0), m_kstMaxMsgSrcFileCacheSize(128), m_kstMaxSourcePathFinallyBackCache(128)
{

}

CAttachmentFileMgr::~CAttachmentFileMgr()
{

}

CAttachmentFileMgr& CAttachmentFileMgr::GetInstance()
{
    static CAttachmentFileMgr theAttachmentFileMgr;
    return theAttachmentFileMgr;
}

bool CAttachmentFileMgr::GetInitFlag()
{
    return (0 != m_lInitFlag);
}

void CAttachmentFileMgr::SetInitFlag(_In_ bool bInit)
{
    InterlockedExchange(&m_lInitFlag, (bInit ? 1 : 0));
}

bool CAttachmentFileMgr::GetUseDragDropFlag()
{
    return (0 != m_lUseDragDropFlag);
}

void CAttachmentFileMgr::SetUseDragDropFlag(_In_ bool bUse)
{
    InterlockedExchange(&m_lUseDragDropFlag, (bUse ? 1 : 0));
}

bool CAttachmentFileMgr::Init(_In_ const std::wstring& kwstrOETempFolder, _In_ const std::wstring& kwstrOutlookTempFolder, _In_ const std::wstring& kwstrContentWordTempFolder, _In_ const std::wstring& kwstrContentMsoTempFolder,
                              _In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType)
{NLONLY_DEBUG
    bool bRet = false;

    NLPRINT_DEBUGVIEWLOG(L"OETemfolder:[%s], OutlookTempFolder:[%s], ContentWordTemp:[%s]\n", kwstrOETempFolder.c_str(), kwstrOutlookTempFolder.c_str(), kwstrContentWordTempFolder.c_str());
    if (!GetInitFlag())
    {
        if (IsFullLocalPath(kwstrOETempFolder, true) && IsFullLocalPath(kwstrOutlookTempFolder, true) && IsFullLocalPath(kwstrContentMsoTempFolder, true))
        {
            /**< 
                If when outlook first opened, in sometimes the outlook temp folder do not exist. OE temp must be exist.
                If the file not exist or other exception, NLGetLongFilePath will return L"".
            */
            m_wstrOETempFolder = NLGetLongFilePath(kwstrOETempFolder);
            m_wstrOutlookTempFolder = NLGetLongFilePathEx(kwstrOutlookTempFolder);
            m_wstrContentMsoTempFolder = NLGetLongFilePathEx(kwstrContentMsoTempFolder);    // Add attachment from net mapped folder or file server need this.
            bRet = (!m_wstrOETempFolder.empty()) && (!m_wstrOutlookTempFolder.empty() && (!m_wstrContentMsoTempFolder.empty()));
            if (IsNeedUseDragDrop(kemOutlookVersion, kemOutlookImageType, kemPlateformVersion, kemPlateformImageType))
            {
                NLPRINT_DEBUGVIEWLOG(L"NeedUseDragDrop, kemOutlookVersion:[%d], kemOutlookImageType:[%d], kemPlateformVersion:[%d], kemPlateformImageType:[%d]\n", kemOutlookVersion, kemOutlookImageType, kemPlateformVersion, kemPlateformImageType);
                if (IsFullLocalPath(kwstrContentWordTempFolder, true))
                {
                    m_wstrContentWordTempFolder = NLGetLongFilePathEx(kwstrContentWordTempFolder);
                    m_wstrINetCacheContentWordTempFolder = GetINetCacheContentWordTempFolder(m_wstrContentWordTempFolder);
                    bRet &= (!m_wstrContentWordTempFolder.empty());
                }
                else
                {
                    NLPRINT_DEBUGVIEWLOG(L"!!!Parameters Error: ContentWord temp folder is not local full path or empty. ContentWord:[%s]\n", kwstrContentWordTempFolder.c_str());
                }
            }
            else
            {
                m_wstrContentWordTempFolder = L"";
                m_wstrINetCacheContentWordTempFolder = L"";
            }

            if (!bRet)
            {
                NLPRINT_DEBUGVIEWLOG(L"!!!Init Error: OETemp:[%s], OutlookTemp:[%s], ContentWord:[%s], ContentMso:[%s]\n", kwstrOETempFolder.c_str(), kwstrOutlookTempFolder.c_str(), m_wstrContentWordTempFolder.c_str(), m_wstrContentMsoTempFolder.c_str());
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!Parameters Error: OE/Outlook temp folder is not local full path or empty. OETemp:[%s], OutlookTemp:[%s]\n", kwstrOETempFolder.c_str(), kwstrOutlookTempFolder.c_str());
        }
        SetUseDragDropFlag(!m_wstrContentWordTempFolder.empty());
        SetInitFlag(bRet);
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!Logic Error: reinvoke Init or default init flag is wrong\n");
    }

    if (bRet)
    {
        NLPRINT_DEBUGVIEWLOG(L"Init success: OETemfolder:[%s], OutlookTempFolder:[%s], ContentWordTemp:[%s], ContentMso:[%s]\n", m_wstrOETempFolder.c_str(), m_wstrOutlookTempFolder.c_str(), m_wstrContentWordTempFolder.c_str(), m_wstrContentMsoTempFolder.c_str());
    }

    return bRet;
}

void CAttachmentFileMgr::UnInit()
{
    SetInitFlag(false);
}

bool CAttachmentFileMgr::GetAttachmentFilePath(_In_ Outlook::Attachment* pAttachment, _Out_ std::wstring& wstrSourceFileFullPath, _Out_ std::wstring& wstrOETempFileFullPath, _In_ const EMNLOE_ATTACHMENTFROM kemAttachmentFrom,ITEM_TYPE origEmailType) /*const*/
{NLONLY_DEBUG
    bool bRet = false;
    if (NULL != pAttachment)
    {
        // Record the time before save as and then check if the outlook temp file create after this time
        // If so, it not user add case, it is add by forward or third application
        SYSTEMTIME stuCurTime = { 0 };
        GetSystemTime(&stuCurTime);

        // Save as and get OE temp file path
        wstrOETempFileFullPath = InnerSaveAsAttachmentToOETempFolder(pAttachment);
        if (!wstrOETempFileFullPath.empty())
        {
            if ((emAttachmentFromUserAdd == kemAttachmentFrom) || (emAttachmentFromUnknown == kemAttachmentFrom))
            {
                // Get source file path from cache by OE temp file path
                std::wstring wstrOutlookTempFileFullPath = L"";

				logd(L"bug 42690, wstrOETempFileFullPath=%s", wstrOETempFileFullPath.c_str());
                wstrSourceFileFullPath = InnerGetSourceFilePathFromCache(wstrOETempFileFullPath, emFileLogicCaseOESaveAsAttachment, &wstrOutlookTempFileFullPath, origEmailType);
				logd(L"bug 42690, wstrSourceFileFullPath00=%s", wstrSourceFileFullPath.c_str());
				logd(L"bug 42690, wstrOutlookTempFileFullPath=%s", wstrOutlookTempFileFullPath.c_str());
                // Check if the outlook temp file is empty, it maybe msg file
                if ((wstrSourceFileFullPath.empty()) && (wstrOutlookTempFileFullPath.empty()))
                {
                    const unsigned long kulThreadID = GetCurrentThreadId();
                    wstrSourceFileFullPath = GetMsgSrcFilePath(kulThreadID, wstrOETempFileFullPath);    // This function is not const
					logd(L"bug 42690, wstrSourceFileFullPath01=%s", wstrSourceFileFullPath.c_str());
                }

                SYSTEMTIME stuOutlookTempFileAccessTime = { 0 };
                bool bGetSysTime = GetFileSystemTime(wstrOutlookTempFileFullPath, &stuOutlookTempFileAccessTime, 2);
                if (bGetSysTime)
                {
                    if (0 > CompareSysTime(stuCurTime, stuOutlookTempFileAccessTime))   // Need use ">" not ">=", for using auto test tool or some other stools to end email this two time maybe the same.
                    {
                        // The outlook temp file is create after OE try get the attachment file path
                        // This attachment not used add, it can be add by forward, third application or drag old attachment
                        // If current flag is emAttachmentFromUserAdd, OE logic maybe wrong.
                        // If current flag is unknown, but source file path is not empty, the attachment cache is wrong. The cache do not clear cache in right way.
                        NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"Outlook temp file is create after file manager begin to save as the outlook temp file to OE temp folder, this is attachment maybe add by forward, third application or drag old attachment\n");
                        if (emAttachmentFromUserAdd == kemAttachmentFrom)
                        {
                            NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"!!!!!!Logic error, current attachment maybe not add by user but the logic flag is emAttachmentFromUserAdd\n");
                        }
                        if (!wstrSourceFileFullPath.empty())

                        {
							logd(L"bug 42690, !wstrSourceFileFullPath.empty()");
                            NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"!!!!!!Logice error && cache error, current case maybe not source file but we also can get a source file path from cache. The file cache is wrong. Maybe OE donot clear cache in right way or cache manager error\n");

                            // Here just check but not exactly, Add same attachment into Win10 Outlook2016, After Edit forward attachment, ...
                            // If there are some case that OE cannot delete the file cache in right time by right way, open this code to repair. 
                            // Note: open this code need move the function GetAttachmentFilePath "const" declare
                            // InnerDeleteFileFromCache(wstrOutlookTempFileFullPath, emAttachmentFromUserAdd);
                        }
                    }
                    else
                    {
                        // The outlook temp file is create before OE try get the attachment file path
                        // This attachment is add by used or by drag old attachment. (drag old attachment many times, outlook sometimes will create some temp file for attachment.)
                    }
                }
                NLPRINT_DEBUGVIEWLOG(L"SourceFilePath:[%s], OETempFilePath:[%s], OutlookTempFilePath:[%s]\n", wstrSourceFileFullPath.c_str(), wstrOETempFileFullPath.c_str(), wstrOutlookTempFileFullPath.c_str());
            }
            else
            {
                // For forward attachment and drag new attachment from old attachment no source path
                // Third application(Office, adobe, explore) add attachment cannot get source path in OE process
                NLPRINT_DEBUGVIEWLOG(L"Attachment is from forward or third application, no need find source file from cache\n");
            }
            bRet = (emAttachmentFromUserAdd == kemAttachmentFrom) && (wstrSourceFileFullPath.empty()) ? false : true;
        }
    }
    return bRet;
}

bool CAttachmentFileMgr::AddFileIntoCache(_In_ const std::wstring& kwstrInSourceFullPath, _In_ const std::wstring& kwstrInTargetFullPath)
{NLONLY_DEBUG
    bool bRet = false;

    std::wstring wstrOrgSourceFullPath = NLGetLongFilePathEx(kwstrInSourceFullPath);
    const std::wstring kwstrTargetFullPath = NLGetLongFilePathEx(kwstrInTargetFullPath);

    // Get file location and make clean where the source and target come from
    EMNLOE_FILELOCATION emSourceFileLocation = GetFileLocation(wstrOrgSourceFullPath);
    EMNLOE_FILELOCATION emTargetFileLocation = GetFileLocation(kwstrTargetFullPath);

    // Check special case: win10 drag/drop, add attachment from file server
    if (emFileLocationContentMsoTemp == emSourceFileLocation)
    {
        std::wstring wstrRealFileServerSourceFullPath = GetAddDeleteSourcePathFromFileServerCache(wstrOrgSourceFullPath);
        if (!wstrRealFileServerSourceFullPath.empty())
        {
            wstrOrgSourceFullPath = wstrRealFileServerSourceFullPath;
        }
        emSourceFileLocation = emFileLocationUnknown;
    }
    else if ((emFileLocationContentWordTemp == emSourceFileLocation) || (emFileLocationINetCacheContentWord == emSourceFileLocation))
    {
        if (GetUseDragDropFlag())
        {
            // This is Win10 Outlook2016 drag/drop case, need find out really source file path
            unsigned long ulThreadID = ::GetCurrentThreadId();
            std::wstring wstrRealSourceFullPath = GetAndDeleteDropSrcFilePath(ulThreadID, wstrOrgSourceFullPath);
            if (!wstrRealSourceFullPath.empty())
            {
                wstrOrgSourceFullPath = wstrRealSourceFullPath;
            }
            else
            {
                if (emFileLocationINetCacheContentWord == emSourceFileLocation)
                {
                    NLPRINT_DEBUGVIEWLOG(L"Get source file from finally back cache. This is maybe Ctrl + c/v case for win10 x86 outlook2016\n");
                    wstrOrgSourceFullPath = GetSourceFilePathFromFinallyBackCache(wstrOrgSourceFullPath);
                }
            }
            emSourceFileLocation = emFileLocationUnknown;
        }
    }
    const std::wstring& kwstrSourceFullPath = wstrOrgSourceFullPath;
    
    NLPRINT_DEBUGVIEWLOG(L"SourceFilePath:[%s],Location:[%d], TempFilePath:[%s], Location:[%d]\n", kwstrSourceFullPath.c_str(), emSourceFileLocation, kwstrTargetFullPath.c_str(), emTargetFileLocation);

    // Save file path into cache
    switch (emSourceFileLocation)
    {
    case emFileLocationOutlookTemp:
    {
        if (emFileLocationOETemp == emTargetFileLocation)
        {
            // From outlook temp to OE temp
            // It is OE save as attachment case
            bRet = InnerAddFileToCache(kwstrTargetFullPath, kwstrSourceFullPath, emFileLogicCaseOESaveAsAttachment);
        }
        else
        {
            // From outlook temp to other folder, not OE temp
            // It is save as attachment case but not save to OE temp folder
            // This maybe caused by other outlook add-in or user drag attachment from another email. If this also caused by OE, it is an OE logic error
            NLPRINT_DEBUGVIEWLOG(L"!!!Attention: A file copy from Outlook temp folder to another(not OE) temp folder. This maybe cause by other outlook add-in, or user drag attachment from another email, or an OE logic error\n");
        }
        break;
    }
    case emFileLocationOETemp:
    {
        if (emFileLocationOutlookTemp == emTargetFileLocation)
        {
            // From OE temp to outlook temp
            // It is OE add new attachment case
            bRet = InnerAddFileToCache(kwstrSourceFullPath, kwstrTargetFullPath, emFileLogicCaseOEAddAttachment);
        }
        else
        {
            // From OE temp to other folder, not outlook temp
            // It is OE logic case, file cache manager no need  care
            NLPRINT_DEBUGVIEWLOG(L"A file copy from OE temp folder to another(not outlook) temp folder. It is OE logice case, file manager no need care\n");
        }
        break;
    }
    case emFileLocationUnknown:
    {
        if (emFileLocationOutlookTemp == emTargetFileLocation)
        {
            // From unknown folder to outlook temp
            // It is user add attachment case
            bRet = InnerAddFileToCache(kwstrTargetFullPath, kwstrSourceFullPath, emFileLogicCaseUserAddAttachment);
        }
        else
        {
            // From unknown folder to other folder, not outlook temp
            // It is outlook logic case or OE logic case or third add-in logic case, file cache manager no need care
            NLPRINT_DEBUGVIEWLOG(L"A file copy from unknown temp folder to another(not outlook) temp folder. Unknown case file manager no need care\n");
        }
        break;
    }
    default:
    {
        break;
    }
    }
    return bRet;
}

void CAttachmentFileMgr::DeleteFileFromCache(_In_ const std::wstring& kwstrInTempFileFullPath)
{NLONLY_DEBUG
    // If the file path is in OETemp folder we delete OEAdd cache and OESaveAs cache
    // If the file path is in OutlookTemp folder we delete UserAdd cache
    const std::wstring kwstrTempFileFullPath = NLGetLongFilePathEx(kwstrInTempFileFullPath);
    EMNLOE_FILELOCATION emFileLocation = GetFileLocation(kwstrTempFileFullPath);
    if (emFileLocationOETemp == emFileLocation)
    {
        InnerDeleteFileFromCache(kwstrTempFileFullPath, emFileLogicCaseOESaveAsAttachment);
        InnerDeleteFileFromCache(kwstrTempFileFullPath, emFileLogicCaseOEAddAttachment);
        NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"Delete OEAdd and OESaveAs cache, kwstrTempFileFullPath:[%s]\n", kwstrTempFileFullPath.c_str());
    }
    else if (emFileLocationOutlookTemp == emFileLocation)
    {
        InnerDeleteFileFromCache(kwstrTempFileFullPath, emFileLogicCaseUserAddAttachment);
        NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"Delete UserAdd cache, kwstrTempFileFullPath:[%s]\n", kwstrTempFileFullPath.c_str());
    }
    else if (emFileLocationUnknown == emFileLocation)
    {
        InnerDeleteMsgFileCache(kwstrTempFileFullPath);
    }
}

Outlook::Attachment* CAttachmentFileMgr::AddFileInAttachment(_In_ Outlook::Attachments* pAttachments,
    _Inout_ std::wstring& wstrOETempFileFullPath,
    _In_ const std::wstring& kwstrInSourceFileFullPath,
    _In_ const std::wstring& kwstrInOldOETempFileFullPath,
    _In_ const EMNLOE_FILECACHOP kemFileCacheOp)
{NLONLY_DEBUG

    NLPRINT_DEBUGVIEWLOG(L"Parameters: pAttachments:[0x%p], wstrOETempFileFullPath:[%s], kwstrSourceFileFullPath:[%s], kwstrOldOETempFileFullPath:[%s], kemFileCacheOp:[%d]\n",
                            pAttachments, wstrOETempFileFullPath.c_str(), kwstrInSourceFileFullPath.c_str(), kwstrInOldOETempFileFullPath.c_str(), kemFileCacheOp);

    wstrOETempFileFullPath = NLGetLongFilePathEx(wstrOETempFileFullPath);
    const std::wstring kwstrSourceFileFullPath = NLGetLongFilePathEx(kwstrInSourceFileFullPath);
    const std::wstring kwstrOldOETempFileFullPath = NLGetLongFilePathEx(kwstrInOldOETempFileFullPath);

    if (NULL == pAttachments)
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!Parameter error: pAttachments is NULL\n");
        return NULL;
    }

    std::wstring wstrOETempFolder = GetTempFolder(emFileLocationOETemp);
    if (((emFileCacheDelete == kemFileCacheOp) || (emFileCacheUpdate == kemFileCacheOp)) &&
        (!IsFileInSpecifyFolder(kwstrOldOETempFileFullPath, wstrOETempFolder))
       )
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!Parameter logic error: need delete or update cache but donot pass the right path key. kwstrOldOETempFileFullPath:[%s]\n", kwstrOldOETempFileFullPath.c_str());
        return NULL;
    }

    if ((wstrOETempFileFullPath.empty()) || (!PathFileExistsW(wstrOETempFileFullPath.c_str())))
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!wstrOETempFileFullPath is empty or path not exsit. wstrOETempFileFullPath:[%s]\n", wstrOETempFileFullPath.c_str());
        return NULL;
    }

    // Make a right attached file path
    std::wstring wstrNewOETempFileFullPath = wstrOETempFileFullPath;
    if (!kwstrSourceFileFullPath.empty())
    {
        std::wstring wstrNewFileName = GetNewFileNameForAttachment(kwstrSourceFileFullPath, wstrOETempFileFullPath);
        std::wstring wstrTempFileName = GetFileName(wstrOETempFileFullPath);
        if ((!IsFileInSpecifyFolder(wstrOETempFileFullPath, wstrOETempFolder)) || (0 != _wcsicmp(wstrNewFileName.c_str(), wstrTempFileName.c_str())))
        {
            std::wstring wstrUniqueFolder = CreateAUniqueSubFolder(wstrOETempFolder);
            wstrNewOETempFileFullPath = wstrUniqueFolder + wstrNewFileName;
            ::SetLastError(ERROR_SUCCESS);
            if (!::MoveFileExW(wstrOETempFileFullPath.c_str(), wstrNewOETempFileFullPath.c_str(), MOVEFILE_REPLACE_EXISTING))
            {
                NLPRINT_DEBUGVIEWLOG(L"!!!MoveFileExW from [%s] to [%s] failed with last error:[%d]\n", wstrOETempFileFullPath.c_str(), wstrNewOETempFileFullPath.c_str(), ::GetLastError());
                return NULL;
            }
        }
    }
    NLPRINT_DEBUGVIEWLOG(L"NewOETempFileFullPath:[%s], wstrOETempFileFullPath;[%s]", wstrNewOETempFileFullPath.c_str(), wstrOETempFileFullPath.c_str());

    // Add attachment
    Outlook::Attachment* pAttachment = InnerAddFileInAttachment(pAttachments, wstrNewOETempFileFullPath);
    if ((NULL == pAttachment))
    {
        // Add attachment failed, try to revert: move the temp file back
        if (wstrNewOETempFileFullPath != wstrOETempFileFullPath)
        {
            ::SetLastError(ERROR_SUCCESS);
            if (!::MoveFileExW(wstrNewOETempFileFullPath.c_str(), wstrOETempFileFullPath.c_str(), MOVEFILE_REPLACE_EXISTING))
            {
                NLPRINT_DEBUGVIEWLOG(L"!!!MoveFileExW back from [%s] to [%s] failed with last error:[%d]\n", wstrNewOETempFileFullPath.c_str(), wstrOETempFileFullPath.c_str(), ::GetLastError());
                wstrOETempFileFullPath = wstrNewOETempFileFullPath;
            }
        }
        return NULL;
    }
    else
    {
        // Add attachment success, set new temp file path out
        wstrOETempFileFullPath = wstrNewOETempFileFullPath;
    }
    NLPRINT_DEBUGVIEWLOG(L"NewOETempFileFullPath:[%s], wstrOETempFileFullPath;[%s]", wstrNewOETempFileFullPath.c_str(), wstrOETempFileFullPath.c_str());

    // Update or delete cache
    switch (kemFileCacheOp)
    {
    case emFileCacheUpdate:
    {
        // Update user add cache, cache source, outlook temp not always unique in cache
        const std::wstring wstrOutlookTemp = InnerGetFileFromCache(wstrOETempFileFullPath, emFileLogicCaseOEAddAttachment);
        if (!wstrOutlookTemp.empty())
        {
            InnerAddFileToCache(wstrOutlookTemp, kwstrSourceFileFullPath, emFileLogicCaseUserAddAttachment);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!!!!Attention, MSG format file or Cache logic error, do not add file cache for OE add attachment case. Current OE temp:[%s], Source:[%s]\n", wstrOutlookTemp.c_str(), kwstrSourceFileFullPath.c_str());
        }

        // Delete old cache
        if (kwstrOldOETempFileFullPath == wstrOETempFileFullPath)
        {
            // Delete old OE save as cache
            InnerDeleteFileFromCache(kwstrOldOETempFileFullPath, emFileLogicCaseOESaveAsAttachment);
        }
        else
        {
            // Delete old OE add and save as cache
            InnerDeleteFileFromCache(kwstrOldOETempFileFullPath, emFileLogicCaseOEAddAttachment);
            InnerDeleteFileFromCache(kwstrOldOETempFileFullPath, emFileLogicCaseOESaveAsAttachment);
        }

        // If old OE temp file is MSG format, delete source path used times
        InnerDeleteMsgFileCache(kwstrSourceFileFullPath);
        break;
    }
    case emFileCacheDelete:
    {
        DeleteFileFromCache(kwstrOldOETempFileFullPath);
        DeleteFileFromCache(wstrOETempFileFullPath);        // pAttachments->Add will trigger CopyFileW and add cache to m_mapOEAddOETempToOutlookTemp
        break;
    }
    case emFileCacheAddNew:
    {
        const std::wstring wstrOutlookTemp = InnerGetFileFromCache(wstrOETempFileFullPath, emFileLogicCaseOEAddAttachment);
        if (!wstrOutlookTemp.empty())
        {
            InnerAddFileToCache(wstrOutlookTemp, kwstrSourceFileFullPath, emFileLogicCaseUserAddAttachment);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!Cache logic error, do not add file cache for OE add attachment case. Current OE temp:[%s], Source:[%s]\n", wstrOutlookTemp.c_str(), kwstrSourceFileFullPath.c_str());
        }
    }
    default:
    {
        break;
    }
    }

    return pAttachment;
}


/////////////////////////////////tools///////////////////////////////////////////////
std::wstring CAttachmentFileMgr::GetTempFolder(EMNLOE_FILELOCATION kemFileLocation) const
{
    switch (kemFileLocation)
    {
    case emFileLocationOETemp:
    {
        return m_wstrOETempFolder;
    }
    case  emFileLocationOutlookTemp:
    {
        return NLGetLongFilePathEx(m_wstrOutlookTempFolder);
    }
    case emFileLocationContentWordTemp:
    {
        return NLGetLongFilePathEx(m_wstrContentWordTempFolder);
    }
    case emFileLocationContentMsoTemp:
    {
        return NLGetLongFilePathEx(m_wstrContentMsoTempFolder);
    }
    case emFileLocationINetCacheContentWord:
    {
        return NLGetLongFilePathEx(m_wstrINetCacheContentWordTempFolder);
    }
    default:
    {
        break;
    }
    }
    return L"";
}

EMNLOE_FILELOCATION CAttachmentFileMgr::GetFileLocation(_In_ const std::wstring& kwstrFileFullPath) const
{
    const std::wstring kwstrOETempFolder = GetTempFolder(emFileLocationOETemp);
    const std::wstring kwstrOutlookTempFolder = GetTempFolder(emFileLocationOutlookTemp);
    const std::wstring kwstrContentWordTempFolder = GetTempFolder(emFileLocationContentWordTemp);
    const std::wstring kwstrContentMsoTempFolder = GetTempFolder(emFileLocationContentMsoTemp);
    const std::wstring kwstrINetcacheContentWordTempFolder = GetTempFolder(emFileLocationINetCacheContentWord);

    NLPRINT_DEBUGVIEWLOG(L"OETempFolder:[%s], OutlookTempFolder:[%s], ContentWordTempFolder:[%s], wstrContentMsoTempFolder:[%s], wstrINetcacheContentWordTempFolder:[%s]\n", kwstrOETempFolder.c_str(), kwstrOutlookTempFolder.c_str(), kwstrContentWordTempFolder.c_str(), kwstrContentMsoTempFolder.c_str(), kwstrINetcacheContentWordTempFolder.c_str());

    // Judge where the source and target come from
    EMNLOE_FILELOCATION emFileLocation = emFileLocationUnknown;
    if (IsFileInSpecifyFolder(kwstrFileFullPath, kwstrOETempFolder))
    {
        emFileLocation = emFileLocationOETemp;
    }
    else if (IsFileInSpecifyFolder(kwstrFileFullPath, kwstrOutlookTempFolder))
    {
        emFileLocation = emFileLocationOutlookTemp;
    }
    else if (IsFileInSpecifyFolder(kwstrFileFullPath, kwstrContentWordTempFolder))
    {
        emFileLocation = emFileLocationContentWordTemp;
    }
    else if (IsFileInSpecifyFolder(kwstrFileFullPath, kwstrContentMsoTempFolder))
    {
        emFileLocation = emFileLocationContentMsoTemp;
    }
    else if (IsFileInSpecifyFolder(kwstrFileFullPath, kwstrINetcacheContentWordTempFolder))
    {
        emFileLocation = emFileLocationINetCacheContentWord;
    }
    return emFileLocation;
}

bool CAttachmentFileMgr::InnerAddFileToCache(_In_ const std::wstring& kwstrKeyPath, _In_ const std::wstring& kwstrValuePath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase)
{
    bool bRet = true;
    switch (kemFileLogicCase)
    {
    case emFileLogicCaseUserAddAttachment:
    {
        writeLock(m_rwUserAddAttachment);
        m_mapUserAddSourceToOutlookTemp[kwstrKeyPath] = kwstrValuePath;
        break;
    }
    case emFileLogicCaseOESaveAsAttachment:
    {
        writeLock(m_rwOESaveAsAttachment);
        m_mapOESaveAsOutlookTempToOETemp[kwstrKeyPath] = kwstrValuePath;
        break;
    }
    case emFileLogicCaseOEAddAttachment:
    {
        writeLock(m_rwOEAddAttachment);
        m_mapOEAddOETempToOutlookTemp[kwstrKeyPath] = kwstrValuePath;
        break;
    }
    default:
    {
        bRet = false;
        break;
    }
    }
    return bRet;
}

std::wstring CAttachmentFileMgr::InnerGetFileFromCache(_In_ const std::wstring& kwstrKeyPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase) const
{
    std::wstring wstrValuePath = L"";
    bool bGetSuccess = false;
    switch (kemFileLogicCase)
    {
    case emFileLogicCaseUserAddAttachment:
    {
        readLock(m_rwUserAddAttachment);
        bGetSuccess = GetValueFromMapByKey(m_mapUserAddSourceToOutlookTemp, kwstrKeyPath, wstrValuePath);
        break;
    }
    case emFileLogicCaseOESaveAsAttachment:
    {
        readLock(m_rwOESaveAsAttachment);
        bGetSuccess = GetValueFromMapByKey(m_mapOESaveAsOutlookTempToOETemp, kwstrKeyPath, wstrValuePath);
        break;
    }
    case emFileLogicCaseOEAddAttachment:
    {
        readLock(m_rwOEAddAttachment);
        bGetSuccess = GetValueFromMapByKey(m_mapOEAddOETempToOutlookTemp, kwstrKeyPath, wstrValuePath);
        break;
    }
    default:
    {
        break;
    }
    }

    NLPRINT_DEBUGVIEWLOG(L"KeyPath:[%s], ValuePath:[%s], LogicFlag:[%d]", kwstrKeyPath.c_str(), wstrValuePath.c_str(), kemFileLogicCase);
    return bGetSuccess ? wstrValuePath : L"";
}

bool CAttachmentFileMgr::InnerDeleteFileFromCache(_In_ const std::wstring& kwstrKeyPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase)
{
    size_t stEraseNum = 0;
    switch (kemFileLogicCase)
    {
    case emFileLogicCaseUserAddAttachment:
    {
        writeLock(m_rwUserAddAttachment);
        stEraseNum = m_mapUserAddSourceToOutlookTemp.erase(kwstrKeyPath);
        break;
    }
    case emFileLogicCaseOESaveAsAttachment:
    {
        writeLock(m_rwOESaveAsAttachment);
        stEraseNum = m_mapOESaveAsOutlookTempToOETemp.erase(kwstrKeyPath);
        break;
    }
    case emFileLogicCaseOEAddAttachment:
    {
        writeLock(m_rwOEAddAttachment);
        stEraseNum = m_mapOEAddOETempToOutlookTemp.erase(kwstrKeyPath);
        break;
    }
    default:
    {
        break;
    }
    }
    return (1 == stEraseNum);
}

std::wstring CAttachmentFileMgr::InnerGetSourceFilePathFromCache(_In_ const std::wstring& kwstrTempFileFullPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase, _Out_ std::wstring* pwstrOutlookTempFileFullPath, ITEM_TYPE origEmailType) const
{NLONLY_DEBUG
    std::wstring wstrSourceFilePath = L"";
    if (emFileLogicCaseUserAddAttachment == kemFileLogicCase)
    {
        wstrSourceFilePath = InnerGetFileFromCache(kwstrTempFileFullPath, emFileLogicCaseUserAddAttachment);
    }
    else if ((emFileLogicCaseOESaveAsAttachment == kemFileLogicCase) || (emFileLogicCaseOEAddAttachment == kemFileLogicCase))
    {
        std::wstring wstrOutlookTempFileFullPath = InnerGetFileFromCache(kwstrTempFileFullPath, kemFileLogicCase);

		logd(L"[InnerGetSourceFilePathFromCache]wstrOutlookTempFileFullPath=%s", wstrOutlookTempFileFullPath.c_str());

        if (!wstrOutlookTempFileFullPath.empty())
        {
			//logd(L"bug 42690, origEmailType=%d", origEmailType);
			//std::wstring wstrRealOutlookTempFileFullPath = wstrOutlookTempFileFullPath;
			//if (TASK_REQUEST_ITEM == origEmailType || TASK_ITEM == origEmailType){
			//	std::wstring wstrPostfix = wstrOutlookTempFileFullPath.substr(wstrOutlookTempFileFullPath.find_last_of('.'), wstrOutlookTempFileFullPath.length() -		wstrOutlookTempFileFullPath.find_last_of('.'));
			//	std::wstring wstrPrefix = wstrOutlookTempFileFullPath.substr(0, wstrOutlookTempFileFullPath.find_last_of('(') - 1);

			//	wstrRealOutlookTempFileFullPath = wstrPrefix + wstrPostfix;
			//}
			//logd(L"bug 42690, wstrRealOutlookTempFileFullPath=%s, wstrOutlookTempFileFullPath=%s", wstrRealOutlookTempFileFullPath.c_str(),wstrOutlookTempFileFullPath.c_str());
			//wstrSourceFilePath = InnerGetFileFromCache(wstrRealOutlookTempFileFullPath, emFileLogicCaseUserAddAttachment);//InnerGetFileFromCache(wstrOutlookTempFileFullPath, emFileLogicCaseUserAddAttachment);
			
			wstrSourceFilePath = InnerGetFileFromCache(wstrOutlookTempFileFullPath, emFileLogicCaseUserAddAttachment);

            if (NULL != pwstrOutlookTempFileFullPath)
            {
               *pwstrOutlookTempFileFullPath = wstrOutlookTempFileFullPath;
				// *pwstrOutlookTempFileFullPath = wstrRealOutlookTempFileFullPath;
            }
        }
    }
    else
    {
        // Unknown case
    }
    NLPRINT_DEBUGVIEWLOG(L"SourceFilePath:[%s], TempFilePath:[%s], LogicFlag:[%d]", wstrSourceFilePath.c_str(), kwstrTempFileFullPath.c_str(), kemFileLogicCase);
    return wstrSourceFilePath;
}

Outlook::Attachment* CAttachmentFileMgr::InnerAddFileInAttachment(_In_ Outlook::Attachments* pAttachments, _In_ const std::wstring& kwstrFileFullPath) const
{NLONLY_DEBUG
    // Add attachment
    Outlook::Attachment* pAttachment = NULL;
    CComVariant comVarSource(kwstrFileFullPath.c_str());
    CComVariant comVarByVal(Outlook::olByValue);
    CComVariant comVarOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
    HRESULT hr = pAttachments->Add(comVarSource, comVarByVal, comVarOptional, comVarOptional, &pAttachment);
    if (FAILED(hr))
    {
		loge(L"[InnerAddFileInAttachment]File=%s, %#x, pAttachment@%p", kwstrFileFullPath.c_str(), hr, pAttachment);
        pAttachment = NULL;
	}
    return pAttachment;
}

std::wstring CAttachmentFileMgr::InnerSaveAsAttachmentToOETempFolder(_In_ Outlook::Attachment* pAttachment) const
{NLONLY_DEBUG
    std::wstring wstrOETempFileFullPath = L"";
    CComBSTR comBstrFileName = NULL;
    HRESULT hr = pAttachment->get_FileName(&comBstrFileName);
    if (SUCCEEDED(hr) && (NULL != comBstrFileName))
    {
        std::wstring wstrFileName = comBstrFileName;
        std::wstring wstrUniqueFolder = CreateAUniqueSubFolder(GetTempFolder(emFileLocationOETemp));
        if (!wstrUniqueFolder.empty())
        {
            std::wstring wstrNewFileFullPath = wstrUniqueFolder + wstrFileName;
            NLPRINT_DEBUGVIEWLOG(L"New OE temp file full path:[%s]\n", wstrNewFileFullPath.c_str());

            CComBSTR comBstrOETempFileFullPath = wstrNewFileFullPath.c_str();
            hr = pAttachment->SaveAsFile(comBstrOETempFileFullPath);
            if (SUCCEEDED(hr) && PathFileExistsW(wstrNewFileFullPath.c_str()))
            {
                wstrOETempFileFullPath = wstrNewFileFullPath;
            }
            else
            {
                NLPRINT_DEBUGVIEWLOG(L"!!!!!!ERROR: Save as attachment file to OE temp folder failed, hr:[0x%p]\n", hr);
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"Create a unique folder failed\n");
        }
    }
    return wstrOETempFileFullPath;
}

std::wstring CAttachmentFileMgr::GetNewFileNameForAttachment(_In_ const std::wstring& kwstrSourceFilePath, _In_ const std::wstring& kwstrTempFilePath) const
{
    if (kwstrSourceFilePath.empty())
    {
        return kwstrSourceFilePath;
    }

    std::wstring wstrDefaultRetFileName = GetFileName(kwstrSourceFilePath);
    if (kwstrTempFilePath.empty())
    {
        return wstrDefaultRetFileName;
    }

    std::wstring wstrSourceSuffix = GetSuffixFromFileName(kwstrSourceFilePath);
    std::wstring wstrTempSuffix = GetSuffixFromFileName(kwstrTempFilePath);
    if (0 != _wcsicmp(wstrTempSuffix.c_str(), wstrSourceSuffix.c_str()))
    {
        // For bug 34327, do some special for PDF attachment
        if (0 == _wcsicmp(wstrTempSuffix.c_str(), L"pdf"))
        {
            wstrDefaultRetFileName = GetFileNameWithoutSuffix(kwstrSourceFilePath) + L"." + wstrTempSuffix;
        }
        else
        {
            wstrDefaultRetFileName += L"." + wstrTempSuffix;
        }
    }
    return wstrDefaultRetFileName;
}

bool CAttachmentFileMgr::IsNeedUseDragDrop(_In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType)
{
    /* 
        According current test 
        All environment on Win10 needs use DragDrop,
    */
    UNREFERENCED_PARAMETER(kemOutlookImageType);
    UNREFERENCED_PARAMETER(kemOutlookVersion);
    UNREFERENCED_PARAMETER(kemPlateformImageType);
    return (emPlateformWin10 == kemPlateformVersion);
}

bool CAttachmentFileMgr::IsNeedUseKernelBaseCreateFile(_In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType)
{
    /* 
        According current test 
        win10 x86, outlook 2016, x86, win10 x64 outlook 2016 x86 need this.
        
    */
    return (emPlateformWin10 == kemPlateformVersion) /*&& (emImageTypex86 == kemPlateformImageType)*/ && (OUTLOOK_VER_NUM_2016 == kemOutlookVersion) /*&& (emImageTypex86 == kemOutlookImageType)*/;
}

void CAttachmentFileMgr::SetDropSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrInSourceFileFullPath)
{
    writeLock(m_rwDropSrcFiles);
    const std::wstring kwstrSourceFileFullPath = NLGetLongFilePathEx(kwstrInSourceFileFullPath);
    m_mapDropSrcFiles[ulThreadID].push_back(kwstrSourceFileFullPath);
}

void CAttachmentFileMgr::ClearDropSrcFileCache(_In_ const unsigned long ulThreadID)
{NLONLY_DEBUG
    writeLock(m_rwDropSrcFiles);
    m_mapDropSrcFiles[ulThreadID].clear();
}

size_t CAttachmentFileMgr::GetDropSrcFilesCount(_In_ const unsigned long ulThreadID) const
{NLONLY_DEBUG
    readLock(m_rwDropSrcFiles);
    std::list<std::wstring> listSrcFiles;
    bool bRet = GetValueFromMapByKey(m_mapDropSrcFiles, ulThreadID, listSrcFiles);
    return bRet ? listSrcFiles.size() : 0;
}

std::wstring CAttachmentFileMgr::GetAndDeleteDropSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrContenWordTempFilePath)
{NLONLY_DEBUG
    writeLock(m_rwDropSrcFiles);
    NLPRINT_DEBUGVIEWLOG(L"ThreadID:[%d], ContentWord:[%s]\n", ulThreadID, kwstrContenWordTempFilePath.c_str());

    std::wstring wstrSourceFilePath = L"";
    std::list<std::wstring>& listSourceFiles = m_mapDropSrcFiles[ulThreadID];
    if (!listSourceFiles.empty())
    {
        const std::wstring wstrContentWordTempFileName = GetFileName(kwstrContenWordTempFilePath);
        for (std::list<std::wstring>::iterator kItr = listSourceFiles.begin(); kItr != listSourceFiles.end(); ++kItr)
        {
            // In right way, the first item in my cache must be the right path and the file name is the same as the file in content word temp folder
            // But for some special case like: some file already in Content.Word, drag/drop the same source to Outlook2016 as attachment multiple times
            const std::wstring wstrSrcFileName = GetFileName((*kItr));
            NLPRINT_DEBUGVIEWLOG(L"wstrSrcFileName:[%s], wstrContentWordTempFileName:[%s]\n", wstrSrcFileName.c_str(), wstrContentWordTempFileName.c_str());

            int nCmp = _wcsicmp(wstrSrcFileName.c_str(), wstrContentWordTempFileName.c_str());
            if (0 == nCmp)
            {
                wstrSourceFilePath = *kItr;
                listSourceFiles.erase(kItr);
                break;
            }
        }
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!!!!Attention: we need used drag drop source cache but the cache is empty. This maybe some exception happened in AttachmentFileMgr or some special case like: user drag attachment from another email\n");
    }
    return wstrSourceFilePath;
}

void CAttachmentFileMgr::AddSourceFilesInDropedCache(_In_ const unsigned long ulThreadID, _In_ IDataObject *pDataObject)
{NLONLY_DEBUG
    // Init cache
    ClearDropSrcFileCache(ulThreadID);

    // Set cache
    FORMATETC formatetc = { CF_HDROP, NULL, DVASPECT_CONTENT, 2, TYMED_HGLOBAL };
    if ( pDataObject && SUCCEEDED(pDataObject->QueryGetData (&formatetc)) )
    {
        FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stgm = { 0 };
        if (SUCCEEDED(pDataObject->GetData(&fmte, &stgm))) 
        {
            HDROP hdrop = reinterpret_cast<HDROP>(stgm.hGlobal);
            unsigned int unFileCount = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
            NLPRINT_DEBUGVIEWLOG(L"File count:[%d]\n", unFileCount);

            const std::wstring kwstrContentWordTempFolder = GetTempFolder(emFileLocationContentWordTemp);
            const std::wstring kwstrINetCacheContentWordTempFolder = GetTempFolder(emFileLocationINetCacheContentWord);
            for (unsigned int i = 0; i< unFileCount; ++i)
            {
                // Get file path length
                const unsigned int unLength = DragQueryFile(hdrop, i, NULL, 0)+1; // include '\0'
                wchar_t* pwchFilePath = new wchar_t[unLength];
                wmemset(pwchFilePath, 0, unLength);
                
                // Get file path
                DragQueryFile(hdrop, i, pwchFilePath, unLength);

                // Check and repair content word temp folder
                CheckAndRepairContentWordFoler(kwstrContentWordTempFolder, pwchFilePath);
                CheckAndRepairContentWordFoler(kwstrINetCacheContentWordTempFolder, pwchFilePath);

                // For win10 drag MSG format file to outlook attachment, we need cache the MSG source file path here.
                if (IsMsgFile(pwchFilePath) && IsWin10())
                {
                    HANDLE hFile = NLCALLNEXTCREATEFILEFUNCTION(pwchFilePath,GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
                    SetMsgSrcFilePath(hFile, pwchFilePath);
                    ::CloseHandle(hFile);
                }

                SetDropSrcFilePath(ulThreadID, pwchFilePath);
                NLPRINT_DEBUGVIEWLOG(L"DropFiles Files[%d]:[%s]\n", i, pwchFilePath);

                // Delete
                delete[] pwchFilePath;
                pwchFilePath = NULL;
            }
        }
        ReleaseStgMedium(&stgm);
    }
}

std::wstring CAttachmentFileMgr::GetAddDeleteSourcePathFromFileServerCache(_In_ const std::wstring& kwstrContentMsoTempFileFullPath)
{
    writeLock(m_rwUserAddAttachmentFromFileServer);
    const std::wstring kwstrSourceFullPath = m_mapUserAddSourceToContentMsoTemp[kwstrContentMsoTempFileFullPath];
    m_mapUserAddSourceToContentMsoTemp.erase(kwstrContentMsoTempFileFullPath);

    return kwstrSourceFullPath;
}

void CAttachmentFileMgr::AddSourceFilesInFileServerCache(_In_ const std::wstring& kwstrFileServerSourceFullPath, _In_ const std::wstring& kwstrContentMsoTempFullPath)
{
    // Check if kwstrContentMsoTempFullPath is the content mso temp path
    const std::wstring kwstrContentMsoTempFolder = GetTempFolder(emFileLocationContentMsoTemp);
    if (IsFileInSpecifyFolder(kwstrContentMsoTempFullPath, kwstrContentMsoTempFolder))
    {
        writeLock(m_rwUserAddAttachmentFromFileServer);
        m_mapUserAddSourceToContentMsoTemp[kwstrContentMsoTempFullPath] = kwstrFileServerSourceFullPath;
    }
}

void CAttachmentFileMgr::InnerDeleteMsgFileCache(_In_ const std::wstring& kwstrMsgSrcFilePath)
{
    if (IsMsgFile(kwstrMsgSrcFilePath))
    {
        // Ignore temp folder: OE Temp, Outlook Temp, Content.Word temp, Content.Mso temp
        const std::wstring kwstrSourceFileFullPath = NLGetLongFilePathEx(kwstrMsgSrcFilePath);
        EMNLOE_FILELOCATION emFileLocation = GetFileLocation(kwstrSourceFileFullPath);
        if (emFileLocationUnknown == emFileLocation)
        {
            const unsigned long kulThreadID = GetCurrentThreadId();
            writeLock(m_rwMsgSrcFiles);
            std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >& vecInnerSrcFiles = m_mapMsgSrcFiles[kulThreadID];
            for (std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >::reverse_iterator kitr = vecInnerSrcFiles.rbegin(); kitr != vecInnerSrcFiles.rend(); ++kitr)
            {
                if (0 == _wcsicmp(kwstrMsgSrcFilePath.c_str(), (kitr->first).c_str()))
                {
                    STUNLOE_MSGFILECATCHINFO& kstuMsgFileCatchInfo = kitr->second;
                    if (0 == kstuMsgFileCatchInfo.nUsedTimes)
                    {
                        // Note: vector erase only can delete iterator and return the next iterator, cannot delete reverse_iterator directly
                        vecInnerSrcFiles.erase(--(kitr.base())); //--(kitr.base()) is the element we need to delete!!!
                    }
                    else
                    {
                        --kstuMsgFileCatchInfo.nUsedTimes;
                    }
                    break;
                }
            }
        }
    }
}

void CAttachmentFileMgr::SetMsgSrcFilePath(_In_ HANDLE hFile, _In_ const std::wstring& kwstrInSourceFileFullPath)
{
    // Check parameter
    if ((INVALID_HANDLE_VALUE == hFile) || (kwstrInSourceFileFullPath.empty()))
    {
        return ;
    }

    // Check if it is MSG file.
    if (IsMsgFile(kwstrInSourceFileFullPath))
    {
        // Ignore temp folder: OE Temp, Outlook Temp, Content.Word temp, Content.Mso temp
        const std::wstring kwstrSourceFileFullPath = NLGetLongFilePathEx(kwstrInSourceFileFullPath);
        EMNLOE_FILELOCATION emFileLocation = GetFileLocation(kwstrSourceFileFullPath);
        if (emFileLocationUnknown == emFileLocation)
        {
            // Get thread ID
            const unsigned long ulThreadID = GetCurrentThreadId();

            // Get file size
            STUNLOE_MSGFILECATCHINFO stuMsgFileCatchInfo(0, 0, L"", L"");
            stuMsgFileCatchInfo.ulFileSize = GetFileSize(hFile, NULL);
            stuMsgFileCatchInfo.wstrFileName = GetFileName(kwstrSourceFileFullPath);
            NLPRINT_DEBUGVIEWLOG(L"Msg file add, thread id:[%d], file path:[%s], ulFileSize:[%u], file name:[%s]\n", ulThreadID, kwstrSourceFileFullPath.c_str(), stuMsgFileCatchInfo.ulFileSize, stuMsgFileCatchInfo.wstrFileName.c_str());

            {
                // add source file info into cache
                writeLock(m_rwMsgSrcFiles);
                bool bFind = false;
                std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >& vecInnerSrcFiles = m_mapMsgSrcFiles[ulThreadID];
                for (std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >::reverse_iterator kitr = vecInnerSrcFiles.rbegin(); kitr != vecInnerSrcFiles.rend(); ++kitr)
                {
                    if (0 == _wcsicmp((kitr->first).c_str(), kwstrSourceFileFullPath.c_str()))
                    {
                        (kitr->second).Copy(stuMsgFileCatchInfo);
                        bFind = true;
                        break;
                    }
                }
                if (!bFind)
                {
                    if (vecInnerSrcFiles.size() >= m_kstMaxMsgSrcFileCacheSize)
                    {
                        // If the Msg source file cache is over the max cache size, remove the first half cache.
                        std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >::iterator itrHalf = vecInnerSrcFiles.begin();
                        const size_t kstHalfCacheSize = (vecInnerSrcFiles.size() / 2) ;
                        for (size_t stIndex = 0; stIndex < kstHalfCacheSize; ++itrHalf, ++stIndex)
                        {
                            // continue, just want to get the iterator which specify the item in the middle of the vector.
                        }
                        vecInnerSrcFiles.erase(vecInnerSrcFiles.begin(), itrHalf);
                    }
                    vecInnerSrcFiles.push_back(std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO>(kwstrSourceFileFullPath, stuMsgFileCatchInfo));
                }
            }
        }
    }
}

std::wstring CAttachmentFileMgr::GetMsgSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrInOESaveAsTempFileFullPath)
{NLONLY_DEBUG
    NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"OE save as path:[%s]\n", kwstrInOESaveAsTempFileFullPath.c_str());
    /*
        We compare the source file by the file size, cannot use file name or attachment display name.
        If the file is msg format, outlook read the file and get display name from msg file content.
        e.g.:
            1. an msg format file "RE OE 8.1 Pilot.msg" add as an attachment, the display name is "RE:OE 8.1 Pilot"
            2. rename source msg ""RE OE 8.1 Pilot.msg"" as "AAAA.nxl", the display name also is "RE:OE 8.1 Pilot"
        That's why we cannot use file name and display name to compare the source path.
    */
    std::wstring wstrRetSrcFilePath = L"";
    // Check if it is MSG file
    if (IsMsgFile(kwstrInOESaveAsTempFileFullPath))
    {
        const std::wstring kwstrOESaveAsTempFileFullPath = NLGetLongFilePathEx(kwstrInOESaveAsTempFileFullPath);
        EMNLOE_FILELOCATION emFileLocation = GetFileLocation(kwstrOESaveAsTempFileFullPath);
        if (emFileLocationOETemp == emFileLocation)
        {
            // Get OE temp file size and file name
            const unsigned long kulOETempFileSize = NLGetFileSize(kwstrOESaveAsTempFileFullPath, true);
            const std::wstring kwstrOETempFileName = GetFileName(kwstrOESaveAsTempFileFullPath);

            {
                writeLock(m_rwMsgSrcFiles);
                std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >& vecInnerSrcFiles = m_mapMsgSrcFiles[ulThreadID];
                size_t stMatchedIndex = vecInnerSrcFiles.size();
                EMNLOE_MATCHLEVEL emSrcMatchLevel = emMatchLevelUnmatched;

                std::pair<std::wstring, EMNLOE_MATCHLEVEL> prFirstMatchRetFilePath = NLMatchMsgSourceFilePath(vecInnerSrcFiles, kulOETempFileSize, kwstrOETempFileName, 1, stMatchedIndex); // firs compare int the items which never matched before 
                wstrRetSrcFilePath = prFirstMatchRetFilePath.first;
                emSrcMatchLevel = prFirstMatchRetFilePath.second;
                if (emMatchLevel1 < emSrcMatchLevel)
                {
                    size_t stSecondMatchedIndex = vecInnerSrcFiles.size();
                    std::pair<std::wstring, EMNLOE_MATCHLEVEL> prSecondMatchRetFilePath = NLMatchMsgSourceFilePath(vecInnerSrcFiles, kulOETempFileSize, kwstrOETempFileName, 2, stSecondMatchedIndex); // firs compare int the items which never matched before 
                    if (prSecondMatchRetFilePath.second < prFirstMatchRetFilePath.second)
                    {
                        wstrRetSrcFilePath = prSecondMatchRetFilePath.first;
                        emSrcMatchLevel = prSecondMatchRetFilePath.second;
                        stMatchedIndex = stSecondMatchedIndex;
                    }
                }

                if (emMatchLevelUnmatched == emSrcMatchLevel)
                {
                    wstrRetSrcFilePath = L"";
                }
                else
                {
                    // Update item match times
                    if ((!wstrRetSrcFilePath.empty()) && ( 0 <= stMatchedIndex) && (vecInnerSrcFiles.size() > stMatchedIndex))
                    {
                        ++(vecInnerSrcFiles[stMatchedIndex].second.nUsedTimes);
                    }
                }
            }
        }
    }
    NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"Real file path:[%s]\n", wstrRetSrcFilePath.c_str());
    return wstrRetSrcFilePath;
}

bool CAttachmentFileMgr::IsMsgFile(_In_ const std::wstring& kwstrFileFullPath) const
{
    // In fact we need check MSG format, but here we just consider the suffix
    std::wstring wstrFileSuffix = GetSuffixFromFileName(kwstrFileFullPath);
    return (0 == _wcsicmp(wstrFileSuffix.c_str(), L"msg"));
}

unsigned long CAttachmentFileMgr::NLGetFileSize(_In_ const std::wstring& kwstrFileFullPath, _In_ bool bUseHookNextCreateFileFunction) const
{
    // Get file handle
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if (bUseHookNextCreateFileFunction)
    {
        hFile = NLCALLNEXTCREATEFILEFUNCTION(kwstrFileFullPath.c_str(),GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
    }
    else
    {
        hFile = CreateFileW(kwstrFileFullPath.c_str(),GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
    }

    unsigned long ulFileSize = 0;
    if (hFile != INVALID_HANDLE_VALUE)
    {
        ulFileSize = GetFileSize(hFile,NULL);
        CloseHandle(hFile);
    }
    return ulFileSize;
}

// knCompareModule: 0 compare all, 1 only compare never match items, 2 only compare the item which matched before
// return <file path, match level>, match level: 0: exact match, 1->10 high -> low,
std::pair<std::wstring, EMNLOE_MATCHLEVEL> CAttachmentFileMgr::NLMatchMsgSourceFilePath(_In_ const std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >& kvecInnerSrcFiles, _In_ const unsigned long kulOETempFileSize, _In_ const std::wstring& kwstrOETempFileName, _In_ const int knCompareModel, _Out_ size_t& stMatchedIndex) const
{
    std::pair<std::wstring, EMNLOE_MATCHLEVEL> pairRetFilePath(L"", emMatchLevelUnmatched);

    int nMatchedItemUsedTimes = -1;
    size_t stIndex = kvecInnerSrcFiles.size();
    for (std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >::const_reverse_iterator kitr = kvecInnerSrcFiles.rbegin(); kitr != kvecInnerSrcFiles.rend(); ++kitr, --stIndex)
    {
        const STUNLOE_MSGFILECATCHINFO& kstuMsgFileCatchInfo = kitr->second;

        // Check compare model
        if (1 == knCompareModel)
        {
            if (0 != kstuMsgFileCatchInfo.nUsedTimes)
            {
                continue;
            }
        }
        else if (2 == knCompareModel)
        {
            if (0 == kstuMsgFileCatchInfo.nUsedTimes)
            {
                continue;
            }
        }

        // Compare file size, matched level 1, Compare file size, matched level 2, Both matched level 0
        bool bMatchedFileName = (0 == _wcsicmp(kwstrOETempFileName.c_str(), kstuMsgFileCatchInfo.wstrFileName.c_str()));
        bool bMatchedFileSize = (kulOETempFileSize == kstuMsgFileCatchInfo.ulFileSize);
        EMNLOE_MATCHLEVEL emMatchLevel = emMatchLevelUnmatched;
        if (bMatchedFileSize)
        {
            emMatchLevel = bMatchedFileName ? emMatchLevelExact : emMatchLevel1;
        }
        else
        {
            emMatchLevel = bMatchedFileName ? emMatchLevel2 : emMatchLevelUnmatched;
        }

        // First we use the highest similarity item, second if the  similarity is the same, we use the item contains the least used times.
        if ((pairRetFilePath.second > emMatchLevel) || ((pairRetFilePath.second == emMatchLevel) && (kstuMsgFileCatchInfo.nUsedTimes < nMatchedItemUsedTimes)))
        {
            pairRetFilePath.first = kitr->first;
            pairRetFilePath.second = emMatchLevel;
            stMatchedIndex = stIndex-1;
            nMatchedItemUsedTimes = kstuMsgFileCatchInfo.nUsedTimes;
        }

        if (emMatchLevelExact == pairRetFilePath.second)
        {
            break;
        }
    }

    NLPRINT_DEBUGVIEWLOGEX(nldebug::g_kbForceOutputDebugLog, L"FilePath[%s], MatchLevel:[%d], index:[%d]\n", pairRetFilePath.first.c_str(), pairRetFilePath.second, stMatchedIndex);
    return pairRetFilePath;
}

std::wstring CAttachmentFileMgr::GetINetCacheContentWordTempFolder(_In_ const std::wstring& kwstrContentWordFolder)
{
    std::wstring wstrTempContentWordFolderWithoutLastSep = kwstrContentWordFolder;
    size_t stLength = kwstrContentWordFolder.length();
    if ('\\' == kwstrContentWordFolder[stLength-1])
    {
        stLength -= 1;
        wstrTempContentWordFolderWithoutLastSep = kwstrContentWordFolder.substr(0, stLength);
    }
    size_t stLastSepPos = wstrTempContentWordFolderWithoutLastSep.rfind('\\');
    if (std::wstring::npos != stLastSepPos)
    {
        return wstrTempContentWordFolderWithoutLastSep.substr(0, stLastSepPos) + wstrTempContentWordFolderWithoutLastSep.substr(stLastSepPos + 1, stLength - stLastSepPos) + L"\\";
    }
    return L"";
}

void CAttachmentFileMgr::CheckAndRepairContentWordFoler(_In_ const std::wstring& kwstrContentWordTempFolder, _In_ wchar_t* pwchFilePath)
{
    if (!kwstrContentWordTempFolder.empty())
    {
        std::wstring wstrContentWordTempFile = kwstrContentWordTempFolder + GetFileName(pwchFilePath);
        if (PathFileExistsW(wstrContentWordTempFile.c_str()))
        {
            if (0 != _wcsicmp(wstrContentWordTempFile.c_str(), pwchFilePath))
            {
                NLPRINT_DEBUGVIEWLOG(L"ContenWord temp folder already has a temp file wstrContentWordTempFile:[%s], OE delete it.\n", wstrContentWordTempFile.c_str());
                DeleteFolderOrFile(wstrContentWordTempFile.c_str(), false);
            }
            else
            {
                NLPRINT_DEBUGVIEWLOG(L"User drag/drop a file:[%s] from Content.Word, OE cannot get the attachment source file path in right way.\n", pwchFilePath);
            }
        }
    }
}

bool CAttachmentFileMgr::IsNeedSetIntoFinallyBackCache(_In_ const std::wstring& kwstrSourceFileFullPath)
{
    bool bNeedSet = true;
    EMNLOE_FILELOCATION emFileLocation = GetFileLocation(kwstrSourceFileFullPath);
    if (emFileLocationUnknown == emFileLocation)
    {
        std::wstring wstrFileSuffix = GetSuffixFromFileName(kwstrSourceFileFullPath);
        if ((0 == _wcsicmp(wstrFileSuffix.c_str(), L"ost")) || (0 == _wcsicmp(wstrFileSuffix.c_str(), L"pst")))
        {
            bNeedSet = false;
        }
        else if(NULL != wcsstr(kwstrSourceFileFullPath.c_str(), L"Google Desktop"))  // SkipEvaluationOnGDS
        {
            bNeedSet = false;
        }
        else if(0==wcsnicmp(L"\\\\.\\", kwstrSourceFileFullPath.c_str(), 4)
            || 0==_wcsicmp(L"C:\\WINNT\\system32\\mlang.dat", kwstrSourceFileFullPath.c_str())
            || 0==_wcsicmp(L"C:\\WINNT\\system32\\STDOLE2.TLB", kwstrSourceFileFullPath.c_str()))
        {
            bNeedSet = false;
        }
        else
        {
            bNeedSet = true;
        }
    }
    else
    {
        bNeedSet = false;
    }
    return bNeedSet;
}

void CAttachmentFileMgr::SetFilePathIntoFinallyBackCache(_In_ HANDLE hFile, _In_ const std::wstring& kwstrInSourceFilePath)
{
    // Check parameter
    if ((INVALID_HANDLE_VALUE == hFile) || (kwstrInSourceFilePath.empty()))
    {
        return ;
    }

    const std::wstring kwstrSourceFileFullPath = NLGetLongFilePathEx(kwstrInSourceFilePath);
    if (IsMsgFile(kwstrSourceFileFullPath))
    {
        SetMsgSrcFilePath(hFile, kwstrSourceFileFullPath);
    }
    else
    {
        if (IsNeedSetIntoFinallyBackCache(kwstrSourceFileFullPath))
        {
            writeLock(m_rwSourcePathFinallyBackCache);
            if (m_vecSourcePathFinallyBackCache.size() >= m_kstMaxSourcePathFinallyBackCache)
            {
                // If the Msg source file cache is over the max cache size, remove the first half cache.
                std::vector<std::wstring>::iterator itrHalf = m_vecSourcePathFinallyBackCache.begin();
                const size_t kstHalfCacheSize = (m_vecSourcePathFinallyBackCache.size() / 2) ;
                for (size_t stIndex = 0; stIndex < kstHalfCacheSize; ++itrHalf, ++stIndex)
                {
                    // continue, just want to get the iterator which specify the item in the middle of the vector.
                }
                m_vecSourcePathFinallyBackCache.erase(m_vecSourcePathFinallyBackCache.begin(), itrHalf);
            }
            m_vecSourcePathFinallyBackCache.push_back(kwstrSourceFileFullPath);
        }
    }
}

std::wstring CAttachmentFileMgr::GetSourceFilePathFromFinallyBackCache(_In_ const std::wstring& kwstrOETempFilePath)
{NLONLY_DEBUG
    std::wstring wstrSourceFilePath = L"";
    size_t stMatchedIndex = 0;
    unsigned long ulTempFileSize = NLGetFileSize(kwstrOETempFilePath, true);

    const std::wstring kwstrTempFileName = GetFileName(kwstrOETempFilePath);
    std::vector<std::pair<std::wstring, size_t> > vecMatchFiles;

    // Matched file name first
    writeLock(m_rwSourcePathFinallyBackCache);
    for (size_t stIndex = 0; stIndex < m_vecSourcePathFinallyBackCache.size(); ++stIndex)
    {
        if (0 == _wcsicmp(kwstrTempFileName.c_str(), (GetFileName(m_vecSourcePathFinallyBackCache[stIndex])).c_str()))
        {
            vecMatchFiles.push_back(std::pair<std::wstring, size_t>(m_vecSourcePathFinallyBackCache[stIndex], stIndex));
        }
    }

    if (vecMatchFiles.empty())
    {
        // Match file size
        for (size_t stIndex = 0; stIndex < m_vecSourcePathFinallyBackCache.size(); ++stIndex)
        {
            if (ulTempFileSize == NLGetFileSize(m_vecSourcePathFinallyBackCache[stIndex], true))
            {
                wstrSourceFilePath = m_vecSourcePathFinallyBackCache[stIndex];
                stMatchedIndex = stIndex;
                break;
            }
        }
    }
    else
    {
        wstrSourceFilePath = vecMatchFiles[0].first;
        stMatchedIndex = vecMatchFiles[0].second;

        if (1 < vecMatchFiles.size())
        {
            // Match file size
            for (size_t stIndex = 0; stIndex < vecMatchFiles.size(); ++stIndex)
            {
                // Get file size
                if (ulTempFileSize == NLGetFileSize(vecMatchFiles[stIndex].first, true))
                {
                    wstrSourceFilePath = vecMatchFiles[stIndex].first;
                    stMatchedIndex = vecMatchFiles[stIndex].second;
                    break;
                }
            }
        }
    }

    if (!wstrSourceFilePath.empty() && (stMatchedIndex < m_vecSourcePathFinallyBackCache.size()))
    {
        m_vecSourcePathFinallyBackCache.erase(m_vecSourcePathFinallyBackCache.begin() + stMatchedIndex);
    }
    return wstrSourceFilePath;
}