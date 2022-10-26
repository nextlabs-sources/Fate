//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "dstypes.h"		
#include <lm.h>
#include <lmdfs.h>
#include <hash_map>
#include <string>
#include <list>
#include <ctype.h>
#include "FileShareRegistry.h"

using namespace std;

#define MAX_MULTI_SZ_SIZE			1024

#define CONFIG_FILE_ENTRY_TYPE_DOMAIN_MAP                 "[DOMAIN_MAP]"
#define CONFIG_FILE_ENTRY_TYPE_WORKSTATION                "[WORKSTATION_LIST]"
#define CONFIG_FILE_ENTRY_TYPE_DFS_DOMAIN                 "[DFS_DOMAINS]"

#define FORCE_UNICODE

/*
Struct to maintain information during a DFS file path Tree recursion
*/
struct DfsPathRecursionState {
    LPWSTR serverName;
    LPWSTR localPath;
    LPWSTR partialPathToShare;
};

// Declare method for later use. 
BOOL FindDFSShareDup(LPWSTR, PULONG, hash_map<wstring, FileShareRegistry*>*, list<DfsPathRecursionState*>*);

/*
Map of NetBios names to DNS names
*/
hash_map<wstring, wstring> netbiosNameToDnsNameMap;

/*
Lowercase a wide character array
*/
BOOL ToLower(LPWSTR wstringToLower) 
{
    LPWSTR current = NULL;

    if (wstringToLower != NULL)
    {
        for (current=wstringToLower; current < wstringToLower + wcslen(wstringToLower); current++)
        {
            *current = towlower(*current);
        }
    }

    return TRUE;
}

/*
Print application usage
*/
void PrintUsage(void)
{
    printf("\nUsage: ResourcePathDiscovery OutputFile ConfigFile\n\n");
    printf("\n  OutputFile:  File in which the found network shares and their corresponding local directories will be listed\n");
    printf("\n  ConfigFile:  File specifying configuration information describing the environment in which the tool is being run.  An example of this file is shown below: \n");
    printf("\n\t\t[DOMAIN_MAP]");
    printf("\n\t\tCORP=corp.com");
    printf("\n\t\tTEST=test.corp.com");
    printf("\n\t\t[WORKSTATION_LIST]");
    printf("\n\t\tblue.corp.com");
    printf("\n\t\tgreen.test.corp.com");
    printf("\n\t\tred.corp.com");
    printf("\n\t\t[DFS_DOMAINS]");
    printf("\n\t\tcorp.com\n");
    printf("\n\n\t[DOMAIN_MAP] - optional - This section is required if the network to be investigated contains DFS shares which span across multiple domains.  In the above example, \"CORP\" and \"TEST\" are domain names which are resolved in the network DNS through \"corp.com\" and \"test.corp.com\" respectively.\n");
    printf("\t[WORKSTATION_LIST] - This section contains the list of machines from which to locate and record network shares.\n");
    printf("\t[DOMAIN_MAP] - optional - This section is required if the network contains Domain DFS shares which must be analyzed and recorded.  In the above example, the network to be analyzed contains Domain DFS shares of the form \\\\corp.com\\...\n\n");

    return;
}

/*
Write all information found to the output file
*/
BOOL WriteToFile(FILE *pOutputFile, FileShareRegistry* shares)
{
    if (pOutputFile == NULL || shares == NULL)
    {
        printf("\nError! - WriteToFile - Invalid parameter(s) received\n");
        return FALSE;
    }

    FileShareRegistry::Iterator registryIterator = shares->begin();
    while (registryIterator != shares->end())
    {
        const wchar_t* localPath = registryIterator->getLocalPath().c_str();
        fprintf(pOutputFile, "'%ls'    '%ls'\n",registryIterator->getSharePath().c_str(), localPath);
        hash_set<wstring> duplicateShares = registryIterator->getDuplicateSharePaths();
        hash_set<wstring>::iterator duplicateShareIterator = duplicateShares.begin();
        while (duplicateShareIterator != duplicateShares.end()) {
            fprintf(pOutputFile, "'%ls'    '%ls'\n",duplicateShareIterator->c_str(), localPath);
            duplicateShareIterator++;
        }
        fprintf(pOutputFile, "\n");
        ++registryIterator;
    }
	return TRUE;
}

/*
Given a workstation name, get its full name (machine name plus domain).  This method first checks if the provided machine name already contains the domain name.  If so, it copies it into the foundFullNameBuffer and returns.  If not, it obtains the NETBios Name for the workstation and then performs a lookup
in the NetBiosToDNSName map to retrieve the domain name configured through the config file.  If not found, the provided default domain name will be used.
The full machine name will be saved in the buffer pointed to by "foundFullNameBuffer". 
Note that this method will allocate memory for the provided buffer of the necessary size, but the client must free it.
If the domain cannot be found, FALSE is returned
*/
BOOL GetFullMachineName(LPWSTR machineName, LPWSTR* foundFullNameBuffer, LPWSTR defaultDomain, PULONG pulError)
{
    PWKSTA_INFO_100 temporaryBuffer = NULL;
    NET_API_STATUS netApiStatus;
    wstring* dnsDomainName;

    if (machineName == NULL) 
    {
        printf("\nError! - GetFullMachineName - NULL machineName parameter received");
        return FALSE;
    }

    if (wcsstr(machineName, L".") != NULL) 
    {
        *foundFullNameBuffer = _wcsdup(machineName);
    } 
    else
    {
        netApiStatus = NetWkstaGetInfo((LPWSTR)machineName, 100, (LPBYTE*)&temporaryBuffer);

        if (netApiStatus == ERROR_SUCCESS)
        {
            ToLower((LPWSTR)temporaryBuffer->wki100_langroup);
            wstring netbiosAsWstring = wstring((LPWSTR)temporaryBuffer->wki100_langroup);
		    hash_map<wstring, wstring>::iterator dnsDomainNameIteratorPosition = netbiosNameToDnsNameMap.find(netbiosAsWstring);
            if (dnsDomainNameIteratorPosition == netbiosNameToDnsNameMap.end())
            {
                dnsDomainName = new  wstring(defaultDomain);
                netbiosNameToDnsNameMap[netbiosAsWstring] = *dnsDomainName;
            }
		    else 
		    {
			    dnsDomainName = &dnsDomainNameIteratorPosition->second;
		    }

            *foundFullNameBuffer = new (std::nothrow) WCHAR[wcslen(machineName) + dnsDomainName->length() + 2];
			if(NULL != *foundFullNameBuffer)
			{
            	wcsncpy_s(*foundFullNameBuffer, wcslen(machineName) + dnsDomainName->length() + 2, machineName, _TRUNCATE);
            	wcsncat_s(*foundFullNameBuffer, wcslen(machineName) + dnsDomainName->length() + 2, L".", _TRUNCATE);
				wcsncat_s(*foundFullNameBuffer, wcslen(machineName) + dnsDomainName->length() + 2, dnsDomainName->c_str(), _TRUNCATE);
			}
        }
        else
        {
            wprintf(L"\nError! - Failed to retrieve domain name for workstation, %s.  Net API Error Code: %ld\n", machineName, netApiStatus);
            *pulError = netApiStatus;
            return FALSE;
        }
    }

    if (temporaryBuffer != NULL)
    {
        NetApiBufferFree(temporaryBuffer);
    }

    return TRUE;
}

/*
Given the specified share, find all nested shares and record
*/
BOOL FindDuplicateSharedName(LPWSTR lpwzServerName, LPWSTR shareName, LPWSTR localPath, FileShareRegistry* pSharedName) 
{
    if (lpwzServerName == NULL || shareName == NULL || localPath == NULL || pSharedName == NULL)
    {
        printf("\nError! - FindDuplicateSharedName - Invalid parameter(s) received\n");
        return	FALSE;
    }

    wstring* localPathAsWString = new (std::nothrow) wstring(localPath);
    wstring* fullShareName = new (std::nothrow) wstring(L"\\\\");
    fullShareName->append(lpwzServerName);
    fullShareName->append(L"\\");
    fullShareName->append(shareName);

    pSharedName->addShare(*localPathAsWString, *fullShareName);
	delete localPathAsWString;
	delete fullShareName;

    return TRUE;
}

/*
Given the specified share, find all nested shares and record
*/
BOOL FindDuplicateSharedName(LPWSTR lpwzServerName, FileShareRegistry* pSharedName, PSHARE_INFO_2 pSharedInfo2)
{
    if (lpwzServerName == NULL || pSharedName == NULL || pSharedInfo2 == NULL)
    {
        printf("\nError! - FindDuplicateSharedName - Invalid parameter(s) received\n");
        return	FALSE;
    }

    return FindDuplicateSharedName(lpwzServerName, (LPWSTR)pSharedInfo2->shi2_netname, (LPWSTR)pSharedInfo2->shi2_path, pSharedName);
}

/*
Determine if the storage info points to a DFS Root
*/
BOOL IsDfsRoot(LPWSTR serverName, LPWSTR shareName, BOOL& result, PULONG pulError) 
{
    NET_API_STATUS napiStatus;
    PSHARE_INFO_1005 shareInfo = NULL;

    if (serverName == NULL)
    {
        printf("\nError! - IsDfsRoot - NULL  serverName received\n");
        return FALSE;
    }

    if (shareName == NULL)
    {
        printf("\nError! - IsDfsRoot - NULL shareName received\n");
        return FALSE;
    }

    if (pulError == NULL)
    {
        printf("\nError! - IsDfsRoot - NULL pulError received\n");
        return FALSE;
    }

    napiStatus = NetShareGetInfo((LPWSTR)serverName, (LPWSTR)shareName, 1005, (LPBYTE *)&shareInfo);
    if (napiStatus == ERROR_SUCCESS)
    {
        result = ((shareInfo->shi1005_flags & SHI1005_FLAGS_DFS_ROOT) != 0);
    }
    else
    {
        printf("\nError! - IsDfsRoot - Failed to determine if \\\\%ls\\%ls is a Dfs Root.  Net API Error Code: %ld\n", serverName, shareName, napiStatus);
        *pulError = napiStatus;
        return FALSE;
    }

    if (shareInfo)
    {
        NetApiBufferFree(shareInfo);
    }

    return TRUE;
}

FileShareRegistry* GetSharesForServer(LPWSTR serverName, hash_map<wstring,FileShareRegistry*>* serversToSharesMap) {
    FileShareRegistry* sharesToReturn = NULL;
    hash_map<wstring, FileShareRegistry*>::iterator serverForShareIterator = serversToSharesMap->find(serverName);
    if (serverForShareIterator == serversToSharesMap->end()) 
    {
        wstring* serverNameAsWString = new wstring(serverName);
        sharesToReturn = new (std::nothrow) FileShareRegistry(*serverNameAsWString);  // Free'd in _tmain
		delete serverNameAsWString;
        (*serversToSharesMap)[serverName] = sharesToReturn;
    }
    else
    {
        sharesToReturn = serverForShareIterator->second;
    }

    return sharesToReturn;
}

/*
Create DfsRecursionState with the specified parameters
*/
BOOL BuildDfsRecursionState(DfsPathRecursionState* pathRecursionState, PDFS_STORAGE_INFO dfsStorageInfo, LPWSTR defaultDomain, PULONG pulError) 
{
    PSHARE_INFO_2 shareInfoBuffer = NULL;
    NET_API_STATUS napiStatus;
    LPWSTR localPathServer = NULL;
    BOOL valueToReturn = TRUE;
	
	wchar_t *next_token = NULL;
	
    if (!GetFullMachineName((LPWSTR)dfsStorageInfo->ServerName, &localPathServer, defaultDomain, pulError))
    {
        printf("\nError! - Failed to retrieve domain name for workstation %ls.  Error code: %ld\n", dfsStorageInfo->ServerName, *pulError);
        valueToReturn = FALSE;
    }
    else
    {
        LPWSTR shareName = new WCHAR[wcslen(dfsStorageInfo->ShareName) + 1];
        wcsncpy_s(shareName, wcslen(dfsStorageInfo->ShareName) + 1, dfsStorageInfo->ShareName, _TRUNCATE);
        shareName = wcstok_s(shareName, L"\\", &next_token);
        LPWSTR pathRemainder = wcstok_s(NULL, L"\\", &next_token);

        napiStatus = NetShareGetInfo((LPWSTR)localPathServer, (LPWSTR)shareName, 2, (LPBYTE *)&shareInfoBuffer);
        if (napiStatus == ERROR_SUCCESS)
        {	
            pathRecursionState->serverName = new WCHAR[wcslen(localPathServer) + 1];
            wcsncpy_s(pathRecursionState->serverName, wcslen(localPathServer) + 1, localPathServer, _TRUNCATE);
            ToLower(pathRecursionState->serverName);

            if (pathRemainder == NULL) {
                pathRecursionState->localPath = new WCHAR[wcslen((LPWSTR)shareInfoBuffer->shi2_path) + 1];
                wcsncpy_s(pathRecursionState->localPath, wcslen((LPWSTR)shareInfoBuffer->shi2_path) + 1, (LPWSTR)shareInfoBuffer->shi2_path, _TRUNCATE);
            } else {
                pathRecursionState->localPath = new WCHAR[wcslen((LPWSTR)shareInfoBuffer->shi2_path) + wcslen(pathRemainder) + 3];
                wcsncpy_s(pathRecursionState->localPath, wcslen((LPWSTR)shareInfoBuffer->shi2_path) + wcslen(pathRemainder) + 3, (LPWSTR)shareInfoBuffer->shi2_path, _TRUNCATE);
                wcsncat_s(pathRecursionState->localPath, wcslen((LPWSTR)shareInfoBuffer->shi2_path) + wcslen(pathRemainder) + 3, L"\\", _TRUNCATE);
                wcsncat_s(pathRecursionState->localPath, wcslen((LPWSTR)shareInfoBuffer->shi2_path) + wcslen(pathRemainder) + 3, pathRemainder, _TRUNCATE);
            }

            pathRecursionState->partialPathToShare = new WCHAR[MAX_NAME_LENGTH];
        }
        else
        {
            wprintf(L"\nError! - BuildDfsRecursionState - Failed to retrieve share information for share, \\\\%ls\\%ls.  Net API Error code: %lu.\n", 
                localPathServer, 
                (LPWSTR)dfsStorageInfo->ShareName, 
                napiStatus);
            *pulError = napiStatus;
            valueToReturn = FALSE;
        }
    }

    if (shareInfoBuffer)
    {
        NetApiBufferFree(shareInfoBuffer);
    }

    if (localPathServer)
    {
        delete localPathServer;
    }

    return valueToReturn;
}

/*
Process the child DfsRoot found within a DFS_STORAGE_INFO instance through a call to NetDfsEnum
*/
BOOL ProcessChildDFSShare(LPWSTR serverName, LPWSTR shareName, LPWSTR defaultDomain, hash_map<wstring, FileShareRegistry*>* serversToSharesMap, list<DfsPathRecursionState*>* foundLocalPaths, PULONG pulError)
{
    LPWSTR storageDfsMachine;
    BOOL valueToReturn = TRUE;

    if (!GetFullMachineName(serverName, &storageDfsMachine, defaultDomain, pulError))
    {
        printf("\nError! - Failed to find domain for workstation, %ls.  It may be outside current domain.  Error code: %ld.\n", serverName, *pulError); 
        valueToReturn = FALSE;
    }
    else 
    {
        ToLower(storageDfsMachine);

        LPWSTR storageDfsRoot = new WCHAR[wcslen(storageDfsMachine) + wcslen(shareName) + 3];
        wcsncpy_s(storageDfsRoot, wcslen(storageDfsMachine) + wcslen(shareName) + 3, storageDfsMachine, _TRUNCATE);
        wcsncat_s(storageDfsRoot, wcslen(storageDfsMachine) + wcslen(shareName) + 3, L"\\", _TRUNCATE);
        wcsncat_s(storageDfsRoot, wcslen(storageDfsMachine) + wcslen(shareName) + 3, shareName, _TRUNCATE);

        if (!FindDFSShareDup(storageDfsRoot, pulError, serversToSharesMap, foundLocalPaths))
        {
            printf("\nError! - Failed to find shares in child DFS Root, %ls.  Error code: %ld.\n", storageDfsRoot, *pulError); 
            valueToReturn = FALSE;
        }
    }

    return valueToReturn;
}

/*
Find shares in the DFS path pointed to by lpwszServerName
*/
BOOL FindDFSShareDup(LPWSTR lpwszServerName, PULONG pulError, hash_map<wstring, FileShareRegistry*>* serversToSharesMap, list<DfsPathRecursionState*>* foundLocalPaths)
{
    ULONG ulCount;
    ULONG storageCount;
    NET_API_STATUS napiStatus;
    PDFS_INFO_3 pDfsInfo = NULL;
    PDFS_INFO_3 pDfsInfoTemp = NULL;
    PDFS_STORAGE_INFO dfsStorageInfoTemp = NULL;
    DWORD entriesRead;
    BOOL dfsRoot;
    LPWSTR dfsRootServerName;
    LPWSTR dfsRootServerShare;
    LPWSTR dfsRootDomainName;
    hash_map<LPWSTR, list<PDFS_STORAGE_INFO>* > nonDfsRootLinks;
    list<PDFS_STORAGE_INFO>* nonDfsRootLinkStorageInfoList;

	
	wchar_t *next_token = NULL;

    if (pulError == NULL)
    {
        printf("FindDFSShareDup - Error - NULL pulError received\n");
        return FALSE;
    }

    if (lpwszServerName == NULL)
    {
        printf("FindDFSShareDup - Error - NULL lpwszServerName received\n");
        *pulError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    /*
    Find the DFS links within this root
    */
    #pragma warning(push)
	#pragma warning(disable:6309 6387)
    napiStatus = NetDfsEnum (lpwszServerName, 
        3, 
        MAX_PREFERRED_LENGTH, 
        (LPBYTE *) &pDfsInfo,
        &entriesRead, 									 
        NULL);
	#pragma warning(pop)

    if(napiStatus == ERROR_SUCCESS)
    {
        dfsRootServerName = wcstok_s(lpwszServerName, L"\\", &next_token);
        dfsRootServerShare = wcstok_s(NULL, L"\\", &next_token);
        dfsRootDomainName = wcspbrk(dfsRootServerName, L".") + 1;

        pDfsInfoTemp = pDfsInfo;
        for(ulCount = 1; ulCount <= entriesRead; ulCount++)
        {
            /*
            If we pass a DFS root to the above method, the same root will be returned.  We can tell this is
            the case, because the path will be of the form \\A\B where B is the root.  All links will be of
            the form \\A\\B\C
            */
            (void)wcstok_s(pDfsInfoTemp->EntryPath+2, L"\\", &next_token);
            (void)wcstok_s(NULL, L"\\", &next_token);
            LPWSTR pathToLink = wcstok_s(NULL, L"\\", &next_token);
            if (pathToLink == NULL) {
                // This is the DFS root.  Storage locations for DFS Roots are DFS Roots themselves.  Therefore, they're added for later processing in the same fashion a non DFS Root link storage locations are treated
                dfsStorageInfoTemp = pDfsInfoTemp->Storage;

                /*
                We need to do a depth first search.  Therefore, save the non-root links for later processing
                */
                nonDfsRootLinkStorageInfoList = new list<PDFS_STORAGE_INFO>();
                nonDfsRootLinks[L""] = nonDfsRootLinkStorageInfoList;
                for (storageCount = 1; storageCount <= pDfsInfoTemp->NumberOfStorages; storageCount++) 
                {
                    printf("Found storage location, \\\\%ls\\%ls, for DFS Root, \\\\%ls\n",
                        dfsStorageInfoTemp->ServerName,
                        dfsStorageInfoTemp->ShareName,
                        lpwszServerName);

                    nonDfsRootLinkStorageInfoList->push_back(dfsStorageInfoTemp);	
                    dfsStorageInfoTemp++;
                }
            } else {
                printf("Found DFS Link, %ls, for DFS Root, \\\\%ls\\%ls.\n", pathToLink, dfsRootServerName, dfsRootServerShare);

                dfsStorageInfoTemp = pDfsInfoTemp->Storage;

                /*
                We need to do a depth first search.  Therefore, save the non-root links for later processing
                */
                nonDfsRootLinkStorageInfoList = new list<PDFS_STORAGE_INFO>();
                nonDfsRootLinks[pathToLink] = nonDfsRootLinkStorageInfoList;
                for (storageCount = 1; storageCount <= pDfsInfoTemp->NumberOfStorages; storageCount++) 
                {
                    LPWSTR dfsStorageServerName = dfsStorageInfoTemp->ServerName;
                    LPWSTR dfsStorageShareName = new WCHAR[wcslen(dfsStorageInfoTemp->ShareName) + 1];
                    wcsncpy_s(dfsStorageShareName, wcslen(dfsStorageInfoTemp->ShareName) + 1, dfsStorageInfoTemp->ShareName, _TRUNCATE);

                    printf("Found storage location, \\\\%ls\\%ls, for DFS Link, \\\\%ls\\%ls\n",
                        dfsStorageServerName,
                        dfsStorageShareName,
                        lpwszServerName,
                        pathToLink);

                    LPWSTR dfsStorageShareNetName = wcstok_s(dfsStorageShareName, L"\\", &next_token);
                    LPWSTR dfsStorageShareRemainder = wcstok_s(NULL, L" ", &next_token);

                    if (IsDfsRoot(dfsStorageServerName, dfsStorageShareNetName, dfsRoot, pulError) == TRUE) 
                    {	
                        if (dfsRoot) 
                        {
							printf("Previously found storage location is a DFS path.  Processing hierarchy.\n");

                            if (!ProcessChildDFSShare(dfsStorageServerName, dfsStorageShareNetName, dfsRootDomainName, serversToSharesMap, foundLocalPaths, pulError))
                            {
                                printf("\nError! - Failed to process child DFS root, \\\\%ls\\%ls, found in DFS Storage Info.  Some shares will not be found.  Error code: %ld.\n", dfsStorageInfoTemp->ServerName, dfsStorageInfoTemp->ShareName, *pulError); 								
                            }
                            else
                            {
                                /*
                                With the child DFS Share processed, we should have all local path information stored in the foundLocalPaths recursion state
                                Map this DFS share to those local paths based on dfsStorageShareRemainder
                                */	
                                DfsPathRecursionState* currentDfsRecursionState = NULL;
                                DfsPathRecursionState* emptyPartialPathShareRecursionState = NULL;
                                list<DfsPathRecursionState*>::iterator foundLocalPathsIterator = foundLocalPaths->begin();
                                while (foundLocalPathsIterator != foundLocalPaths->end())
                                {
                                    currentDfsRecursionState = (DfsPathRecursionState*)*foundLocalPathsIterator;

                                    hash_map<wstring, FileShareRegistry*>::iterator serverForShareIterator = serversToSharesMap->find(currentDfsRecursionState->serverName);

                                    LPWSTR currentPartialPathToShare = currentDfsRecursionState->partialPathToShare;
                                    if (dfsStorageShareRemainder == NULL) {
                                        /*
                                        Add the current link to the recursion state path to share
                                        */
                                        LPWSTR newPartialPathToShare = new WCHAR[wcslen(pathToLink) + wcslen(currentPartialPathToShare) + 3];
                                        wcsncpy_s(newPartialPathToShare, wcslen(pathToLink) + wcslen(currentPartialPathToShare) + 3, pathToLink, _TRUNCATE);
                                        if (wcslen(currentPartialPathToShare) > 0) {
                                            wcsncat_s(newPartialPathToShare, wcslen(pathToLink) + wcslen(currentPartialPathToShare) + 3, L"\\", _TRUNCATE);	
                                            wcsncat_s(newPartialPathToShare, wcslen(pathToLink) + wcslen(currentPartialPathToShare) + 3, currentPartialPathToShare, _TRUNCATE);
                                        }
                                        currentDfsRecursionState->partialPathToShare = newPartialPathToShare;

                                        /*
                                        Now add the DFSRoot to the partial path to share to get the full share path from the server name
                                        */
                                        LPWSTR sharePathToRecord = new WCHAR[wcslen(dfsRootServerShare) + wcslen(newPartialPathToShare) + 3];
                                        wcsncpy_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(newPartialPathToShare) + 3, dfsRootServerShare, _TRUNCATE);
                                        wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(newPartialPathToShare) + 3, L"\\", _TRUNCATE);
                                        wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(newPartialPathToShare) + 3, newPartialPathToShare, _TRUNCATE);

                                        LPWSTR localPathToRecord = currentDfsRecursionState->localPath;

                                        /*
                                        Run this path through the FindDuplicateSharedName to find all duplicates and record mapping
                                        */
                                        _wcslwr_s(dfsStorageShareRemainder, wcslen(dfsStorageShareRemainder));
										_wcslwr_s(currentPartialPathToShare, wcslen(currentPartialPathToShare));
                                        if (!FindDuplicateSharedName(dfsRootServerName, sharePathToRecord, localPathToRecord, serverForShareIterator->second))
                                        {
                                            printf("\nError! - Failed to record share, \\\\%ls\\%ls, to local path, %ls.  Some shares will not be found.\n", dfsRootServerName, dfsRootServerShare, currentDfsRecursionState->localPath); 								
                                        }

                                        /*
                                          Update recursion state
                                        */
                                        currentDfsRecursionState->partialPathToShare = newPartialPathToShare;

                                        foundLocalPathsIterator++;
                                    } else if ((wcslen(currentPartialPathToShare) > 0) &&
                                        (wcsstr(dfsStorageShareRemainder, currentPartialPathToShare) == dfsStorageShareRemainder)) {
                                        //FIX ME - Reeaxmine check above.  Is it the only case when a link is in the path
                                        LPWSTR localPathToRecord = new WCHAR[wcslen(currentDfsRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3];
                                        LPWSTR storageRemainderWithoutLink = _wcsdup(dfsStorageShareRemainder);
                                        (void)wcstok_s(storageRemainderWithoutLink, L"\\", &next_token);
                                        storageRemainderWithoutLink = wcstok_s(NULL, L" ", &next_token);

                                        wcsncpy_s(localPathToRecord, wcslen(currentDfsRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, currentDfsRecursionState->localPath, _TRUNCATE);
                                        if (storageRemainderWithoutLink != NULL) {
                                            wcsncat_s(localPathToRecord, wcslen(currentDfsRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, L"\\", _TRUNCATE);
                                            wcsncat_s(localPathToRecord, wcslen(currentDfsRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, storageRemainderWithoutLink, _TRUNCATE);
                                        }
                                        
                                        LPWSTR sharePathToRecord = new WCHAR[wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3];
                                        wcsncpy_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, dfsRootServerShare, _TRUNCATE);
                                        wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, L"\\", _TRUNCATE);
                                        wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, pathToLink, _TRUNCATE);

                                        /*
                                        Run this path through the FindDuplicateSharedName to find all duplicates and record mapping
                                        */
                                        if (!FindDuplicateSharedName(dfsRootServerName, sharePathToRecord, localPathToRecord, serverForShareIterator->second))
                                        {
                                            printf("\nError! - Failed to record share, \\\\%ls\\%ls, to local path, %ls.  Some shares will not be found.\n", dfsRootServerName, dfsRootServerShare, currentDfsRecursionState->localPath); 								
                                        }

                                        /*
                                          Update Recursion State
                                        */
                                        currentDfsRecursionState->partialPathToShare = pathToLink;
                                        currentDfsRecursionState->localPath = localPathToRecord;

                                        foundLocalPathsIterator++;
                                    } else if ((wcslen(currentPartialPathToShare) == 0)) {
                                        // We need to save this.  This is the case when the storage path points to a DFS root, but the remainder path does not contain a link.  In other words, in the path, //MACHINE/DFSROOT/X/...., X is a local folder on the machine hosting the root rather than a dfs link.
                                        emptyPartialPathShareRecursionState = currentDfsRecursionState;
                                        foundLocalPathsIterator++;
                                    } else {
                                        // If it doesn't pass the dfs storage remainder path filter, remove it from the local paths found.  
                                        foundLocalPathsIterator = foundLocalPaths->erase(foundLocalPathsIterator);
                                    }
                                }

                                  
                                if (foundLocalPaths->empty()) {
                                    // If this list is empty (e.g. all have been removed), then this is the case when the storage path points to a DFS root, but the remainder path does not contain a link.  In other words, in the path, //MACHINE/DFSROOT/X/...., X is a local folder on the machine hosting the root rather than a dfs link.
                                    hash_map<wstring, FileShareRegistry*>::iterator serverForShareIterator = serversToSharesMap->find(emptyPartialPathShareRecursionState->serverName);

                                    LPWSTR localPathToRecord = new WCHAR[wcslen(emptyPartialPathShareRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3];
                                    wcsncpy_s(localPathToRecord, wcslen(emptyPartialPathShareRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, emptyPartialPathShareRecursionState->localPath, _TRUNCATE);
                                    wcsncat_s(localPathToRecord, wcslen(emptyPartialPathShareRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, L"\\", _TRUNCATE);
                                    wcsncat_s(localPathToRecord, wcslen(emptyPartialPathShareRecursionState->localPath) + wcslen(dfsStorageShareRemainder) + 3, dfsStorageShareRemainder, _TRUNCATE);    

                                    LPWSTR sharePathToRecord = new WCHAR[wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3];
                                    wcsncpy_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, dfsRootServerShare, _TRUNCATE);
                                    wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, L"\\", _TRUNCATE);
                                    wcsncat_s(sharePathToRecord, wcslen(dfsRootServerShare) + wcslen(pathToLink) + 3, pathToLink, _TRUNCATE);

                                    /*
                                    Run this path through the FindDuplicateSharedName to find all duplicates and record mapping
                                    */
                                    if (!FindDuplicateSharedName(dfsRootServerName, sharePathToRecord, localPathToRecord, serverForShareIterator->second))
                                    {
                                        printf("\nError! - Failed to record share, \\\\%ls\\%ls, to local path, %ls.  Some shares will not be found.\n", dfsRootServerName, dfsRootServerShare, currentDfsRecursionState->localPath); 								
                                    }
                                }
                            }
                        }
                        else 
                        {
                            nonDfsRootLinkStorageInfoList->push_back(dfsStorageInfoTemp);			
                        }
                    } 
                    else
                    {
                        printf("\nError! - Failed to determine if path, \\\\%ls\\%ls, is a DFS Root.  Some shares will not be found.  Error code: %ld.\n", dfsStorageInfoTemp->ServerName, dfsStorageInfoTemp->ShareName, *pulError); 								
                    }
						
                    dfsStorageInfoTemp++;
                }									
            }

            pDfsInfoTemp++;
        }

        /*
        Now, process the non-root dfs link
        */
        hash_map<LPWSTR, list<PDFS_STORAGE_INFO>* >::iterator nonDfsRootLinksIterator = nonDfsRootLinks.begin();
        while (nonDfsRootLinksIterator != nonDfsRootLinks.end())
        {
            LPWSTR pathToLink = nonDfsRootLinksIterator->first;
            list<PDFS_STORAGE_INFO>* nonDfsRootLinkStorageInfoList_l = nonDfsRootLinksIterator->second;
            list<PDFS_STORAGE_INFO>::iterator nonDfsRootLinkStorageInfoIterator = nonDfsRootLinkStorageInfoList_l->begin();
            while (nonDfsRootLinkStorageInfoIterator != nonDfsRootLinkStorageInfoList_l->end())
            {
                DfsPathRecursionState* pathRecursionState = new DfsPathRecursionState();
                if (!BuildDfsRecursionState(pathRecursionState, *nonDfsRootLinkStorageInfoIterator, dfsRootDomainName, pulError))
                {
                    printf("Failed to find info about non-dfs share, \\\\%ls\\%ls.  Some shares may not be found.  Error code: %ld\n", (*nonDfsRootLinkStorageInfoIterator)->ServerName, (*nonDfsRootLinkStorageInfoIterator)->ShareName, *pulError);
                }
                else
                {
                    wcsncpy_s(pathRecursionState->partialPathToShare, MAX_NAME_LENGTH, pathToLink, _TRUNCATE);
                    foundLocalPaths->push_back(pathRecursionState);

                    FileShareRegistry* serverForShare = GetSharesForServer(pathRecursionState->serverName, serversToSharesMap);

                    if (!FindDuplicateSharedName(pathRecursionState->serverName, (LPWSTR)(*nonDfsRootLinkStorageInfoIterator)->ShareName, pathRecursionState->localPath, serverForShare))
                    {
                        printf("Failed to record share, \\\\%ls\\%ls to local path %ls on workstation %ls.  Some shares may not be found.\n", pathRecursionState->serverName, (*nonDfsRootLinkStorageInfoIterator)->ShareName, pathRecursionState->localPath, pathRecursionState->serverName);
                        return FALSE;
                    }

                    LPWSTR dfsRootSharePathToLocalPath = new WCHAR[wcslen(dfsRootServerShare) + wcslen(pathRecursionState->partialPathToShare) + 3];
                    wcsncpy_s(dfsRootSharePathToLocalPath, wcslen(dfsRootServerShare) + wcslen(pathRecursionState->partialPathToShare) + 3, dfsRootServerShare, _TRUNCATE);

                    // The partial path to share may be empty if recursion state represents a root storage location for DFS Root
                    if (wcslen(pathRecursionState->partialPathToShare) > 0) {
                        wcsncat_s(dfsRootSharePathToLocalPath, wcslen(dfsRootServerShare) + wcslen(pathRecursionState->partialPathToShare) + 3, L"\\", _TRUNCATE);
                        wcsncat_s(dfsRootSharePathToLocalPath, wcslen(dfsRootServerShare) + wcslen(pathRecursionState->partialPathToShare) + 3, pathRecursionState->partialPathToShare, _TRUNCATE);
                    }

                    if (!FindDuplicateSharedName(dfsRootServerName, dfsRootSharePathToLocalPath, pathRecursionState->localPath, serverForShare))
                    {
                        printf("Failed to record share, \\\\%ls\\%ls to local path %ls on workstation %ls.  Some shares may not be found.\n", dfsRootServerName, dfsRootSharePathToLocalPath, pathRecursionState->serverName, pathRecursionState->localPath);
                        delete[] dfsRootSharePathToLocalPath;
                        return FALSE;
                    }

                    delete[] dfsRootSharePathToLocalPath;
                }
		
                nonDfsRootLinkStorageInfoIterator++;
            }

			hash_map<LPWSTR, list<PDFS_STORAGE_INFO>* >::iterator itemToRemove = nonDfsRootLinksIterator;			
            nonDfsRootLinksIterator++;
			nonDfsRootLinks.erase(itemToRemove);
			delete nonDfsRootLinkStorageInfoList_l;
        }


		NetApiBufferFree(pDfsInfo);

    }
    else 
    {
        printf("\nError! - Failed to find DFS links for DFS path %ls.  Net API Error code: %ld\n", lpwszServerName, napiStatus);
        *pulError = napiStatus;
        return FALSE;
    }

    return TRUE;
}

/*
Find shares in a DFS path
*/
BOOL FindDFSShareDup(LPWSTR lpwszServerName, PULONG pulError, hash_map<wstring, FileShareRegistry*>* serversToSharesMap)
{
    list<DfsPathRecursionState*> localPathList;
    return FindDFSShareDup(lpwszServerName, pulError, serversToSharesMap, &localPathList);
}

/*
Find shares in a domain DFS
*/
BOOL FindDomainDFSShareDup(LPWSTR domainName, PULONG pulError, hash_map<wstring, FileShareRegistry*>* serversToSharesMap)
{
    NET_API_STATUS napiStatus;
    DWORD entriesRead;
    PDFS_INFO_200 pDfsInfo = NULL;
    PDFS_INFO_200 pDfsInfoTemp = NULL;
    ULONG ulCount;

    if (pulError == NULL)
    {
        printf("\nError! - FindDomainDFSShareDup - NULL pulError received\n");
        return FALSE;
    }	

    if (domainName == NULL)
    {
        printf("\nError! - FindDomainDFSShareDup - NULL domainName received\n");
        *pulError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (serversToSharesMap == NULL)
    {
        printf("\nError! - FindDomainDFSShareDup - NULL domainName received\n");
        *pulError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    /*
    Find the DFS roots in the domain
    */
    #pragma warning(push)
	#pragma warning(disable:6309 6387)
    napiStatus = NetDfsEnum(domainName, 
        200, 
        MAX_PREFERRED_LENGTH, 
        (LPBYTE *) &pDfsInfo,
        &entriesRead, 									 
        NULL);
	#pragma warning(pop)

    if (napiStatus == ERROR_SUCCESS)
    {
        pDfsInfoTemp = pDfsInfo;
        for (ulCount = 0; ulCount < entriesRead; ulCount++)
        {
            LPWSTR dfsRootPath = new WCHAR[wcslen(domainName) + wcslen((LPWSTR)pDfsInfoTemp->FtDfsName) + 3];
            wcsncpy_s(dfsRootPath, wcslen(domainName) + wcslen((LPWSTR)pDfsInfoTemp->FtDfsName) + 3, domainName, _TRUNCATE);
            wcsncat_s(dfsRootPath, wcslen(domainName) + wcslen((LPWSTR)pDfsInfoTemp->FtDfsName) + 3, L"\\", _TRUNCATE);
            wcsncat_s(dfsRootPath, wcslen(domainName) + wcslen((LPWSTR)pDfsInfoTemp->FtDfsName) + 3, (LPWSTR)pDfsInfoTemp->FtDfsName, _TRUNCATE);

            printf("Found Domain DFS Root, %ls.\n", dfsRootPath);

            if (FindDFSShareDup(dfsRootPath, pulError, serversToSharesMap) == FALSE)
            {
                printf("\nError! - Failed to follow DFS path, %ls.  Some shares may not be recorded.  Error code - %ld\n", dfsRootPath, *pulError);					
            }

            delete[] dfsRootPath;

            pDfsInfoTemp++;
        }
    }
    else
    {
        wprintf(L"\nError! - FindDomainDFSShareDup - Failed to enumerate DFS Roots for domain, %s.  Net API Error code: %lu.\n", domainName, napiStatus);
        *pulError = napiStatus;
        return FALSE;
    }

    if (pDfsInfo)
    {
        NetApiBufferFree(pDfsInfo);
    }

    return TRUE;
}

/*
Find all network shares (including DFS shares) that are present on workstation specified by lpwszServerName
*/
BOOL FindDesktopAgentShareDup(LPWSTR lpwszServerName, PULONG pulError, hash_map<wstring, FileShareRegistry*>* serversToSharesMap)
{
    ULONG			ulCount;
    NET_API_STATUS	napiStatus;
    DWORD			dwSharesRead, dwTotalShares;
    PSHARE_INFO_2	pSharedInfo2 = NULL;
    PSHARE_INFO_2	pSharedInfo2Temp = NULL;
    BOOL            dfsShare;
    FileShareRegistry* pSharedName;

    if (pulError == NULL)
    {
        printf("\nError! - FindDesktopAgentShareDup - NULL pulError received\n");
        return FALSE;
    }

    if (lpwszServerName == NULL)
    {
        printf("\nError! - FindDesktopAgentShareDup - NULL lpwszServerName received\n");
        *pulError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    ToLower(lpwszServerName);
    pSharedName = GetSharesForServer(lpwszServerName, serversToSharesMap);

    napiStatus = NetShareEnum ((LPWSTR)lpwszServerName, 
        2, 
        (LPBYTE *) &pSharedInfo2, 
        MAX_PREFERRED_LENGTH, 
        &dwSharesRead, 
        &dwTotalShares, 
        NULL);

    //
    // If the call succeeds,
    //
    if(napiStatus == ERROR_SUCCESS)
    {
        pSharedInfo2Temp = pSharedInfo2;

        //
        // Loop through the entries;
        // print retrieved data.
        //
        for(ulCount = 1; ulCount <= dwSharesRead; ulCount++)
        {
            if (pSharedInfo2Temp->shi2_type == STYPE_DISKTREE)
            {
                printf("Found Share \\\\%ls\\%ls.\n", lpwszServerName, pSharedInfo2Temp->shi2_netname);
                FindDuplicateSharedName(lpwszServerName, pSharedName, pSharedInfo2Temp);

                /*
                If it's a DFS root, follow the DFS path and record shares
                */
                if (IsDfsRoot(lpwszServerName, (LPWSTR)pSharedInfo2Temp->shi2_netname, dfsShare, pulError))
                {
                    if (dfsShare)
                    {
                        printf("Found DSF Share \\\\%ls\\%ls.\n", lpwszServerName, pSharedInfo2Temp->shi2_netname);
                        LPWSTR dfsRootPath = new WCHAR[wcslen(lpwszServerName) + wcslen((LPWSTR)pSharedInfo2Temp->shi2_netname) + 3];
                        wcsncpy_s(dfsRootPath, wcslen(lpwszServerName) + wcslen((LPWSTR)pSharedInfo2Temp->shi2_netname) + 3, lpwszServerName, _TRUNCATE);
                        wcsncat_s(dfsRootPath, wcslen(lpwszServerName) + wcslen((LPWSTR)pSharedInfo2Temp->shi2_netname) + 3, L"\\", _TRUNCATE);
                        wcsncat_s(dfsRootPath, wcslen(lpwszServerName) + wcslen((LPWSTR)pSharedInfo2Temp->shi2_netname) + 3, (LPWSTR)pSharedInfo2Temp->shi2_netname, _TRUNCATE);
                        if (FindDFSShareDup(dfsRootPath, pulError, serversToSharesMap) == FALSE)
                        {					
                            printf("\nError! - Failed to follow DFS path, %ls.  Some shares may not be recorded.  Error code - %ld\n", dfsRootPath, *pulError);
                        }

                        delete[] dfsRootPath;
                    }
                }
                else
                {
                    printf("\nError! - Failed to determine if \\\\%ls\\%ls is a DFS root.  If it is, some shares may not be recorded.  Error code - %ld\n", lpwszServerName, pSharedInfo2Temp->shi2_netname, *pulError);
                }
            }

            pSharedInfo2Temp++;
        }

        if (dwSharesRead == 0)
        {
            printf("No shares found on workstation %ls", lpwszServerName);
        }
    }
    else 
    {
        printf("\nError! - Failed to enumerate shares on server, %ls.  Net API Status Error code: %ld.\n",lpwszServerName, napiStatus);
        *pulError = napiStatus;
        return FALSE;
    }

    //
    // Free the allocated buffer even if call failed
    //
    if (pSharedInfo2)
    {
        NetApiBufferFree(pSharedInfo2);
    }

    return TRUE;
}

/*************************** Main Function ********************************/

int _tmain(int argc, _TCHAR* argv[])
{
    ULONG		ulErrorCode;
    char 		szMachineName[MAX_NAME_LENGTH];
    WCHAR       wszMachineName[MAX_NAME_LENGTH];
    FILE		*pMachineFile;
    FILE		*pOutputFile;
    string      machineListType = CONFIG_FILE_ENTRY_TYPE_WORKSTATION;
    hash_map<wstring, FileShareRegistry*> serversToSharesMap;
	FILE *pFile = NULL;
	size_t *pReturnValue = NULL;
	
	wchar_t *next_token = NULL;
	
	errno_t err;

    if (argc != 3)
    {
        PrintUsage();
        return 1;
    }

    printf("Searching for network shares...\n");
	err = _wfopen_s(&pFile, argv[2], _T("r"));
	if(0 != err)
	{
		wprintf(L"\nError! - Failed to open config file, %s.  Please check path and file permissions and try again.\n",argv[2]);
        return 1;
	}
    if ((pMachineFile = pFile) == NULL)
    {
        wprintf(L"\nError! - Failed to open config file, %s.  Please check path and file permissions and try again.\n",argv[2]);
        return 1;
    }
	err = _wfopen_s(&pFile, argv[1], _T("w+"));
	if(0 != err)
	{
		wprintf(L"\nError! - Failed to open config file, %s.  Please check path and file permissions and try again.\n",argv[1]);
        return 1;
	}
    if ((pOutputFile = pFile) == NULL)
    {
        wprintf(L"\nError! - Failed to open OutputFile, %s.  Please check path and file permissions and try again.\n",argv[1]);
        fclose(pMachineFile);
        return 1;
    }

    memset(szMachineName, 0, MAX_NAME_LENGTH);
    while (fscanf_s(pMachineFile, "%s", szMachineName) != EOF)
    {
        if (strcmp(szMachineName, CONFIG_FILE_ENTRY_TYPE_DOMAIN_MAP) == 0)
        {
            machineListType = CONFIG_FILE_ENTRY_TYPE_DOMAIN_MAP;	
        }
        else if (strcmp(szMachineName, CONFIG_FILE_ENTRY_TYPE_DFS_DOMAIN) == 0)
        {
            machineListType = CONFIG_FILE_ENTRY_TYPE_DFS_DOMAIN;	
        } 
        else if (strcmp(szMachineName, CONFIG_FILE_ENTRY_TYPE_WORKSTATION) == 0)
        {
            machineListType = CONFIG_FILE_ENTRY_TYPE_WORKSTATION;	
        }
        else 
        {
            mbstowcs_s(pReturnValue, wszMachineName, MAX_NAME_LENGTH, szMachineName, _TRUNCATE);
            if (machineListType == CONFIG_FILE_ENTRY_TYPE_DOMAIN_MAP)
            {
                /*
                We've read a domain map entry NetBiosName=DNSDomainName (e.g. TEST=test.bluejungle.com)
                */
                LPWSTR netBiosName = wcstok_s(wszMachineName, L"=", &next_token);
                LPWSTR dnsDomainName = wcstok_s(NULL, L"=", &next_token);
                if ((netBiosName == NULL) || (dnsDomainName == NULL))
                {
                    printf("Error - Domain Map Entry, %s, is not valid.  Must be of the form NetBiosName = DNSDomanName.  Please correct the config file and try again.\n", szMachineName);
                    return 1;
                }

                ToLower(netBiosName);
                ToLower(dnsDomainName);

                LPWSTR netBiosNameCopy = new WCHAR[wcslen(netBiosName) + 1];
                wcsncpy_s(netBiosNameCopy, wcslen(netBiosName) + 1, netBiosName, _TRUNCATE);
                LPWSTR dnsDomainNameCopy = new WCHAR[wcslen(dnsDomainName) + 1];
                wcsncpy_s(dnsDomainNameCopy, wcslen(dnsDomainName) + 1, dnsDomainName, _TRUNCATE);

                printf("Adding domain mapping %ls=%ls\n", netBiosNameCopy, dnsDomainNameCopy);

                netbiosNameToDnsNameMap[netBiosNameCopy] = dnsDomainNameCopy;

				delete[] netBiosNameCopy;
				delete[] dnsDomainNameCopy;

            }
            else if (machineListType == CONFIG_FILE_ENTRY_TYPE_DFS_DOMAIN)
            {
                printf("\nProcessing Domain DFS, %s\n", szMachineName);
                if (FindDomainDFSShareDup(wszMachineName, &ulErrorCode, &serversToSharesMap) == FALSE)
                {
                    fprintf(pOutputFile,"%s %d\n", szMachineName, ulErrorCode);
                    printf("\nError! - Failed to process domain, %s.  Error code: %d\n", szMachineName, ulErrorCode);
                }
            } 
            else 
            {
                printf("\nProcessing Workstation, %s\n", szMachineName);
                if (FindDesktopAgentShareDup(wszMachineName, &ulErrorCode, &serversToSharesMap) == FALSE)
                {
                    fprintf(pOutputFile,"%s %d\n", szMachineName, ulErrorCode);
                    printf("\nError! - Failed to process workstation, %s.  Error code: %d\n", szMachineName, ulErrorCode);
                }
            }
        }

        memset(szMachineName, 0, MAX_NAME_LENGTH);
    }

    /*
    Write all data to file
    */
    hash_map<wstring, FileShareRegistry*>::iterator serversToSharesMapIterator; 
    for (serversToSharesMapIterator = serversToSharesMap.begin(); serversToSharesMapIterator != serversToSharesMap.end(); serversToSharesMapIterator++) {
        wstring machineName = serversToSharesMapIterator->first;
        wcstombs_s(pReturnValue, szMachineName, MAX_NAME_LENGTH, machineName.c_str(), _TRUNCATE);
        FileShareRegistry* shares = serversToSharesMapIterator->second;
        fprintf(pOutputFile,"%s 0\n", szMachineName);
        WriteToFile(pOutputFile, shares);
        delete shares;
    }

	netbiosNameToDnsNameMap.clear();

    fclose(pMachineFile);
    fclose(pOutputFile);

    printf("Execution complete.\n");

    return 0;
}

