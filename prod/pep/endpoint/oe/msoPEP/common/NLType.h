#pragma once

/** boost */
#pragma warning( push )
#pragma warning( disable: 4996 4512 4244 6011 ) // warning level 4
#include "boost\algorithm\string.hpp"
#include "boost\thread.hpp"
#include "boost\thread\shared_mutex.hpp"
#pragma warning( pop )

/////////////////////////////boost read write lock type begin/////////////////////////////////////////////
typedef boost::shared_mutex rwmutex;
typedef boost::shared_lock<rwmutex> readLock;
typedef boost::unique_lock<rwmutex> writeLock;
/////////////////////////////boost read write lock type end/////////////////////////////////////////////

#define MESSAGE_TITLE L"Nextlabs Enforcer For Microsoft Outlook"
/////////////////////////////attachment file manager type begin/////////////////////////////////////////////
typedef enum tyEMNLOE_FILECACHOP
{
    emFileCacheUnknown = 0,

    emFileCacheNothing,
    emFileCacheUpdate,
    emFileCacheDelete,
    emFileCacheAddNew
}EMNLOE_FILECACHOP;

typedef enum tyEMNLOE_ATTACHMENTFROM
{
    emAttachmentFromUnknown = 0,

    emAttachmentFromUserAdd,
    emAttachmentFromForward,
    emAttachmentFromThirdAppAdd
}EMNLOE_ATTACHMENTFROM;

typedef enum tyEMNLOE_FILELOCATION
{
    emFileLocationUnknown = 0,

    emFileLocationOETemp,
    emFileLocationOutlookTemp,
    emFileLocationContentWordTemp,
    emFileLocationContentMsoTemp,
    emFileLocationINetCacheContentWord  // Win10 x86, outlook 2016, bug34728
}EMNLOE_FILELOCATION;

typedef enum tyEMNLOE_FILELOGICCASE
{
    emFileLogicCaseUnknown = 0,

    emFileLogicCaseUserAddAttachment,
    emFileLogicCaseOESaveAsAttachment,
    emFileLogicCaseOEAddAttachment
}EMNLOE_FILELOGICCASE;

typedef enum tyEMNLOE_OUTLOOKVERSION
{
    emOutlookVersionUnknown = 0,
	OUTLOOK_VER_NUM_2000 = 9, 
	OUTLOOK_VER_NUM_XP = 10, 
	OUTLOOK_VER_NUM_2003 = 11, 
	OUTLOOK_VER_NUM_2007 = 12, 
	OUTLOOK_VER_NUM_2010 = 14, 
	OUTLOOK_VER_NUM_2013 = 15, 
	OUTLOOK_VER_NUM_2016 = 16
}EMNLOE_OUTLOOKVERSION;

typedef enum tyEMNLOE_IMAGETYPE
{
    emImageTypeUnknown = 0,

    emImageTypex86,
    emImageTypex64
}EMNLOE_IMAGETYPE;

typedef enum tyEMNLOE_PLATEFORMVERSION
{
    emPlateformVersionUnknown = 0,

    emPlateformWin7,
    emPlateformWin10
}EMNLOE_PLATEFORMVERSION;

typedef enum tyEMNLOE_MATCHLEVEL
{
    emMatchLevelExact = 0,
    emMatchLevel1  = 1,
    emMatchLevel2  = 2,
    emMatchLevelUnmatched = 10
}EMNLOE_MATCHLEVEL;

struct STUNLOE_MSGFILECATCHINFO
{
     unsigned long ulFileSize;
     int nUsedTimes;
     std::wstring wstrDisplayName;   // reserve, now no used
     std::wstring wstrFileName;

     STUNLOE_MSGFILECATCHINFO(unsigned long ulParmFileSize = 0, int nParmUsedTimes =0, std::wstring wstrParmDisplayName =L"", std::wstring wstrParmFileName = L"")
         : ulFileSize(ulParmFileSize), nUsedTimes(nParmUsedTimes), wstrDisplayName(wstrParmDisplayName), wstrFileName(wstrParmFileName) {}
     STUNLOE_MSGFILECATCHINFO(_In_ const STUNLOE_MSGFILECATCHINFO& kstuOEMsgFileCacheInfo)
     {
         Copy(kstuOEMsgFileCacheInfo);
     }

     void Copy(_In_ const STUNLOE_MSGFILECATCHINFO& kstuOEMsgFileCacheInfo)
     {
        this->ulFileSize = kstuOEMsgFileCacheInfo.ulFileSize;
        this->nUsedTimes = kstuOEMsgFileCacheInfo.nUsedTimes;
        this->wstrDisplayName = kstuOEMsgFileCacheInfo.wstrDisplayName;
        this->wstrFileName = kstuOEMsgFileCacheInfo.wstrFileName;
     }
};

typedef enum tyEMNLOE_TEMPLOCATION
{
    emTempLocationUnknown = 0,
    emTempLocationDump,
    emTempLocationOverTimeLog,
}EMNLOE_TEMPLOCATION;
/////////////////////////////attachment file manager type end/////////////////////////////////////////////

