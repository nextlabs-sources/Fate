#pragma once

#include <string>
#include <map>
#include "NLType.h"

class CAttachmentFileMgr
{
private:
    CAttachmentFileMgr();
    ~CAttachmentFileMgr();

public:
    static CAttachmentFileMgr& GetInstance();

    // Init
    bool GetInitFlag();
private:
    void SetInitFlag(_In_ bool bInit);

    // function
public:
    /**
    *  @brief this function is used for initialize file cache
    *  This function must be invoked before we invoke other functions.
    *  It must be only invoked once.
    *  The thread safety: unsafe.
    *
    *  @param kwstrOETempFolder [IN] this is a temp folder full path which create and used by NEXTLABS OE. The folder path must end with "\".
    *  @param kwstrOutlookTempFolder [IN] this is a temp folder full path which create and used by outlook. The folder path must end with "\".
    *  @retval true means success. If user already invoked this function to initialize file cache or parameters error it will return false.
    */
    bool Init(_In_ const std::wstring& kwstrOETempFolder, _In_ const std::wstring& kwstrOutlookTempFolder, _In_ const std::wstring& kwstrContentWordTempFolder, _In_ const std::wstring& kwstrContentMsoTempFolder,
              _In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType);
    /**
    *  @brief uninitialized file cache
    *
    *  @retval void.
    */
    void UnInit();
    /**
    *  @brief Get the specify attachment file path by the attachment COM object.
    *
    *  @param pAttachment            [IN]  the attachment object pointer.
    *  @param wstrSourceFileFullPath [OUT] out the attachment real source file path. 
    *                                      If current attachment is add by forward, drag old attachment or third application, no source path and this out source path will be empty.
    *                                      If the cache do not clear in the right way, for these three case, it maybe out a wrong source file path.
    *  @param wstrOETempFileFullPath [OUT] out an temp file path in OE temp folder which is the same as the attachment outlook temp file.
    *  @param emAttachmentFromUnknown [IN] the attachment add from "User", "Forward", "Third Application" or "Unknown"
    *  @retval true means get attachment path success, otherwise failed with some errors.
    */
    bool GetAttachmentFilePath(_In_ Outlook::Attachment* pAttachment, _Out_ std::wstring& wstrSourceFileFullPath, _Out_ std::wstring& wstrOETempFileFullPath, _In_ const EMNLOE_ATTACHMENTFROM kemAttachmentFrom = emAttachmentFromUnknown, ITEM_TYPE origEmailType = MAIL_ITEM) /*const*/;
    /**
    *  @brief add file path into cache. In hook CopyFileW invoked this function, OE main logic no need care this.
    *
    *  @param kwstrSourceFullPath [IN] source file path where the file from.
    *  @param kwstrTargetFullPath [IN] target file path where the file to.
    *  @retval true success otherwise failed or the file path is not attachment (source/outlook temp/OE temp) file path.
    */
    bool AddFileIntoCache(_In_ const std::wstring& kwstrInSourceFullPath, _In_ const std::wstring& kwstrInTargetFullPath);
    /**
    *  @brief delete file cache.
    *
    *  @param wstrOETempFileFullPath [IN] OE temp file path which out by GetAttachmentFilePath.
    *  @retval void.
    */
    void DeleteFileFromCache(_In_ const std::wstring& wstrInTempFileFullPath);
    /**
    *  @brief add attachment to attachments.
    *
    *  @param pAttachments [IN]               outlook attachments object
    *  @param wstrOETempFileFullPath  [INOUT] a new file path which need add to attachments. If the file is not in OE temp folder it will be moved to OE temp folder and out the new file path.
    *  @param kwstrSourceFileFullPath    [IN] new attachment real source file path.
    *  @param kwstrOldOETempFileFullPath [IN] old attachment OE temp file path which out by GetAttachmentFilePath. This is used for delete and update file path.
    *  @param kemFileCacheOp             [IN] cache operator: "Add New", "Update", "Delete" or "Nothing to do".
    *  @retval the new attachment object pointer, if failed the pointer is NULL.
    */
    Outlook::Attachment* AddFileInAttachment(_In_ Outlook::Attachments* pAttachments,
                      _Inout_ std::wstring& wstrOETempFileFullPath,
                      _In_ const std::wstring& kwstrInSourceFileFullPath,
                      _In_ const std::wstring& kwstrInOldOETempFileFullPath,
                      _In_ const EMNLOE_FILECACHOP kemFileCacheOp);

    // For drag drop: Win10 Outlook2016 x86, Win10 Outlook2013 x64
    void AddSourceFilesInDropedCache(_In_ const unsigned long ulThreadID, _In_ IDataObject *pDataObject);
    size_t GetDropSrcFilesCount(_In_ const unsigned long ulThreadID) const;
    static bool IsNeedUseDragDrop(_In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType);

    // For attachment add from net mapped folder or file server file
    void AddSourceFilesInFileServerCache(_In_ const std::wstring& kwstrFileServerSourceFullPath, _In_ const std::wstring& kwstrContentMsoTempFullPath);

    // For msg format file
    void SetMsgSrcFilePath(_In_ HANDLE hFile, _In_ const std::wstring& kwstrInSourceFileFullPath);

    // For win10 x86, outlook2016 x86, Ctrl + c/v
    static bool IsNeedUseKernelBaseCreateFile(_In_ const EMNLOE_OUTLOOKVERSION kemOutlookVersion, _In_ const EMNLOE_IMAGETYPE kemOutlookImageType, _In_ const EMNLOE_PLATEFORMVERSION kemPlateformVersion, _In_ const EMNLOE_IMAGETYPE kemPlateformImageType);
    void SetFilePathIntoFinallyBackCache(_In_ HANDLE hFile, _In_ const std::wstring& kwstrInSourceFilePath);

    // Tools
private:
    std::wstring GetTempFolder(_In_ const EMNLOE_FILELOCATION kemFileLocation) const;
    EMNLOE_FILELOCATION GetFileLocation(_In_ const std::wstring& kwstrFileFullPath) const;
    
    bool InnerAddFileToCache(_In_ const std::wstring& kwstrKeyPath, _In_ const std::wstring& kwstrValuePath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase);
    std::wstring InnerGetFileFromCache(_In_ const std::wstring& kwstrKeyPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase) const;
    bool InnerDeleteFileFromCache(_In_ const std::wstring& kwstrKeyPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase);

    std::wstring InnerGetSourceFilePathFromCache(_In_ const std::wstring& kwstrTempFileFullPath, _In_ const EMNLOE_FILELOGICCASE kemFileLogicCase, _Out_ std::wstring* pwstrOutlookTempFileFullPath = NULL, ITEM_TYPE origEmailType = MAIL_ITEM) const;
    
    Outlook::Attachment* InnerAddFileInAttachment(_In_ Outlook::Attachments* pAttachments, _In_ const std::wstring& kwstrFileFullPath) const;
    std::wstring InnerSaveAsAttachmentToOETempFolder(_In_ Outlook::Attachment* pAttachment) const;
    std::wstring GetNewFileNameForAttachment(_In_ const std::wstring& kwstrSourceFilePath, _In_ const std::wstring& kwstrTempFilePath) const;

    void SetUseDragDropFlag(_In_ bool bUse);
    bool GetUseDragDropFlag();
    void SetDropSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrSourceFileFullPath);
    std::wstring GetAndDeleteDropSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrContenWordTempFilePath);
    void ClearDropSrcFileCache(_In_ const unsigned long ulThreadID);

    std::wstring GetAddDeleteSourcePathFromFileServerCache(_In_ const std::wstring& kwstrContentMsoTempFileFullPath);

    std::wstring GetMsgSrcFilePath(_In_ const unsigned long ulThreadID, _In_ const std::wstring& kwstrInOESaveAsTempFileFullPath);
    void InnerDeleteMsgFileCache(_In_ const std::wstring& kwstrMsgSrcFilePath);

    bool IsNeedSetIntoFinallyBackCache(_In_ const std::wstring& kwstrSourceFileFullPath);
    std::wstring GetSourceFilePathFromFinallyBackCache(_In_ const std::wstring& kwstrOETempFilePath);

    // Independent tools
private:
    bool  IsMsgFile(_In_ const std::wstring& kwstrFileFullPath) const;
    unsigned long NLGetFileSize(_In_ const std::wstring& kwstrFileFullPath, _In_ bool bUseHookNextCreateFileFunction) const;
    std::pair<std::wstring, EMNLOE_MATCHLEVEL> NLMatchMsgSourceFilePath(_In_ const std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> >& kvecInnerSrcFiles, _In_ const unsigned long kulOETempFileSize, _In_ const std::wstring& kwstrOETempFileName, _In_ const int knCompareModel, _Out_ size_t& stMatchedIndex) const;
    std::wstring GetINetCacheContentWordTempFolder(_In_ const std::wstring& kwstrContentWordFolder);
    void CheckAndRepairContentWordFoler(_In_ const std::wstring& kwstrContentWordTempFolder, _In_ wchar_t* pwchFilePath);

private:
    rwmutex m_rwUserAddAttachment;
    std::map<std::wstring, std::wstring> m_mapUserAddSourceToOutlookTemp;  // user add attachment, <key,value>:<OutlookTemp, Source>
    
    rwmutex m_rwOESaveAsAttachment;
    std::map<std::wstring, std::wstring> m_mapOESaveAsOutlookTempToOETemp;   // OE save as attachment, <key, value>:<OETemp, OutlookTemp>
    
    rwmutex m_rwOEAddAttachment;
    std::map<std::wstring, std::wstring> m_mapOEAddOETempToOutlookTemp;      // OE add attachment, <key, value>:<OETemp, OutlookTemp>

    std::wstring m_wstrOETempFolder;
    std::wstring m_wstrOutlookTempFolder;
    std::wstring m_wstrContentWordTempFolder;
    std::wstring m_wstrContentMsoTempFolder;
    std::wstring m_wstrINetCacheContentWordTempFolder; // Win10 x86, outlook2016: C:\Users\~\AppData\Local\Microsoft\Windows\INetCacheContent.Word
    long m_lInitFlag;   // 0, false, others, true

    // For win10 outlook2016 drag/drop case
    long m_lUseDragDropFlag;   // 0, false, others, true
    rwmutex m_rwDropSrcFiles;
    std::map<unsigned long, std::list<std::wstring> > m_mapDropSrcFiles;    // In win10 outlook2016 user drag/drop add attachment, <key, value>:<ThreadID, DropFiles>

    // For msg format file, use map to cache source file path, one source one cache one copy
    rwmutex m_rwMsgSrcFiles;
    std::map<unsigned long, std::vector<std::pair<std::wstring, STUNLOE_MSGFILECATCHINFO> > > m_mapMsgSrcFiles;   // For use add msg format files, <key, <key, value> >:<ThreadID, <MsgSrcFilePath, MsgSrcFileSize> >
    const size_t m_kstMaxMsgSrcFileCacheSize;

    /* 
        For attachment add from net mapped folder or file server file
        1. Outlook first use "CopyFileExW" from mapped folder/file server to content mso temp
        2. And then use "CopyFileW" to copy the file from content mso temp to outlook temp.
    */
    rwmutex m_rwUserAddAttachmentFromFileServer;
    std::map<std::wstring, std::wstring> m_mapUserAddSourceToContentMsoTemp;    // // User add attachment from file server, <key, value>:<ContentMso temp, file server source file>

    // For win10 x86, outlook2016 x86, Ctrl + c/v
    rwmutex m_rwSourcePathFinallyBackCache;
    std::vector<std::wstring> m_vecSourcePathFinallyBackCache;
    const size_t m_kstMaxSourcePathFinallyBackCache;
};