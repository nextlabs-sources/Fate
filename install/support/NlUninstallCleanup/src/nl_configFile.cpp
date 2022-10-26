/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * ConfigFile Class
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

#include "nl_configFile.h"

// read config file
int ConfigFile::read_config_file(std::wstring filename, FILE* logFile, ItemsList& itemsList)
{
	enum CleanType cType;
	printf("\nConfig File : %ls ", filename.c_str());
	fprintf(logFile, "\nConfig File : %ls ", filename.c_str());
	FILE * pFile;
	wchar_t oneline[MAX_LENGTH];
	//wchar_t newlineinfo[MAX_LENGTH];
	wchar_t tmponeline[MAX_LENGTH];
	int noSyntaxError;
	int errorCode = _wfopen_s (&pFile,filename.c_str(),L"r");
	if( errorCode != 0 )
	{
		printf("\nConfig File open ERROR: %ls ", filename.c_str());
		return CLEANUP_CFG_FILE_READ_ERROR;
	}
	if (pFile != NULL)
	{
		//fputs ("fopen example",pFile);
		cType = CleanLast;
		noSyntaxError =0;
		while (fgetws(oneline, MAX_LENGTH/*sizeof(oneline)*/, pFile)) 
		{
			size_t len = wcslen(oneline);// Strip off trailing '\n' from the string           
			if( len > 0 && oneline[len-1] == L'\n' )                
				oneline[wcslen(oneline)-1] = 0;

			if( wcsncmp(oneline,L"#", 1) == 0 || wcscmp(oneline,L"\0") == 0)
			{
				continue;// skip comment line or blank line
			} else
			if( wcsncmp(oneline,L"[", 1) == 0)
			{
				if( wcscmp(oneline,L"[FileDir]") == 0)  // file or dir entry
				{
					cType = CleanFileDir;
					printf("\n\t%ls", oneline);
					fprintf(logFile, "\n\t%ls", oneline);
					continue;
				} else
				if( wcscmp(oneline,L"[Service]") == 0) // service entry
				{
					cType = CleanService;
					printf("\n\t%ls", oneline);
					fprintf(logFile, "\n\t%ls", oneline);
					continue;
				} else
				if( wcscmp(oneline,L"[Registry]") == 0) // registry entry
				{
					cType = CleanRegistry;
					printf("\n\t%ls", oneline);
					fprintf(logFile, "\n\t%ls", oneline);
					continue;

				} else
				{
					noSyntaxError++;
					printf("\nConfig file format ERROR: ");
					continue; //return CLEANUP_CFG_FILE_READ_ERROR;
				}
			}
			printf("\n\t%ls", oneline);
			fprintf(logFile, "\n\t%ls", oneline);
#if 1
			// support system/user environment variables
			wcscpy_s(tmponeline, MAX_LENGTH, oneline);
			size_t tt;
			if( ( wcsncmp(tmponeline , L"%", 1) == 0 )
				|| ( wcsncmp(tmponeline , L"-%", 2) == 0 )  )
			{
				wchar_t* startPtr = tmponeline+1;
				wchar_t* endPtr = tmponeline+1;
				tt = 1;
				if( wcsncmp(tmponeline , L"-%", 2) == 0 )
				{
					startPtr++;
					endPtr++;
					tt++;
				}
				bool foundEnvVariable = false;
				for( ; tt < wcslen(tmponeline); tt++ )
				{
					if( wcsncmp(&tmponeline[tt] , L"%", 1) == 0 )
					{
						tmponeline[tt] = '\0';
						endPtr++;
						foundEnvVariable = true;
						break;
					}
					endPtr++;
				}
				if( foundEnvVariable == false )
				{
					noSyntaxError++;
					printf("\nConfig file format ERROR: check environment variable");
					continue; //return CLEANUP_CFG_FILE_PARSE_ERROR;
				}
				// startPtr represents environment variable
				
				size_t pinx = 0;
				if( wcsncmp(tmponeline , L"-", 1) == 0 )
				{
					// copy '-' symbol to newlineinfo
					//newlineinfo[0] = tmponeline[0];
					pinx = 1;
				}
				size_t requiredSize = MAX_LENGTH-pinx;
				int errno_t = _wgetenv_s(&requiredSize, oneline+pinx, requiredSize,startPtr); 
				if( (errno_t != 0 ) || requiredSize == 0 )
				{
					noSyntaxError++;
					printf("\nConfig file format ERROR: check environment variable");
					continue; //return CLEANUP_CFG_FILE_PARSE_ERROR;
				}
				// wcslen: The terminating null wide-character code is not included 
				// in the number count. 
				//size_t strlength = wcslen(newlineinfo);

				swprintf_s(oneline+requiredSize-1+pinx, MAX_LENGTH-requiredSize+1-pinx, L"%ws",endPtr);
				//wcscpy_s(newlineinfo+requiredSize-1+pinx, MAX_LENGTH-requiredSize+1-pinx, endPtr );
				//wcscpy_s(oneline, MAX_LENGTH, newlineinfo);

			} else
			{
				// check '%' symbol
				for( tt =1; tt < wcslen(oneline); tt++ )
				{
					if( wcsncmp(&oneline[tt] , L"%", 1) == 0 )
					{
						noSyntaxError++;
						printf("\nConfig file format ERROR: found invalid char");
						continue; //return CLEANUP_CFG_FILE_READ_ERROR;
					}
				}
			}
#else
			// support system/user environment variables
			size_t tt;
			if( ( wcsncmp(oneline , L"%", 1) == 0 )
				|| ( wcsncmp(oneline , L"-%", 2) == 0 )  )
			{
				wchar_t* startPtr = oneline+1;
				wchar_t* endPtr = oneline+1;
				tt = 1;
				if( wcsncmp(oneline , L"-%", 2) == 0 )
				{
					startPtr++;
					endPtr++;
					tt++;
				}
				bool foundEnvVariable = false;
				for( ; tt < wcslen(oneline); tt++ )
				{
					if( wcsncmp(&oneline[tt] , L"%", 1) == 0 )
					{
						oneline[tt] = '\0';
						endPtr++;
						foundEnvVariable = true;
						break;
					}
					endPtr++;
				}
				if( foundEnvVariable == false )
				{
					noSyntaxError++;
					printf("\nConfig file format ERROR: check environment variable");
					continue; //return CLEANUP_CFG_FILE_PARSE_ERROR;
				}
				// startPtr represents environment variable
				
				size_t pinx = 0;
				if( wcsncmp(oneline , L"-", 1) == 0 )
				{
					// copy '-' symbol to newlineinfo
					newlineinfo[0] = oneline[0];
					pinx = 1;
				}
				size_t requiredSize = MAX_LENGTH-pinx;
				int errno_t = _wgetenv_s(&requiredSize, newlineinfo+pinx, requiredSize,startPtr); 
				if( (errno_t != 0 ) || requiredSize == 0 )
				{
					noSyntaxError++;
					printf("\nConfig file format ERROR: check environment variable");
					continue; //return CLEANUP_CFG_FILE_PARSE_ERROR;
				}
				// wcslen: The terminating null wide-character code is not included 
				// in the number count. 
				//size_t strlength = wcslen(newlineinfo);
				wcscpy_s(newlineinfo+requiredSize-1+pinx, MAX_LENGTH-requiredSize+1-pinx, endPtr );
				wcscpy_s(oneline, MAX_LENGTH, newlineinfo);

			} else
			{
				// check '%' symbol
				for( tt =1; tt < wcslen(oneline); tt++ )
				{
					if( wcsncmp(&oneline[tt] , L"%", 1) == 0 )
					{
						noSyntaxError++;
						printf("\nConfig file format ERROR: found invalid char");
						continue; //return CLEANUP_CFG_FILE_READ_ERROR;
					}
				}
			}
#endif
			switch (cType) {
				// add new item
				case CleanFileDir:
					itemsList.filepaths.push_back(wstring(oneline));
					break;
				case CleanService:
					itemsList.services.push_back(wstring(oneline));
					break;
				case CleanRegistry:
					itemsList.registries.push_back(wstring(oneline));
					break;
				case CleanLast:
				default:
					break;

			}
		}

		fclose (pFile);
		if(noSyntaxError > 0)
		{
			printf("\nConfig file format ERROR: found %d invalid lines", noSyntaxError);
			return CLEANUP_CFG_FILE_PARSE_ERROR;
		}
		return CLEANUP_SUCCESS;
	}
	return CLEANUP_CFG_FILE_READ_ERROR;
}
