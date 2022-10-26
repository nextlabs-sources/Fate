REM ******************************************************************************************
REM **                              NEXTLABS INSTALL                                        **
REM ******************************************************************************************
set "errorlevel="

:NEXTLABSINSTALL
@ECHO Installing Nextlabs Policy Controller...
@ECHO OFF

echo %PROCESSOR_ARCHITECTURE% | find /i "x86" > nul
if not %errorlevel%==0 (
	echo Current Operating System is 64-bit, please use Nextlabs_x64_Install.bat installation script!
	pause
	exit /b 0
)

start /wait msiexec /i EDP-PolicyController-setup.msi /L*v %temp%\policy_controller_install.log /passive ENABLE_SERVICE_START_ON_INSTALL=0 REBOOT=ReallySuppress ICENET_SERVER_LOCATION=138.125.41.223
if %errorlevel% neq 0 (
if %errorlevel% neq 3010 (
start /wait msiexec /i EDP-PolicyController-setup.msi /L*v %temp%\policy_controller_install.log /passive ENABLE_SERVICE_START_ON_INSTALL=0 REBOOT=ReallySuppress ICENET_SERVER_LOCATION=138.125.41.223
@ECHO "policy controller installer finished"
)
)

@ECHO Installing Nextlabs Endpoint...
@ECHO OFF
start /wait msiexec /i EDP-DesktopEnforcerForWindows-setup.msi /L*v %temp%\desktop_enforcer_install.log /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0
if %errorlevel% neq 0 (
@ECHO "windows desktop enforcer failed to install" >> %temp%/log.txt
)
start /wait msiexec /i EDP-OutlookEnforcer-setup.msi /L*v %temp%\outlook_enforcer_install.log /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0
if %errorlevel% neq 0 (
@ECHO "outlook desktop enforcer failed to install" >> %temp%/log.txt
)
start /wait msiexec /i EDP-NetworkEnforcer-setup.msi /L*v %temp%\network_enforcer_install.log /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0
if %errorlevel% neq 0 (
@ECHO "network desktop enforcer failed to install" >> %temp%/log.txt
)
start /wait msiexec /i CC-KeyManagementForWindows-setup.msi /L*v %temp%\key_management_install.log /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0
if %errorlevel% neq 0 (
@ECHO "key management failed to install" >> %temp%/log.txt
)
start /wait msiexec /i EDP-SystemEncryptionForWindows-setup.msi /L*v %temp%\rights_management_install.log /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0
if %errorlevel% neq 0 (
if %errorlevel% neq 3010 ( 
rem 3010 means ERROR_SUCCESS_REBOOT_REQUIRED
@ECHO "rights management client failed to install, error level is %errorlevel%"
)
)
