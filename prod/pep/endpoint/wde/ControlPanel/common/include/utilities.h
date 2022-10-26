#pragma once

#include <string>
#include <shellapi.h>
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "celog.h"
#include "nlconfig.hpp"

using namespace std;

#define EDPM_MODULE_MAIN L"main"
#define EDPM_MODULE_DIAGS L"diagnostic"
#define EDPM_MODULE_UTILITIES L"utilities"
#define EDPM_MODULE_DLG L"dialog"
#define EDPM_MODULE_NOTIFY L"notify"
#define EDPM_MODULE_ENHANCE L"enhancement"

namespace edp_manager
{

	class CCommonUtilities
	{
	public:

		/*
		
		get edp manager installation path, indicated by InstallDir in registry.
		
		*/
		static bool GetComponentInstallPath(wstring& Path)
		{
			wchar_t installPath[1024] = {0};
#if 0
			if (!NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Desktop Enforcer", installPath, 1024))
			{
				return false;
			}
#else
            if (!NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\ControlPanelDir", installPath, 1024))
            {
                Path = wstring(installPath);
                return false;
            }
#endif
			Path = wstring(installPath);
			return true;
		}

		/*

		get pc installation path, indicated by InstallDir in registry.

		*/
		static bool GetPCInstallPath(wstring& path)
		{
			wchar_t installPath[1024] = {0};
			if (!NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller", installPath, 1024))
			{
				return false;
			}
		//	wcscat_s(installPath, 1024, L"Policy Controller\\");

			path = wstring(installPath);
			return true;			
		}


		/*
		we try to delete all files under Files.
		it is not assured files will be deleted due to permission reason.

		the folder itself will not be deleted.

		parameter:
		Files	--	should be "c:\folders" like, not "c:\folders\"

		*/
		static bool DeleteFiles(const wstring& Files)
		{   
			//	below is parameter checking ........................
			DWORD dwAttr = GetFileAttributesW(Files.c_str());

			if(INVALID_FILE_ATTRIBUTES == dwAttr)
			{
				//	invalid parameter
				return FALSE;
			}

			//	below is delete process	...............................
			//	check if Files is a directory
			if(FILE_ATTRIBUTE_DIRECTORY & dwAttr)
			{
				//	yes, Files is a directory, we need to enum all files/sub folders under Files,

				//	enum all files/sub folders under Files,
				WIN32_FIND_DATAW wfd = {0};

				std::wstring strFind = Files;
				strFind += L"\\*";

				//	find the first file/sub folder under pszSrc
				HANDLE  hFind = ::FindFirstFileW(strFind.c_str(), &wfd);

				//	check if the first file exists
				if(INVALID_HANDLE_VALUE != hFind)
				{
					//	yes, the first file exists
					std::wstring strFile;
					do 
					{
						if(0==wcscmp(wfd.cFileName, L".") || 0==wcscmp(wfd.cFileName, L".."))
						{
							//	this file/sub folder should be ignored.
							continue;
						}

						//	get this file/sub folder' full name
						strFile = Files; 
						strFile += L"\\"; 
						strFile += wfd.cFileName;

						//	check if this file/sub folder is a directory
						dwAttr = GetFileAttributesW(strFile.c_str());

						if(INVALID_FILE_ATTRIBUTES == dwAttr)
						{
							//	unexpected case;
							continue;
						}
						if(FILE_ATTRIBUTE_DIRECTORY & dwAttr)
						{
							//	this file/sub folder is a directory
							DeleteFiles(strFile);
						}
						else
						{
							//	this file/sub folder is a single file
							//	delete single file
							DeleteFile(strFile.c_str());
						}
					} while (::FindNextFileW(hFind, &wfd));

					//	files or child folders are all deleted
					//	try to delete empty folder
					//int ret = (int)ShellExecute(NULL, L"open", L"rmdir.exe", Files.c_str(), NULL, SW_HIDE);
					//ret = ret;
				}
				else	//	INVALID_HANDLE_VALUE != hFind
				{
					//	invalid input
					return FALSE;
				}
			}
			else
			{
				//	Files is not a folder, but a single file, this is invalid parameter, we only process folder
				return FALSE;
			}

			return TRUE;
		}




		/*

		DeleteFilesViaShell

		*/
		static bool DeleteFilesViaShell(wstring Files)
		{
			SHFILEOPSTRUCT   fo;   

			Files   +=   L'\0';   
			memset(&fo,   0,   sizeof(fo));   
			fo.fFlags   =   FOF_SILENT   |   FOF_NOCONFIRMATION;   
			fo.wFunc   =   FO_DELETE;   
			fo.pFrom   =   Files.c_str();

			return   SHFileOperation(&fo) == 0;
		}



		/*

		get module name



		*/
		static BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)
		{
			if(!lpszProcessName || nLen > 1024)
				return FALSE;

			if(hMod)
			{
				DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);
				return dwCount > 0 ? TRUE: FALSE;
			}

			static wchar_t filename[1025] = {0};
			if( *filename == 0 )//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
			{
				GetModuleFileNameW(hMod, filename, 1024);
			}

			if( *filename != 0 )
			{
				memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
				return TRUE;
			}

			return FALSE;
		}




		/*


		init celog



		*/
		static BOOL InitLog(CELog& instanceCelog, const wchar_t* strCompName)
		{
			// If debug mode is enabled write to log file as well
			if( NLConfig::IsDebugMode() == true )
			{
				/* Generate a path using the image name of the current process.  Set log policy for DebugView
				* and file on log instance.  Path will be [NextLabs]/Network Enforcer/diags/logs/.
				*/
				wchar_t image_name[MAX_PATH * 2 + 1] = {0};

				if( !GetModuleName(image_name, MAX_PATH * 2, NULL) )
				{
					return FALSE;
				}
				wchar_t* image_name_ptr = wcsrchr(image_name,'\\'); // get just image name w/o full path
				if( image_name_ptr == NULL )
				{
					image_name_ptr = image_name;                   // when failed use default image name
				}
				else
				{
					image_name_ptr++;                              // move past '\\'
				}

				wstring install_path;
				if (GetComponentInstallPath(install_path))
				{
					wchar_t log_file[MAX_PATH * 3 + 1] = {0};
					_snwprintf_s(log_file,_countof(log_file), _TRUNCATE,L"%s\\diags\\logs\\EDPControlPanel_%s.txt",install_path.c_str(),strCompName);

					wstring temp = wstring(log_file);
					std::string strLogPath = string( temp.begin(), temp.end() );
					instanceCelog.SetPolicy( new CELogPolicy_File(strLogPath.c_str()) );
				}
				instanceCelog.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
				instanceCelog.Enable();                              // enable log
				instanceCelog.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
			}

			return TRUE;
		}




	};
	

}

