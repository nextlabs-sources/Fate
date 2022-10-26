@ECHO OFF

set _icenet_server=rx86cc1:8443

if X%1 == X/? (
	echo Syntax : %0 [msi location]
	echo.
	goto exit
)

set bitness=undefined
set version=undefined

REM Check Windows Version
ver | findstr /i "5\.0\." > nul
IF %ERRORLEVEL% EQU 0 set version=Windows 2000
ver | findstr /i "5\.1\." > nul
IF %ERRORLEVEL% EQU 0 set version=Windows XP
ver | findstr /i "5\.2\." > nul
IF %ERRORLEVEL% EQU 0 set version=Windows 2003
ver | findstr /i "6\.0\." > nul
IF %ERRORLEVEL% EQU 0 set version=Windows Vista
ver | findstr /i "6\.1\." > nul
IF %ERRORLEVEL% EQU 0 set version=Windows 7

set cmd="wmic os get osarchitecture | findstr bit"
FOR /F %%I IN (' %cmd% ') DO set bitness=%%I


@For /F "tokens=2,3,4 delims=/ " %%A in ('Date /t') do @( 
	Set Month=%%A
	Set Day=%%B
	Set Year=%%C
)

set hour=%time:~0,2%
if "%hour:~0,1%" == " " set hour=0%hour:~1,1%
set min=%time:~3,2%
if "%min:~0,1%" == " " set min=0%min:~1,1%
set secs=%time:~6,2%
if "%secs:~0,1%" == " " set secs=0%secs:~1,1%

set dt=%Year%%Month%%Day%-%hour%%min%
rem echo %dt%
set logfile=c:\temp\nextlabs\install-%dt%.log


echo ********************************************************************
echo **                   NEXTLABS INSTALL                             **
echo ********************************************************************

echo Welcome to NextLabs Install setup.
echo.
echo Please make sure that all the endpoints msi files are in the same 
echo folder with this script. Preferably all the 32 and 64 bit installers.
echo The script will auto detect the bitness of installed MS Office. 
echo.

echo Checking the version of MS Office ...
@reg query hklm\software\microsoft\office\14.0\outlook | findstr /i "x64" > nul
IF %ERRORLEVEL% EQU 0 (
	set OEbitness=x64
	set oesetup=setup64
) else (
	set OEbitness=x86
	set oesetup=setup
)

set setup=setup
if %bitness% EQU 64-bit set setup=setup64

echo OS version      : %version%
echo OS version      : %version% >> %logfile% 
echo OS architecture : %bitness%
echo OS architecture : %bitness% >> %logfile%
echo Outlook bitness : %OEbitness%
echo Outlook bitness : %OEbitness% >> %logfile%
ECHO Icenet server   : %_icenet_server%
ECHO Icenet server   : %_icenet_server% >> %logfile%
echo Log file        : %logfile%
echo Log file        : %logfile% >> %logfile%
echo.

SET /P ANSWER=The installation will reboot automatically. Do you want to continue (y/n) ?
if /i {%ANSWER%}=={y} (goto :yes)
if /i {%ANSWER%}=={yes} (goto :yes)

echo Installation is canceled by user!! >> %logfile%
Goto :no

:yes


@echo Creating C:\temp
IF NOT EXIST C:\temp mkdir c:\temp
@echo Creating C:\temp\nextlabs
IF NOT EXIST C:\temp\nextlabs mkdir c:\temp\nextlabs


@echo Cleaning up registry...
regedit.exe /s cleanup.reg
echo.
@ECHO Installing Nextlabs Policy Controller...

start /wait "MSI Error level" msiexec /i %1EDP-PolicyController-%setup%.msi /passive ENABLE_SERVICE_START_ON_INSTALL=0 COMMAND_LINE_OPTS=DetectSysclockRollback REBOOT=ReallySuppress ICENET_SERVER_LOCATION=%_icenet_server% /l c:\temp\pc6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Policy Controller failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Policy Controller failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
	rem goto no	
) 

@ECHO Installing Nextlabs Desktop Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-DesktopEnforcerForWindows-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\wde6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Desktop Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Desktop Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Key Management...
start /wait "MSI Error level" msiexec /i %1CC-KeyManagementForWindows-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\kmc6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Key Management failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Key Management failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Network Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-NetworkEnforcer-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\ne6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Network Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Network Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs System Encryption...
start /wait "MSI Error level" msiexec /i %1EDP-SystemEncryptionForWindows-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\se6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo System Encryption failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo System Encryption failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Outlook Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-OutlookEnforcer-%oesetup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\oe6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Outlook Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Outlook Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Office Communicator Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-OfficeCommunicatorEnforcer-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\oce6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Office Communicator Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Office Communicator Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Live Meeting Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-LiveMeetingEnforcer-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\lme6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Live Meeting Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Live Meeting Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

@ECHO Installing Nextlabs Removeable Device Enforcer...
start /wait "MSI Error level" msiexec /i %1EDP-RemovableDeviceEnforcer-%setup%.msi /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 /l c:\temp\rde6.0-install-%dt%.log
echo Error level : %ERRORLEVEL%
echo Error level : %ERRORLEVEL% >> %logfile%
if %ERRORLEVEL% NEQ 0 if %ERRORLEVEL% NEQ 3010 (
	echo Removeable Device Enforcer failed to install. Please contact administrator or support!
	net helpmsg %ERRORLEVEL% 
	echo Removeable Device Enforcer failed to install. Please contact administrator or support! >> %logfile%
	net helpmsg %ERRORLEVEL% >> %logfile%
) 

echo Rebooting the system in 10 seconds ...
Shutdown /r /t 10
exit /b 0

:no
echo Please launch the setup next time.
exit /b 1
GOTO Exit


:Exit
@ECHO -----------------------------------------------------------------
@ECHO Installing Complete...
