#include "StdAfx.h"
#include <fstream>
#include <shellapi.h>
#include "CollectZipLog.h"
#include "EDPMgrUtilities.h"
#include "ThreadHelper.h"
#include "utilities.h"
#include "decrypt.h"
#include "AdapterBase.h"
#include "nlconfig.hpp"

#define BUFFER_SIZE 2048

//	temp folder for all diagnostic logs
//	we copy all logs into this temp folder, then, zip them
#define TEMP_DIAGNOSTIC_FOLDER L"temp_diags"

#define ENFORCER_DIAGS L"diags"

extern BOOL Decrypt(const wstring& strPassword);

static BOOL GetAgentLogFolder(const wstring& strPCDir, wstring& strLocation);


//	this is the fixed password for encryption, add for encryption, 9/23/2010
#define NL_EDPMGR_LOG_ENC_PWD L"!8k5m.support"

#define NL_EDPMGR_TMP_ZIP_NAME L"edplogs.zip"

static std::wstring GetNextLabsCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

//	get if we need to encrypt log files.
static BOOL GetEncryptFlag()
{
	HKEY hKey = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer",0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"GetEncryptFlag can't open key\n");
		return FALSE;
	}

	const wchar_t* key_name = L"Options";
	BYTE keyValue[4] = {0};
	DWORD keyValueLen = 4;
	rstatus = RegQueryValueExW(hKey,key_name,NULL,NULL,(LPBYTE)keyValue,&keyValueLen);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"GetEncryptFlag can't query key\n");
		return FALSE;
	}

	if( *((DWORD*)keyValue) & 0x1 )
	{
		//	bit 0 is not zero, so need to encrypt
		g_log.Log(CELOG_DEBUG, L"GetEncryptFlag key bit is set\n");
		return TRUE;
	}

	g_log.Log(CELOG_DEBUG, L"GetEncryptFlag key bit is not set\n");
	return FALSE;
}

/*
add for encryption, 9/23/2010

parameter:
hZipLib			--			[in]	handle to zip lib
wstrPwd			--			[in]	password
wstrSrc			--			[in]	full path name for src file to be encrypted
wstrDst			--			[in]	full path name for dst file, NOTICE that the real dst filename maybe changed by zip lib.
wstrRealDst		--			[out]	full path name for dst file.
wstrErrorInfo	--			[out]	When Encrypt return error, it contains error messages. 
*/
static EA_Error ZipWithEncryption(HMODULE hZipLib, 
								  const wstring& wstrPwd, 
								  const wstring& wstrSrc, 
								  const wstring& wstrDst, 
								  wstring& wstrRealDst, 
								  wstring& wstrErrorInfo)
{
	//	check input
	if (!hZipLib)
	{
		g_log.Log(CELOG_DEBUG, L"ZipWithEncryption failed in hZipLib is NULL\n");
		return EA_E_BADPASS;
	}

	//	try to get function from zip lib
	typedef EA_Error (WINAPI *EncryptType)(EncryptionAdapterData *lpData);
	EncryptType pEncryptFunc = (EncryptType)GetProcAddress(hZipLib, "Encrypt");

	if (!pEncryptFunc)
	{
		g_log.Log(CELOG_DEBUG, L"ZipWithEncryption failed in hZipLib has no exported Encrypt\n");
		return EA_E_BADPASS;
	}
	
	//	parse wstrDst, try to get pure path and pure filename
	wstring::size_type pos = wstrDst.rfind(L'\\');
	if ( wstring::npos == pos )
	{
		//	input error
		g_log.Log(CELOG_DEBUG, L"ZipWithEncryption failed in bad wstrDst, %s\n", wstrDst.c_str());
		return EA_E_BADPASS;
	}

	//	yes, we can get pure path and pure filename
	wstring wstrPurePath = wstrDst.substr(0, pos);
	wstring wstrPureName = wstrDst.substr(pos + 1, wstrDst.length() - pos - 1);
	
	//	trip file extension if exist
	pos = wstrPureName.rfind(L'.');
	if (pos != wstring::npos)
	{
		//	trip extension
		wstrPureName = wstrPureName.substr(0, pos);
	}

	//	before zip, try delete zip file, because if the zip file is already exist, zipping will fail
	if (wstrDst.c_str())
	{
		DeleteFile(wstrDst.c_str());
	}

	//	ok, continue to work.
	EncryptionAdapterData encData;
	encData.wstrSrcFile = wstrSrc;
	encData.wstrBaseFileName = wstrPureName;
	encData.wstrDstFolder = wstrPurePath;
	encData.encryptContext.wstrPassword = wstrPwd;

	EA_Error eCode = pEncryptFunc(&encData);

	//	handle result
	g_log.Log(CELOG_DEBUG, L"ZipWithEncryption finished, eCode %d, %s, %s\n", \
		eCode, encData.wstrActualDstFile.c_str(), encData.wstrErrorInfo.length() ? encData.wstrErrorInfo.c_str() : L"no information" );

	wstrRealDst = encData.wstrActualDstFile;
	wstrErrorInfo = encData.wstrErrorInfo;

	return eCode;
}


CCollectZipLog::CCollectZipLog()
{
	m_pThreadHelper = NULL;
	m_pOnCompletedFunc = NULL;
	m_pOnCompletedParam = NULL;

	::InitializeCriticalSection(&m_completionOrCanceledCS);
	m_bCompletedOrCanceled = FALSE;
}

CCollectZipLog::~CCollectZipLog(void)
{
	if (m_pThreadHelper)
	{
		if (!m_pThreadHelper->IsRunning())
		{
			//	only free resource when thread is stopped
			delete m_pThreadHelper;
			m_pThreadHelper = NULL;
		}
	}

	::DeleteCriticalSection(&m_completionOrCanceledCS);
}


CCollectZipLog& CCollectZipLog::GetInstance()
{
	static CCollectZipLog collectAndZiper;
	return collectAndZiper;
}



/*

async function, when collect/zip is finished, will call callback.

collect logs and zip it to specified location. 



parameter:

pOnCompleted	--	callback called when collect/zip is completed.

pwd			--		password string for decrypting bundle.

pszLocation		--	the location is specified by pszLocation in construction.

return result:

true --  collect and zip is started;
false -- failed to start


*/
BOOL CCollectZipLog::CollectAndZip(const wstring & pwd, wchar_t* pszLocation, OnCompletedType pOnCompleted, PVOID param)
{
	if (!pszLocation || !pOnCompleted)
	{
		return FALSE;
	}
	
	m_pOnCompletedFunc = pOnCompleted;
	m_pOnCompletedParam = param;
	m_pwd = pwd;
	m_Location = pszLocation;

	//	initialize thread object once
	if (!m_pThreadHelper)
	{
		m_pThreadHelper = new CThreadHelper(ThreadProc, this);
		if (!m_pThreadHelper)
		{
			return FALSE;
		}
	}

	//	check if collect and zipping is already started
	if (m_pThreadHelper->IsRunning())
	{
		//	it is in working status
		//	you can not ask to start a collect and zip when it is already started.
		return TRUE;
	}

	//	collect and zipping is not started, so we can 
	//	reset m_bCompletedOrCanceled and start collect and zipping
	m_bCompletedOrCanceled = FALSE;

	if (!m_pThreadHelper->Create())
	{
		return FALSE;
	}
	if (!m_pThreadHelper->Resume())
	{
		return FALSE;
	}

	return TRUE;
}


/*

copy folder

pszSrc		 --	 source folder full name, its format is L"c:\\source", not L"c:\\source\\"
pszDst		--	dest folder full name, its format is L"c:\\source", not L"c:\\source\\"

*/
BOOL CCollectZipLog::CopyFolder(const wchar_t* pszSrc, const wchar_t* pszDst)
{
	//	below is parameter checking ........................
	DWORD dwAttr = 0;

	if(!pszSrc || !pszDst)
	{
		//	invalid parameter
		return FALSE;
	}

	dwAttr = GetFileAttributesW(pszSrc);

	if(INVALID_FILE_ATTRIBUTES == dwAttr)
	{
		//	invalid parameter
		g_log.Log(CELOG_DEBUG, L"GetFileAttributesW failed in CopyFolder\n");
		return FALSE;
	}

	//	below is copy process	...............................
	//	check if pszSrc is a directory
	if(FILE_ATTRIBUTE_DIRECTORY & dwAttr)
	{
		//	yes, pszSrc is a directory, we need to enum all files/sub folders under pszSrc,

		//	but firstly check if pszDst exists, if it doesn't exists, create an empty folder
		{
			if ( INVALID_FILE_ATTRIBUTES == GetFileAttributes(pszDst) )
			{
				//	directory don't exist
				//	create it
				if (!CreateDirectory(pszDst, NULL))
				{
					//	unexpected case
					g_log.Log(CELOG_DEBUG, L"CreateDirectory %s failed in CopyFolder, error code %d\n", pszDst, GetLastError());
					return FALSE;
				}
			}
		}

		//	enum all files/sub folders under pszSrc,
		WIN32_FIND_DATAW wfd = {0};

		std::wstring strFind = pszSrc;
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
				strFile = pszSrc; 
				strFile += L"\\"; 
				strFile += wfd.cFileName;

				//	check if this file/sub folder is a directory
				dwAttr = GetFileAttributesW(strFile.c_str());

				if(INVALID_FILE_ATTRIBUTES == dwAttr)
				{
					//	unexpected case;
					g_log.Log(CELOG_DEBUG, L"GetFileAttributesW %s failed in CopyFolder\n", strFile.c_str());
					return FALSE;
				}
				if(FILE_ATTRIBUTE_DIRECTORY & dwAttr)
				{
					//	this file/sub folder is a directory

					//	get new pszDst first.
					wchar_t szTemp[1024] = {0};
					_tcsncpy_s(szTemp, 1024, strFile.c_str(), _TRUNCATE);
					wchar_t* pTemp = szTemp + _tcslen(pszSrc);
					wchar_t szNewDst[1024] = {0};
					_tcsncpy_s(szNewDst, 1024, pszDst, _TRUNCATE);
					_tcsncat_s(szNewDst, 1024, pTemp, _TRUNCATE);

					if (!CopyFolder(strFile.c_str(), szNewDst))
					{
						g_log.Log(CELOG_DEBUG, L"CopyFolder %s to %s failed\n", strFile.c_str(), szNewDst);
						return FALSE;
					}
					
				}
				else
				{
					//	this file/sub folder is a single file
					//	copy this file to dest folder

					//	get new pszDst first.
					wchar_t szTemp[1024] = {0};
					_tcsncpy_s(szTemp, 1024, strFile.c_str(), _TRUNCATE);
					wchar_t* pTemp = szTemp + _tcslen(pszSrc);
					wchar_t szNewDst[1024] = {0};
					_tcsncpy_s(szNewDst, 1024, pszDst, _TRUNCATE);
					_tcsncat_s(szNewDst, 1024, pTemp, _TRUNCATE);

					//	copy single file
					if (!CopyFile(strFile.c_str(), szNewDst, FALSE))
					{
						DeleteFile(szNewDst);
						if (!CopyFile(strFile.c_str(), szNewDst, FALSE))
						{
							//	try again
							g_log.Log(CELOG_DEBUG, L"CopyFolder %s to %s failed\n", strFile.c_str(), szNewDst);
							return FALSE;
						}
					}

				}
			} while (::FindNextFileW(hFind, &wfd));
		}
		else	//	INVALID_HANDLE_VALUE != hFind
		{
			//	unexpected case
			g_log.Log(CELOG_DEBUG, L"CopyFolder unexpected case\n");
			return FALSE;
		}
	}
	else
	{
		//	pszSrc is not a folder, but a single file, this is invalid parameter, we assume CopyFolder only process folder
		return FALSE;
	}


	return TRUE;
}

DWORD  CCollectZipLog::ThreadProc(LPVOID lpParameter)
{
	DIAGNSOTIC_ERROR_CODE res = E_OK;

	HMODULE hLib = NULL;

	wstring installPath;

	vector<wstring> vFiles;
	vector< pair<wstring, wstring> > v_NameDirPair;
	static wstring strCommonPath(L"");

	//	add these two variables, used by second encryption zip.
	wstring wstrErrorInfo;
	wstring wstrRealDst;

	BOOL bEncrypt = FALSE;
	wstring wstrFirstZip;

	CCollectZipLog* pthis = (CCollectZipLog*)lpParameter;

	if (!lpParameter)
	{
		g_log.Log(CELOG_DEBUG, L"diagnosticor invalid parameter\n");
		res = E_ERROR;
		goto FUN_EXIT;
	}

	

	//	check PC status
	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
	if (utilities.IsPCRunning())
	{
		//	you can not ask to start a collect and zip when pc is running, 
		//	impossible case.
		g_log.Log(CELOG_DEBUG, L"diagnosticor while pc is running\n");
		res = E_ERROR;
		goto FUN_EXIT;
	}

	//	check if we need to encrypt..............
	bEncrypt = GetEncryptFlag();

	//	figure out the target zip filename of the first zipping
	if (bEncrypt)
	{
		//	we are using a fixed temp filename as the target filename for first zipping
		wchar_t szTempFolder[1024] = {0};
		DWORD dwLen = GetTempPath(MAX_PATH, szTempFolder);
		wchar_t dstTmp[1024] = {0};

		if (szTempFolder[dwLen-1] == L'\\')
		{
			_snwprintf_s(dstTmp, 1024, _TRUNCATE, L"%s%s", szTempFolder, NL_EDPMGR_TMP_ZIP_NAME);
		}
		else
		{
			_snwprintf_s(dstTmp, 1024, _TRUNCATE, L"%s\\%s", szTempFolder, NL_EDPMGR_TMP_ZIP_NAME);
		}

		wstrFirstZip = dstTmp;
	}
	else
	{
		wstrFirstZip = pthis->m_Location;
	}

	//	get installed components' name and dir pair.
	if ( !utilities.GetComponentsInstallDir(v_NameDirPair) )
	{
		//	error
		g_log.Log(CELOG_DEBUG, L"diagnosticor GetComponentsInstallDir failed\n");
		res = E_ERROR;
		goto FUN_EXIT;
	}

	//	we need to get 
	//	1, agent log, it is under PC folder.
	//  2, bundle.bin->bundle.out, it is under PC folder.
	//	3, "diags" folder under each component except PC.

	for (DWORD i = 0; i < v_NameDirPair.size(); i++)
	{
		if (v_NameDirPair[i].first == L"NextLabs Policy Controller")
		{

			
			g_log.Log(CELOG_DEBUG, L"policy controller: %s\n", v_NameDirPair[i].second.c_str());


			//	this is policy controller
			//	process agent log	.....................................
			//	learn where is agent log
			wstring strLocation;
			if (GetAgentLogFolder(v_NameDirPair[i].second, strLocation))
			{
			//	push it into vFiles
				vFiles.push_back(strLocation);

				g_log.Log(CELOG_DEBUG, L"agent log: %s\n", strLocation.c_str());
			}	

			//	decrypt and copy bundle.out ............................
			//	decrypt bundle.out
			//CDecryptBundle::DecryptViaTool(pthis->m_pwd);
			CoInitialize(0);
			CDecryptBundle::DecryptWithEnh(pthis->m_pwd);
			CoUninitialize();

			//	process bundle.out
			//	learn where is bundle.out
			wchar_t src[1024] = {0};
			wcsncpy_s(src, 1024, v_NameDirPair[i].second.c_str(), _TRUNCATE);
			wcsncat_s(src, 1024, L"bundle.out", _TRUNCATE);

			//	process bundle.out
			vFiles.push_back(src);

			g_log.Log(CELOG_DEBUG, L"bundle: %s\n", src);
		}
		else
		{
			//	no, current component is not policy controller
			//	process it diags folder....................................................

			//	copy "diags" folder to temp_logs\\componentname\\diags folder

			//	learn where is component diags log
			wchar_t src[1024] = {0};
			wcsncpy_s(src, 1024, v_NameDirPair[i].second.c_str(), _TRUNCATE);
			wcsncat_s(src, 1024, ENFORCER_DIAGS, _TRUNCATE);





			//	learn where should diags log folder should be copied to 
			//	installPath\bin
			wchar_t szTempPath[1024] = {0};
			GetTempPath(MAX_PATH, szTempPath);

			wchar_t dst[1024] = {0};
			_snwprintf_s(dst, 1024, _TRUNCATE, L"%s\\%s_diags", szTempPath, v_NameDirPair[i].first.c_str());

			g_log.Log(CELOG_DEBUG, L"try to CopyFolder from [%s] to [%s]\n", src, dst);

			//	copy folder
			CopyFolder(src, dst);

			//	process it
			vFiles.push_back(dst);

			
		}
	}


	//	all logs are recorded to vFiles, zip it to target location..............
	//	load zip adapter dll
	//	load library
	

	if(strCommonPath.empty())
	{
		strCommonPath = GetNextLabsCommonComponentsDir();
	}

#ifdef _WIN64
	installPath = strCommonPath + wstring(L"zip_adapter.dll");
#else
	installPath = strCommonPath + wstring(L"zip_adapter32.dll");
#endif

	
	hLib = LoadLibraryW(installPath.c_str());
	if (!hLib)
	{
		//	load failed
		g_log.Log(CELOG_DEBUG, L"LoadLibrary zip_adapter failed\n");
		res = E_ERROR;
		goto FUN_EXIT;
	}
g_log.Log(CELOG_DEBUG, L"Load %s successfully\r\n", installPath.c_str());

	//	get function
	ZipFileType pf = (ZipFileType)GetProcAddress(hLib, "ZipFile");

	if (!pf)
	{
		g_log.Log(CELOG_DEBUG, L"GetProcAddress zip failed\n");
		//	GetProcAddress failed
		res = E_ERROR;
		goto FUN_EXIT;
	}


	//	zipping first time..........................

	//	delete zip file before zipping, because if the zip file is already exist, zipping will fail
	if (wstrFirstZip.c_str())
	{
		DeleteFile(wstrFirstZip.c_str());
	}

	//	ok, start first zip
	g_log.Log(CELOG_DEBUG, L"diagnosticor started zip .....\n");

	int ret = pf(vFiles, wstrFirstZip.c_str());
	if (ret)
	{
		g_log.Log(CELOG_DEBUG, L"diagnosticor zip failed %d, we try to zip to %s\n", ret, wstrFirstZip.c_str());

		for (vector<wstring>::iterator it = vFiles.begin(); it != vFiles.end(); it++)
		{
			g_log.Log(CELOG_DEBUG, L"diagnosticor try to zip %s\n", (*it).c_str());
		}

		res = E_ERROR;
		goto FUN_EXIT;
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"diagnosticor zip succeed, we zipped the following files\n");
		for (DWORD i = 0; i < vFiles.size(); i++)
		{
			g_log.Log(CELOG_DEBUG, L"%s\n", vFiles[i].c_str());
		}
	}

	//	check if we need to zip the second time
	if (bEncrypt)
	{
		//	ok, we finished the first zip, then, we need to zip again with encryption.............................
		//	we add this encryption code 9/23/2010
		EA_Error eError = ZipWithEncryption(hLib, NL_EDPMGR_LOG_ENC_PWD, wstrFirstZip.c_str(), pthis->m_Location, wstrRealDst, wstrErrorInfo);
		if (eError != EA_OK)
		{
			//	delete temp zip file which is used for first zipping
			DeleteFile(wstrFirstZip.c_str());

			g_log.Log(CELOG_DEBUG, L"diagnosticor zip the second time failed %d, we try to zip to %s, error msg %s\n", \
				eError, pthis->m_Location.c_str(), wstrErrorInfo.c_str());
			res = E_ERROR;
			goto FUN_EXIT;
		}
		else
		{
			//	delete temp zip file which is used for first zipping
			DeleteFile(wstrFirstZip.c_str());

			g_log.Log(CELOG_DEBUG, L"diagnosticor zip the second time succeed, we really zipped to %s\n", wstrRealDst.c_str());
		}
	}

	//	done
	res = E_OK;

FUN_EXIT:
	//	delete temp_diags folder
	{
		for(DWORD i = 0; i < vFiles.size(); i++)
		{
			if(vFiles[i].find(L"_diags") != wstring::npos)
			{
				if (!edp_manager::CCommonUtilities::DeleteFiles(vFiles[i].c_str()))
				{
					g_log.Log(CELOG_DEBUG, L"delete %s failed\n", vFiles[i].c_str());
				}
				
			}
		}
	}

	//	free resource
	if (hLib)
	{
		FreeLibrary(hLib);	//	zipping process finished
	}

	//	call on completed
	if (pthis && pthis->m_pOnCompletedFunc)
	{
		//	lock before try to notify user completion event
		::EnterCriticalSection(&pthis->m_completionOrCanceledCS);
		//	check if user have canceled the collect and zipping process
		if (!pthis->m_bCompletedOrCanceled)
		{
			//	no, user have not canceled, so we can notify user the completion event,	
			//	and set m_bCompletedOrCanceled then user can't cancel it.
			pthis->m_pOnCompletedFunc(pthis->m_pOnCompletedParam, res);
			pthis->m_bCompletedOrCanceled = TRUE;
		}
		else
		{
			//	yes, user have canceled, so we do nothing
			g_log.Log(CELOG_DEBUG, L"diagnosticor completed with res %d, but user canceled diagnostic, so we don't callback to notify completion\n", res);
		}
		::LeaveCriticalSection(&pthis->m_completionOrCanceledCS);
	}

	return 0;
}

BOOL CCollectZipLog::Cancel()
{
	if (!m_pThreadHelper)
	{
		//	this is a invalid status calling
		return FALSE;	
	}

	if (m_pThreadHelper->IsRunning())
	{
		//	lock before try to cancel collect and zipping
		::EnterCriticalSection(&m_completionOrCanceledCS);
		if (!m_bCompletedOrCanceled)
		{
			//	we only try to kill collect and zipping process while we have not notify user completion event.
			m_pThreadHelper->Kill();
			//	set m_bCompletedOrCanceled, then we will not notify user completion event.
			m_bCompletedOrCanceled = TRUE;
		}
		else
		{
			//	we already notify user completion event, so we can't cancel the process.
			g_log.Log(CELOG_DEBUG, L"user want to cancel diagnostic, but diagnostic have completed already, so we do nothing\n");
			::LeaveCriticalSection(&m_completionOrCanceledCS);
			return FALSE;
		}
		::LeaveCriticalSection(&m_completionOrCanceledCS);
	}
	else
	{
		//	this is a invalid status calling
		return FALSE;
	}

	return TRUE;
}

BOOL CCollectZipLog::QueryState(WORKING_STATUS& status)
{
	if (!m_pThreadHelper)
	{
		return FALSE;
	}

	BOOL res = m_pThreadHelper->IsRunning();

	status = res ? E_COLLECT_AND_ZIPPING : E_IDLE;

	return TRUE;
}

/*

copy folder using shellexecute

*/
BOOL CCollectZipLog::CopyFolderByShell(const wchar_t* pszSrc, const wchar_t* pszDst)
{
	wchar_t parameter[1024];

	_snwprintf_s(parameter, 1024, _TRUNCATE, L"%s %s\\ /E", pszSrc, pszDst);

	ShellExecute(NULL, L"open", L"xcopy", parameter, NULL, SW_HIDE);

	return TRUE;
}

BOOL GetAgentLogFolder(const wstring& strPCDir, wstring& strLocation)
{
	//	learn where is logging.properties first
	wchar_t szlogProperty[1024] = {0};
	wcsncpy_s(szlogProperty, 1024, strPCDir.c_str(), _TRUNCATE);
	wcsncat_s(szlogProperty, 1024, L"config\\logging.properties", _TRUNCATE);

	//	then, open logging.properties as an input stream
	wifstream file(szlogProperty);
	if(!file)
	{
		g_log.Log(CELOG_DEBUG, L"can't open %s\n", szlogProperty);
		return FALSE;
	}
	if( file.fail() )
	{
		g_log.Log(CELOG_DEBUG, L"failed to open %s\n", szlogProperty);
		return FALSE;
	}

	//	try to read every line from input stream
	wchar_t buffer[BUFFER_SIZE] = {0} ;
	BOOL bFound = FALSE;
	while( !file.eof())
	{
		//	we are trying to read every line from input stream, and copy the line to output stream
		file.getline(  buffer, BUFFER_SIZE )  ;

		if( wcsstr(buffer, L"java.util.logging.FileHandler.pattern =") )
		{
			//	found
			g_log.Log(CELOG_DEBUG, L"find agentlog folder in logging.properties\n");
			bFound = TRUE;

			//	get agentlog location out
			wchar_t* pValue = wcschr(buffer, L'=');
			if (!pValue)
			{
				g_log.Log(CELOG_DEBUG, L"can't find =, in fact this is impossible\n");
				return FALSE;
			}

			//	pass space after "="
			while ( *( ++pValue ) == L' ' )
			{
			}

			//	now, pValue points to the beginning of the location exactly
			//	try to get the last '/' at the end of the location
			wchar_t* pEnd = wcsrchr(pValue, L'/');
			if (!pEnd)
			{
				g_log.Log(CELOG_DEBUG, L"can't find the final / at the end of the location string, in fact this is impossible\n");
				return FALSE;
			}

			*pEnd = 0;
			strLocation = (wstring)pValue;
			g_log.Log(CELOG_DEBUG, L"agent log folder is at %s\n", strLocation.c_str());

			return TRUE;
		}
	}
	if ( FALSE == bFound)
	{
		//	didn't find, why
		g_log.Log(CELOG_DEBUG, L"can't find agentlog folder in logging.properties\n");
		return FALSE;
	}

	return FALSE;
}

