/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * Service Class
 * January 2012 
 ***************************************************************************************/

#include <windows.h>
#include <string>
#include <iostream>
#include <list>

#include <tchar.h>
#include <strsafe.h>
#include <aclapi.h>
#include <stdio.h>

#include "nl_serviceCleanup.h"


// create services: not used - only for unit testing
int ServiceCleanup::createItems(itemList& items)
{
	printf("\nCreate items in ServiceCleanup\n");
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		createItem(*str);
	}	
	return 0;
};
// delete services
int ServiceCleanup::deleteItems(itemList& items)
{
	if( items.size() == 0 ) return 0;
	fprintf(m_File, "\n\nDelete services ...\n");
	printf("\n\nDelete services ...\n");

	TCHAR szSvcName[80];
	WCHAR errStr[1024];	

	SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
      SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
		int errorCode = GetLastError();
		getLastErrorStr(errorCode, errStr);
		if( errorCode ==  ERROR_ACCESS_DENIED )
		{
			printf("\tERROR: OpenSCManager failed - %ws. Administrator privilege is required.\n", errStr);
			fprintf(m_File, "\tERROR: OpenSCManager failed - ACCESS_DENIED. Administrator privilege is required.\n");
			fprintfLastError(errorCode);
			
		} else
		{
			fprintf(m_File, "\tERROR: OpenSCManager failed - %ws (%d)\n",errStr,  errorCode);
			printf("\tERROR: OpenSCManager failed - %ws (%d)\n", errStr, errorCode);
		}
		m_numErrors++;
        return m_numErrors;
    }
	// processing services
	itemListIter str = items.begin();
	for( ; str != items.end(); str++)
	{
		StringCchCopy(szSvcName, 80, (*str).c_str());
		// Get a handle to the service.
		schService = OpenService( 
			schSCManager,       // SCM database 
			szSvcName,          // name of service 
			DELETE);            // need delete access 
	 
		if (schService == NULL)
		{ 
			int errCode = GetLastError();
			getLastErrorStr(errCode, errStr);
			if( 1060 != errCode )
			{
				printf(" --- OpenService failed %ws(%d)\n", errStr, errCode); 
				fprintf(m_File, "\tservice %ls -- OpenService failed %ws(%d)\n",(*str).c_str(), errStr, errCode);
			}
			fprintf(m_File, "\tservice %ls not exists: %ws\n", (*str).c_str(), errStr );	
			continue;
		}

		// Delete the service.
	    if( m_Delete == true )
		{
			// delete service physically
			if ( !DeleteService(schService) ) 
			{
		        printf("\n\tService %ls  ",(*str).c_str()); 
		        fprintf(m_File, "\n\tservice %ls  ",(*str).c_str());
			    int errCode = GetLastError();
				getLastErrorStr(errCode, errStr);
				if(  errCode == ERROR_ACCESS_DENIED )
				{
				     m_numErrors++;
					 printf("\tDeleteService failed: ERROR code(%d)\n\t\t The handle does not have the DELETE access right.\n", errCode); 
			         fprintf(m_File,"\tDeleteService function failed: ERROR code (%d)\n\t\t The handle does not have the DELETE access right.\n", errCode); 
				} else
                if(  errCode == ERROR_INVALID_HANDLE  )
				{
				     m_numErrors++;
					 printf("\tDeleteService failed: ERROR code (%d)\n\t\t The specified handle is invalid\n", errCode); 
				     fprintf(m_File,"\tDeleteService function failed: ERROR code (%d)\n\t\t The specified handle is invalid\n", errCode); 
				} else
                if(  errCode == ERROR_SERVICE_MARKED_FOR_DELETE )
				{
				     printf("\tDeleteService failed: ERROR code (%d) \n\t\tThe specified service has already been marked for deletion\n", errCode); 
				     fprintf(m_File,"\tDeleteService function failed: ERROR code (%d) \n\t\tThe specified service has already been marked for deletion\n", errCode); 
				} else
				{
				     m_numErrors++;
			         printf("\tDeleteService function failed: ERROR code %ws(%d) \n", errStr, errCode); 
					 fprintf(m_File,"\tDeleteService function failed: ERROR code %ws(%d) \n", errStr, errCode); 
					 
				}
			}
			else 
			{
		        printf("\n\tDeleted service %ls (%ls)\n",(*str).c_str(), msg ); 
		        fprintf(m_File, "\n\tDeleted service %ls (%ls)\n",(*str).c_str(), msg);
			}

		} else
		{
			// just checking
		    printf("\n\tDeleted service %ls (%ls)\n",(*str).c_str(), msg ); 
		    fprintf(m_File, "\n\tDeleted service %ls (%ls)\n",(*str).c_str(), msg);
		}
		CloseServiceHandle(schService); 
	}
    CloseServiceHandle(schSCManager);
	return m_numErrors;	
};
