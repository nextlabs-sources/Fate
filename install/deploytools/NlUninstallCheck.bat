@echo off
setlocal

rem ---------------------------------------------------------------
rem	Copyright 2011 NextLabs, Inc. All rights reserved
rem
rem Description:
rem   NextLabs script to check for error in 5.5.x endpoint product uninstall
rem
rem Written by:
rem	  Poon Fung
rem
rem Version:
rem  0.1.0 - 7/19/2011 Initial checkin
rem  0.2.0 - 7/20/2011 Added comments and enhancements
rem  0.3.0 - 7/22/2011 Added plug-in checking
rem  0.3.6 - 8/13/2011 Check temp folder
rem  0.3.7 - 1/23/2012 Echo more environment variables to help debug running in DOS-32
rem          under x64
rem
rem Notes:
rem  1.	There should be a bin directory in the same directory as NlUninstallCheck.bat.
rem		However, the bin directory is optional. If bin directory exists, this script
rem		will run additional command to collect more info.
rem  2.	The bin directory contains GNUWin32 tools.
rem	 3. FINDSTR is used because tasklist does not set ERRORLEVEL.
rem  4. FINDSTR is used because GNU find does not set ERRORLEVEL.
rem  5. PATH is altered at the being of the script to add bin directory. As a result,
rem		this script will pick up bin/find.exe instead of C:/Windows/System32/find.exe.
rem  6. WMIC output is in UNICODE. Pipe the output through TYPE to convert it to ASCII.
rem  7. There is no pattern to determine NextLabs entries under GPO registry key.
rem		Therefore, we are not doing any checking, just listing GPO entries at the end
rem		and they must be examined manually.
rem  8. GNU find.exe cannot traverse C:/Program Files/NextLabs/Policy Controller directory.
rem		So we cannot run find.exe on C:/Program Files/NextLabs. We need to traverse each
rem		product directory under C:/Program Files/NextLabs instead. Because there is never
rem		a C:/Program Files/NextLabs (x86)/Policy Controller directory, find.exe can
rem		operate on C:/Program Files (x86)/NextLabs.
rem  9. This script checks for existence of "C:/Program Files (x86)" automatically,
rem		so it works on x64 as well as x86 machines.
rem  10. Command "reg query" write errors to CON so it cannot be redirected. To avoid 
rem		confusion, we echo registry key on error to accompany the error message.
rem  11. Command "sc query" returns error status on Windows 7. But on Windows XP and 
rem		Window Server 2003, it always return 0. Need to use findstr to work around this
rem		problem.
rem ---------------------------------------------------------------


echo NextLabs 5.5.x Endpoint Product Uninstall Verification v0.3.7


rem --------------------
rem Jump table

if "%1" == "" goto LBL_PrintUsage
goto LBL_Start


rem --------------------
rem Help

:LBL_PrintUsage
echo Usage: NlUninstallCheck ^<label^>
echo   label   Label used to make up log file name NlUninstallCheck-label.log
echo Example:
echo   NlUninstallCheck "WinXP-WAYA-first"
goto LBL_Exit


rem --------------------
rem Main program

:LBL_Start
set TMP_OUT=NlUninstallCheck00.tmp

ver | findstr /C:"Microsoft Windows" > %TMP_OUT%

rem Initialize variables
set HOST_ARCH=undef
if "%ProgramFiles(x86)%" == "" set HOST_ARCH=x86
if not "%ProgramFiles(x86)%" == "" set HOST_ARCH=x64
set HOST_OS=undef
findstr /C:"Microsoft Windows XP" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 set HOST_OS=WinXP
findstr /C:"Microsoft Windows [Version 5.2." %TMP_OUT% > nul
if %ERRORLEVEL% == 0 set HOST_OS=WS2003
findstr /C:"Microsoft Windows [Version 6.1" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 set HOST_OS=Win7
set LOG_FILE=NlUninstallCheck-%1.log
set PASS_COUNT=0
set FAIL_COUNT=0
set WARN_COUNT=0

rem Add bin directory to path
set PATH=bin;%PATH%

rem Remove log file
if exist %LOG_FILE% del /F %LOG_FILE%


rem Print summary
echo NextLabs 5.5.x Endpoint Product Uninstall Verification v0.3.7 >> %LOG_FILE%
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo TIME = %DATE% %TIME% >> %LOG_FILE%
echo COMPUTERNAME = %COMPUTERNAME% >> %LOG_FILE%
echo PROCESSOR_ARCHITECTURE = %PROCESSOR_ARCHITECTURE% >> %LOG_FILE%
echo PROCESSOR_ARCHITECTURE6432 = %PROCESSOR_ARCHITECTURE6432% >> %LOG_FILE%
echo ProgramFiles = %ProgramFiles% >> %LOG_FILE%
echo ProgramFiles(x86) = %ProgramFiles(x86)% >> %LOG_FILE%
echo ProgramW6432 = %ProgramW6432% >> %LOG_FILE%
echo SystemRoot = %SystemRoot% >> %LOG_FILE%
echo windir = %windir% >> %LOG_FILE%
echo USERNAME = %USERNAME% >> %LOG_FILE%
echo HOST_ARCH = %HOST_ARCH% >> %LOG_FILE%
echo HOST_OS = %HOST_OS% >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
if HOST_OS == WinXP goto LBL_Skip_Whoami
whoami /all >> %LOG_FILE%
:LBL_Skip_Whoami
echo -------------------------------------------------------------- >> %LOG_FILE%

if exist %TMP_OUT% del /F %TMP_OUT%

rem DEBUG: Uncomment the following line and change section name to jump to a section
rem goto LBL_Section13


:LBL_Section1
echo Section 1: Check user mode services
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 1: Check user mode services >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlUninstallCheck01.tmp

echo. >> %LOG_FILE%
set SERVICE_NAME=ComplianceEnforcerService
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_1_1_Fail
echo 1.1 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_1_1_Done
:LBL_1_1_Fail 
echo 1.1 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_1_1_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section2
echo Section 2: Check kernel services (driver)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 2: Check kernel services (driver) >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlUninstallCheck02.tmp

echo. >> %LOG_FILE%
set SERVICE_NAME=nltamper
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_1_Fail
echo 2.1 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_1_Done
:LBL_2_1_Fail 
echo 2.1 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_1_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlcc
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_2_Fail
echo 2.2 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_2_Done
:LBL_2_2_Fail 
echo 2.2 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_2_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlinjection
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_3_Fail
echo 2.3 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_3_Done
:LBL_2_3_Fail 
echo 2.3 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_3_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=procdetect
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_4_Fail
echo 2.4 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_4_Done
:LBL_2_4_Fail 
echo 2.4 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_4_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlSysEncryption
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_5_Fail
echo 2.5 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_5_Done
:LBL_2_5_Fail 
echo 2.5 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_5_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlSysEncryptionFW
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_6_Fail
echo 2.6 PASS - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_2_6_Done
:LBL_2_6_Fail 
echo 2.6 FAIL - "%SERVICE_NAME%" is installed >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_2_6_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section3
echo Section 3: Check driver files (.sys)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 3: Check driver files (.sys) >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DRIVER_FILE=%windir%\System32\drivers\nlcc.sys
if not exist %DRIVER_FILE% echo 3.1 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.1 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1

if %HOST_ARCH% == x86 set DRIVER_FILE=%windir%\System32\drivers\nlinjection32.sys
if %HOST_ARCH% == x64 set DRIVER_FILE=%windir%\System32\drivers\nlinjection64.sys
if not exist %DRIVER_FILE% echo 3.2 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.2 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1
:LBL_3_2_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_crypto.dll
if not exist %DRIVER_FILE% echo 3.3 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.3 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1

set DRIVER_FILE=%windir%\System32\drivers\nl_klog.dll
if not exist %DRIVER_FILE% echo 3.4 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.4 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1

set DRIVER_FILE=%windir%\System32\drivers\nl_SysEncryption.sys
if not exist %DRIVER_FILE% echo 3.5 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.5 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1

set DRIVER_FILE=%windir%\System32\drivers\nl_SysEncryptionFW.sys
if not exist %DRIVER_FILE% echo 3.6 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.6 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1

set DRIVER_FILE=%windir%\System32\drivers\nl_tamper.sys
if not exist %DRIVER_FILE% echo 2.7 PASS -  %DRIVER_FILE% does not exist >> %LOG_FILE%
if not exist %DRIVER_FILE% set /a PASS_COUNT += 1
if exist %DRIVER_FILE% echo 3.7 WARN - %DRIVER_FILE% exists >> %LOG_FILE%
if exist %DRIVER_FILE% set /a WARN_COUNT += 1


:LBL_Section4
echo Section 4: Check processes
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 4: Check processes >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlUninstallCheck04.tmp

tasklist > %TMP_OUT%

echo. >> %LOG_FILE%
set PROCESS_NAME=cepdpman.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_1_Process_Running
echo 4.1 PASS - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_4_1_Done
:LBL_4_1_Process_Running 
echo 4.1 FAIL - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_4_1_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=nlsce.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_2_Process_Running
echo 4.2 PASS - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_4_2_Done
:LBL_4_2_Process_Running 
echo 4.2 FAIL - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_4_2_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=edpmanager.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_3_Process_Running
echo 4.3 PASS - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_4_3_Done
:LBL_4_3_Process_Running 
echo 4.3 FAIL - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_4_3_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=nlca_service.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_4_Process_Running
echo 4.4 PASS - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_4_4_Done
:LBL_4_4_Process_Running 
echo 4.4 FAIL - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_4_4_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section5
echo Section 5: Check registry
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 5: Check registry >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set REGKEY=HKLM\SOFTWARE\NextLabs
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_1_Fail
echo 5.1.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.1.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_1_1_Done
:LBL_5_1_1_Fail 
echo 5.1.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_1_1_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_2_Fail
echo 5.1.2 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.1.2 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_1_2_Done
:LBL_5_1_2_Fail 
echo 5.1.2 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_1_2_Done

set REGKEY=HKLM\SOFTWARE\NextLabs
reg query "%REGKEY%" /V DebugMode > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_3_Fail
echo 5.1.3 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%\DebugMode"
echo 5.1.3 PASS - Registry key "%REGKEY%\DebugMode" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_1_3_Done
:LBL_5_1_3_Fail 
echo 5.1.3 FAIL - Registry key "%REGKEY%\DebugMode" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_5_1_3_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs
reg query "%REGKEY%" /V DebugMode > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_4_Fail
echo 5.1.4 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%\DebugMode"
echo 5.1.4 PASS - Registry key "%REGKEY%\DebugMode" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_1_4_Done
:LBL_5_1_4_Fail 
echo 5.1.4 FAIL - Registry key "%REGKEY%\DebugMode" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_5_1_4_Done

set REGKEY=HKLM\SYSTEM\CurrentControlSet\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_1_Fail
echo 5.2.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.2.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_2_1_Done
:LBL_5_2_1_Fail 
echo 5.2.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_2_1_Done

set REGKEY=HKLM\SYSTEM\CurrentControlSet\Services\ProcDetect
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_2_Fail
echo 5.2.2 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.2.2 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_2_2_Done
:LBL_5_2_2_Fail 
echo 5.2.2 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_2_2_Done

set REGKEY=HKLM\SYSTEM\CurrentControlSet\services\ComplianceEnforcerService
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_3_Fail
echo 5.2.3 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.2.3 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_2_3_Done
:LBL_5_2_3_Fail 
echo 5.2.3 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_2_3_Done

set REGKEY=HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Minimal\ComplianceEnforcerService
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_4_Fail
echo 5.2.4 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.2.4 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_2_4_Done
:LBL_5_2_4_Fail 
echo 5.2.4 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_2_4_Done

set REGKEY=HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Network\ComplianceEnforcerService
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_5_Fail
echo 5.2.5 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.2.5 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_2_5_Done
:LBL_5_2_5_Fail 
echo 5.2.5 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_2_5_Done

set REGKEY=HKLM\SYSTEM\ControlSet001\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_3_1_Fail
echo 5.3.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.3.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_3_1_Done
:LBL_5_3_1_Fail 
echo 5.3.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_3_1_Done

set REGKEY=HKLM\SYSTEM\ControlSet002\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_4_1_Fail
echo 5.4.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.4.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_4_1_Done
:LBL_5_4_1_Fail 
echo 5.4.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_4_1_Done

set REGKEY=HKLM\SYSTEM\ControlSet003\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_1_Fail
echo 5.5.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 5.5.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_5_5_1_Done
:LBL_5_5_1_Fail 
echo 5.5.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_5_5_1_Done

rem set REGKEY=HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify\ComplianceAgentLogon
rem reg query "%REGKEY%" > nul
rem if %ERRORLEVEL% == 0 goto LBL_5_6_1_Fail
rem echo 5.6.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
rem echo 5.6.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
rem set /a PASS_COUNT += 1
rem goto LBL_5_6_1_Done
rem :LBL_5_6_1_Fail 
rem echo 5.6.1 WARN - Registry key "%REGKEY%" exists >> %LOG_FILE%
rem reg query "%REGKEY%" /s >> %LOG_FILE%
rem set /a WARN_COUNT += 1
rem :LBL_5_6_1_Done
 

:LBL_Section6
echo Section 6: Check Program Files
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 6: Check Program Files >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%ProgramFiles%\NextLabs\Common
if exist "%DIR_PATH%" goto LBL_6_1_1_Fail
echo 6.1.1 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_1_Done
:LBL_6_1_1_Fail
echo 6.1.1 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_1_Done

set DIR_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer
if exist "%DIR_PATH%" goto LBL_6_1_2_Fail
echo 6.1.2 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_2_Done
:LBL_6_1_2_Fail
echo 6.1.2 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_2_Done

set DIR_PATH=%ProgramFiles%\NextLabs\Network Enforcer
if exist "%DIR_PATH%" goto LBL_6_1_3_Fail
echo 6.1.3 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_3_Done
:LBL_6_1_3_Fail
echo 6.1.3 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_3_Done

set DIR_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer
if exist "%DIR_PATH%" goto LBL_6_1_4_Fail
echo 6.1.4 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_4_Done
:LBL_6_1_4_Fail
echo 6.1.4 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_4_Done

set DIR_PATH=%ProgramFiles%\NextLabs\Policy Controller
if exist "%DIR_PATH%" goto LBL_6_1_5_Fail
echo 6.1.5 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_5_Done
:LBL_6_1_5_Fail
echo 6.1.5 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_5_Done

set DIR_PATH=%ProgramFiles%\NextLabs\System Encryption
if exist "%DIR_PATH%" goto LBL_6_1_6_Fail
echo 6.1.6 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_1_6_Done
:LBL_6_1_6_Fail
echo 6.1.6 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_1_6_Done

if "%ProgramFiles(x86)%" == "" goto LBL_6_2_Done
echo. >> %LOG_FILE%
set DIR_PATH=%ProgramFiles(x86)%\NextLabs\Common
if exist "%DIR_PATH%" goto LBL_6_2_1_Fail
echo 6.2.1 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_1_Done
:LBL_6_2_1_Fail
echo 6.2.1 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_1_Done

set DIR_PATH=%ProgramFiles(x86)%\NextLabs\Desktop Enforcer
if exist "%DIR_PATH%" goto LBL_6_2_2_Fail
echo 6.2.2 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_2_Done
:LBL_6_2_2_Fail
echo 6.2.2 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_2_Done

set DIR_PATH=%ProgramFiles(x86)%\NextLabs\Network Enforcer
if exist "%DIR_PATH%" goto LBL_6_2_3_Fail
echo 6.2.3 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_3_Done
:LBL_6_2_3_Fail
echo 6.2.3 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_3_Done

set DIR_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer
if exist "%DIR_PATH%" goto LBL_6_2_4_Fail
echo 6.2.4 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_4_Done
:LBL_6_2_4_Fail
echo 6.2.4 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_4_Done

set DIR_PATH=%ProgramFiles(x86)%\NextLabs\Policy Controller
if exist "%DIR_PATH%" goto LBL_6_2_5_Fail
echo 6.2.5 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_5_Done
:LBL_6_2_5_Fail
echo 6.2.5 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_5_Done

set DIR_PATH=%ProgramFiles(x86)%\NextLabs\System Encryption
if exist "%DIR_PATH%" goto LBL_6_2_6_Fail
echo 6.2.6 PASS - %DIR_PATH% does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_2_6_Done
:LBL_6_2_6_Fail
echo 6.2.6 WARN - %DIR_PATH% exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_2_6_Done
:LBL_6_2_Done

set DIR_PATH=%ProgramFiles%\NextLabs
if not exist "%DIR_PATH%" goto LBL_6_3_1_NoDir
set TMP_OUT=NlUninstallCheck06.tmp

dir "%DIR_PATH%\*.exe" "%DIR_PATH%\*.dll" /s > %TMP_OUT%

echo. >> %LOG_FILE%
findstr ".exe$ .dll$" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_6_3_1_Fail
echo 6.3.1 PASS - No binary is left in "%DIR_PATH%" >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_3_1_Done
:LBL_6_3_1_Fail 
echo 6.3.1 WARN - One or more binary is left in "%DIR_PATH%" >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_3_1_Done

if exist %TMP_OUT% del /F %TMP_OUT%
:LBL_6_3_1_NoDir

if "%ProgramFiles(x86)%" == "" goto LBL_6_3_2_NoDir
set DIR_PATH=%ProgramFiles(x86)%\NextLabs
if not exist "%DIR_PATH%" goto LBL_6_3_2_NoDir
set TMP_OUT=NlUninstallCheck06.tmp

dir "%DIR_PATH%\*.exe" "%DIR_PATH%\*.dll" /s > %TMP_OUT%

echo. >> %LOG_FILE%
findstr ".exe$ .dll$" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_6_3_2_Fail
echo 6.3.2 PASS - No binary is left in "%DIR_PATH%" >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_6_3_2_Done
:LBL_6_3_2_Fail 
echo 6.3.2 WARN - One or more binary is left in "%DIR_PATH%" >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_6_3_2_Done

if exist %TMP_OUT% del /F %TMP_OUT%
:LBL_6_3_2_NoDir


:LBL_Section7
echo Section 7: Check Add/Remove Programs (or Uninstall or change a program)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 7: Check Add/Remove Programs (or Uninstall or change a program) >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT1=NlUninstallCheck07_1.tmp
set TMP_OUT2=NlUninstallCheck07_2.tmp

wmic PRODUCT GET > %TMP_OUT1%
type %TMP_OUT1% > %TMP_OUT2%

echo. >> %LOG_FILE%
set PRODUCT_NAME=Policy Controller
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_1_Fail
echo 7.1 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_1_Done
:LBL_7_1_Fail 
echo 7.1 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_1_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=Enterprise Data Protection Network Enforcer For Windows
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_2_Fail
echo 7.2 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_2_Done
:LBL_7_2_Fail 
echo 7.2 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_2_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=Control Center Key Management for Windows
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_3_Fail
echo 7.3 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_3_Done
:LBL_7_3_Fail 
echo 7.3 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_3_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=System Encryption
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_4_Fail
echo 7.4 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_4_Done
:LBL_7_4_Fail 
echo 7.4 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_4_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=Enforcer for Microsoft Outlook
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_5_Fail
echo 7.5 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_5_Done
:LBL_7_5_Fail 
echo 7.5 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_5_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=Enterprise DP Desktop Enforcer for Windows
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_7_6_Fail
echo 7.6 PASS - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_7_6_Done
:LBL_7_6_Fail 
echo 7.6 FAIL - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_7_6_Done

if exist %TMP_OUT1% del /F %TMP_OUT1%
if exist %TMP_OUT2% del /F %TMP_OUT2%


:LBL_Section8
echo Section 8: Check Windows services
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 8: Check Windows services >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT1=NlUninstallCheck08_1.tmp
set TMP_OUT2=NlUninstallCheck08_2.tmp

wmic SERVICE GET > %TMP_OUT1%
type %TMP_OUT1% > %TMP_OUT2%

echo. >> %LOG_FILE%
set SERVICE_NAME=ComplianceEnforcerService
findstr /I /C:"%SERVICE_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_8_1_Fail
echo 8.1 PASS - "%SERVICE_NAME%" is not listed >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_8_1_Done
:LBL_8_1_Fail 
echo 8.1 FAIL - "%SERVICE_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%SERVICE_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a FAIL_COUNT += 1
:LBL_8_1_Done

if exist %TMP_OUT1% del /F %TMP_OUT1%
if exist %TMP_OUT2% del /F %TMP_OUT2%


:LBL_Section9
echo Section 9: Check plug-ins
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 9: Check plug-ins >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlUninstallCheck09.tmp

rem iePEP
echo. >> %LOG_FILE%
set REGKEY=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_1_Plugin_Exist
echo 9.1 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_1_Done
:LBL_9_1_Plugin_Exist
echo 9.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_1_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_2_Plugin_Exist
echo 9.2 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_2_Done
:LBL_9_2_Plugin_Exist
echo 9.2 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.2 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_2_Done

rem enchancement
set REGKEY=HKLM\SOFTWARE\Classes\AppID\{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_3_Plugin_Exist
echo 9.3 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_3_Done
:LBL_9_3_Plugin_Exist
echo 9.3 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.3 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_3_Done

rem NLVLOfficeEnforcer
set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_4_Plugin_Exist
echo 9.4 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_4_Done
:LBL_9_4_Plugin_Exist
echo 9.4 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.4 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_4_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_5_Plugin_Exist
echo 11.5 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_5_Done
:LBL_9_5_Plugin_Exist
echo 9.5 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.5 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_5_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_6_Plugin_Exist
echo 9.6 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_6_Done
:LBL_9_6_Plugin_Exist
echo 9.6 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.6 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_6_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_7_Plugin_Exist
echo 9.7 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_7_Done
:LBL_9_7_Plugin_Exist
echo 9.7 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.7 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_7_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_8_Plugin_Exist
echo 9.8 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_8_Done
:LBL_9_8_Plugin_Exist
echo 9.8 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.8 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_8_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\PowerPoint\Addins\NLVLOfficeEnforcer.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_9_Plugin_Exist
echo 9.9 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_9_Done
:LBL_9_9_Plugin_Exist
echo 9.9 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.9 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_9_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_10_Plugin_Exist
echo 9.10 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_10_Done
:LBL_9_10_Plugin_Exist
echo 9.10 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.10 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_10_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_11_Plugin_Exist
echo 9.11 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_11_Done
:LBL_9_11_Plugin_Exist
echo 9.11 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.11 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_11_Done

set REGKEY=HKCR\*\shellex\ContextMenuHandlers\CE_Explorer
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_12_Plugin_Exist
echo 9.12 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_12_Done
:LBL_9_12_Plugin_Exist
echo 9.12 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.12 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_12_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_13_Plugin_Exist
echo 9.13 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_13_Done
:LBL_9_13_Plugin_Exist
echo 9.13 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.13 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_13_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_14_Plugin_Exist
echo 9.14 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_14_Done
:LBL_9_14_Plugin_Exist
echo 9.14 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.14 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_14_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_15_Plugin_Exist
echo 9.15 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_15_Done
:LBL_9_15_Plugin_Exist
echo 9.15 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.15 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_15_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\PowerPoint\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_16_Plugin_Exist
echo 9.16 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_16_Done
:LBL_9_16_Plugin_Exist
echo 9.16 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.16 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_16_Done

set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_17_Plugin_Exist
echo 9.17 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_17_Done
:LBL_9_17_Plugin_Exist
echo 9.17 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.17 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_17_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_18_Plugin_Exist
echo 9.18 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_18_Done
:LBL_9_18_Plugin_Exist
echo 9.18 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.18 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_18_Done

rem OutlookAddin
set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\OutlookAddin.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_19_Plugin_Exist
echo 9.19 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_19_Done
:LBL_9_19_Plugin_Exist
echo 9.19 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.19 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_19_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\OutlookAddin.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_20_Plugin_Exist
echo 9.20 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_20_Done
:LBL_9_20_Plugin_Exist
echo 9.20 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.20 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_20_Done

rem NlPortableEncryptionCtx
set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\NlPortableEncryptionCtx.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_21_Plugin_Exist
echo 9.21 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_21_Done
:LBL_9_21_Plugin_Exist
echo 9.21 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.21 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_21_Done

set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\NlPortableEncryptionCtx.1
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_9_22_Plugin_Exist
echo 9.22 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_22_Done
:LBL_9_22_Plugin_Exist
echo 9.22 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 9.22 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_9_22_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section10
echo Section 10: Check file extension
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 10: Check file extension >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlUninstallCheck10.tmp

echo. >> %LOG_FILE%
set REGKEY=HKCR\.nxl
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_10_1_Mapping_Exist
echo 10.1 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_10_1_Done
:LBL_10_1_Mapping_Exist
echo 10.1 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 10.1 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_10_1_Done

set REGKEY=HKCR\nxlfile
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_10_2_Mapping_Exist
echo 10.2 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_10_2_Done
:LBL_10_2_Mapping_Exist
echo 10.2 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 10.2 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_10_2_Done

set REGKEY=HKLM\SOFTWARE\Classes\.nxl
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_10_3_Mapping_Exist
echo 10.3 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_10_3_Done
:LBL_10_3_Mapping_Exist
echo 10.3 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 10.3 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_10_3_Done

set REGKEY=HKLM\SOFTWARE\Classes\nxlfile
reg query "%REGKEY%" > %TMP_OUT%
if not %ERRORLEVEL% == 0 goto LBL_10_4_Mapping_Exist
echo 10.4 FAIL - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_10_4_Done
:LBL_10_4_Mapping_Exist
echo 10.4 INFO: Explain error. This is not an error. The specified registry key is "%REGKEY%"
echo 10.4 PASS - Registry key "%REGKEY%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_10_4_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section11
echo Section 11: List folder permission
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 11: List folder permission >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%ProgramFiles%\NextLabs
echo 11.1 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
if %HOST_OS% == WinXP cacls "%DIR_PATH%" >> %LOG_FILE%
if not %HOST_OS% == WinXP icacls "%DIR_PATH%" >> %LOG_FILE%

if "%ProgramFiles(x86)%" == "" goto LBL_11_2_x64_Only
set DIR_PATH=%ProgramFiles(x86)%\NextLabs
echo 11.2 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
if %HOST_OS% == WinXP cacls "%DIR_PATH%" >> %LOG_FILE%
if not %HOST_OS% == WinXP icacls "%DIR_PATH%" >> %LOG_FILE%
:LBL_11_2_x64_Only

set DIR_PATH=%windir%\System32\drivers
echo 11.3 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
if %HOST_OS% == WinXP cacls "%DIR_PATH%" >> %LOG_FILE%
if not %HOST_OS% == WinXP icacls "%DIR_PATH%" >> %LOG_FILE%


:LBL_Section12
echo Section 12: Check MSI log
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 12: Check MSI log >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%windir%\Temp
echo 12.1 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_12_1_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_12_1_Done
:LBL_12_1_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_12_1_Done

set DIR_PATH=%SystemDrive%\Temp
echo 12.2 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_12_2_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_12_2_Done
:LBL_12_2_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_12_2_Done

set DIR_PATH=%TEMP%
echo 12.3 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_12_3_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_12_3_Done
:LBL_12_3_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_12_3_Done

rem DEBUG: Uncomment the following line to skip to end
rem goto LBL_SectionDone


:LBL_Section13
echo Section 13: Check temp folder
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 13: Check temp folder >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
echo 13.0 INFO: Checking temp folder - %TEMP% >> %LOG_FILE%
set FILE_PATH=%TEMP%\installercommon.dll
if exist "%FILE_PATH%" goto LBL_13_1_Found
echo 13.1 PASS - "%FILE_PATH%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_13_1_Done
:LBL_13_1_Found
echo 13.1 WARN - "%FILE_PATH%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_13_1_Done

set FILE_PATH=%TEMP%\PDPStop.dll
if exist "%FILE_PATH%" goto LBL_13_2_Found
echo 13.2 PASS - "%FILE_PATH%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_13_2_Done
:LBL_13_2_Found
echo 13.2 WARN - "%FILE_PATH%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_13_2_Done


:LBL_Section14
echo Section 14: List GPO registry entries (must be reviewed manually)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 14: List GPO registry entries >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
echo 14.1 INFO - GPO install registry entries does not have searchable pattern, just list them here to help debugging >> %LOG_FILE%
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Group Policy\AppMgmt" /s >> %LOG_FILE%


:LBL_SectionDone
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Done >> %LOG_FILE%
echo -- >> %LOG_FILE%

rem DEBUG: Uncomment the following line to show log file content
rem type %LOG_FILE%

echo Additional output is written to log file %LOG_FILE%
echo %PASS_COUNT% passed, %FAIL_COUNT% failed, %WARN_COUNT% warning(s)
echo. >> %LOG_FILE%
echo %PASS_COUNT% passed, %FAIL_COUNT% failed, %WARN_COUNT% warning(s) >> %LOG_FILE%


:LBL_Exit
rem --------------------
rem Clean up

endlocal
