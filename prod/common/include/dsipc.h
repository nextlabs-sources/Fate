// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _DSIPC_H_
#define _DSIPC_H_

#include "dstypes.h"
#include "Actions.h"

typedef struct _IPCREQUEST
{
	ULONG ulSize;
	TCHAR methodName [64];
	TCHAR params [7][256];
}IPCREQUEST, *PIPCREQUEST;

typedef struct _CORE_SID
{
  TCHAR aUserSID[MAX_SID_LENGTH];
  ULONG ulUserSIDLength;
  ULONG ulUserSIDAttribute;
  TCHAR aOwnerSID[MAX_SID_LENGTH];
  ULONG ulOwnerSIDLength;
  TCHAR aGroupSID[MAX_SID_LENGTH];
  ULONG ulGroupSIDLength;
  ULONG ulLinuxUid;
}CORE_SID,*PCORE_SID;

typedef struct tagFILE_INFO
{
	IN  HANDLE  hFileHandle;
      DWORD ulHighCreationTime;
      DWORD ulLowCreationTime;
      DWORD ulHighLastAccessTime;
      DWORD ulLowLastAccessTime;
      DWORD ulHighLastWriteTime;
      DWORD ulLowLastWriteTime;
  // ------------------------------------------------------
  // These are the older fields, and they should get removed
  OUT ULONG ulAttribute;
  OUT ULONG ulHighAllocationSize;
  OUT ULONG ulLowAllocationSize;
  OUT ULONG ulHighEndOfFile;
  OUT ULONG ulLowEndOfFile;
  OUT ULONG ulAccessMaskFlag;
  OUT ULONG ulMode;
  OUT ULONG ulHighCurrentByteOffset;
  OUT ULONG ulLowCurrentByteOffset;
  OUT ULONG ulHighUniqueID;
  OUT ULONG ulLowUniqueID;
  OUT ULONG ulNumOfLinks;
  OUT UCHAR fDirectory;
  OUT UCHAR fDeletePending;
  OUT ULONG ulFileNameLength;
  // ------------------------------------------------------
  
      OUT TCHAR	wzFileName[MAX_NAME_LENGTH];

} FILE_INFO,*PFILE_INFO;

typedef struct tagDIR_LINKS
{
	FILE_INFO	strFileInfo;
	ULONG		ulNumDuplicate;
	struct tagDIR_LINKS *pNext;
	struct tagDIR_LINKS *pRight;
	ULONG		ulReserved;
}DIR_LINKS, *PDIR_LINKS;

typedef struct tagDIRS_RECORD
{
	ULONG		ulNumOfUniqueDirs;
	PDIR_LINKS	pDirLinks;
	ULONG		ulReserved;
}DIRS_RECORD, *PDIRS_RECORD;


typedef struct tagSHAREDNAME_LINKS
{
	ULONG	ulNumOfDuplicates;
	TCHAR	wzSharedName[MAX_NAME_LENGTH];
	TCHAR	wzLocalPath[MAX_NAME_LENGTH];
	struct	tagSHAREDNAME_LINKS *pNext;
	struct	tagSHAREDNAME_LINKS *pRight;
	ULONG	ulReserved;
}SHAREDNAME_LINKS, *PSHAREDNAME_LINKS;

typedef struct tagSHAREDNAME
{
	ULONG				ulNumOfShares;
	PSHAREDNAME_LINKS	pSharedLink;		//Link of duplicate shares
	PSHAREDNAME_LINKS	pSharedLinkNext;	//Link of share found on network
	ULONG				ulReserved;
}SHAREDNAME, *PSHAREDNAME;


// interface for IPC requests to policy enforcement engine
// used by file server agent (kernel mode) and IO interceptor (user mode).
typedef struct _IPC_POLICY_REQUEST
{
	// Size of IPC policy request data structure
	ULONG   ulSize;

        // Name of the method to invoke. 
        TCHAR   szwMethodName [64];

	// App must copy this value in following DevIoCtrl(POLICY_RESPONSE)
	LONG    UniqueIndex;

	// Process Id, control module must check it and avoid re-enter
	ULONG	ulProcessId;

	// Local path name of the file being accessed (h:\directory\file.txt)
	TCHAR   szwFromFileName [512];
  
	// The equivalent path name for the FromFile (\\home\share\directory\file.txt)
	TCHAR   szwFromFileEquivalentName [512];

	//Full path name to the shared file \\machine\shareName\directory\file.txt. Empty if local access
	TCHAR   ShareName[512];

	// Information about the file (size, creation date, etc). See existing FILE_INFO
	FILE_INFO fromFileInfo;

	// Action done on the from file
	TCHAR	szwAction[MAX_ACTION_NAME_SIZE];
        ULONG   ulAction;       /* Sould be removed eventually, linux-only use for now */

	// client's name (current machine name for local access), preferably full name (machine.domain.com)

	TCHAR   szwHostName [128];

	// client's IP address
	IN ULONG        ulIPAddress;

	// Full path to the application executable (used for local access only)
	TCHAR   szwApplicationName [128];

	// True when obligations are ignored (silent policy check)
	BOOL    bIgnoreObligations;

	//UNICODE_STRING describing the OS type
	TCHAR   ClientOsType[64];

	//UNICODE_STRING describing the Lan Manager type
	TCHAR   ClientLanManType[64];

	//User SID information (See CORE_SID for details)
	CORE_SID CoreSid;

        // Local path of the "to file" being accessed (for rename / copy / move, etc), e:\directory\abc.txt
	TCHAR   szwToFileName [512];

	// The equivalent path name for the To File (\\home\share\directory\file.txt)
	TCHAR   szwToFileEquivalentName [512];

	// Noise level - confidence level that policy evaluation is for a genuine user event (and is not noise):
        // Level 3 (Default) - Event is generally directly correlated with a user event. 
        // Level 2 - We are not able to distinguish between noise and true user events.
        //           e.g. a folder that contains images is opened with Windows Explorer.
        //           The thumbnails are shown, but it isn’t clear if the user actually opened
        //           and viewed the images.
        // Level 1 - Event is generally noise.  Classic examples are temp file access.
        ULONG ulNoiseLevel;

}IPC_POLICY_REQUEST, *PIPC_POLICY_REQUEST;


// interface for IPC response from policy enforcement engine
// to the requestor

typedef struct _IPC_POLICY_RESPONSE
{
	// Size of IPC policy response data structure
	ULONG   ulSize;

	LONG UniqueIndex;

	ULONG Allow;
	ULONG AllowType;
}IPC_POLICY_RESPONSE,*PIPC_POLICY_RESPONSE;

#endif
