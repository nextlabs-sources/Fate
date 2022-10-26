/*
* parseargs.c    
* Author: Helen Friedland
* All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
* Redwood City CA, Ownership remains with Blue Jungle Inc, 
* All rights reserved worldwide. 
*
* This code is based on sample code provided by Microsoft
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <windows.h>

#define MAX_ARGLEN 4096

#define PREFIX1 "-D"
#define PREFIX2 "/D"
#define PREFIX3 "-X"
#define PREFIX4 "/X"
#define WRKDIR  "wrkdir="
#define FILESERVER_ARG "FileServer"

///////////////////////////////////////////////////////////////////////////
// Parse out the working directory out of the args
//////////////////////////////////////////////////////////////////////////

char* getWorkingDirectory(int dwArgc, char* *lpszArgv)
{
    int    i = 0;
    UINT    uCount = 0;
    size_t  iLen = 0; 
    char*   dir = NULL;
	
    iLen = strlen(WRKDIR);
    for(i=0,uCount=0; i < dwArgc; i++)
    {
        if(strlen(lpszArgv[i]) > iLen && !_strnicmp(lpszArgv[i],WRKDIR,iLen))
        {
            dir = lpszArgv[i]+iLen;
            if (dir [0] == '\"')
            {
                dir++;
                dir [strlen (dir) - 1] = '\0';
            }
			//Temporary fix for the installer. If the end is "\ ", take off the trailing
			//space and backslash
			if (dir [strlen(dir) - 2] == '\\' && dir [strlen(dir) - 1] == ' ')
			{
				dir [strlen (dir) - 2] = '\0';
			}
            return dir;
        }
    }

    return dir;
}

///////////////////////////////////////////////////////////////////////////
// Parse out the PDP type
//////////////////////////////////////////////////////////////////////////

bool getPDPType(int dwArgc, char* *lpszArgv)
{
    int    i = 0;
    size_t  iLen = 0; 
    bool bDesktop=true;

    iLen = strlen(FILESERVER_ARG);
    for(i=0; i < dwArgc; i++) {
      if(strlen(lpszArgv[i]) == iLen && 
	 _strnicmp(lpszArgv[i],FILESERVER_ARG,iLen)==0) {
	return false;
      } 	
    }
    return bDesktop;
}
////////////////////////////////////////////////////////////////////////////////////////
// Parse out Java args out of the args passed in to the service
////////////////////////////////////////////////////////////////////////////////////
char **getJavaArgs(char **lpszArgs, int* pdwLen, int dwArgc, char **lpszArgv)
{
    int    i,
        uCount;
    UINT j, index = 0;
    size_t argLen = 0;
    size_t iLen1 = 0; 
    size_t iLen2 = 0; 
    size_t iLen3 = 0; 
    size_t iLen4 = 0; 

    char   szArg[MAX_ARGLEN];

    iLen1 = strlen (PREFIX1);
    iLen2 = strlen (PREFIX2);
    iLen3 = strlen (PREFIX3);
    iLen4 = strlen (PREFIX4);

    // first count the number of java arguments
    for(i=0,uCount=0; i < dwArgc; i++)
    {
        if(!strncmp(lpszArgv[i],PREFIX1,iLen1) || !strncmp(lpszArgv[i],PREFIX2,iLen2)
            || !strncmp(lpszArgv[i],PREFIX3,iLen3) || !strncmp(lpszArgv[i],PREFIX4,iLen4))
        {
            uCount++;
        }
    }

    if(uCount == 0)
        return NULL;

    // allocate an array to hold the arguments
    lpszArgs = (LPSTR *)GlobalAlloc(GMEM_FIXED, uCount * sizeof(LPSTR));
	if (lpszArgs == NULL)
	{
		return NULL;
	}
    *pdwLen = uCount;

    // parse out the arguments again and save them into the array
    for(i=0,uCount=0; i < dwArgc; i++)
    {
        if(!strncmp(lpszArgv[i],PREFIX1,iLen1) || !strncmp(lpszArgv[i],PREFIX2,iLen2)
            || !strncmp(lpszArgv[i],PREFIX3,iLen3) || !strncmp(lpszArgv[i],PREFIX4,iLen4))
        {
            strncpy_s(szArg, MAX_ARGLEN, lpszArgv[i], _TRUNCATE);

	    #pragma warning(push)
	    #pragma warning(disable:6386)
            lpszArgs[uCount] = (LPSTR)GlobalAlloc(GMEM_FIXED,strlen(szArg)+1);
	    #pragma warning(pop)
			if (lpszArgs[uCount] == NULL)
			{
				return NULL;
			}

            argLen = strlen (szArg);
            for (j=0, index = 0; j < argLen; j++)
            {
                if (szArg[j] != '\"')
                {
                    lpszArgs[uCount][index++] = szArg[j];
                }
            }
	    #pragma warning(push)
	    #pragma warning(disable:6386)
            lpszArgs[uCount][index++] = '\0';
	    #pragma warning(pop)

            uCount++;
        }
    }

    return lpszArgs;
}

void pareseargs_free(char **args, int argc)
{
  for(int i=0; i<argc; i++)
    GlobalFree(args[i]);
  GlobalFree(args);
}

//NOP here
void parser_freeworkdir(char *dir)
{
  return;
}
////////////////////////////////////////////////////////////////////////////////////////
// Convert argument string (taken from the registry) to the argument array
// so that we can start  the service with the right arguments
/////////////////////////////////////////////////////////////////////////////////////////

LPSTR *convertArgStringToArgList(LPSTR *lpszArgs, PDWORD pdwLen, LPSTR lpszArgstring)
{
    UINT uCount;
    LPSTR lpszArg, lpszToken;
    char param [4096];


    if(strlen(lpszArgstring) == 0)
    {
        *pdwLen = 0;
        lpszArgs = NULL;
        return NULL;
    }

    if(NULL == (lpszArg = (LPSTR)GlobalAlloc(GMEM_FIXED,strlen(lpszArgstring)+1)))
    {
        *pdwLen = 0;
        lpszArgs = NULL;
        return NULL;
    }

    strncpy_s(lpszArg, strlen(lpszArgstring)+1, lpszArgstring, _TRUNCATE);

    char* next_token = NULL;
    lpszToken = strtok_s( lpszArg, " ", &next_token); 
    uCount = 0;
    while( lpszToken != NULL )
    {
        uCount++;
        if (strstr (lpszToken, "\"") != NULL)
        {
            do
            {
                lpszToken = strtok_s( NULL, " ", &next_token);   
            } while( lpszToken != NULL && strstr(lpszToken, "\"") == NULL);

        }
        lpszToken = strtok_s( NULL, " ", &next_token);   
    }

    GlobalFree((HGLOBAL)lpszArg);

    lpszArgs = (LPSTR *)GlobalAlloc(GMEM_FIXED,uCount * sizeof(LPSTR));
    *pdwLen = uCount;


    lpszToken = strtok_s(lpszArgstring," ", &next_token);
    uCount = 0;
    while(lpszToken != NULL)
    {
        strncpy_s (param, 4096, lpszToken, _TRUNCATE);
        if (strstr (lpszToken, "\"") != NULL)
        {
            do
            {
                lpszToken = strtok_s( NULL, " ", &next_token);   
                if (lpszToken)
                {
                    strncat_s (param, 4096, " ", _TRUNCATE);
                    strncat_s (param, 4096, lpszToken, _TRUNCATE);
                }
            } while( lpszToken != NULL && strstr(lpszToken, "\"") == NULL);

        }
	#pragma warning(push)
	#pragma warning(disable:6386)	
        lpszArgs[uCount] = (LPSTR)GlobalAlloc(GMEM_FIXED,strlen(param)+1);
	#pragma warning(pop)
		if (lpszArgs[uCount] != NULL)
		{
	        strncpy_s(lpszArgs[uCount],strlen(param)+1, param, _TRUNCATE);
	        uCount++;
		}
        lpszToken = strtok_s( NULL, " ", &next_token); 
    }


    return lpszArgs;

}
////////////////////////////////////////////////////////////////////////////////////////
// Convert argument array to the argument string
// so that we can save the list in the registry for later use
/////////////////////////////////////////////////////////////////////////////////////////
LPSTR convertArgListToArgString(LPSTR lpszTarget, DWORD dwStart, DWORD dwArgc, LPSTR *lpszArgv)
{
    UINT i;

    if(dwStart >= dwArgc)
    {
        return NULL;
    }

    *lpszTarget = 0;

    for(i=dwStart; i<dwArgc; i++)
    {
        if(i != dwStart)
        {
            strncat_s(lpszTarget, 10000, " ", _TRUNCATE);
        }
        strncat_s(lpszTarget, 10000, lpszArgv[i], _TRUNCATE);
    }

    return lpszTarget;
}

