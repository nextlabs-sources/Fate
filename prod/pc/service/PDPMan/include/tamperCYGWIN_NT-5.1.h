#pragma once

#define MAX_KEY_COUNT 24
#define MAX_KEY_SIZE 100

// Registry Keys that we want to protect

#define CONFIG_DIR                            _T("config")
#define HARDWARE_PROFILE_ROOT                 _T("SYSTEM\\CurrentControlSet\\Hardware Profiles")
#define SAFEBOOT_MINIMAL_DA_SERVICE           _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\ComplianceAgentService")
#define SAFEBOOT_NETWORK_DA_SERVICE           _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\ComplianceAgentService")
#define SAFEBOOT_MINIMAL_NTPROCDRV            _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\NTProcDrv")
#define SAFEBOOT_NETWORK_NTPROCDRV            _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\NTProcDrv")
#define SAFEBOOT_MINIMAL_FSA_SERVICE          _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\ComplianceEnforcerService")
#define SAFEBOOT_NETWORK_FSA_SERVICE          _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\ComplianceEnforcerService")
#define SAFEBOOT_MINIMAL_COREDRV              _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\DSCORE")
#define SAFEBOOT_NETWORK_COREDRV              _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\DSCORE")
#define SAFEBOOT_MINIMAL_IFSDRV               _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\dsifsflt")
#define SAFEBOOT_NETWORK_IFSDRV               _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\dsifsflt")
#define DESKTOP_AGENT_SERVICE_KEY             _T("SYSTEM\\CurrentControlSet\\Services\\ComplianceAgentService")
#define FILESERVER_AGENT_SERVICE_KEY          _T("SYSTEM\\CurrentControlSet\\Services\\ComplianceEnforcerService")
#define CORE_DRIVER_KEY                       _T("SYSTEM\\CurrentControlSet\\Services\\dscore")
#define IFS_DRIVER_KEY                        _T("SYSTEM\\CurrentControlSet\\Services\\dsifsflt")
#define NTPROCDRV_KEY                         _T("SYSTEM\\CurrentControlSet\\Services\\ntprocdrv")
#define AGENT_HARDWARE_PROFILE_KEY            _T("LEGACY_COMPLIANCEAGENTSERVICE")
#define DESKTOP_AGENT_HARDWARE_PROFILE_KEY    _T("LEGACY_COMPLIANCEAGENTSERVICE")
#define FILESERVER_AGENT_HARDWARE_PROFILE_KEY _T("LEGACY_COMPLIANCEENFORCERSERVICE")
#define LOGON_NOTIFY_KEY                      _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\ComplianceAgentLogon")


struct KeyToFileMapping 
{
    HKEY hRootKey;
    TCHAR keyName [MAX_PATH];
    HKEY hKey;
    TCHAR fileName [MAX_PATH];
};

/**
* This class protects keys in the registry from being changed. It restores
* protected keys to their original values if they are changed.
*
*/
class RegistryProtector
{
public:
    RegistryProtector(void);
    virtual ~RegistryProtector(void);

	//Adds a key to be protected
    BOOL AddKey (HKEY hRootKey, LPCTSTR pKeyName);
	
	//Sets the name of the hardware profile key to protect.
	//Handling of hardware profile keys is special
	BOOL SetHardwareProfileKey (LPCTSTR pKeyName);

    void Start ();

    void Stop ();

private:
    void Init();

	//Restore the hardware profile settings
	BOOL RestoreHardwareProfileKey (HKEY hKey, LPCTSTR pHardwareProfileKey);

    KeyToFileMapping m_keyMappings [MAX_KEY_COUNT];
    DWORD m_nCount;
	WCHAR m_hardwareProfileKey[MAX_KEY_SIZE];
    HANDLE m_hStopEvent;
    void   removeKeys();
};
