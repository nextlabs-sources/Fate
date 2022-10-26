#include "StdAfx.h"
#include <fstream>
#include <string>
#include "nlconfig.hpp"
#include "LogMgrImp.h"
#include "RegUtility.h"
#include "sysutils.h"

using namespace std;

CLogMgrImp::CLogMgrImp(void)
{
	m_bVerlogEnabled = FALSE;

	m_bInited = FALSE;
}

CLogMgrImp::~CLogMgrImp(void)
{
}

BOOL CLogMgrImp::SetVerlogStatus(BOOL bEnable)
{
	if (!m_bInited)
	{
		//	we have not init verbose log status, init it first -- try to get current verbose log status
		BOOL bUnused;
		if (!GetVerlogStatus(bUnused) || !m_bInited)
		{
			//	get log status failed
			g_log.Log(CELOG_DEBUG, L"(!GetVerlogStatus(bUnused) || !m_bInited) failed in SetVerlogStatus\n");	
			return FALSE;
		}
	}

	//	we have initialized verbose log status, check it
	if (m_bVerlogEnabled == bEnable)
	{
		//	status incorrect, we assume this should not happen
		g_log.Log(CELOG_DEBUG, L"warning !!!!!!!!!		m_bVerlogEnabled == bEnable in SetVerlogStatus\n");
		return FALSE;
	}

	//	user want to turn on/off verbose log
	//	we need to 
	//	1, switch on/off debug mode in registry
	//	2, switch on/off agent log in pc folder
	if (!EnableDebugModeReg(bEnable))
	{
		g_log.Log(CELOG_DEBUG, L"warning !!!!!!!!!		EnableDebugModeReg failed\n");
		return FALSE;
	}

	//	debugmode in registry is set, let's switch on agentlog under pc folder
	//	we assume pc is not running
	if (!EnableAgentLog(bEnable))
	{
		g_log.Log(CELOG_DEBUG, L"warning !!!!!!!!!		EnableAgentLog failed\n");
		return FALSE;
	}

	//	set flag to \\Nextlabs\\EDP Manager\\VerboseLogEnabled
	if (!SetVerboseLogRegistry(bEnable))
	{
		g_log.Log(CELOG_DEBUG, L"warning !!!!!!!!!		SetVerboseLogRegistry failed\n");
		return FALSE;
	}

	//	we finish switch work
	m_bVerlogEnabled = bEnable;

	return TRUE;
}

BOOL CLogMgrImp::GetVerlogStatus(BOOL& bEnable)
{
	if (!m_bInited)
	{
		//	we have not init verbose log status, init it first -- try to get current verbose log status
		HKEY hEDPManager = NULL;
		if (!CRegUtility::OpenEDPMKey(&hEDPManager) || !hEDPManager)
			{
			//	open edp manager registry failed
			g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in GetVerlogStatus\n");	
			return FALSE;
		}
		
		//	opened, query key value
		BYTE keyValue[4] = {0};
		DWORD keyValueLen = 4;
		LONG rstatus = RegQueryValueExW(hEDPManager,EDPM_REG_VERBOSE_LOG,NULL,NULL,(LPBYTE)keyValue,&keyValueLen);
		if( rstatus != ERROR_SUCCESS )
		{
			g_log.Log(CELOG_DEBUG, L"no VerboseLogEnabled in registry, or no permission, we use default value - disabled\n");			
		}
		else
		{
			g_log.Log(CELOG_DEBUG, L"has VerboseLogEnabled in registry for the first time, %d\n", *((DWORD*)keyValue));
		}
		m_bInited = TRUE;
		m_bVerlogEnabled = *((DWORD*)keyValue);

		//	we have initialized verbose log status
		//	close key
		CRegUtility::CloseEDPMKey(&hEDPManager);
	}
	
	bEnable = m_bVerlogEnabled;
	return TRUE;
}

BOOL CLogMgrImp::EnableAgentLog(BOOL bOn)
{
#if 1
	return UAC_EnableAgentLog(bOn, NULL) == 0? TRUE: FALSE;
#else
	//	get logging.properties path first.
	//	get pc folder root path.
	wchar_t szPCDir[1024] = {0};
	if (!NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller", szPCDir, 1024))
	{
		return FALSE;
	}
	//	get logging.properties full file path
	wstring szPCLogProperties = (wstring)szPCDir + (wstring)L"Policy Controller\\config\\logging.properties";

	//	get temp logging.properties path
	wstring szTmpLogProperties = (wstring)szPCDir + (wstring)L"Policy Controller\\config\\logging_tmp.properties";
	
	//	open logging.properties as an input stream
	wifstream file(szPCLogProperties.c_str());

	//	open temp as an output stream
	wofstream temp_file(szTmpLogProperties.c_str());

	//	check if input stream is ok
	if(!file || !temp_file)
	{
		return FALSE ;
	}
	if( file.fail() || temp_file.fail())
	{
		return FALSE ;
	}
	//	try to read every line from input stream
	wchar_t buffer[BUFFER_SIZE] = {0} ;
	while( !file.eof())
	{
		//	we are trying to read every line from input stream, and copy the line to output stream
		file.getline(  buffer, BUFFER_SIZE )  ;

		if( wcsstr(buffer, L"com.bluejungle.level = ") )
		{
			//	we find the line "com.bluejungle.level = "
			//	we need to modify it to "FINEST"/"", so we modify content of this line, write the new data into output stream
			if (bOn)
			{
				temp_file << L"com.bluejungle.level = FINEST" << "\n";
			}
			else
			{
				temp_file << L"com.bluejungle.level = SEVERE" << "\n";
			}
		}
		else
		{
			//	we don't modify this line, we only modify the line contain "com.bluejungle.level = "
			temp_file << buffer << "\n";
		}
	}
	//	loop finish, all data are copied to temp file
	file.close() ;
	temp_file.close();

	//	delete original logging.properties, and rename logging_temp.properites to logging.properties
	DeleteFile(szPCLogProperties.c_str());
	string ansiTmp(szTmpLogProperties.begin(), szTmpLogProperties.end());
	string ansiFile(szPCLogProperties.begin(), szPCLogProperties.end());
	if (rename(ansiTmp.c_str(), ansiFile.c_str()))
	{
		//	rename failed, return error
		return FALSE;
	}

	//	ok, we finish agent log switch
	return TRUE;
#endif
}


BOOL CLogMgrImp::EnableDebugModeReg(BOOL bOn)
{
#if 1
	return UAC_SetDebugMode(bOn, NULL) == 0? TRUE: FALSE;
#else
	//	switch on debug mode in registry, this is under local machine, nextlabs\\debugmode -- DWORD
	HKEY hNextlabs = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"Software\\Nextlabs",0,KEY_SET_VALUE,&hNextlabs);
	if( rstatus != ERROR_SUCCESS )
	{
		//	there is no nextlabs, this is error. or there is no permission
		g_log.Log(CELOG_DEBUG, L"RegOpenKeyExW failed with KEY_SET_VALUE in EnableDebugModeReg\n");

		return FALSE;
	}
	//	opened, set debugmode value
	DWORD dwDebugmode = bOn ? 1 : 0;
	rstatus = RegSetValueEx(
		hNextlabs,
		L"debugmode",
		0,
		REG_DWORD,
		(BYTE*)&dwDebugmode,
		sizeof(dwDebugmode));
	if (rstatus)
	{
		//	error
		RegCloseKey(hNextlabs);
		return FALSE;
	}

	//	set Ok, close reg key handle and return true
	RegCloseKey(hNextlabs);
	return TRUE;
#endif
}

BOOL CLogMgrImp::SetVerboseLogRegistry(BOOL bOn)
{
	HKEY hEDPManager = NULL;
	if (!CRegUtility::OpenEDPMKey(&hEDPManager))
		{
		//	failed to open edp manager registry
		g_log.Log(CELOG_DEBUG, L"OpenEDPMKey failed in SetVerboseLogRegistry\n");
		return FALSE;
	}

	//	opened, set VerboseLogEnabled value
	DWORD dwOn = bOn ? 1 : 0;
	LONG rstatus = RegSetValueEx(
		hEDPManager,
		EDPM_REG_VERBOSE_LOG,
		0,
		REG_DWORD,
		(BYTE*)&dwOn,
		sizeof(dwOn));
	if (rstatus)
	{
		//	error
		CRegUtility::CloseEDPMKey(&hEDPManager);
		return FALSE;
	}

	//	set Ok, close reg key handle and return true
	CRegUtility::CloseEDPMKey(&hEDPManager);
	return TRUE;
}