#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include "ctrlmodCYGWIN_NT-5.1.h"
#include "tamperCYGWIN_NT-5.1.h"

using namespace ctrlmod_win;



RegistryProtector::RegistryProtector(void)
{
  m_nCount = 0;
  m_hStopEvent = NULL;

  Init();
  SetHardwareProfileKey(_T(""));
	
  //Always listen to the hardware profile root
  AddKey(HKEY_LOCAL_MACHINE, HARDWARE_PROFILE_ROOT);
}

RegistryProtector::~RegistryProtector(void)
{
  ::CloseHandle (m_hStopEvent);
  removeKeys ();
}

// Remove all the keys from the structure, better grouping of actions
// to avoid missing something (e.g. like closing the keys, but not deleting files
void RegistryProtector::removeKeys(void)
{
  for (DWORD i = 0; i < m_nCount; i++)
  {
    ::RegCloseKey (m_keyMappings[i].hKey);
    ::DeleteFile  (m_keyMappings[i].fileName);
  }
  m_nCount = 0;
}

/**
* Add previleges to backup and restore registry keys.
*/
void RegistryProtector::Init ()
{
    HANDLE           hToken;
    LUID             sebackupnameValue;
    LUID             serestorenameValue;
    TOKEN_PRIVILEGES tp;

    if (::OpenProcessToken( 
	    GetCurrentProcess(),
	    TOKEN_ADJUST_PRIVILEGES | // to adjust privileges
	    TOKEN_QUERY,              // to get old privileges setting
	    &hToken 
	    ))
    {
        //
        // Given a privilege's name SeBackupPrivilege, we should locate its local LUID mapping.
        //
        ::LookupPrivilegeValue( NULL, SE_BACKUP_NAME , &sebackupnameValue );

        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = sebackupnameValue;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if ( !::AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
        {
	        // AdjustTokenPrivileges() failed
          ::CloseHandle( hToken );
          AddToMessageLog (_T("Could not set registry backup privilege"));
          return;
        }

        ::LookupPrivilegeValue( NULL, SE_RESTORE_NAME , &serestorenameValue );
        tp.Privileges[0].Luid = serestorenameValue;
        if ( !::AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
        {
	        // AdjustTokenPrivileges() failed
          ::CloseHandle( hToken );
          AddToMessageLog (_T("Could not set registry restore privilege"));
          return;
        }

        ::CloseHandle( hToken );
    }

}

/**
 * Add key to be protected. Creates backup of the specified key to 
 * a file so it can be restored later.
 */
BOOL RegistryProtector::AddKey (HKEY hRootKey, LPCTSTR pKeyName)
{
    HKEY hKey = NULL;
    if (ERROR_SUCCESS == ::RegOpenKeyEx (hRootKey, pKeyName, 0, KEY_ALL_ACCESS, &hKey))
    {
        TCHAR fileName [MAX_PATH];
        _tcsncpy_s (fileName, MAX_PATH, pKeyName, _TRUNCATE);
        size_t len = _tcslen (fileName);
        for (size_t i = 0; i < len; i++)
        {
            if (fileName [i] == L'\\')
            {
                fileName [i] = L'_';
            }
        }
        TCHAR fullFileName [MAX_PATH];
        _stprintf (fullFileName, _T("%s\\%s.dat"), CONFIG_DIR, fileName);
        ::DeleteFile (fullFileName);
        if (ERROR_SUCCESS == ::RegSaveKey (hKey, fullFileName, NULL))
        {
            m_keyMappings[m_nCount].hRootKey = hRootKey;
            _tcsncpy_s (m_keyMappings[m_nCount].keyName, MAX_PATH, pKeyName, _TRUNCATE);
            m_keyMappings[m_nCount].hKey = hKey;
            _tcsncpy_s(m_keyMappings[m_nCount++].fileName, MAX_PATH, fullFileName, _TRUNCATE);
            return (TRUE);
        }
    }
    return (FALSE);
}

////////////////////////////////////////////////////////
// Restore the hardware profile settings for the agent.
// This function enumerates all the hardware profiles, 
// and makes sure that the service is always enabled in
// all of them. If it is not, it turns it back on.
// hKey [in] : handle to the hardware profile root key
// pHardwareProfileKey [in] : key to always enable under each profile
////////////////////////////////////////////////////////
BOOL RegistryProtector::RestoreHardwareProfileKey (HKEY hKey, LPCTSTR pHardwareProfileKey)
{
  if (_tcslen(pHardwareProfileKey) == 0)
  {
    AddToMessageLog(_T("Could not restore hardware profile key. Hardware profile key not set."));
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
                       NULL) == ERROR_SUCCESS)
  {
    WCHAR* childKeyName = new WCHAR[dwMaxKeySize+1];
    _tcsncpy_s(childKeyName, dwMaxKeySize+1, _T(""), _TRUNCATE);
    LONG status = ERROR_SUCCESS;
    status = RegEnumKey(hKey, dwCurrentChildIndex, childKeyName, dwMaxKeySize+1);
    while (status == ERROR_SUCCESS)
    {
      //Appends key name
      WCHAR* keyToRestoreName = new WCHAR[_tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey)];
      _tcsncpy_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), HARDWARE_PROFILE_ROOT, _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), _T("\\"), _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), childKeyName, _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), _T("\\"), _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), _T("SYSTEM\\CurrentControlSet\\Enum\\ROOT"), _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), _T("\\"), _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), m_hardwareProfileKey, _TRUNCATE);
      _tcsncat_s(keyToRestoreName, _tcslen(HARDWARE_PROFILE_ROOT) + 50 + _tcslen(childKeyName) + _tcslen(m_hardwareProfileKey), _T("\\0000"), _TRUNCATE);
      
      //Opens the key and see if it is set properly. If the key is not there, we are fine
      //since windows will enable the service if the key is missing.
      HKEY hKeyToRestore;
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyToRestoreName, 0, KEY_ALL_ACCESS, &hKeyToRestore) == ERROR_SUCCESS)
      {
        DWORD dwValue = 0;
        if (ERROR_SUCCESS != ::RegSetValueEx(hKeyToRestore, _T("CSConfigFlags"), (DWORD) 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)))
        {
          //Log error, but keep going for the other keys
          AddToMessageLog(_T("Could not restore hardware profile key. Error setting the CSConfigFlags value."));
        }
        RegCloseKey(hKeyToRestore);
      }
      delete []keyToRestoreName;
      keyToRestoreName = NULL;
			
      //Go on to the next child key
      dwCurrentChildIndex++;
      status = RegEnumKey(hKey, dwCurrentChildIndex, childKeyName, dwMaxKeySize+1);			
    }
    delete[] childKeyName;
    childKeyName = NULL;
    if (status != ERROR_NO_MORE_ITEMS) 
    {
      AddToMessageLog(_T("Could not restore hardware profile key. Error browsing children key list.")); 
      return FALSE;
    }
		
  }
  else
  {
    AddToMessageLog(_T("Could not restore hardware profile key. Error fetching largest key size.")); 
    return FALSE;
  }

  return TRUE;
}

////////////////////////////////////////////////////////////////
// Sets the name of the hardware profile key to protect
// pKeyName [in] : name of the key
// Returns TRUE if the key can be assigned, false otherwise
////////////////////////////////////////////////////////////////
BOOL RegistryProtector::SetHardwareProfileKey (LPCTSTR pKeyName)
{
  if (_tcslen(pKeyName) < MAX_KEY_SIZE) 
  {
    _tcsncpy_s(m_hardwareProfileKey, MAX_KEY_SIZE, pKeyName, _TRUNCATE);
    return TRUE;
  }
  return FALSE;
}

/**
 * Start protecting the registry. If any of the specified keys are changed, 
 * they will be restored to their original values.
 */
void RegistryProtector::Start ()
{
  //We know for sure the 0 index is taken for the hardware profile. Attempts to restore now
  //in case somebody tried to disable profiles while the agent was stopped
  RestoreHardwareProfileKey(m_keyMappings[0].hKey, m_hardwareProfileKey);

  HANDLE eventArray [MAX_KEY_COUNT + 1];
  for (DWORD i = 0; i < m_nCount; i++)
  {
    eventArray [i] = ::CreateEvent (NULL, FALSE, FALSE, NULL);
    ::RegNotifyChangeKeyValue(m_keyMappings[i].hKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , eventArray [i], TRUE);
  }
  m_hStopEvent =  ::CreateEvent (NULL, FALSE, FALSE, NULL);
  eventArray [m_nCount] = m_hStopEvent;

  while (TRUE)
  {
    DWORD waitResult = ::WaitForMultipleObjects(m_nCount + 1, eventArray, FALSE, INFINITE);
    if (waitResult == WAIT_FAILED)
    {
      removeKeys();
      return;
    }
    else if (waitResult >= WAIT_OBJECT_0 && waitResult <= WAIT_OBJECT_0 + m_nCount)
    {
      DWORD index = waitResult - WAIT_OBJECT_0;
      if (index == m_nCount)
      {
        // Destructor should clean it up when we receive the stop event
        return;
      }
      else 
      {
        AddToMessageLog (_T("Registry Modification Attempted."));
        //If this is a hardware profile change, special handling is required
        if (!wcscmp(HARDWARE_PROFILE_ROOT, m_keyMappings[index].keyName)) 
        {
          AddToMessageLog (_T("Hardware Profile Modification Attempted."));
          RestoreHardwareProfileKey(m_keyMappings[index].hKey, m_hardwareProfileKey);
        }
        else if (ERROR_SUCCESS != ::RegRestoreKey (m_keyMappings [index].hKey, m_keyMappings[index].fileName, REG_FORCE_RESTORE))
        {
          if (ERROR_SUCCESS == ::RegOpenKeyEx (m_keyMappings [index].hRootKey, m_keyMappings [index].keyName, 0, KEY_ALL_ACCESS, &(m_keyMappings [index].hKey)))
          {
            if (ERROR_SUCCESS != ::RegRestoreKey (m_keyMappings [index].hKey, m_keyMappings[index].fileName, REG_FORCE_RESTORE))
            {
              AddToMessageLog (_T("Could not restore registry."));
            }
          }
          else if (ERROR_SUCCESS == ::RegCreateKey (m_keyMappings [index].hRootKey, m_keyMappings [index].keyName, &(m_keyMappings [index].hKey)))
          {
            if (ERROR_SUCCESS != ::RegRestoreKey (m_keyMappings [index].hKey, m_keyMappings[index].fileName, REG_FORCE_RESTORE))
            {
              AddToMessageLog (_T("Could not restore registry."));
            }
          }
        }
                
        ::RegNotifyChangeKeyValue(m_keyMappings[index].hKey, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET , eventArray [index], TRUE);

      }
    }
  }
}

/**
 * Fires the stop event to force the Start method to return.
 */
void RegistryProtector::Stop ()
{
  if (m_hStopEvent)
  {
    ::SetEvent (m_hStopEvent);
  }
}
