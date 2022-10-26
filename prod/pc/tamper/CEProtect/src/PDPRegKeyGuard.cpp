/*========================PDPRegKeyGuard.cpp================================*
*                                                                          *
* All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
* San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
* All rights reserved worldwide.                                           *
*                                                                          * 
* Author :                                                                 *
* Date   : 08/16/2007                                                      *
* Note   : The registration key protection module.                         *
*          This is only applicable to Windows platform                     *
*==========================================================================*/
#if defined (WIN32) || defined (_WIN64)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <vector>
#include "cetype.h"
#include "PDPRegKeyGuard.h"
#include "nlTamperproofConfig.h"
#include "celog.h"

#define CONFIG_DIR _T("config")
#define HARDWARE_PROFILE_ROOT _T("SYSTEM\\CurrentControlSet\\Hardware Profiles")

//The followings are the protected registration keys of PDP
#define SAFEBOOT_MINIMAL_SERVICE _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\ComplianceEnforcerService")
#define SAFEBOOT_NETWORK_SERVICE _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\ComplianceEnforcerService")
#define SAFEBOOT_MINIMAL_NTPROCDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\NTProcDrv")
#define SAFEBOOT_NETWORK_NTPROCDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\NTProcDrv")
#define SAFEBOOT_MINIMAL_COREDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\DSCORE")
#define SAFEBOOT_NETWORK_COREDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\DSCORE")
#define SAFEBOOT_MINIMAL_IFSDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\dsifsflt")
#define SAFEBOOT_NETWORK_IFSDRV _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\dsifsflt")
#define AGENT_SERVICE_KEY _T("SYSTEM\\CurrentControlSet\\Services\\ComplianceEnforcerService")
#define AGENT_HARDWARE_PROFILE_KEY _T("LEGACY_COMPLIANCEENFORCERSERVICE")
#define CORE_DRIVER_KEY _T("SYSTEM\\CurrentControlSet\\Services\\dscore")
#define IFS_DRIVER_KEY _T("SYSTEM\\CurrentControlSet\\Services\\dsifsflt")
#define NTPROCDRV_KEY _T("SYSTEM\\CurrentControlSet\\Services\\ntprocdrv")
#define LOGON_NOTIFY_KEY _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\ComplianceEnforcerLogon")
#define POLICY_CONTROLLER_KEY _T("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller")
//ENd-- the protected registration keys of PDP

typedef enum _WinVersion
{
	emWinXP=0,
	emWin7=1,
	emWinUnknow=256
}WinVersion;

static WinVersion g_winVersion = emWinUnknow;
static BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;
	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;
		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}
		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;
		//	g_log.Log(CELOG_DEBUG, L"HTTPE::OS version, Major: %d, Minor: %d", sMajor, sMinor);
	}
	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;
	return TRUE;
}

static BOOL IsWin7(void /*include Vista*/)
{
	if(g_winVersion == emWinUnknow)
	{
		DWORD dwMajor = 0;
		DWORD dwMinor = 0;
		if(GetOSInfo(dwMajor, dwMinor) && dwMajor == 6)
		{
			g_winVersion = emWin7;
			TRACE(0, _T("running on [win7]\n"));
			return TRUE;
		}
		else
		{
			TRACE(0, _T("not running on [win7]\n"));
			g_winVersion = emWinXP;
			return FALSE;
		}
	}
	else
	{
		if(g_winVersion == emWin7)	
			return TRUE;
		else	
			return FALSE;
	}
}

typedef struct
{
	HKEY hRootKey;
	wstring wstrKeyName;
	wstring wstrKeyFullName;
}Parent_Key;

static Parent_Key g_szParentKey[] = {
	{HKEY_LOCAL_MACHINE, (wstring)(L"SOFTWARE\\NextLabs"), (wstring)(_T("HKEY_LOCAL_MACHINE")) + (wstring)(L"SOFTWARE\\NextLabs")},
	{HKEY_LOCAL_MACHINE, (wstring)(L"SOFTWARE\\Wow6432Node\\NextLabs"), (wstring)(_T("HKEY_LOCAL_MACHINE")) + (wstring)(L"SOFTWARE\\Wow6432Node\\NextLabs")}
};

using namespace PDPREGKEYGUARD;
/*==========================================================================*
* Interanl Global variables and functions scoped in this file.             *
*==========================================================================*/
namespace {
	//The only instance of RegKey guard
	RegKeyGuard regKeyGuard;
}

Request::Request(HKEY r, LPCTSTR k, OPERATION p)
{
	nlthread_mutex_init(&mutex);  
	nlthread_cond_init(&cond);
	rootKey=r;
	keyName=k;
	op=p;
	status=WAITING;
}

RegKeyGuard::RegKeyGuard(void)
{
	nlthread_mutex_init(&reqQMutex);

	m_stopEvent = NULL;
	m_reqEvent=NULL;

	m_eventArrayLen=1024;
	m_eventArrayTail=0;
	m_eventArray = new HANDLE[m_eventArrayLen];

	//Create pdp stop event and add to the event array
	m_stopEvent =  ::CreateEvent (NULL, FALSE, FALSE, NULL);
	m_eventArray[m_eventArrayTail]=m_stopEvent;

	//Create pdp request event and add to the event array
	m_eventArrayTail++;
	m_reqEvent =  ::CreateEvent (NULL, FALSE, FALSE, NULL);
	m_eventArray[m_eventArrayTail]=m_reqEvent;

	SetHardwareProfileKey(_T("LEGACY_COMPLIANCEENFORCERSERVICE"));
}

RegKeyGuard::~RegKeyGuard(void)
{
	removeKeys ();  
	::CloseHandle(m_stopEvent);
	::CloseHandle(m_reqEvent);
	delete [] m_eventArray;   
	nlthread_mutex_destroy(&reqQMutex);
}

// Remove all the keys from the structure, better grouping of actions
// to avoid missing something (e.g. like closing the keys, but not deleting files
void RegKeyGuard::removeKeys(void)
{
	std::map<wstring, KeyToFileMapping *>::iterator it;

	//Remove keys
	for (it=m_keys.begin(); it != m_keys.end(); it++) {
		::RegCloseKey (it->second->hKey);
		::DeleteFile  (it->second->fileName);
		delete it->second;
	}
	m_keys.clear();
	m_eventKey.clear();

	//Close event handlers. 
	for(unsigned int i=2; i<=m_eventArrayTail; i++) {
		if(m_eventArray[i]) {
			::CloseHandle(m_eventArray[i]);
			m_eventArray[i]=NULL;
		}
	}
	//We still have the m_stopEvent and m_reqEvent
	m_eventArrayTail=1;
}

/**
* Add previleges to backup and restore registry keys.
*/
void RegKeyGuard::Init (bool bDesktop)
{
	HANDLE           hToken;
	LUID             sebackupnameValue;
	LUID             serestorenameValue;
	TOKEN_PRIVILEGES tp;

	/* Add previleges to backup and restore registry keys.*/
	if (::OpenProcessToken( 
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | // to adjust privileges
		TOKEN_QUERY,         // to get old privileges setting
		&hToken 
		)) {
			//
			// Given a privilege's name SeBackupPrivilege, we should locate 
			//its local LUID mapping.
			//
			::LookupPrivilegeValue( NULL, SE_BACKUP_NAME , &sebackupnameValue );

			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = sebackupnameValue;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			if(!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
				// AdjustTokenPrivileges() failed
				::CloseHandle( hToken );
				TRACE(0, _T("Could not set registry backup previlege\n"));
				return;
			}

			::LookupPrivilegeValue( NULL, SE_RESTORE_NAME , &serestorenameValue );
			tp.Privileges[0].Luid = serestorenameValue;
			if(!::AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
				// AdjustTokenPrivileges() failed
				::CloseHandle( hToken );
				TRACE(0, _T("Could not set registry restore previlege\n"));
				return;
			}

			::CloseHandle( hToken );
	}

	//Always listen to the hardware profile root
	hardwareProfileKeyStr=_T("HKEY_LOCAL_MACHINE");
	hardwareProfileKeyStr+=HARDWARE_PROFILE_ROOT;
	AddKey(HKEY_LOCAL_MACHINE, HARDWARE_PROFILE_ROOT);

	//Load tamperproof configuration about protected registry keys
	NLTamperproofMap *pKeys=NULL;
	if(NLTamperproofConfiguration_Load(NL_TAMPERPROOF_TYPE_REGKEY, &pKeys)) {
		if(pKeys) {
			NLTamperproofMap::iterator it=pKeys->begin();
			NLTamperproofMap::iterator eit=pKeys->end();
			HKEY rKey;
			const nlchar *subKey;
			for(; it!=eit; it++) {
				subKey=it->first.c_str();
				if(it->second.root == _T("HKEY_LOCAL_MACHINE")) {
					rKey=HKEY_LOCAL_MACHINE;
					subKey=&subKey[nlstrlen(_T("HKEY_LOCAL_MACHINE\\"))];
				} else if(it->second.root == _T("HKEY_CLASSES_ROOT")) {
					rKey=HKEY_CLASSES_ROOT;
					subKey=&subKey[nlstrlen(_T("HKEY_CLASSES_ROOT\\"))];
				} else if(it->second.root == _T("HKEY_CURRENT_USER")) {
					rKey=HKEY_CURRENT_USER;
					subKey=&subKey[nlstrlen(_T("HKEY_CURRENT_USER\\"))];
				} else if(it->second.root == _T("HKEY_USERS")) {
					subKey=&subKey[nlstrlen(_T("HKEY_USERS\\"))];
					rKey=HKEY_USERS;
				} else if(it->second.root == _T("HKEY_CURRENT_CONFIG")){
					rKey=HKEY_CURRENT_CONFIG;
					subKey=&subKey[nlstrlen(_T("HKEY_CURRENT_CONFIG\\"))];
				}else
					continue;
				AddKey(rKey, subKey);
			}

			//	on Win7, when we monitor a key by RegNotifyChangeKeyValue, if its parent key is renamed, there is no notify which is different from WinXP.
			//	sometimes, tamper proof configuration file does not protect parent key explicitly, but it must be protected, 
			//	a good example is, all PC and Enforcers' tamper proof configuration file  don't explicitly protect HKLM\\Software\\Nextlabs,
			//	so, the "Nextlabs" can be renamed, so, we need to protect it. 
			//	However, it is a little different with the key that be protected by configuration file -- when it is changed, not all its subkeys but only the subkeys that are protected by configuration file are restored.
			//	And if there is such a parent key that are protected, its subkeys that are protected by configuration file are not explicitly protected by RegNotifyChangeKeyValue.
			AddPrarentMonitorKeys();

			NLTamperproofConfiguration_Free(pKeys);  
		}   
	}
}

//Add a key operation(protecting/unprotecting) request
CEResult_t RegKeyGuard::AddARequest(OPERATION op, HKEY hRootKey,
																		LPCTSTR pkeyName)
{
	//Add request into the queue
	nlthread_mutex_lock(&reqQMutex);
	Request r(hRootKey, pkeyName, op);
	reqQ.push_back(&r);
	::SetEvent(m_reqEvent);
	nlthread_mutex_unlock(&reqQMutex);

	//Wait for the result
	nlthread_mutex_lock(&r.mutex);
	while(r.status != Request::RETURNED) {
		nlthread_cond_wait(&r.cond, &r.mutex);
	}
	nlthread_mutex_unlock(&r.mutex);
	return r.result;
}
/**
* Add key to be protected. Create backup of the specified key to 
* a file so it can be restored later.
*/
CEResult_t RegKeyGuard::AddKey (HKEY hRootKey, LPCTSTR pKeyName)
{
	HKEY hKey = NULL;
	if (ERROR_SUCCESS == ::RegOpenKeyEx (hRootKey, pKeyName, 
		0, KEY_ALL_ACCESS, &hKey)) {
			TCHAR fileName [MAX_PATH];
			_tcsncpy_s (fileName, MAX_PATH, pKeyName, _TRUNCATE);
			size_t len = _tcslen (fileName);
			for (size_t i = 0; i < len; i++) {
				if (fileName [i] == L'\\') {
					fileName [i] = L'_';
				} else if (fileName [i] == L'*') {
					fileName [i] = L'_';
				}
			}
			TCHAR fullFileName [MAX_PATH];
			_stprintf_s(fullFileName, MAX_PATH,
				_T("%s\\%s.dat"), CONFIG_DIR, fileName);
			::DeleteFile (fullFileName);

			// check if it is a parent key
			long r = ERROR_SUCCESS;
			if (!IsParentKey(hRootKey, pKeyName))
			{
				//	if it is not a parent key, save key
				r=::RegSaveKey (hKey, fullFileName, NULL);
				TRACE(CELOG_DEBUG, _T("save reg key for a non-parent key [%s]\n"), pKeyName);
			}
			else
			{
				//	if it is a parent key, don't save key because it may be too big and it is useless
				fullFileName[0] = 0;
				r = ERROR_SUCCESS;
				TRACE(CELOG_DEBUG, _T("don't save reg key for a parent key [%s]\n"), pKeyName);
			}
			if (ERROR_SUCCESS == r) {
				KeyToFileMapping *p=new KeyToFileMapping;
				p->hRootKey = hRootKey;
				_tcsncpy_s(p->keyName, MAX_PATH, pKeyName, _TRUNCATE);
				p->hKey = hKey;
				_tcsncpy_s(p->fileName, MAX_PATH, fullFileName, _TRUNCATE);
				wstring mid;
				if(hRootKey == HKEY_LOCAL_MACHINE)
					mid=_T("HKEY_LOCAL_MACHINE");
				else if(hRootKey == HKEY_CLASSES_ROOT)
					mid=_T("HKEY_CLASSES_ROOT");
				else if(hRootKey == HKEY_CURRENT_USER)
					mid=_T("HKEY_CURRENT_USER");
				else if(hRootKey == HKEY_USERS)
					mid=_T("HKEY_USERS");
				else if(hRootKey ==HKEY_CURRENT_CONFIG)
					mid=_T("HKEY_CURRENT_CONFIG");	  
				mid+=pKeyName;
				m_keys[mid]=p;
				TRACE(CELOG_DEBUG, _T("Protect reg key '%s'\n"), 
					mid.c_str());  
				return (CE_RESULT_SUCCESS);
			} 
			else 
				TRACE(CELOG_ERR, _T("RegSave Protect reg key '%s' failed: %d\n"), 
				fullFileName,r);        
	} else {
		wstring rootStr;
		if(hRootKey == HKEY_LOCAL_MACHINE)
			rootStr=_T("HKEY_LOCAL_MACHINE");
		else if(hRootKey == HKEY_CLASSES_ROOT)
			rootStr=_T("HKEY_CLASSES_ROOT");
		else if(hRootKey == HKEY_CURRENT_USER)
			rootStr=_T("HKEY_CURRENT_USER");
		else if(hRootKey == HKEY_USERS)
			rootStr=_T("HKEY_USERS");
		else if(hRootKey == HKEY_CURRENT_CONFIG)
			rootStr=_T("HKEY_CURRENT_CONFIG");
		else
			rootStr=_T("Unknown");
		TRACE(CELOG_ERR, _T("The reg key '%s\\%s' is invalid\n"), 
			rootStr.c_str(),pKeyName);  
		return CE_RESULT_PROTECTION_OBJECT_NOT_FOUND;
	}
	return (CE_RESULT_GENERAL_FAILED);
}

////////////////////////////////////////////////////////
// Restore the hardware profile settings for the agent.
// This function enumerates all the hardware profiles, 
// and makes sure that the service is always enabled in
// all of them. If it is not, it turns it back on.
// hKey [in] : handle to the hardware profile root key
// pHardwareProfileKey [in] : key to always enable under each profile
////////////////////////////////////////////////////////
BOOL RegKeyGuard::RestoreHardwareProfileKey (HKEY hKey, 
																						 LPCTSTR pHardwareProfileKey)
{
	if (_tcslen(pHardwareProfileKey) == 0) {
		TRACE(0, _T("Could not restore hardware profile key. Hardware profile key not set.\n"));
		return FALSE;
	}

	DWORD dwMaxKeySize = 0;
	DWORD dwCurrentChildIndex = 0;
	//hKey is opened with all access, so we are fine.
	if (RegQueryInfoKey( hKey,
		NULL,
		NULL,
		NULL,
		NULL,
		&dwMaxKeySize,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL) == ERROR_SUCCESS) {
			WCHAR* childKeyName = new WCHAR[dwMaxKeySize+1];
			_tcsncpy_s(childKeyName, dwMaxKeySize+1, _T(""), _TRUNCATE);
			LONG status = ERROR_SUCCESS;
			status = RegEnumKey(hKey, dwCurrentChildIndex, childKeyName, 
				dwMaxKeySize+1);
			while (status == ERROR_SUCCESS) {
				//Appends key name
				size_t keyNameLen=_tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey);
				WCHAR* keyToRestoreName = new WCHAR[keyNameLen];
				_tcsncpy_s(keyToRestoreName, keyNameLen, HARDWARE_PROFILE_ROOT, _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, _T("\\"), _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, childKeyName, _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, _T("\\"), _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, _T("SYSTEM\\CurrentControlSet\\Enum\\ROOT"), _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, _T("\\"), _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, m_hardwareProfileKey, _TRUNCATE);
				_tcsncat_s(keyToRestoreName, keyNameLen, _T("\\0000"), _TRUNCATE);

				//Opens the key and see if it is set properly. 
				//If the key is not there, we are fine
				//since windows will enable the service if the key is missing.
				HKEY hKeyToRestore;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyToRestoreName, 0, 
					KEY_ALL_ACCESS, &hKeyToRestore) == ERROR_SUCCESS) {
						DWORD dwValue = 0;
						if (ERROR_SUCCESS != ::RegSetValueEx(hKeyToRestore,
							_T("CSConfigFlags"), 
							(DWORD) 0, 
							REG_DWORD, 
							(LPBYTE)&dwValue, 
							sizeof(DWORD))) {
								//Log error, but keep going for the other keys
								TRACE(0,_T("Could not restore hardware profile key. Error setting the CSConfigFlags value.\n"));
						}
						RegCloseKey(hKeyToRestore);
				}
				delete []keyToRestoreName;
				keyToRestoreName = NULL;

				//Go on to the next child key
				dwCurrentChildIndex++;
				status = RegEnumKey(hKey, dwCurrentChildIndex, childKeyName, 
					dwMaxKeySize+1);			
			}
			delete[] childKeyName;
			childKeyName = NULL;
			if (status != ERROR_NO_MORE_ITEMS) {
				TRACE(0, _T("Could not restore hardware profile key. Error browsing children key list.\n")); 
				return FALSE;
			}
	} else {
		TRACE(0,_T("Could not restore hardware profile key. Error fetching largest key size.\n")); 
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////
// Sets the name of the hardware profile key to protect
// pKeyName [in] : name of the key
// Returns TRUE if the key can be assigned, false otherwise
////////////////////////////////////////////////////////////////
BOOL RegKeyGuard::SetHardwareProfileKey (LPCTSTR pKeyName)
{
	if (_tcslen(pKeyName) < MAX_KEY_SIZE) {
		_tcsncpy_s(m_hardwareProfileKey, MAX_KEY_SIZE, pKeyName, _TRUNCATE);
		return TRUE;
	}
	return FALSE;
}

//Handle adding/removing registration keys request
void RegKeyGuard::HandleRequest()
{
	nlthread_mutex_lock(&reqQMutex);
	Request *r;
	wstring mid;
	TRACE(0, _T("Handle Request\n"));
	for(unsigned int i=0; i<reqQ.size(); i++) {
		r=reqQ[i];
		if(r->rootKey == HKEY_LOCAL_MACHINE)
			mid=_T("HKEY_LOCAL_MACHINE");
		else if(r->rootKey == HKEY_CLASSES_ROOT)
			mid=_T("HKEY_CLASSES_ROOT");
		else if(r->rootKey == HKEY_CURRENT_USER)
			mid=_T("HKEY_CURRENT_USER");
		else if(r->rootKey == HKEY_USERS)
			mid=_T("HKEY_USERS");
		else if(r->rootKey ==HKEY_CURRENT_CONFIG)
			mid=_T("HKEY_CURRENT_CONFIG");	  
		mid+=r->keyName;
		if(r->op == ADD) {
			TRACE(0, _T("Handle Adding Request\n"));
			if(m_keys.find(mid) != m_keys.end()) //The key is protected already
				r->result=CE_RESULT_SUCCESS;
			else { //add the key to be protected
				r->result=AddKey(r->rootKey, r->keyName);
				if(r->result == CE_RESULT_SUCCESS) {
					if(m_eventArrayTail+1 < m_eventArrayLen) {
						m_eventArrayTail++;
						m_eventArray[m_eventArrayTail] = ::CreateEvent (NULL, FALSE, 
							FALSE, NULL);
						::RegNotifyChangeKeyValue(m_keys[mid]->hKey, TRUE, 
							REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , 
							m_eventArray[m_eventArrayTail], TRUE);
						m_eventKey[m_eventArrayTail]=mid;	  
					} else { //We need to adjust the event array
						//Clear the mapping between event and keys
						TRACE(0, _T("Handle Adding Request: adjust array\n"));
						m_eventKey.clear();

						//Close event handlers. 
						for(unsigned int j=2; j<=m_eventArrayTail; j++) {
							if(m_eventArray[j]) {
								::CloseHandle(m_eventArray[j]);
							}
						} 
						//Extend the array if it is needed
						if(m_keys.size()+2 > m_eventArrayLen) {
							delete [] m_eventArray;
							m_eventArrayLen+=m_eventArrayLen;
							m_eventArray = new HANDLE[m_eventArrayLen];	    
						} 
						m_eventArrayTail=0;
						m_eventArray[m_eventArrayTail]=m_stopEvent;
						m_eventArrayTail++;
						m_eventArray[m_eventArrayTail]=m_reqEvent;
						//Reset event array
						std::map<wstring, KeyToFileMapping *>::iterator it;
						for(it=m_keys.begin(); it != m_keys.end(); it++) {
							m_eventArrayTail++;
							m_eventArray[m_eventArrayTail] = ::CreateEvent (NULL, FALSE, 
								FALSE, NULL);
							::RegNotifyChangeKeyValue(it->second->hKey, TRUE, 
								REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , 
								m_eventArray[m_eventArrayTail], TRUE);
							m_eventKey[m_eventArrayTail]=it->first;
						}	  
					}
				}
			}
		} else if(r->op == REMOVE) {
			TRACE(0, _T("Handle Remove Request\n"));
			std::map<wstring, KeyToFileMapping *>::iterator it=m_keys.find(mid);
			if(it == m_keys.end()) 
				r->result=CE_RESULT_SUCCESS; //the key is not protected already
			else {
				r->result=CE_RESULT_SUCCESS;
				delete it->second;
				m_keys.erase(it);
			}
		}
		TRACE(0, _T("Handle Request result=%d\n"), r->result);
		nlthread_mutex_lock(&r->mutex);
		r->status=Request::RETURNED;
		nlthread_cond_signal(&r->cond);
		nlthread_mutex_unlock(&r->mutex);      
	}
	reqQ.clear();
	nlthread_mutex_unlock(&reqQMutex);
}

/**
* Start protecting the registry. If any of the specified keys are changed, 
* they will be restored to their original values.
*/
void RegKeyGuard::Run()
{
	//Attempts to restore hardware profile now
	//in case somebody tried to disable profiles while the agent was stopped
	if(m_keys.find(hardwareProfileKeyStr) != m_keys.end() &&
		m_keys[hardwareProfileKeyStr] != NULL)
		RestoreHardwareProfileKey(m_keys[hardwareProfileKeyStr]->hKey, 
		m_hardwareProfileKey);

	//Set event array
	std::map<wstring, KeyToFileMapping *>::iterator it;
	for(it=m_keys.begin(); it != m_keys.end(); it++) {
		m_eventArrayTail++;
		m_eventArray[m_eventArrayTail] = ::CreateEvent (NULL, FALSE, FALSE, NULL);

		//	check if it is a sub key
		if (!HasParentKey(it->first))
		{
			//	if it is a sub key, don't monitor it, because we monitor its parent key.
			//	we only monitor parent key and a key that don't have a parent key.
			TRACE(0, L"RegNotifyChangeKeyValue for keyname [%s]\n", it->first.c_str());
			::RegNotifyChangeKeyValue(it->second->hKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , m_eventArray[m_eventArrayTail], TRUE);
		}
		else
		{
			//	do nothing
			TRACE(0, L"don't monitor for subkey [%s]\n", it->first.c_str());
		}

		m_eventKey[m_eventArrayTail]=it->first;
	}

	wstring keyStr;
	std::map<wstring, KeyToFileMapping *>::iterator mit;

	TRACE(CELOG_DEBUG, _T("RegKeyGuard start running\n"));
	while (TRUE) {
		DWORD waitResult = ::WaitForMultipleObjects(m_eventArrayTail + 1, 
			m_eventArray, FALSE, 
			INFINITE);

		TRACE(CELOG_DEBUG, _T("RegKeyGuard: some key/event occured.\n"));

		if (waitResult == WAIT_FAILED) {
			removeKeys();
			return;
		} 
		else if (waitResult >= WAIT_OBJECT_0 && 
			waitResult <= WAIT_OBJECT_0 + m_eventArrayTail) {  
				DWORD index = waitResult - WAIT_OBJECT_0;
				if (index == 0) {
					// Destructor should clean it up when we receive the stop event
					TRACE(CELOG_INFO, _T("In RegKeyGuard, stop event received\n"));
					return;
				} else if(index == 1) {
					//Adding/removing protected registration keys request
					HandleRequest();
				} else {
					if(m_eventKey.find(index) != m_eventKey.end()) {
						keyStr=m_eventKey[index];
						mit=m_keys.find(keyStr);
						if(mit != m_keys.end()) {
							TRACE(CELOG_DEBUG, 
								_T("RegKeyGuard: key (%s) modification attempted.\n"),
								keyStr.c_str());
							//If this is a hardware profile change, 
							//special handling is required
							if (keyStr == hardwareProfileKeyStr) {
								TRACE(0, _T("Hardware Profile Modification Attempted.\n"));
								RestoreHardwareProfileKey(mit->second->hKey, 
									m_hardwareProfileKey);
							}
							else 
							{
								//	check if it is a Parent key
								if (IsParentKey(mit->first))
								{
									//	it is a Parent key, we need to restore all its protected sub key
									std::map<wstring, KeyToFileMapping *>::iterator map_it;
									for(map_it=m_keys.begin(); map_it != m_keys.end(); map_it++) 
									{
										//	check if it is a sub key
										if (IsSubKey(map_it->first, mit->first))
										{
											//	this map_it is subkey, restore it
											RestoreKey(map_it);
										}
									}
									//	all subkey has been restored after reopen or recreated the subkeys
									//	now, reopen or recreate the parent key, the new key handle is need for next monitor
									if(ERROR_SUCCESS == ::RegOpenKeyEx (mit->second->hRootKey, 
										mit->second->keyName, 
										0, KEY_ALL_ACCESS, 
										&(mit->second->hKey))) 
									{
										TRACE(CELOG_DEBUG, _T("RegKeyGuard: re open key succeed [%s]\n"), mit->first.c_str());
									}
									else if(ERROR_SUCCESS == ::RegCreateKey(mit->second->hRootKey, 
										mit->second->keyName, 
										&(mit->second->hKey))) 
									{
										TRACE(CELOG_DEBUG, _T("RegKeyGuard: re create key succeed [%s]\n"), mit->first.c_str());
									}
									else
									{
										TRACE(CELOG_DEBUG, _T("RegKeyGuard: failed to renew key handle [%s]\n"), mit->first.c_str());
									}
								}
								else
								{
									//	this is not modification to a parent key, it is modification to a key that doesn't have a parent key
									//	we restore it
									RestoreKey(mit);
								}
							}
							TRACE(0, L"RegNotifyChangeKeyValue keyname [%s] after restore\n", mit->first.c_str());
							::RegNotifyChangeKeyValue(mit->second->hKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , m_eventArray [index], TRUE);
						}
					} //else an event on a key that is not protected any more
				}//else an event on a key that is not protected any more
		}
	}
}

/**
* Fires the stop event to force the Start method to return.
*/
void RegKeyGuard::Stop ()
{
	if (m_stopEvent) {
		::SetEvent(m_stopEvent);
	}
}

/*==========================================================================*
* Exported APIs
*==========================================================================*/
/* =======================PDP_CEPROTECT_RegKeyGuard_Init==================*
* Initialization.                                                        *
*                                                                        *
* Return:                                                                *
*   It returns CE_RESULT_SUCCESS, if initalization succeeds.             *
* =======================================================================*/
CEResult_t PDP_CEPROTECT_RegKeyGuard_Init(bool bDesktop)
{
	try {
		regKeyGuard.Init(bDesktop);
		return CE_RESULT_SUCCESS;
	} catch (std::exception &e) {
		TRACE(0, _T("PDP_CEPROTECT_RegKeyGuard_Init failed due to '%s'\n"), 
			e.what());
		return CE_RESULT_GENERAL_FAILED;
	} catch (...) {
		TRACE(0, 
			_T("PDP_CEPROTECT_RegKeyGuard_Init failed due to unknown reason\n"));
		return CE_RESULT_GENERAL_FAILED;
	}
}
/* =======================PDP_CEPROTECT_RegKeyGuard_Run===================*
* Run the protection.                                                    *
*                                                                        *
* Return:                                                                *
* =======================================================================*/
void PDP_CEPROTECT_RegKeyGuard_Run()
{
	try {
		regKeyGuard.Run();
		return;
	} catch (std::exception &e) {
		TRACE(0, _T("PDP_CEPROTECT_RegKeyGuard_Run failed due to '%s'\n"), 
			e.what());
	} catch (...) {
		TRACE(0, 
			_T("PDP_CEPROTECT_RegKeyGuard_Run failed due to unknown reason\n"));
	}
}
/* =======================PDP_CEPROTECT_RegKeyGuard_Stop==================*
* Stop the protection.                                                   *
*                                                                        *
* Return:                                                                *
* =======================================================================*/
void PDP_CEPROTECT_RegKeyGuard_Stop()
{
	regKeyGuard.Stop();
}

CEResult_t PDP_CEPROTECT_LockKey(std::vector<void *> &inputArgs,
																 std::vector<void *> &replyArgs)
{
	try {
		CEString reqID = (CEString)inputArgs[0];
		//CEHandle handle=*((CEHandle *)inputArgs[1]);
		CEKeyRoot_t rootKey=*((CEKeyRoot_t *)inputArgs[2]);
		CEString key=(CEString)inputArgs[3];
		CEResult_t result=CE_RESULT_SUCCESS;
		HKEY root;
		//Construct reply
		replyArgs.push_back(reqID);

		if(rootKey == CE_KEYROOT_CLASSES_ROOT)
			root=HKEY_CLASSES_ROOT;
		else if(rootKey == CE_KEYROOT_CURRENT_USER)
			root=HKEY_CURRENT_USER;
		else if(rootKey == CE_KEYROOT_LOCAL_MACHINE)
			root=HKEY_LOCAL_MACHINE;
		else if(rootKey == CE_KEYROOT_USERS)
			root=HKEY_USERS;
		else if(rootKey == CE_KEYROOT_CURRENT_CONFIG)
			root=HKEY_CURRENT_CONFIG;
		else
			return CE_RESULT_PROTECTION_OBJECT_NOT_FOUND;

		result=regKeyGuard.AddARequest(ADD, root, key->buf);
		return result;
	} catch (std::exception &e) {
		TRACE(0, _T("PDP_CEPROTECT_LockKey failed due to '%s'\n"), 
			e.what());
		return CE_RESULT_GENERAL_FAILED;
	} catch (...) {
		TRACE(0, 
			_T("PDP_CEPROTECT_LockKey failed due to unknown reason\n"));
		return CE_RESULT_GENERAL_FAILED;
	}

}
CEResult_t PDP_CEPROTECT_UnlockKey(std::vector<void *> &inputArgs,
																	 std::vector<void *> &replyArgs)
{
	try {
		CEString reqID = (CEString)inputArgs[0];
		//CEHandle handle=*((CEHandle *)inputArgs[1]);
		CEKeyRoot_t rootKey=*((CEKeyRoot_t *)inputArgs[2]);
		CEString key=(CEString)inputArgs[3];
		CEResult_t result=CE_RESULT_SUCCESS;
		HKEY root;
		//Construct reply
		replyArgs.push_back(reqID);

		if(rootKey == CE_KEYROOT_CLASSES_ROOT)
			root=HKEY_CLASSES_ROOT;
		else if(rootKey == CE_KEYROOT_CURRENT_USER)
			root=HKEY_CURRENT_USER;
		else if(rootKey == CE_KEYROOT_LOCAL_MACHINE)
			root=HKEY_LOCAL_MACHINE;
		else if(rootKey == CE_KEYROOT_USERS)
			root=HKEY_USERS;
		else if(rootKey == CE_KEYROOT_CURRENT_CONFIG)
			root=HKEY_CURRENT_CONFIG;
		else
			return CE_RESULT_PROTECTION_OBJECT_NOT_FOUND;

		result=regKeyGuard.AddARequest(REMOVE, root, key->buf);
		return result;
	} catch (std::exception &e) {
		TRACE(0, _T("PDP_CEPROTECT_UnlockKey failed due to '%s'\n"), 
			e.what());
		return CE_RESULT_GENERAL_FAILED;
	} catch (...) {
		TRACE(0, 
			_T("PDP_CEPROTECT_UnlockKey failed due to unknown reason\n"));
		return CE_RESULT_GENERAL_FAILED;
	}

}

void RegKeyGuard::AddPrarentMonitorKeys()
{
	if (IsWin7())
	{
		//	add key for parent key, all changes apply to it and its subkeys will be notified.
		for(DWORD i = 0; i < sizeof(g_szParentKey)/sizeof(Parent_Key); i++)
		{
			AddKey(g_szParentKey[i].hRootKey, g_szParentKey[i].wstrKeyName.c_str());
		}
	}
	//	do nothing if it is not win7
}

BOOL RegKeyGuard::IsParentKey(HKEY hRootKey, LPCTSTR pKeyName)
{
	if (IsWin7())
	{
		for(DWORD i = 0; i < sizeof(g_szParentKey)/sizeof(Parent_Key); i++)
		{
			if (hRootKey == g_szParentKey[i].hRootKey && (wstring)pKeyName == g_szParentKey[i].wstrKeyName)
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

BOOL RegKeyGuard::IsParentKey(const wstring& KeyName)
{
	if (IsWin7())
	{
		for(DWORD i = 0; i < sizeof(g_szParentKey)/sizeof(Parent_Key); i++)
		{
			if (KeyName == g_szParentKey[i].wstrKeyFullName)
			{
				return TRUE;
			}
		}
		return FALSE;	
	}
	return FALSE;
}

BOOL RegKeyGuard::HasParentKey(const wstring& keyName)
{
	if (IsWin7())
	{
		for(DWORD i = 0; i < sizeof(g_szParentKey)/sizeof(Parent_Key); i++)
		{
			if (keyName.find(g_szParentKey[i].wstrKeyFullName) != wstring::npos && g_szParentKey[i].wstrKeyFullName != keyName)
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

void RegKeyGuard::RestoreKey(std::map<wstring, KeyToFileMapping *>::iterator mit)
{
	if(!IsWin7())
	{
		//	if it is WinXP, try to restore directly,
		//	if it is Win7, RegRestoreKey is not reliable before reopen/recreate a key handle, it will return succeed even it fail in fact.
		if (ERROR_SUCCESS == ::RegRestoreKey (mit->second->hKey, mit->second->fileName, REG_FORCE_RESTORE))
		{
			TRACE(CELOG_DEBUG, _T("RegKeyGuard: restore key succeed directly [%s]\n"), mit->first.c_str());
			return;
		}
	}

	if(ERROR_SUCCESS == ::RegOpenKeyEx (mit->second->hRootKey, 
		mit->second->keyName, 
		0, KEY_ALL_ACCESS, 
		&(mit->second->hKey))) 
	{
		//	open it first and succeed
		TRACE(CELOG_DEBUG, _T("RegKeyGuard: re open key succeed [%s]\n"), mit->first.c_str());

		//	restore
		if(ERROR_SUCCESS != ::RegRestoreKey (mit->second->hKey, 
			mit->second->fileName, 
			REG_FORCE_RESTORE)) 
		{
			TRACE(CELOG_ERR, 
				_T("RegKeyGuard: restore key [%s] failed.\n"),
				mit->first.c_str());
		}
		else
		{
			TRACE(CELOG_ERR, 
				_T("RegKeyGuard: restore key [%s] succeed.\n"),
				mit->first.c_str());
		}
		return;
	}

	//	open fail, recreate it
	if(ERROR_SUCCESS == ::RegCreateKey(mit->second->hRootKey, 
		mit->second->keyName, 
		&(mit->second->hKey))) 
	{
		TRACE(CELOG_DEBUG, _T("RegKeyGuard: re-create key succeed [%s]\n"), mit->first.c_str());

		if (ERROR_SUCCESS != ::RegRestoreKey(mit->second->hKey, 
			mit->second->fileName, 
			REG_FORCE_RESTORE)) 
		{
			TRACE(0, _T("Could not restore registry.\n"));
		}
		else
		{
			TRACE(0, _T("restore registry succeed\n"));
		}
	}
	else
	{
		TRACE(CELOG_DEBUG, _T("RegKeyGuard: re-create key failed [%s]\n"), mit->first.c_str());
	}
}

BOOL RegKeyGuard::IsSubKey(const wstring& KeyName, const wstring& parentKey)
{
	if (IsWin7())
	{
		if (KeyName.find(parentKey) != wstring::npos && parentKey != KeyName)
		{
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

#endif
