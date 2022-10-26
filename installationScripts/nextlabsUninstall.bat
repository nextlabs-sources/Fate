@Echo off
REM ******************************************************************************************
REM **                              NEXTLABS UNINSTALL                                        **
REM ******************************************************************************************

@echo Creating c:\temp\nextlabs folder
if not exist c:\temp mkdir c:\temp
if not exist c:\temp\nextlabs mkdir c:\temp\nextlabs

setlocal
rem setlocal ENABLEEXTENSIONS
setlocal EnableDelayedExpansion

set bitness=32-bit
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



set VALUE_NAME=ProductCode
set VALUE_PASSWORD=password

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
set logfile=c:\temp\nextlabs\uninstall-%dt%.log

echo Checking the version of MS Office ...
@reg query hklm\software\microsoft\office\14.0\outlook | findstr /i "x64" > nul
IF %ERRORLEVEL% EQU 0 (
	set OEbitness=x64
	set oesetup=setup64
) else (
	set OEbitness=x86
	set oesetup=setup
)

echo OS version      : %version%
echo OS version      : %version% >> %logfile% 
echo OS architecture : %bitness%
echo OS architecture : %bitness% >> %logfile%
echo Outlook bitness : %OEbitness%
echo Outlook bitness : %OEbitness% >> %logfile%
echo Log file        : %logfile%
echo Log file        : %logfile% >> %logfile%
echo.

set uninstallPC=1
set uninstallKMC=1
set pcode=""

@ECHO Checking if client obligations and Desktop scanner were installed...
if EXIST "C:\Program Files\NextLabs\Desktop Scanner" (
	ECHO You have Desktop Scanner installed! You need to uninstall it first before running this script
	GOTO :no
)
if EXIST "C:\Program Files\NextLabs\Policy Controller\obligations" (
	ECHO You have Client obligations installed! You need to uninstall it first before running this script
	GOTO :no
)

@ECHO Uninstalling Nextlabs Removable Device Enforcer...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise\Removable Device Enforcer"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v %VALUE_NAME%') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Removable Device Enforcer Product code : !pcode! >> %logfile%
echo Nextlabs Removable Device Enforcer Product code : !pcode!
if !pcode! == ~-38 goto RDEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\rde6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto RDEdone
	if !ERRORLEVEL! NEQ 3010 goto RDEdone
	if !ERRORLEVEL! NEQ 1605 goto RDEnotFound
	if !ERRORLEVEL! NEQ 1 goto RDEdone
	
		echo RDE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo RDE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0	
		goto RDEdone
		
:RDEnotFound	
	echo RDE is not found !! >> %logfile%
	echo RDE is not found !!
:RDEdone

echo uninstallPC=%uninstallPC%
echo.
set pcode=""

@ECHO Uninstalling Nextlabs Office Communicator Enforcer...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Enterprise DLP\Office Communicator Enforcer"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Office Communicator Enforcer Product code : !pcode!
echo Nextlabs Office Communicator Enforcer Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto OCEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\OCE6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto OCEdone
	if !ERRORLEVEL! NEQ 3010 goto OCEdone
	if !ERRORLEVEL! NEQ 1605 goto OCEnotFound
	if !ERRORLEVEL! NEQ 1 goto OCEdone
	
		echo OCE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo OCE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0	
		goto OCEdone
		
:OCEnotFound	
	echo OCE is not found !! >> %logfile%
	echo OCE is not found !!
:OCEdone

echo uninstallPC=%uninstallPC%
echo.
set pcode=""

@ECHO Uninstalling Nextlabs Live Meeting Enforcer...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Enterprise DLP\Live Meeting Enforcer"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Live Meeting Enforcer Product code : !pcode!
echo Nextlabs Live Meeting Enforcer Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto LMEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\LME6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto LMEdone
	if !ERRORLEVEL! NEQ 3010 goto LMEdone
	if !ERRORLEVEL! NEQ 1605 goto LMEnotFound
	if !ERRORLEVEL! NEQ 1 goto LMEdone
	
		echo LME failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo LME failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0	
		goto LMEdone
		
:LMEnotFound	
	echo LME is not found !! >> %logfile%
	echo LME is not found !!
:LMEdone

echo uninstallPC=%uninstallPC%
echo. 
set pcode=""

@ECHO Uninstalling Nextlabs Network Enforcer...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Enterprise DLP\Network Enforcer"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Network Enforcer Product code : !pcode!
echo Nextlabs Network Enforcer Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto NEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\NE6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto NEdone
	if !ERRORLEVEL! NEQ 3010 goto NEdone
	if !ERRORLEVEL! NEQ 1605 goto NEnotFound
	if !ERRORLEVEL! NEQ 1 goto NEdone
	
		echo NE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo NE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0	
		goto NEdone
		
:NEnotFound	
	echo NE is not found !! >> %logfile%
	echo NE is not found !!
:NEdone

echo uninstallPC=%uninstallPC%
echo.
set pcode="" 

@ECHO Uninstalling Nextlabs Outlook Enforcer...

if %OEbitness% == x64 goto OE64bit
if %bitness%== 32-bit goto OE64bit

set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\NextLabs\Compliant Enterprise\Outlook Enforcer"
goto OEcheckDone

:OE64bit	
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise\Outlook Enforcer"

:OEcheckDone

echo keyname=%key_name%
echo Checking %key_name% >> %logfile%

FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Outlook Enforcer Product code : !pcode! 
echo Nextlabs Outlook Enforcer Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto OEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\OE6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto OEdone
	if !ERRORLEVEL! NEQ 3010 goto OEdone
	if !ERRORLEVEL! NEQ 1605 goto OEnotFound
	if !ERRORLEVEL! NEQ 1 goto OEdone
	
		echo OE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo OE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0	
		goto OEdone
		
:OEnotFound	
	echo NE is not found !! >> %logfile%
	echo NE is not found !!
:OEdone


echo uninstallPC=%uninstallPC%
echo. 
set pcode=""
set ValueValue=""

@ECHO Uninstalling Nextlabs System Encryption...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Enterprise DLP\System Encryption"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs System Encryption Product code : !pcode!
echo Nextlabs System Encryption Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto SEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\SE6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto SEdone
	if !ERRORLEVEL! NEQ 3010 goto SEdone
	if !ERRORLEVEL! NEQ 1605 goto SEnotFound
	if !ERRORLEVEL! NEQ 1 goto SEdone
	
		echo SE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo SE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0
		set uninstallKMC=0
		goto SEdone
		
:SEnotFound	
	echo SE is not found !! >> %logfile%
	echo SE is not found !!
:SEdone


echo uninstallPC=%uninstallPC%
echo.
set pcode="" 

@ECHO Uninstalling Nextlabs Desktop Enforcer...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise\Desktop Enforcer"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Desktop Enforcer Product code : !pcode!
echo Nextlabs Desktop Enforcer Product code : !pcode! >> %logfile%
if !pcode! === ~-38 goto WDEnotFound

	start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\WDE6.0-uninstall-%dt%.log
	echo Error level : !ERRORLEVEL!
	echo Error level : !ERRORLEVEL! >> %logfile%
	if !ERRORLEVEL! NEQ 0 goto WDEdone
	if !ERRORLEVEL! NEQ 3010 goto WDEdone
	if !ERRORLEVEL! NEQ 1605 goto WDEnotFound
	if !ERRORLEVEL! NEQ 1 goto WDEdone
	
		echo WDE failed to install. Please contact administrator or support!
		net helpmsg !ERRORLEVEL! 
		echo WDE failed to install. Please contact administrator or support! >> %logfile%
		net helpmsg !ERRORLEVEL! >> %logfile%
		set uninstallPC=0
		goto WDEdone
		
:WDEnotFound	
	echo WDE is not found !! >> %logfile%
	echo WDE is not found !!
:WDEdone


echo uninstallPC=%uninstallPC%
echo. 
set pcode=""
set ValueValue=""

@ECHO Uninstalling Nextlabs Key Management...
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Enterprise DLP\KeyManagementClient"
FOR /F "tokens=* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO set ValueValue=%%A
set pcode=!valuevalue:~-38!
echo Nextlabs Key Management Product code : !pcode!
echo Nextlabs Key Management Product code : !pcode! >> %logfile%
if !pcode! == ~-38 goto KMCnotFound
	echo Uninstall KMC = !uninstallKMC! with %pcode%
	if !uninstallKMC! == 1 (
		start /wait "MSI Error level" msiexec /x %pcode% /passive REBOOT=ReallySuppress ENABLE_SERVICE_START_ON_INSTALL=0 UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\kmc6.0-uninstall-%dt%.log
		echo Error level : !ERRORLEVEL!
		echo Error level : !ERRORLEVEL! >> %logfile%
		if !ERRORLEVEL! NEQ 0 goto KMCdone
		if !ERRORLEVEL! NEQ 3010 goto KMCdone
		if !ERRORLEVEL! NEQ 1605 goto KMCnotFound
		if !ERRORLEVEL! NEQ 1 goto KMCdone
		
			echo KMC failed to install. Please contact administrator or support!
			net helpmsg !ERRORLEVEL! 
			echo KMC failed to install. Please contact administrator or support! >> %logfile%
			net helpmsg !ERRORLEVEL! >> %logfile%
			set uninstallPC=0
			goto :KMCdone
)	
:KMCnotFound
	echo KMC is not found !! >> %logfile%
	echo KMC is not found !!
:KMCdone


echo uninstallPC=%uninstallPC%
echo. 
set pcode=""

set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\NextLabs\Compliant Enterprise\Policy Controller"

if !uninstallPC! EQU 1 (
	ECHO Uninstalling Nextlabs Policy Controller...
	
	FOR /F "tokens=3* delims=/ " %%A IN ('REG QUERY !KEY_NAME! /v !VALUE_NAME!') DO (
		set v1=%%A 
	)
	set pcode=!v1!

	echo Nextlabs Policy Controller Product code : !pcode!
	echo Nextlabs Policy Controller Product code : !pcode! >> %logfile%
	if P!pcode! == P goto PCnotFound
		echo Uninstalling PC !pcode! ...
		start /wait "MSI Error level" msiexec /x !pcode! /passive ENABLE_SERVICE_START_ON_INSTALL=0 COMMAND_LINE_OPTS=DetectSysclockRollback REBOOT=ReallySuppress UNINSTALL_PASSWORD=%VALUE_PASSWORD% /lvoicewarmup c:\temp\nextlabs\pc6.0-uninstall-%dt%.log
		echo Error level : !ERRORLEVEL!
		echo Error level : !ERRORLEVEL! >> %logfile%
		net helpmsg !ERRORLEVEL! 
		net helpmsg !ERRORLEVEL! >> %logfile%
		
		if !ERRORLEVEL! NEQ 0 goto PCdone
		if !ERRORLEVEL! NEQ 3010 goto PCdone
		if !ERRORLEVEL! NEQ 1605 goto PCnotFound
		if !ERRORLEVEL! NEQ 1 goto PCdone
		
			echo PC failed to install. Please contact administrator or support!
			echo PC failed to install. Please contact administrator or support! >> %logfile%
			goto no
		) 
		
:PCDone
		@ECHO Uninstall completes. Please reboot machine...
		@ECHO Please delete folder C:\Program Files\NextLabs after system reboots...
		goto no
:PCnotFound
		echo Policy Controller is not found !! >> %logfile%
		echo Policy Controller is not found !! 

		goto :no
	)
) else (
	echo One or more Endpoints is not uninstall completely. PC uninstallation is denied.
	echo One or more Endpoints is not uninstall completely. PC uninstallation is denied. >> %logfile%
	


)

:no

endlocal


rem Shutdown /r /t 45






