@echo off
setlocal

rem ---------------------------------------------------------------
rem	Copyright 2011 NextLabs, Inc. All rights reserved
rem
rem Description:
rem   NextLabs script to check for error in 5.5.6 and later endpoint product install
rem
rem Written by:
rem	  Poon Fung
rem
rem Version:
rem  0.1.0 - 7/23/2011 Initial checkin
rem  0.2.0 - 7/30/2011 Added plug-in checking
rem  0.2.3 - 8/3/2011 Fix typo and OE Readme file not detected
rem  0.2.5 - 8/13/2011 Check temp folder
rem  0.3.0 - 1/20/2012 Support 5.5.6 release. Fix changes cause by OE installer change 
rem          to support OE-64-5.5.6 and removal of unneeded .cat files.
rem  0.3.1 - 1/23/2012 Echo more environment variables to help debug running in DOS-32
rem          under x64
rem  0.3.2 - 05/31/2012 Adjust according to 6.0 installers
rem
rem Notes:
rem  1.	There should be a bin directory in the same directory as NlInstallCheck.bat.
rem		However, the bin directory is optional. If bin directory exists, this script
rem		will run additional command to collect more info.
rem  2.	The bin directory contains GNUWin32 tools.
rem	 3. FINDSTR is used because tasklist does not set ERRORLEVEL.
rem  4. FINDSTR is used because GNU find does not set ERRORLEVEL.
rem  5. PATH is altered at the being of the script to add bin directory. As a result,
rem		this script will pick up bin/find.exe instead of C:/Windows/System32/find.exe.
rem  8. WMIC output is in UNICODE. Pipe the output through TYPE to convert it to ASCII.
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


echo NextLabs 5.5.6 and later Endpoint Product Install Verification v0.3.1 (1/20/2012) 


rem --------------------
rem Jump table

if "%1" == "" goto LBL_PrintUsage
goto LBL_Start


rem --------------------
rem Help

:LBL_PrintUsage
echo Usage: NlInstallCheck ^<label^>
echo   label   Label used to make up log file name NlInstallCheck-label.log
echo Example:
echo   NlInstallCheck "WinXP-WAYA-first"
goto LBL_Exit


rem --------------------
rem Main program

:LBL_Start
set TMP_OUT=NlInstallCheck00.tmp

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
set LOG_FILE=NlInstallCheck-%1.log
set PASS_COUNT=0
set FAIL_COUNT=0
set WARN_COUNT=0

rem echo Checking the version of MS Office ...
@reg query hklm\software\microsoft\office\14.0\outlook | findstr /i "x64" > nul
IF %ERRORLEVEL% EQU 0 (
	set OEbitness=x64
) else (
	set OEbitness=x86
)

rem Add bin directory to path
set PATH=bin;%PATH%

rem Remove log file
if exist %LOG_FILE% del /F %LOG_FILE%


rem Print summary
echo NextLabs 5.5.6 and later Endpoint Product Install Verification v0.3.1 (1/20/2012)  >> %LOG_FILE%
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
echo MS Office bitness = %OEbitness% >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
if HOST_OS == WinXP goto LBL_Skip_Whoami
whoami /all >> %LOG_FILE%
:LBL_Skip_Whoami
echo -------------------------------------------------------------- >> %LOG_FILE%

if exist %TMP_OUT% del /F %TMP_OUT%

rem DEBUG: Uncomment the following line and change section name to jump to a section
rem goto LBL_Section15


:LBL_Section0
echo Section 0: Locate ReadMe.txt
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 0: Locate ReadMe.txt >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Desktop Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_2_Found
echo 0.2 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_2_Done
:LBL_0_2_Found
echo 0.2 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_2_Done

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Network Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_3_Found
echo 0.3 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_3_Done
:LBL_0_3_Found
echo 0.3 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_3_Done

if %HOST_ARCH% == x64 goto LBL_0_4_x86_Only
echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Outlook Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_4_Found
echo 0.4 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_4_Done
:LBL_0_4_Found
echo 0.4 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_4_Done
:LBL_0_4_x86_Only

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Policy Controller\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_5_Found
echo 0.5 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_5_Done
:LBL_0_5_Found
echo 0.5 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_5_Done

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\System Encryption\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_6_Found
echo 0.6 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_6_Done
:LBL_0_6_Found
echo 0.6 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_6_Done

rem C:\Program Files (x86) is for x64 only
if %HOST_ARCH% == x86 goto LBL_0_x64_Only
echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_7_Found
echo 0.7 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_7_Done
:LBL_0_7_Found
echo 0.7 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_7_Done
:LBL_0_x64_Only

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Live Meeting Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_8_Found
echo 0.8 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_8_Done
:LBL_0_8_Found
echo 0.8 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_8_Done

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Office Communicator Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_10_Found
echo 0.10 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_10_Done
:LBL_0_10_Found
echo 0.10 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_10_Done

echo. >> %LOG_FILE%
set README_FILE=%ProgramFiles%\NextLabs\Removable Device Enforcer\ReadMe.txt
if exist "%README_FILE%" goto LBL_0_12_Found
echo 0.12 FAIL - "%README_FILE%" does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_0_12_Done
:LBL_0_12_Found
echo 0.12 PASS - "%README_FILE%" exists >> %LOG_FILE%
type "%README_FILE%" >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_0_12_Done


:LBL_Section1
echo Section 1: Check user mode services
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 1: Check user mode services >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck01.tmp

echo. >> %LOG_FILE%
set SERVICE_NAME=ComplianceEnforcerService
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_1_1_Installed
echo 1.1 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_1_1_Done
:LBL_1_1_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_1_1_Running
echo 1.1 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_1_1_Done
:LBL_1_1_Running
echo 1.1 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_1_1_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section2
echo Section 2: Check kernel services (driver)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 2: Check kernel services (driver) >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck02.tmp

echo. >> %LOG_FILE%
set SERVICE_NAME=nltamper
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_1_Installed
echo 2.1 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_1_Done
:LBL_2_1_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_1_Running
echo 2.1 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_1_Done
:LBL_2_1_Running
echo 2.1 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_2_1_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlcc
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_2_Installed
echo 2.2 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_2_Done
:LBL_2_2_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_2_Running
echo 2.2 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_2_Done
:LBL_2_2_Running
echo 2.2 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_2_2_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlinjection
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_3_Installed
echo 2.3 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_3_Done
:LBL_2_3_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_3_Running
echo 2.3 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_3_Done
:LBL_2_3_Running
echo 2.3 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_2_3_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=procdetect
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_4_Installed
echo 2.4 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_4_Done
:LBL_2_4_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_4_Running
echo 2.4 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_4_Done
:LBL_2_4_Running
echo 2.4 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_2_4_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlSysEncryption
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_5_Installed
echo 2.5 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_5_Done
:LBL_2_5_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_5_Running
echo 2.5 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_5_Done
:LBL_2_5_Running
echo 2.5 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_2_5_Done

echo. >> %LOG_FILE%
set SERVICE_NAME=nlSysEncryptionFW
sc query "%SERVICE_NAME%" > %TMP_OUT%
findstr "SERVICE_NAME:" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_2_6_Installed
echo 2.6 FAIL - "%SERVICE_NAME%" is not installed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_6_Done
:LBL_2_6_Installed
findstr "STATE" %TMP_OUT% | findstr "RUNNING" > nul
if %ERRORLEVEL% == 0 goto LBL_2_6_Running
echo 2.6 FAIL - "%SERVICE_NAME%" is installed but not running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_2_6_Done
:LBL_2_6_Running
echo 2.6 PASS - "%SERVICE_NAME%" is running >> %LOG_FILE%
type %TMP_OUT% >> %LOG_FILE%
set /a PASS_COUNT += 1
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
if exist %DRIVER_FILE% goto LBL_3_1_Installed
echo 3.1 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_1_Done
:LBL_3_1_Installed
echo 3.1 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_1_Done

if %HOST_ARCH% == x86 set DRIVER_FILE=%windir%\System32\drivers\nlinjection32.sys
if %HOST_ARCH% == x64 set DRIVER_FILE=%windir%\System32\drivers\nlinjection64.sys
if exist %DRIVER_FILE% goto LBL_3_2_Install
echo 3.2 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_2_Done
:LBL_3_2_Install
echo 3.2 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_2_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_crypto.dll
if exist %DRIVER_FILE% goto LBL_3_3_Installed
echo 3.3 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_3_Done
:LBL_3_3_Installed
echo 3.3 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_3_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_klog.dll
if exist %DRIVER_FILE% goto LBL_3_4_Installed
echo 3.4 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_4_Done
:LBL_3_4_Installed
echo 3.4 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_4_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_SysEncryption.sys
if exist %DRIVER_FILE% goto LBL_3_5_Installed
echo 3.5 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_5_Done
:LBL_3_5_Installed
echo 3.5 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_5_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_SysEncryptionFW.sys
if exist %DRIVER_FILE% goto LBL_3_6_Installed
echo 3.6 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_6_Done
:LBL_3_6_Installed
echo 3.6 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_6_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_tamper.sys
if exist %DRIVER_FILE% goto LBL_3_7_Installed
echo 3.7 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_7_Done
:LBL_3_7_Installed
echo 3.7 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_7_Done

set DRIVER_FILE=%windir%\System32\drivers\nl_devenf.sys
if exist %DRIVER_FILE% goto LBL_3_8_Installed
echo 3.8 FAIL - "%DRIVER_FILE%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_3_8_Done
:LBL_3_8_Installed
echo 3.8 PASS - "%DRIVER_FILE%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_3_8_Done

echo. >> %LOG_FILE%
dir %windir%\System32\drivers | findstr "nlcc.sys nlinjection32.sys nlinjection64.sys nl_tamper.sys nl_klog.dll nl_crypto.dll nl_SysEncryption.sys nl_SysEncryptionFW.sys nl_devdef.sys" >> %LOG_FILE%


:LBL_Section4
echo Section 4: Check processes
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 4: Check processes >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck04.tmp

tasklist > %TMP_OUT%

echo. >> %LOG_FILE%
set PROCESS_NAME=cepdpman.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_1_Process_Running
echo 4.1 FAIL - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_4_1_Done
:LBL_4_1_Process_Running 
echo 4.1 PASS - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_4_1_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=nlsce.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_2_Process_Running
echo 4.2 FAIL - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_4_2_Done
:LBL_4_2_Process_Running 
echo 4.2 PASS - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_4_2_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=edpmanager.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_3_Process_Running
echo 4.3 FAIL - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_4_3_Done
:LBL_4_3_Process_Running 
echo 4.3 PASS - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_4_3_Done

echo. >> %LOG_FILE%
set PROCESS_NAME=nlca_service.exe
findstr /B "%PROCESS_NAME%" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_4_4_Process_Running
echo 4.4 FAIL - "%PROCESS_NAME%" is not running >> %LOG_FILE%
findstr /B "%PROCESS_NAME%" %TMP_OUT% >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_4_4_Done
:LBL_4_4_Process_Running 
echo 4.4 PASS - "%PROCESS_NAME%" is running >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_4_4_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section5
echo Section 5: Check registry
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 5: Check registry >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck05.tmp

rem Key #1
echo. >> %LOG_FILE%
set REGKEY=HKLM\SOFTWARE\NextLabs\CommonLibraries
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_1_Key_Exist
echo 5.1 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.1 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_1_Done
:LBL_5_1_Key_Exist

findstr "VISIBLE" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_1_Key_Exist
echo 5.1.1 FAIL - Registry key %REGKEY%\VISIBLE is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_1_1_Done
:LBL_5_1_1_Key_Exist 
echo 5.1.1 PASS - Registry key %REGKEY%\VISIBLE exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_1_1_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_1_2_Key_Exist
echo 5.1.2 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_1_2_Done
:LBL_5_1_2_Key_Exist 
echo 5.1.2 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_1_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_1_Done


rem Key #2
set REGKEY=HKLM\SOFTWARE\NextLabs\Compliant Enterprise\Desktop Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_2_Key_Exist
echo 5.2 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.2 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_Done
:LBL_5_2_Key_Exist

findstr "PluginConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_1_Key_Exist
echo 5.2.1 FAIL - Registry key %REGKEY%\PluginConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_1_Done
:LBL_5_2_1_Key_Exist 
echo 5.2.1 PASS - Registry key %REGKEY%\PluginConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_2_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_2_Key_Exist
echo 5.2.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_2_Done
:LBL_5_2_2_Key_Exist 
echo 5.2.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_2_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_3_Key_Exist
echo 5.2.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_3_Done
:LBL_5_2_3_Key_Exist 
echo 5.2.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_2_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_4_Key_Exist
echo 5.2.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_4_Done
:LBL_5_2_4_Key_Exist 
echo 5.2.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_2_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_2_5_Key_Exist
echo 5.2.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_2_5_Done
:LBL_5_2_5_Key_Exist 
echo 5.2.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_2_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_2_Done


rem Key #3
set REGKEY=HKLM\SOFTWARE\NextLabs\Compliant Enterprise\Policy Controller
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_3_Key_Exist
echo 5.3 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.3 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_3_Done
:LBL_5_3_Key_Exist

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_3_1_Key_Exist
echo 5.3.1 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_3_1_Done
:LBL_5_3_1_Key_Exist 
echo 5.3.1 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_3_1_Done

findstr "PolicyControllerDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_3_2_Key_Exist
echo 5.3.2 FAIL - Registry key %REGKEY%\PolicyControllerDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_3_2_Done
:LBL_5_3_2_Key_Exist 
echo 5.3.2 PASS - Registry key %REGKEY%\PolicyControllerDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_3_2_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_3_3_Key_Exist
echo 5.3.3 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_3_3_Done
:LBL_5_3_3_Key_Exist 
echo 5.3.3 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_3_3_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_3_4_Key_Exist
echo 5.3.4 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_3_4_Done
:LBL_5_3_4_Key_Exist 
echo 5.3.4 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_3_4_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_3_Done


rem Key #4
set REGKEY=HKLM\SOFTWARE\NextLabs\Enterprise DLP\KeyManagementClient
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_4_Key_Exist
echo 5.4 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.4 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_4_Done
:LBL_5_4_Key_Exist

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_4_1_Key_Exist
echo 5.4.1 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_4_1_Done
:LBL_5_4_1_Key_Exist 
echo 5.4.1 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_4_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_4_2_Key_Exist
echo 5.4.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_4_2_Done
:LBL_5_4_2_Key_Exist 
echo 5.4.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_4_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_4_Done


rem Key #5
set REGKEY=HKLM\SOFTWARE\NextLabs\Enterprise DLP\Network Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_5_Key_Exist
echo 5.5 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.5 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_Done
:LBL_5_5_Key_Exist

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_1_Key_Exist
echo 5.5.1 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_1_Done
:LBL_5_5_1_Key_Exist 
echo 5.5.1 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_2_Key_Exist
echo 5.5.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_2_Done
:LBL_5_5_2_Key_Exist 
echo 5.5.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_2_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_3_Key_Exist
echo 5.5.3 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_3_Done
:LBL_5_5_3_Key_Exist 
echo 5.5.3 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_3_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_4_Key_Exist
echo 5.5.4 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_4_Done
:LBL_5_5_4_Key_Exist 
echo 5.5.4 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_4_Done

findstr "ProductCode" %TMP_OUT% > nul
findstr "ServiceInjectionDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_5_Key_Exist
echo 5.5.5 FAIL - Registry key %REGKEY%\ServiceInjectionDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_5_Done
:LBL_5_5_5_Key_Exist 
echo 5.5.5 PASS - Registry key %REGKEY%\ServiceInjectionDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_5_Done

findstr "BinDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_5_6_Key_Exist
echo 5.5.6 FAIL - Registry key %REGKEY%\BinDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_5_6_Done
:LBL_5_5_6_Key_Exist 
echo 5.5.6 PASS - Registry key %REGKEY%\BinDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_5_6_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_5_Done


rem Key #6
set REGKEY=HKLM\SOFTWARE\NextLabs\Enterprise DLP\System Encryption
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_6_Key_Exist
echo 5.6 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.6 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_Done
:LBL_5_6_Key_Exist

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_6_1_Key_Exist
echo 5.8.1 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_1_Done
:LBL_5_6_1_Key_Exist 
echo 5.8.1 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_6_1_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_6_2_Key_Exist
echo 5.8.2 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_2_Done
:LBL_5_6_2_Key_Exist 
echo 5.8.2 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_6_2_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_6_3_Key_Exist
echo 5.8.3 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_3_Done
:LBL_5_6_3_Key_Exist 
echo 5.8.3 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_6_3_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_6_4_Key_Exist
echo 5.8.4 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_4_Done
:LBL_5_6_4_Key_Exist 
echo 5.8.4 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_6_4_Done

findstr "PluginConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_6_5_Key_Exist
echo 5.8.5 FAIL - Registry key %REGKEY%\PluginConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_6_5_Done
:LBL_5_6_5_Key_Exist 
echo 5.8.5 PASS - Registry key %REGKEY%\PluginConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_6_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_6_Done

rem 32-bit hive
set REGKEY=HKLM\SOFTWARE\Wow6432Node
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_7_Wow6432Node_Exist
echo 5.7 INFO: Explain error. The specified registry key is "%REGKEY%"
goto LBL_5_7_Wow6432Node_Done

rem Key #7
:LBL_5_7_Wow6432Node_Exist
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\CommonLibraries
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_7_Key_Exist
echo 5.7 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.7 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_7_Done
:LBL_5_7_Key_Exist

findstr "VISIBLE" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_7_1_Key_Exist
echo 5.7.1 FAIL - Registry key %REGKEY%\VISIBLE is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_7_1_Done
:LBL_5_7_1_Key_Exist 
echo 5.7.1 PASS - Registry key %REGKEY%\VISIBLE exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_7_1_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_7_2_Key_Exist
echo 5.7.2 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_7_2_Done
:LBL_5_7_2_Key_Exist 
echo 5.7.2 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_7_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_7_Done


rem Key #8
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Compliant Enterprise\Desktop Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_8_Key_Exist
echo 5.8 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.8 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_Done
:LBL_5_8_Key_Exist

findstr "PluginConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_8_1_Key_Exist
echo 5.8.1 FAIL - Registry key %REGKEY%\PluginConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_1_Done
:LBL_5_8_1_Key_Exist 
echo 5.8.1 PASS - Registry key %REGKEY%\PluginConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_8_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_8_2_Key_Exist
echo 5.8.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_2_Done
:LBL_5_8_2_Key_Exist 
echo 5.8.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_8_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_8_3_Key_Exist
echo 5.8.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_3_Done
:LBL_5_8_3_Key_Exist 
echo 5.8.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_8_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_8_4_Key_Exist
echo 5.8.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_4_Done
:LBL_5_8_4_Key_Exist 
echo 5.8.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_8_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_8_5_Key_Exist
echo 5.8.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_8_5_Done
:LBL_5_8_5_Key_Exist 
echo 5.8.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_8_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_8_Done


rem Key #9
if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\NextLabs\Compliant Enterprise\Outlook Enforcer
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Compliant Enterprise\Outlook Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_9_Key_Exist
echo 5.9 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.9 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_Done
:LBL_5_9_Key_Exist

findstr "PluginConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_9_1_Key_Exist
echo 5.9.1 FAIL - Registry key %REGKEY%\PluginConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_1_Done
:LBL_5_9_1_Key_Exist 
echo 5.9.1 PASS - Registry key %REGKEY%\PluginConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_9_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_9_2_Key_Exist
echo 5.9.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_2_Done
:LBL_5_9_2_Key_Exist 
echo 5.9.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_9_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_9_3_Key_Exist
echo 5.9.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_3_Done
:LBL_5_9_3_Key_Exist 
echo 5.9.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_9_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_9_4_Key_Exist
echo 5.9.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_4_Done
:LBL_5_9_4_Key_Exist 
echo 5.9.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_9_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_9_5_Key_Exist
echo 5.9.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_9_5_Done
:LBL_5_9_5_Key_Exist 
echo 5.9.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_9_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_9_Done


rem Key #10
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Compliant Enterprise\Policy Controller
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_10_Key_Exist
echo 5.10 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.10 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_10_Done
:LBL_5_10_Key_Exist

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_10_1_Key_Exist
echo 5.10.1 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_10_1_Done
:LBL_5_10_1_Key_Exist 
echo 5.10.1 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_10_1_Done

findstr "PolicyControllerDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_10_2_Key_Exist
echo 5.10.2 FAIL - Registry key %REGKEY%\PolicyControllerDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_10_2_Done
:LBL_5_10_2_Key_Exist 
echo 5.10.2 PASS - Registry key %REGKEY%\PolicyControllerDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_10_2_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_10_3_Key_Exist
echo 5.10.3 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_10_3_Done
:LBL_5_10_3_Key_Exist 
echo 5.10.3 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_10_3_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_10_4_Key_Exist
echo 5.10.4 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_10_4_Done
:LBL_5_10_4_Key_Exist 
echo 5.10.4 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_10_4_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_10_Done


rem Key #11
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Enterprise DLP\KeyManagementClient
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_11_Key_Exist
echo 5.11 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.11 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_11_Done
:LBL_5_11_Key_Exist

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_11_1_Key_Exist
echo 5.11.1 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_11_1_Done
:LBL_5_11_1_Key_Exist 
echo 5.11.1 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_11_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_11_2_Key_Exist
echo 5.11.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_11_2_Done
:LBL_5_11_2_Key_Exist 
echo 5.11.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_11_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_11_Done


rem Key #12
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Enterprise DLP\Network Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_12_Key_Exist
echo 5.12 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.12 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_Done
:LBL_5_12_Key_Exist

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_1_Key_Exist
echo 5.12.1 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_1_Done
:LBL_5_12_1_Key_Exist 
echo 5.12.1 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_2_Key_Exist
echo 5.12.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_2_Done
:LBL_5_12_2_Key_Exist 
echo 5.12.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_2_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_3_Key_Exist
echo 5.12.3 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_3_Done
:LBL_5_12_3_Key_Exist 
echo 5.12.3 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_3_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_4_Key_Exist
echo 5.12.4 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_4_Done
:LBL_5_12_4_Key_Exist 
echo 5.12.4 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_4_Done

findstr "ServiceInjectionDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_5_Key_Exist
echo 5.12.5 FAIL - Registry key %REGKEY%\ServiceInjectionDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_5_Done
:LBL_5_12_5_Key_Exist 
echo 5.12.5 PASS - Registry key %REGKEY%\ServiceInjectionDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_5_Done

findstr "BinDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_12_6_Key_Exist
echo 5.12.6 FAIL - Registry key %REGKEY%\BinDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_12_6_Done
:LBL_5_12_6_Key_Exist 
echo 5.12.6 PASS - Registry key %REGKEY%\BinDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_12_6_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_12_Done


rem Key #13
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Enterprise DLP\System Encryption
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_13_Key_Exist
echo 5.13 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.13 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_Done
:LBL_5_13_Key_Exist

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_13_1_Key_Exist
echo 5.13.1 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_1_Done
:LBL_5_13_1_Key_Exist 
echo 5.13.1 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_13_1_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_13_2_Key_Exist
echo 5.13.2 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_2_Done
:LBL_5_13_2_Key_Exist 
echo 5.13.2 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_13_2_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_13_3_Key_Exist
echo 5.13.3 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_3_Done
:LBL_5_13_3_Key_Exist 
echo 5.13.3 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_13_3_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_13_4_Key_Exist
echo 5.13.4 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_4_Done
:LBL_5_13_4_Key_Exist 
echo 5.13.4 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_13_4_Done

findstr "PluginConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_13_5_Key_Exist
echo 5.13.5 FAIL - Registry key %REGKEY%\PluginConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_13_5_Done
:LBL_5_13_5_Key_Exist 
echo 5.13.5 PASS - Registry key %REGKEY%\PluginConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_13_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_13_Done
:LBL_5_7_Wow6432Node_Done


rem Key #14
set REGKEY=HKLM\SYSTEM\CurrentControlSet\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_14_Key_Exist
echo 5.14 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.14 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_14_Done
:LBL_5_14_Key_Exist

findstr "EventMessageFile" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_14_1_Key_Exist
echo 5.14.1 FAIL - Registry key %REGKEY%\EventMessageFile is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_14_1_Done
:LBL_5_14_1_Key_Exist 
echo 5.14.1 PASS - Registry key %REGKEY%\EventMessageFile exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_14_1_Done

findstr "TypesSupported" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_14_2_Key_Exist
echo 5.14.2 FAIL - Registry key %REGKEY%\TypesSupported is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_14_2_Done
:LBL_5_14_2_Key_Exist 
echo 5.14.2 PASS - Registry key %REGKEY%\TypesSupported exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_14_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_14_Done


rem Key #15
set REGKEY=HKLM\SYSTEM\CurrentControlSet\Services\ProcDetect
reg query "%REGKEY%" /V ImagePath > nul
if %ERRORLEVEL% == 0 goto LBL_5_15_Key_Exist
echo 5.15 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.15 FAIL - Registry key %REGKEY%\ImagePath is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_15_Done
:LBL_5_15_Key_Exist 
echo 5.15 PASS - Registry key %REGKEY%\ImagePath exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_15_Done


rem Key #16
set REGKEY=HKLM\SYSTEM\CurrentControlSet\services\ComplianceEnforcerService
reg query "%REGKEY%" /V ImagePath > nul
if %ERRORLEVEL% == 0 goto LBL_5_16_Key_Exist
echo 5.16 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.16 FAIL - Registry key %REGKEY%\ImagePath is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_16_Done
:LBL_5_16_Key_Exist 
echo 5.16 PASS - Registry key %REGKEY%\ImagePath exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_16_Done


rem Key #17
set REGKEY=HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Minimal\ComplianceEnforcerService
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_17_Key_Exist
echo 5.17 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.17 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_17_Done
:LBL_5_17_Key_Exist 
echo 5.17 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_17_Done

rem Key #18
set REGKEY=HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Network\ComplianceEnforcerService
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_18_Key_Exist
echo 5.18 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.18 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_18_Done
:LBL_5_18_Key_Exist 
echo 5.18 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
reg query "%REGKEY%" /s >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_18_Done

rem Key #19
set REGKEY=HKLM\SYSTEM\ControlSet001
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_19_ControlSet_Exist
echo 5.19 INFO: Explain error. The specified registry key is "%REGKEY%"
goto LBL_5_19_Done
:LBL_5_19_ControlSet_Exist
set REGKEY=HKLM\SYSTEM\ControlSet001\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_19_Key_Exist
echo 5.19 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.19 WARN - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_19_Done
:LBL_5_19_Key_Exist

findstr "EventMessageFile" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_19_1_Key_Exist
echo 5.19.1 WARN - Registry key %REGKEY%\EventMessageFile is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_19_1_Done
:LBL_5_19_1_Key_Exist 
echo 5.19.1 PASS - Registry key %REGKEY%\EventMessageFile exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_19_1_Done

findstr "TypesSupported" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_19_2_Key_Exist
echo 5.19.2 WARN - Registry key %REGKEY%\TypesSupported is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_19_2_Done
:LBL_5_19_2_Key_Exist 
echo 5.19.2 PASS - Registry key %REGKEY%\TypesSupported exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_19_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_19_Done


rem Key #20
set REGKEY=HKLM\SYSTEM\ControlSet002
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_20_ControlSet_Exist
echo 5.20 INFO: Explain error. The specified registry key is "%REGKEY%"
goto LBL_5_20_Done
:LBL_5_20_ControlSet_Exist
set REGKEY=HKLM\SYSTEM\ControlSet002\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_20_Exist
echo 5.20 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.20 WARN - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_20_Done
:LBL_5_20_Exist

findstr "EventMessageFile" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_20_1_Key_Exist
echo 5.20.1 WARN - Registry key %REGKEY%\EventMessageFile is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_20_1_Done
:LBL_5_20_1_Key_Exist 
echo 5.20.1 PASS - Registry key %REGKEY%\EventMessageFile exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_20_1_Done

findstr "TypesSupported" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_20_2_Key_Exist
echo 5.20.2 WARN - Registry key %REGKEY%\TypesSupported is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_20_2_Done
:LBL_5_20_2_Key_Exist 
echo 5.20.2 PASS - Registry key %REGKEY%\TypesSupported exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_20_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_20_Done


rem Key #21
set REGKEY=HKLM\SYSTEM\ControlSet003
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_21_ControlSet_Exist
echo 5.21 INFO: Explain error. The specified registry key is "%REGKEY%"
goto LBL_5_21_Done
:LBL_5_21_ControlSet_Exist
set REGKEY=HKLM\SYSTEM\ControlSet003\services\eventlog\Application\Compliant Enterprise
reg query "%REGKEY%" > nul
if %ERRORLEVEL% == 0 goto LBL_5_21_Exist
echo 5.21 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.21 WARN - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_21_Done
:LBL_5_21_Exist

findstr "EventMessageFile" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_21_1_Key_Exist
echo 5.21.1 WARN - Registry key %REGKEY%\EventMessageFile is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_21_1_Done
:LBL_5_21_1_Key_Exist 
echo 5.21.1 PASS - Registry key %REGKEY%\EventMessageFile exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_21_1_Done

findstr "TypesSupported" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_21_2_Key_Exist
echo 5.21.2 WARN - Registry key %REGKEY%\TypesSupported is missing >> %LOG_FILE%
set /a WARN_COUNT += 1
goto LBL_5_21_2_Done
:LBL_5_21_2_Key_Exist 
echo 5.21.2 PASS - Registry key %REGKEY%\TypesSupported exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_21_2_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_21_Done


rem Key #22
rem set REGKEY=HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify\ComplianceAgentLogon
rem reg query "%REGKEY%" /V ImagePath > nul
rem if %ERRORLEVEL% == 0 goto LBL_5_22_Key_Exist
rem echo 5.22 INFO: Explain error. The specified registry key is "%REGKEY%"
rem echo 5.22 FAIL - Registry key %REGKEY%\ImagePath is missing >> %LOG_FILE%
rem set /a FAIL_COUNT += 1
rem goto LBL_5_22_Done
rem :LBL_5_22_Key_Exist 
rem echo 5.22 PASS - Registry key %REGKEY%\ImagePath exists >> %LOG_FILE%
rem reg query "%REGKEY%" /s >> %LOG_FILE%
rem set /a PASS_COUNT += 1
rem :LBL_5_22_Done


rem Key #23
rem if %HOST_ARCH% == x86 
set REGKEY=HKLM\SOFTWARE\NextLabs\Enterprise DLP\Live Meeting Enforcer
rem if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Enterprise DLP\Live Meeting Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_23_Key_Exist
echo 5.23 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.23 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_Done
:LBL_5_23_Key_Exist

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_23_1_Key_Exist
echo 5.23.1 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_1_Done
:LBL_5_23_1_Key_Exist 
echo 5.23.1 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_23_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_23_2_Key_Exist
echo 5.23.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_2_Done
:LBL_5_23_2_Key_Exist 
echo 5.23.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_23_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_23_3_Key_Exist
echo 5.23.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_3_Done
:LBL_5_23_3_Key_Exist 
echo 5.23.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_23_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_23_4_Key_Exist
echo 5.23.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_4_Done
:LBL_5_23_4_Key_Exist 
echo 5.23.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_23_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_23_5_Key_Exist
echo 5.23.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_23_5_Done
:LBL_5_23_5_Key_Exist 
echo 5.23.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_23_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_23_Done


rem Key #24
rem if %HOST_ARCH% == x86 
set REGKEY=HKLM\SOFTWARE\NextLabs\Enterprise DLP\Office Communicator Enforcer
rem if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs\Enterprise DLP\Office Communicator Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_24_Key_Exist
echo 5.24 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.24 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_Done
:LBL_5_24_Key_Exist

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_24_1_Key_Exist
echo 5.24.1 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_1_Done
:LBL_5_24_1_Key_Exist 
echo 5.24.1 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_24_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_24_2_Key_Exist
echo 5.24.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_2_Done
:LBL_5_24_2_Key_Exist 
echo 5.24.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_24_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_24_3_Key_Exist
echo 5.24.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_3_Done
:LBL_5_24_3_Key_Exist 
echo 5.24.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_24_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_24_4_Key_Exist
echo 5.24.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_4_Done
:LBL_5_24_4_Key_Exist 
echo 5.24.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_24_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_24_5_Key_Exist
echo 5.24.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_24_5_Done
:LBL_5_24_5_Key_Exist 
echo 5.24.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_24_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_24_Done


rem Key #25
set REGKEY=HKLM\SOFTWARE\NextLabs\Compliant Enterprise\Removable Device Enforcer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_5_25_Key_Exist
echo 5.25 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 5.25 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_Done
:LBL_5_25_Key_Exist

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_25_1_Key_Exist
echo 5.25.1 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_1_Done
:LBL_5_25_1_Key_Exist 
echo 5.25.1 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_25_1_Done

findstr "ProductCode" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_25_2_Key_Exist
echo 5.25.2 FAIL - Registry key %REGKEY%\ProductCode is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_2_Done
:LBL_5_25_2_Key_Exist 
echo 5.25.2 PASS - Registry key %REGKEY%\ProductCode exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_25_2_Done

findstr "TamperResistanceConfigDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_25_3_Key_Exist
echo 5.25.3 FAIL - Registry key %REGKEY%\TamperResistanceConfigDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_3_Done
:LBL_5_25_3_Key_Exist 
echo 5.25.3 PASS - Registry key %REGKEY%\TamperResistanceConfigDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_25_3_Done

findstr "InstallDir" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_25_4_Key_Exist
echo 5.25.4 FAIL - Registry key %REGKEY%\InstallDir is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_4_Done
:LBL_5_25_4_Key_Exist 
echo 5.25.4 PASS - Registry key %REGKEY%\InstallDir exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_25_4_Done

findstr "ProductVersion" %TMP_OUT% > nul
if %ERRORLEVEL% == 0 goto LBL_5_25_5_Key_Exist
echo 5.25.5 FAIL - Registry key %REGKEY%\ProductVersion is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_5_25_5_Done
:LBL_5_25_5_Key_Exist 
echo 5.25.5 PASS - Registry key %REGKEY%\ProductVersion exists >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_5_25_5_Done

type %TMP_OUT% >> %LOG_FILE%
:LBL_5_25_Done
 
if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section8
echo Section 8: Check Program Files
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 8: Check Program Files >> %LOG_FILE%
echo -- >> %LOG_FILE%

rem Content of C:\Program Files\NextLabs\Common
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Common
if exist "%FILE_PATH%" goto LBL_8_1_File_Exist
echo 8.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_1_Done
:LBL_8_1_File_Exist
rem set FILE_PATH=%ProgramFiles%\NextLabs\Common\ReadMe.txt
rem if not exist "%FILE_PATH%" echo 8.1.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
rem if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
rem if exist "%FILE_PATH%" echo 8.1.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
rem if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Common\bin32
set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32
if exist "%FILE_PATH%" goto LBL_8_1_1_File_Exist
echo 8.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_1_1_Done
:LBL_8_1_1_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\atl90.dll
if not exist "%FILE_PATH%" echo 8.1.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cebrain32.dll
if not exist "%FILE_PATH%" echo 8.1.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cecem32.dll
if not exist "%FILE_PATH%" echo 8.1.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\ceconn32.dll
if not exist "%FILE_PATH%" echo 8.1.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\ceeval32.dll
if not exist "%FILE_PATH%" echo 8.1.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\celog32.dll
if not exist "%FILE_PATH%" echo 8.1.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\celogging32.dll
if not exist "%FILE_PATH%" echo 8.1.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cemarshal5032.dll
if not exist "%FILE_PATH%" echo 8.1.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cepepman32.dll
if not exist "%FILE_PATH%" echo 8.1.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\ceprivate32.dll
if not exist "%FILE_PATH%" echo 8.1.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cesdk32.dll
if not exist "%FILE_PATH%" echo 8.1.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cesec32.dll
if not exist "%FILE_PATH%" echo 8.1.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\ceservice32.dll
if not exist "%FILE_PATH%" echo 8.1.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\cetransport32.dll
if not exist "%FILE_PATH%" echo 8.1.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\freetype6.dll
if not exist "%FILE_PATH%" echo 8.1.1.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\libtiff.dll
if not exist "%FILE_PATH%" echo 8.1.1.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\mfc90.dll
if not exist "%FILE_PATH%" echo 8.1.1.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\mfc90u.dll
if not exist "%FILE_PATH%" echo 8.1.1.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\mfcm90.dll
if not exist "%FILE_PATH%" echo 8.1.1.19 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.19 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\mfcm90u.dll
if not exist "%FILE_PATH%" echo 8.1.1.20 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.20 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\Microsoft.VC90.ATL.manifest
if not exist "%FILE_PATH%" echo 8.1.1.21 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.21 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\Microsoft.VC90.CRT.manifest
if not exist "%FILE_PATH%" echo 8.1.1.22 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.22 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\Microsoft.VC90.MFC.manifest
if not exist "%FILE_PATH%" echo 8.1.1.23 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.23 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\Microsoft.VC90.OpenMP.manifest
if not exist "%FILE_PATH%" echo 8.1.1.24 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.24 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\msvcm90.dll
if not exist "%FILE_PATH%" echo 8.1.1.25 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.25 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\msvcp90.dll
if not exist "%FILE_PATH%" echo 8.1.1.26 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.26 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\msvcr90.dll
if not exist "%FILE_PATH%" echo 8.1.1.27 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.27 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\NextLabs.CSCInvoke.dll
if not exist "%FILE_PATH%" echo 8.1.1.28 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.28 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\NextLabsTaggingLib32.dll
if not exist "%FILE_PATH%" echo 8.1.1.29 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.29 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\nlcc_ulib32.dll
if not exist "%FILE_PATH%" echo 8.1.1.30 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.30 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\nl_sysenc_lib32.dll
if not exist "%FILE_PATH%" echo 8.1.1.31 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.31 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pafUI32.dll
if not exist "%FILE_PATH%" echo 8.1.1.32 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.32 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pa_encrypt32.dll
if not exist "%FILE_PATH%" echo 8.1.1.33 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.33 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pa_filetagging32.dll
if not exist "%FILE_PATH%" echo 8.1.1.34 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.34 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pa_pe32.dll
if not exist "%FILE_PATH%" echo 8.1.1.35 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.35 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pdflib32.dll
if not exist "%FILE_PATH%" echo 8.1.1.36 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.36 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\pgp_adapter32.dll
if not exist "%FILE_PATH%" echo 8.1.1.37 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.37 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\PoDoFoLib.dll
if not exist "%FILE_PATH%" echo 8.1.1.38 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.38 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\resattrlib32.dll
if not exist "%FILE_PATH%" echo 8.1.1.39 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.39 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\resattrmgr32.dll
if not exist "%FILE_PATH%" echo 8.1.1.40 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.40 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\TagViewMenu32.dll
if not exist "%FILE_PATH%" echo 8.1.1.41 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.41 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\tag_office2k732.dll
if not exist "%FILE_PATH%" echo 8.1.1.42 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.42 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\vcomp90.dll
if not exist "%FILE_PATH%" echo 8.1.1.43 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.43 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\zip_adapter32.dll
if not exist "%FILE_PATH%" echo 8.1.1.44 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.44 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin32\zlib1.dll
if not exist "%FILE_PATH%" echo 8.1.1.45 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.45 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_1_1_46_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\nlcc_ulib.dll
if not exist "%FILE_PATH%" echo 8.1.1.46 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.1.46 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_1_1_46_x64_Only


rem Content of C:\Program Files\NextLabs\Common\bin64
if %HOST_ARCH% == x86 goto LBL_8_1_2_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\atl90.dll
if not exist "%FILE_PATH%" echo 8.1.2.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cebrain.dll
if not exist "%FILE_PATH%" echo 8.1.2.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cecem.dll
if not exist "%FILE_PATH%" echo 8.1.2.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\ceconn.dll
if not exist "%FILE_PATH%" echo 8.1.2.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\ceeval.dll
if not exist "%FILE_PATH%" echo 8.1.2.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\celog.dll
if not exist "%FILE_PATH%" echo 8.1.2.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\celogging.dll
if not exist "%FILE_PATH%" echo 8.1.2.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cemarshal50.dll
if not exist "%FILE_PATH%" echo 8.1.2.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cepepman.dll
if not exist "%FILE_PATH%" echo 8.1.2.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\ceprivate.dll
if not exist "%FILE_PATH%" echo 8.1.2.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cesdk.dll
if not exist "%FILE_PATH%" echo 8.1.2.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cesec.dll
if not exist "%FILE_PATH%" echo 8.1.2.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\ceservice.dll
if not exist "%FILE_PATH%" echo 8.1.2.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\cetransport.dll
if not exist "%FILE_PATH%" echo 8.1.2.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\libtiff.dll
if not exist "%FILE_PATH%" echo 8.1.2.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\mfc90.dll
if not exist "%FILE_PATH%" echo 8.1.2.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\mfc90u.dll
if not exist "%FILE_PATH%" echo 8.1.2.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\mfcm90.dll
if not exist "%FILE_PATH%" echo 8.1.2.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\mfcm90u.dll
if not exist "%FILE_PATH%" echo 8.1.2.19 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.19 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\Microsoft.VC90.ATL.manifest
if not exist "%FILE_PATH%" echo 8.1.2.20 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.20 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\Microsoft.VC90.CRT.manifest
if not exist "%FILE_PATH%" echo 8.1.2.21 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.21 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\Microsoft.VC90.MFC.manifest
if not exist "%FILE_PATH%" echo 8.1.2.22 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.22 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\Microsoft.VC90.OpenMP.manifest
if not exist "%FILE_PATH%" echo 8.1.2.23 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.23 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\msvcm90.dll
if not exist "%FILE_PATH%" echo 8.1.2.24 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.24 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\msvcp90.dll
if not exist "%FILE_PATH%" echo 8.1.2.25 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.25 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\msvcr90.dll
if not exist "%FILE_PATH%" echo 8.1.2.26 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.26 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\NextLabs.CSCInvoke.dll
if not exist "%FILE_PATH%" echo 8.1.2.27 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.27 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\NextLabsTaggingLib.dll
if not exist "%FILE_PATH%" echo 8.1.2.28 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.28 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\nl_sysenc_lib.dll
if not exist "%FILE_PATH%" echo 8.1.2.29 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.29 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pafUI.dll
if not exist "%FILE_PATH%" echo 8.1.2.30 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.30 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pa_encrypt.dll
if not exist "%FILE_PATH%" echo 8.1.2.31 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.31 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pa_filetagging.dll
if not exist "%FILE_PATH%" echo 8.1.2.32 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.32 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pa_pe.dll
if not exist "%FILE_PATH%" echo 8.1.2.33 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.33 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pdflib.dll
if not exist "%FILE_PATH%" echo 8.1.2.34 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.34 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\pgp_adapter.dll
if not exist "%FILE_PATH%" echo 8.1.2.35 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.35 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\PoDoFoLib.dll
if not exist "%FILE_PATH%" echo 8.1.2.36 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.36 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\resattrlib.dll
if not exist "%FILE_PATH%" echo 8.1.2.37 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.37 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\resattrmgr.dll
if not exist "%FILE_PATH%" echo 8.1.2.38 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.38 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\TagViewMenu.dll
if not exist "%FILE_PATH%" echo 8.1.2.39 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.39 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\tag_office2k7.dll
if not exist "%FILE_PATH%" echo 8.1.2.40 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.40 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\vcomp90.dll
if not exist "%FILE_PATH%" echo 8.1.2.41 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.41 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\zip_adapter.dll
if not exist "%FILE_PATH%" echo 8.1.2.42 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.42 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Common\bin64\zlibwapi.dll
if not exist "%FILE_PATH%" echo 8.1.2.43 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.2.43 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_1_2_x64_Only

if %HOST_ARCH% == x86 goto LBL_8_1_3_1_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Common\config\encryption_adapters.conf
if not exist "%FILE_PATH%" echo 8.1.3.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.3.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_1_3_1_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Common\config\encryption_adapters32.conf
if not exist "%FILE_PATH%" echo 8.1.3.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.1.3.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Common" /s >> %LOG_FILE%
:LBL_8_1_Done


rem Content of C:\Program Files\NextLabs\Desktop Enforcer
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer
if exist "%FILE_PATH%" goto LBL_8_2_File_Exist
echo 8.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_2_Done
:LBL_8_2_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.2.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Desktop Enforcer\bin
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin
if exist "%FILE_PATH%" goto LBL_8_2_1_File_Exist
echo 8.2.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_2_1_Done
:LBL_8_2_1_File_Exist

if %HOST_ARCH% == x86 goto LBL_8_2_1_1_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\basepep.dll
if not exist "%FILE_PATH%" echo 8.2.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_1_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\basepep32.dll
if not exist "%FILE_PATH%" echo 8.2.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\BasePEPPlugin32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\BasePEPPlugin.dll
if not exist "%FILE_PATH%" echo 8.2.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ce_deny.gif
if not exist "%FILE_PATH%" echo 8.2.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ce_deny.html
if not exist "%FILE_PATH%" echo 8.2.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\diagnostic32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\diagnostic.dll
if not exist "%FILE_PATH%" echo 8.2.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\dialog.ini
if not exist "%FILE_PATH%" echo 8.2.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\edpmanager.exe
if not exist "%FILE_PATH%" echo 8.2.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\edpmdlg32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\edpmdlg.dll
if not exist "%FILE_PATH%" echo 8.2.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\edpmgrutility32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\edpmgrutility.dll
if not exist "%FILE_PATH%" echo 8.2.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\enhancement32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\enhancement.dll
if not exist "%FILE_PATH%" echo 8.2.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

rem BUG - enhancement32.dll is not needed on x64
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\enhancement32.dll
if not exist "%FILE_PATH%" echo 8.2.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_13_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\iePEP.dll
if not exist "%FILE_PATH%" echo 8.2.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_13_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\iePEP32.dll
if not exist "%FILE_PATH%" echo 8.2.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ipcproxy32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ipcproxy.dll
if not exist "%FILE_PATH%" echo 8.2.1.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ipcstub32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\ipcstub.dll
if not exist "%FILE_PATH%" echo 8.2.1.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\menu_l.ini
if not exist "%FILE_PATH%" echo 8.2.1.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\menu_r.ini
if not exist "%FILE_PATH%" echo 8.2.1.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.ATL\atl90.dll
if not exist "%FILE_PATH%" echo 8.2.1.19 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.19 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.ATL\Microsoft.VC90.ATL.manifest
if not exist "%FILE_PATH%" echo 8.2.1.20 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.20 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest
if not exist "%FILE_PATH%" echo 8.2.1.21 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.21 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.CRT\msvcm90.dll
if not exist "%FILE_PATH%" echo 8.2.1.22 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.22 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.CRT\msvcp90.dll
if not exist "%FILE_PATH%" echo 8.2.1.23 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.23 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\Microsoft.VC90.CRT\msvcr90.dll
if not exist "%FILE_PATH%" echo 8.2.1.24 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.24 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NlRegisterPlugins.exe
if not exist "%FILE_PATH%" echo 8.2.1.25 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.25 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\nlsce.exe
if not exist "%FILE_PATH%" echo 8.2.1.26 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.26 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_27_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\nlscekeeper.dll
if not exist "%FILE_PATH%" echo 8.2.1.27 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.27 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_27_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\nlscekeeper32.dll
if not exist "%FILE_PATH%" echo 8.2.1.28 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.28 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_29_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVisualLabelingPA2003.dll
if not exist "%FILE_PATH%" echo 8.2.1.29 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.29 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_29_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVisualLabelingPA200332.dll
if not exist "%FILE_PATH%" echo 8.2.1.30 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.30 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_31_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVisualLabelingPA2007.dll
if not exist "%FILE_PATH%" echo 8.2.1.31 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.31 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_31_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVisualLabelingPA200732.dll
if not exist "%FILE_PATH%" echo 8.2.1.32 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.32 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_33_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLOfficePEP.dll
if not exist "%FILE_PATH%" echo 8.2.1.33 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.33 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_33_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLOfficePEP32.dll
if not exist "%FILE_PATH%" echo 8.2.1.34 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.34 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

rem if %HOST_ARCH% == x86 goto LBL_8_2_1_34_x64_Only
rem set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVLOfficeEnforcer2007.dll
rem if not exist "%FILE_PATH%" echo 8.2.1.35 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
rem if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
rem if exist "%FILE_PATH%" echo 8.2.1.35 PASS - %FILE_PATH% exists >> %LOG_FILE%
rem if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_34_x64_Only

rem set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVLOfficeEnforcer200732.dll
rem if not exist "%FILE_PATH%" echo 8.2.1.36 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
rem if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
rem if exist "%FILE_PATH%" echo 8.2.1.36 PASS - %FILE_PATH% exists >> %LOG_FILE%
rem if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_2_1_37_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVLViewPrint.dll
if not exist "%FILE_PATH%" echo 8.2.1.37 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.37 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_2_1_37_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\NLVLViewPrint32.dll
if not exist "%FILE_PATH%" echo 8.2.1.38 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.38 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\notification32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\notification.dll
if not exist "%FILE_PATH%" echo 8.2.1.39 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.39 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\notify.ini
if not exist "%FILE_PATH%" echo 8.2.1.40 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.40 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\PCStatus32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\PCStatus.dll
if not exist "%FILE_PATH%" echo 8.2.1.41 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.41 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\status_plugin.ini
if not exist "%FILE_PATH%" echo 8.2.1.42 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.42 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Desktop Enforcer\bin\WdeAddTags.exe
if not exist "%FILE_PATH%" echo 8.2.1.43 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.2.1.43 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Desktop Enforcer" /s >> %LOG_FILE%
:LBL_8_2_Done


rem Content of C:\Program Files\NextLabs\Network Enforcer
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer
if exist "%FILE_PATH%" goto LBL_8_3_File_Exist
echo 8.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_3_Done
:LBL_8_3_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.3.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Network Enforcer\bin
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin
if exist "%FILE_PATH%" goto LBL_8_3_1_File_Exist
echo 8.3.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_3_1_Done
:LBL_8_3_1_File_Exist

if %HOST_ARCH% == x86 goto LBL_8_3_1_1_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\ftpe.dll
if not exist "%FILE_PATH%" echo 8.3.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_3_1_1_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\ftpe32.dll
if not exist "%FILE_PATH%" echo 8.3.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_3_1_3_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\hpe.dll
if not exist "%FILE_PATH%" echo 8.3.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_3_1_3_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\hpe32.dll
if not exist "%FILE_PATH%" echo 8.3.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_3_1_5_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\Httpe.dll
if not exist "%FILE_PATH%" echo 8.3.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_3_1_5_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\Httpe32.dll
if not exist "%FILE_PATH%" echo 8.3.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_3_1_7_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\InstallSPICaller.exe
if not exist "%FILE_PATH%" echo 8.3.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_3_1_7_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\InstallSPICaller32.exe
if not exist "%FILE_PATH%" echo 8.3.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\config\FTPE.ini
if not exist "%FILE_PATH%" echo 8.3.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\config\HPE.ini
if not exist "%FILE_PATH%" echo 8.3.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Network Enforcer\bin\config\HTTPE.ini
if not exist "%FILE_PATH%" echo 8.3.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.3.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Network Enforcer" /s >> %LOG_FILE%
:LBL_8_3_Done


rem Content of C:\Program Files\NextLabs\Outlook Enforcer
if %HOST_ARCH% == x64 goto LBL_8_4_x86_Only
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer
if exist "%FILE_PATH%" goto LBL_8_4_File_Exist
echo 8.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_4_Done
:LBL_8_4_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.4.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Outlook Enforcer\bin
set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin
if exist "%FILE_PATH%" goto LBL_8_4_1_File_Exist
echo 8.4.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_4_1_Done
:LBL_8_4_1_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\adaptercomm32.dll
if not exist "%FILE_PATH%" echo 8.4.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\adaptermanager32.dll
if not exist "%FILE_PATH%" echo 8.4.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\approvaladapter.ini
if not exist "%FILE_PATH%" echo 8.4.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\approvaladapter32.dll
if not exist "%FILE_PATH%" echo 8.4.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\boost_regex-vc90-mt-1_43.dll
if not exist "%FILE_PATH%" echo 8.4.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\CEOffice32.dll
if not exist "%FILE_PATH%" echo 8.4.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\CE_Explorer32.dll
if not exist "%FILE_PATH%" echo 8.4.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\InjectExp32.dll
if not exist "%FILE_PATH%" echo 8.4.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\mso2K3PEP32.dll
if not exist "%FILE_PATH%" echo 8.4.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\mso2K7PEP32.dll
if not exist "%FILE_PATH%" echo 8.4.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\NlRegisterPlugins.exe
if not exist "%FILE_PATH%" echo 8.4.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\odhd2K332.dll
if not exist "%FILE_PATH%" echo 8.4.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\odhd2K732.dll
if not exist "%FILE_PATH%" echo 8.4.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\OEService32.dll
if not exist "%FILE_PATH%" echo 8.4.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.4.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Outlook Enforcer" /s >> %LOG_FILE%
:LBL_8_4_Done
:LBL_8_4_x86_Only


rem Content of C:\Program Files\NextLabs\Policy Controller
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller
if exist "%FILE_PATH%" goto LBL_8_5_File_Exist
echo 8.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_Done
:LBL_8_5_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\app-icon.ico
if not exist "%FILE_PATH%" echo 8.5.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bundle.bin
if not exist "%FILE_PATH%" echo 8.5.0.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\clientinfo.bin
if not exist "%FILE_PATH%" echo 8.5.0.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\evalcache.data
if not exist "%FILE_PATH%" echo 8.5.0.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\evalcache.index
if not exist "%FILE_PATH%" echo 8.5.0.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.5.0.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\regexp.bin
if not exist "%FILE_PATH%" echo 8.5.0.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.0.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\bin
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin
if exist "%FILE_PATH%" goto LBL_8_5_1_File_Exist
echo 8.5.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_1_Done
:LBL_8_5_1_File_Exist

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cebrain32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cebrain.dll
if not exist "%FILE_PATH%" echo 8.5.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cecem32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cecem.dll
if not exist "%FILE_PATH%" echo 8.5.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceconn32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceconn.dll
if not exist "%FILE_PATH%" echo 8.5.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceeval32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceeval.dll
if not exist "%FILE_PATH%" echo 8.5.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceinjection32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceinjection.dll
if not exist "%FILE_PATH%" echo 8.5.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cekif32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cekif.dll
if not exist "%FILE_PATH%" echo 8.5.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\celog32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\celog.dll
if not exist "%FILE_PATH%" echo 8.5.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cemarshal5032.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cemarshal50.dll
if not exist "%FILE_PATH%" echo 8.5.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpconn32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpconn.dll
if not exist "%FILE_PATH%" echo 8.5.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpeval32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpeval.dll
if not exist "%FILE_PATH%" echo 8.5.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpgeneric32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpgeneric.dll
if not exist "%FILE_PATH%" echo 8.5.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdplog32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdplog.dll
if not exist "%FILE_PATH%" echo 8.5.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpman.exe
if not exist "%FILE_PATH%" echo 8.5.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpprivate32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpprivate.dll
if not exist "%FILE_PATH%" echo 8.5.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpprotect32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpprotect.dll
if not exist "%FILE_PATH%" echo 8.5.1.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpsec32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepdpsec.dll
if not exist "%FILE_PATH%" echo 8.5.1.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepepman32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cepepman.dll
if not exist "%FILE_PATH%" echo 8.5.1.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceprivate32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceprivate.dll
if not exist "%FILE_PATH%" echo 8.5.1.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cesdk32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cesdk.dll
if not exist "%FILE_PATH%" echo 8.5.1.19 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.19 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceTamperproof32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\ceTamperproof.dll
if not exist "%FILE_PATH%" echo 8.5.1.20 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.20 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cetransctrl32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cetransctrl.dll
if not exist "%FILE_PATH%" echo 8.5.1.21 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.21 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cetransport32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\cetransport.dll
if not exist "%FILE_PATH%" echo 8.5.1.22 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.22 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Decrypt.exe
if not exist "%FILE_PATH%" echo 8.5.1.23 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.23 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\IPCJNI32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\IPCJNI.dll
if not exist "%FILE_PATH%" echo 8.5.1.24 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.24 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\IPCStub32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\IPCStub.dll
if not exist "%FILE_PATH%" echo 8.5.1.25 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.25 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_5_1_26_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\mch30_setup.dll
if not exist "%FILE_PATH%" echo 8.5.1.26 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.26 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_5_1_26_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\mch30_setup32.dll
if not exist "%FILE_PATH%" echo 8.5.1.27 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.27 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\mch_install_test.exe
if not exist "%FILE_PATH%" echo 8.5.1.28 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.28 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_client32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_client.dll
if not exist "%FILE_PATH%" echo 8.5.1.29 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.29 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_framework32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_framework.dll
if not exist "%FILE_PATH%" echo 8.5.1.30 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.30 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_plugin32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_plugin.dll
if not exist "%FILE_PATH%" echo 8.5.1.31 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.31 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlca_service.exe
if not exist "%FILE_PATH%" echo 8.5.1.32 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.32 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlTamperproofConfig32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nlTamperproofConfig.dll
if not exist "%FILE_PATH%" echo 8.5.1.33 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.33 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nl_tamper_plugin32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\nl_tamper_plugin.dll
if not exist "%FILE_PATH%" echo 8.5.1.34 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.34 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\pdpjni32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\pdpjni.dll
if not exist "%FILE_PATH%" echo 8.5.1.35 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.35 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\PDPStop32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\PDPStop.dll
if not exist "%FILE_PATH%" echo 8.5.1.36 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.36 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\StopEnforcer.exe
if not exist "%FILE_PATH%" echo 8.5.1.37 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.37 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.ATL\atl90.dll
if not exist "%FILE_PATH%" echo 8.5.1.38 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.38 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.ATL\Microsoft.VC90.ATL.manifest
if not exist "%FILE_PATH%" echo 8.5.1.39 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.39 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest
if not exist "%FILE_PATH%" echo 8.5.1.40 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.40 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.CRT\msvcm90.dll
if not exist "%FILE_PATH%" echo 8.5.1.41 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.41 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.CRT\msvcp90.dll
if not exist "%FILE_PATH%" echo 8.5.1.42 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.42 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.CRT\msvcr90.dll
if not exist "%FILE_PATH%" echo 8.5.1.43 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.43 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.MFC\mfc90.dll
if not exist "%FILE_PATH%" echo 8.5.1.44 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.44 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.MFC\mfc90u.dll
if not exist "%FILE_PATH%" echo 8.5.1.45 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.45 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.MFC\mfcm90.dll
if not exist "%FILE_PATH%" echo 8.5.1.46 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.46 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.MFC\mfcm90u.dll
if not exist "%FILE_PATH%" echo 8.5.1.47 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.47 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.MFC\Microsoft.VC90.MFC.manifest
if not exist "%FILE_PATH%" echo 8.5.1.48 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.48 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.OPENMP\Microsoft.VC90.OpenMP.manifest
if not exist "%FILE_PATH%" echo 8.5.1.49 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.49 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\bin\Microsoft.VC90.OPENMP\vcomp90.dll
if not exist "%FILE_PATH%" echo 8.5.1.50 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.1.50 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem BUG - devcon.exe is a MS program that should not be in the installer at all, it is not used


rem Content of C:\Program Files\NextLabs\Policy Controller\driver
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver
if exist "%FILE_PATH%" goto LBL_8_5_2_File_Exist
echo 8.5.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_2_Done
:LBL_8_5_2_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nlcc.inf
if not exist "%FILE_PATH%" echo 8.5.2.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nlcc.x86.cat
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nlcc.x64.cat
if not exist "%FILE_PATH%" echo 8.5.2.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nldevcon.exe
if not exist "%FILE_PATH%" echo 8.5.2.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\x86\WdfCoInstaller01009.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\amd64\WdfCoInstaller01009.dll
if not exist "%FILE_PATH%" echo 8.5.2.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nl_tamper\nl_tamper.x86.cat
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nl_tamper\nl_tamper.x64.cat
if not exist "%FILE_PATH%" echo 8.5.2.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\driver\nl_tamper\nl_tamper.inf
if not exist "%FILE_PATH%" echo 8.5.2.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.2.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\help
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help
if exist "%FILE_PATH%" goto LBL_8_5_3_File_Exist
echo 8.5.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_3_Done
:LBL_8_5_3_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\Index.html
if not exist "%FILE_PATH%" echo 8.5.3.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\Information.html
if not exist "%FILE_PATH%" echo 8.5.3.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\Notifications.html
if not exist "%FILE_PATH%" echo 8.5.3.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\images\GreyInfo.gif
if not exist "%FILE_PATH%" echo 8.5.3.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\images\GreyNotifs.gif
if not exist "%FILE_PATH%" echo 8.5.3.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\help\images\GreyWhatsThis.gif
if not exist "%FILE_PATH%" echo 8.5.3.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.3.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\jlib
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib
if exist "%FILE_PATH%" goto LBL_8_5_4_File_Exist
echo 8.5.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_4_Done
:LBL_8_5_4_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\activation.jar
if not exist "%FILE_PATH%" echo 8.5.4.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\agent-common.jar
if not exist "%FILE_PATH%" echo 8.5.4.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\agent-controlmanager.jar
if not exist "%FILE_PATH%" echo 8.5.4.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\agent-ipc.jar
if not exist "%FILE_PATH%" echo 8.5.4.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\agent-tools.jar
if not exist "%FILE_PATH%" echo 8.5.4.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\agent-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\antlr.jar
if not exist "%FILE_PATH%" echo 8.5.4.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\axis.jar
if not exist "%FILE_PATH%" echo 8.5.4.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\castor-0.9.5.4.jar
if not exist "%FILE_PATH%" echo 8.5.4.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\client-pf.jar
if not exist "%FILE_PATH%" echo 8.5.4.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-domain-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-domain.jar
if not exist "%FILE_PATH%" echo 8.5.4.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-framework-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-framework.jar
if not exist "%FILE_PATH%" echo 8.5.4.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-oil.jar
if not exist "%FILE_PATH%" echo 8.5.4.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-pf.jar
if not exist "%FILE_PATH%" echo 8.5.4.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-version-impl.jar
if not exist "%FILE_PATH%" echo 8.5.4.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\common-version.jar
if not exist "%FILE_PATH%" echo 8.5.4.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\commons-cli.jar
if not exist "%FILE_PATH%" echo 8.5.4.19 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.19 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\commons-collections-2.1.1.jar
if not exist "%FILE_PATH%" echo 8.5.4.20 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.20 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\commons-discovery-0.2.jar
if not exist "%FILE_PATH%" echo 8.5.4.21 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.21 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\commons-logging.jar
if not exist "%FILE_PATH%" echo 8.5.4.22 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.22 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\crypt.jar
if not exist "%FILE_PATH%" echo 8.5.4.23 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.23 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\dabs-agent-services.jar
if not exist "%FILE_PATH%" echo 8.5.4.24 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.24 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\dabs-common-services.jar
if not exist "%FILE_PATH%" echo 8.5.4.25 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.25 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\deployment-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.26 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.26 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\dnsjava.jar
if not exist "%FILE_PATH%" echo 8.5.4.27 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.27 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\ehcache-1.1.jar
if not exist "%FILE_PATH%" echo 8.5.4.28 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.28 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\jargs.jar
if not exist "%FILE_PATH%" echo 8.5.4.29 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.29 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\jaxrpc.jar
if not exist "%FILE_PATH%" echo 8.5.4.30 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.30 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\log-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.31 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.31 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\mail.jar
if not exist "%FILE_PATH%" echo 8.5.4.32 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.32 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\management-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.33 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.33 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\policy-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.34 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.34 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\saaj.jar
if not exist "%FILE_PATH%" echo 8.5.4.35 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.35 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\server-base.jar
if not exist "%FILE_PATH%" echo 8.5.4.36 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.36 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\server-shared-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.37 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.37 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\version-types.jar
if not exist "%FILE_PATH%" echo 8.5.4.38 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.38 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\version.jar
if not exist "%FILE_PATH%" echo 8.5.4.39 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.39 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\wsdl4j-1.5.1.jar
if not exist "%FILE_PATH%" echo 8.5.4.40 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.40 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jlib\xercesImpl.jar
if not exist "%FILE_PATH%" echo 8.5.4.41 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.4.41 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem There are too many JRE files, we just check if java.exe exists for now
rem Content of C:\Program Files\NextLabs\Policy Controller\jre
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jre
if exist "%FILE_PATH%" goto LBL_8_5_5_File_Exist
echo 8.5.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_5_Done
:LBL_8_5_5_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jre\bin\java.exe
if not exist "%FILE_PATH%" echo 8.5.5.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.5.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jre\bin\server\jvm.dll
if not exist "%FILE_PATH%" echo 8.5.5.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.5.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\jservice
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice
if exist "%FILE_PATH%" goto LBL_8_5_6_File_Exist
echo 8.5.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_6_Done
:LBL_8_5_6_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\config\KeyManagementService.properties
if not exist "%FILE_PATH%" echo 8.5.6.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\config\NLCCService.properties
if not exist "%FILE_PATH%" echo 8.5.6.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\config\SystemEncryptionService.properties
if not exist "%FILE_PATH%" echo 8.5.6.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\jar\nlcc\NLCCService.jar
if not exist "%FILE_PATH%" echo 8.5.6.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\jar\nlcc\nlcc_dispatcher32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\jar\nlcc\nlcc_dispatcher.dll
if not exist "%FILE_PATH%" echo 8.5.6.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_5_6_6_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\cesdk.dll
if not exist "%FILE_PATH%" echo 8.5.6.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_5_6_6_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\cesdk32.dll
if not exist "%FILE_PATH%" echo 8.5.6.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_5_6_8_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\ceservice.dll
if not exist "%FILE_PATH%" echo 8.5.6.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_5_6_8_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\ceservice32.dll
if not exist "%FILE_PATH%" echo 8.5.6.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 goto LBL_8_5_6_10_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\KeyManagementConsumer.dll
if not exist "%FILE_PATH%" echo 8.5.6.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_5_6_10_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\KeyManagementConsumer32.dll
if not exist "%FILE_PATH%" echo 8.5.6.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\bin\KeyUtil.exe
if not exist "%FILE_PATH%" echo 8.5.6.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\KeyManagement\jar\KeyManagementService.jar
if not exist "%FILE_PATH%" echo 8.5.6.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\SystemEncryption\pcs_server32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\SystemEncryption\pcs_server.dll
if not exist "%FILE_PATH%" echo 8.5.6.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\jservice\SystemEncryption\SystemEncryptionService.jar
if not exist "%FILE_PATH%" echo 8.5.6.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.6.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\license
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\license
if exist "%FILE_PATH%" goto LBL_8_5_7_File_Exist
echo 8.5.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_7_Done
:LBL_8_5_7_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\license\license.cfg
if not exist "%FILE_PATH%" echo 8.5.7.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.7.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Policy Controller\service
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service
if exist "%FILE_PATH%" goto LBL_8_5_8_File_Exist
echo 8.5.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_5_8_Done
:LBL_8_5_8_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\ftpte.exe.ini
if not exist "%FILE_PATH%" echo 8.5.8.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\procexp.exe.ini
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\procexp64.exe.ini
if not exist "%FILE_PATH%" echo 8.5.8.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\smartftp.exe.ini
if not exist "%FILE_PATH%" echo 8.5.8.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\taskkill.exe.ini
if not exist "%FILE_PATH%" echo 8.5.8.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\service\injection\taskmgr.exe.ini
if not exist "%FILE_PATH%" echo 8.5.8.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.5.8.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Policy Controller" /s >> %LOG_FILE%
:LBL_8_5_Done


rem Content of C:\Program Files\NextLabs\System Encryption
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption
if exist "%FILE_PATH%" goto LBL_8_6_File_Exist
echo 8.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_6_Done
:LBL_8_6_File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.6.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\System Encryption\bin
set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin
if exist "%FILE_PATH%" goto LBL_8_6_1_File_Exist
echo 8.6.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_6_1_Done
:LBL_8_6_1_File_Exist

if %HOST_ARCH% == x86 goto LBL_8_6_1_1_x64_Only
set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\IconBadging.dll
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1
:LBL_8_6_1_1_x64_Only

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\IconBadging32.dll
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlsefw_plugin32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlsefw_plugin.dll
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlse_plugin32.dll
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlse_plugin.dll
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlSysEncryption.exe
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nlSysEncryptionObligation.exe
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\nl_autounwrapper.exe
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\NLSE.inf
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\NLSEFW.inf
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\objfre_wxp_x86\i386\nl_SysEncryption.sys
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\objfre_win7_amd64\amd64\nl_SysEncryption.sys
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\objfre_wxp_x86\i386\nl_SysEncryptionFW.sys
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\bin\driver\objfre_win7_amd64\amd64\nl_SysEncryptionFW.sys
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\System Encryption\config\SystemEncryption.cfg
if not exist "%FILE_PATH%" echo 8.6.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.6.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\System Encryption" /s >> %LOG_FILE%
:LBL_8_6_Done


rem C:\Program Files (x86) is for x64 only
if %OEbitness% == x64 goto LBL_8_x64_Only
if %HOST_ARCH% == x86 goto LBL_8_x64_Only

rem Content of C:\Program Files (x86)\NextLabs\Outlook Enforcer
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer
if exist "%FILE_PATH%" goto LBL_8_7_File_Exist
echo 8.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_7_Done
:LBL_8_7_File_Exist

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.7.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files (x86)\NextLabs\Outlook Enforcer\bin
set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin
if exist "%FILE_PATH%" goto LBL_8_7_1_File_Exist
echo 8.7.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_7_1_Done
:LBL_8_7_1_File_Exist

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\adaptercomm32.dll
if not exist "%FILE_PATH%" echo 8.7.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\adaptermanager32.dll
if not exist "%FILE_PATH%" echo 8.7.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\approvaladapter.ini
if not exist "%FILE_PATH%" echo 8.7.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\approvaladapter32.dll
if not exist "%FILE_PATH%" echo 8.7.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\boost_regex-vc90-mt-1_43.dll
if not exist "%FILE_PATH%" echo 8.7.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\CEOffice32.dll
if not exist "%FILE_PATH%" echo 8.7.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\CE_Explorer.dll
if not exist "%FILE_PATH%" echo 8.7.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\CE_Explorer32.dll
if not exist "%FILE_PATH%" echo 8.7.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\InjectExp.dll
if not exist "%FILE_PATH%" echo 8.7.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\InjectExp32.dll
if not exist "%FILE_PATH%" echo 8.7.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\InstallOEX64.exe
if not exist "%FILE_PATH%" echo 8.7.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\mso2K3PEP32.dll
if not exist "%FILE_PATH%" echo 8.7.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\mso2K7PEP32.dll
if not exist "%FILE_PATH%" echo 8.7.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\NlRegisterPlugins.exe
if not exist "%FILE_PATH%" echo 8.7.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\odhd2K332.dll
if not exist "%FILE_PATH%" echo 8.7.1.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\odhd2K732.dll
if not exist "%FILE_PATH%" echo 8.7.1.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\OEService.dll
if not exist "%FILE_PATH%" echo 8.7.1.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Outlook Enforcer\bin\OEService32.dll
if not exist "%FILE_PATH%" echo 8.7.1.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles(x86)%\NextLabs\Outlook Enforcer" /s >> %LOG_FILE%
:LBL_8_7_Done


rem Content of C:\Program Files (x86)\NextLabs\Policy Controller
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Policy Controller
if exist "%FILE_PATH%" goto LBL_8_8_File_Exist
echo 8.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_8_Done
:LBL_8_8_File_Exist


rem Content of C:\Program Files (x86)\NextLabs\Policy Controller\config
set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Policy Controller\config
if exist "%FILE_PATH%" goto LBL_8_8_1_File_Exist
echo 8.8.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
rem goto LBL_8_8_1_Done
:LBL_8_8_1_File_Exist

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Policy Controller\config\plugin\nl_OE_plugin32.cfg
if not exist "%FILE_PATH%" echo 8.8.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles(x86)%\NextLabs\Policy Controller\config\tamper_resistance\OutlookEnforcer_TamperResistance.cfg
if not exist "%FILE_PATH%" echo 8.8.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

goto LBL_8_8_x86_Done

:LBL_8_x64_Only



rem Content of C:\Program Files\NextLabs\Outlook Enforcer
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer
if exist "%FILE_PATH%" goto LBL_8_7_1File_Exist
echo 8.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_7_1Done
:LBL_8_7_1File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.7.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Outlook Enforcer\bin
set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin
if exist "%FILE_PATH%" goto LBL_8_7_1_File_Exist
echo 8.7.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_7_1_1Done
:LBL_8_7_1_1File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\adaptercomm32.dll
if not exist "%FILE_PATH%" echo 8.7.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\adaptermanager32.dll
if not exist "%FILE_PATH%" echo 8.7.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\approvaladapter.ini
if not exist "%FILE_PATH%" echo 8.7.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\approvaladapter32.dll
if not exist "%FILE_PATH%" echo 8.7.1.4 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\boost_regex-vc90-mt-1_43.dll
if not exist "%FILE_PATH%" echo 8.7.1.5 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\CEOffice32.dll
if not exist "%FILE_PATH%" echo 8.7.1.6 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\CE_Explorer.dll
if not exist "%FILE_PATH%" echo 8.7.1.7 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\CE_Explorer32.dll
if not exist "%FILE_PATH%" echo 8.7.1.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.8 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\InjectExp.dll
if not exist "%FILE_PATH%" echo 8.7.1.9 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.9 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\InjectExp32.dll
if not exist "%FILE_PATH%" echo 8.7.1.10 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.10 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\InstallOEX64.exe
if not exist "%FILE_PATH%" echo 8.7.1.11 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.11 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\mso2K3PEP32.dll
if not exist "%FILE_PATH%" echo 8.7.1.12 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.12 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\mso2K7PEP32.dll
if not exist "%FILE_PATH%" echo 8.7.1.13 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.13 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\NlRegisterPlugins.exe
if not exist "%FILE_PATH%" echo 8.7.1.14 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.14 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\odhd2K332.dll
if not exist "%FILE_PATH%" echo 8.7.1.15 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.15 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\odhd2K732.dll
if not exist "%FILE_PATH%" echo 8.7.1.16 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.16 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\OEService.dll
if not exist "%FILE_PATH%" echo 8.7.1.17 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.17 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Outlook Enforcer\bin\OEService32.dll
if not exist "%FILE_PATH%" echo 8.7.1.18 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.7.1.18 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles%\NextLabs\Outlook Enforcer" /s >> %LOG_FILE%
:LBL_8_7_1Done


rem Content of C:\Program Files\NextLabs\Policy Controller
echo. >> %LOG_FILE%
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller
if exist "%FILE_PATH%" goto LBL_8_8_1File_Exist
echo 8.8 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_8_1Done
:LBL_8_8_1File_Exist


rem Content of C:\Program Files\NextLabs\Policy Controller\config
set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config
if exist "%FILE_PATH%" goto LBL_8_8_1_1File_Exist
echo 8.8.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
rem goto LBL_8_8_1_1Done
:LBL_8_8_1_1File_Exist

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\plugin\nl_OE_plugin.cfg
if not exist "%FILE_PATH%" echo 8.8.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\OutlookEnforcer_TamperResistance.cfg
if not exist "%FILE_PATH%" echo 8.8.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


:LBL_8_8_x86_Done


if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\plugin\nlse_plugin32.cfg 
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\plugin\nlse_plugin.cfg
if not exist "%FILE_PATH%" echo 8.8.1.3 FAIL - %FILE_PATH% does not exist (no access to config directory when Policy Controller is running) >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\plugin\nlsefw_plugin32.cfg 
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\plugin\nlsefw_plugin.cfg
if not exist "%FILE_PATH%" echo 8.8.1.4 FAIL - %FILE_PATH% does not exist (no access to config directory when Policy Controller is running) >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.4 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\LiveMeetingEnforcer_TamperResistance32.cfg 
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\LiveMeetingEnforcer_TamperResistance.cfg
if not exist "%FILE_PATH%" echo 8.8.1.5 FAIL - %FILE_PATH% does not exist (no access to config directory when Policy Controller is running) >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.5 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\OfficeCommunicatorEnforcer_TamperResistance32.cfg 
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\OfficeCommunicatorEnforcer_TamperResistance.cfg
if not exist "%FILE_PATH%" echo 8.8.1.6 FAIL - %FILE_PATH% does not exist (no access to config directory when Policy Controller is running) >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.6 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

if %HOST_ARCH% == x86 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\RemovableDeviceEnforcer_TamperResistance32.cfg
if %HOST_ARCH% == x64 set FILE_PATH=%ProgramFiles%\NextLabs\Policy Controller\config\tamper_resistance\RemovableDeviceEnforcer_TamperResistance.cfg

if not exist "%FILE_PATH%" echo 8.8.1.7 FAIL - %FILE_PATH% does not exist (no access to config directory when Policy Controller is running) >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.8.1.7 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%ProgramFiles(x86)%\NextLabs\Policy Controller" /s >> %LOG_FILE%
:LBL_8_8_Done



rem Content of C:\Program Files\NextLabs\Live Meeting Enforcer
echo. >> %LOG_FILE%
rem if %HOST_ARCH% == x86 
set PROD_PATH=%ProgramFiles%\NextLabs\Live Meeting Enforcer
rem if %HOST_ARCH% == x64 set PROD_PATH=%ProgramFiles(x86)%\NextLabs\Live Meeting Enforcer
if exist "%PROD_PATH%" goto LBL_8_9_File_Exist
echo 8.9 FAIL - %PROD_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_9_Done
:LBL_8_9_File_Exist

set FILE_PATH=%PROD_PATH%\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.9.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.9.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Live Meeting Enforcer\bin
set FILE_PATH=%PROD_PATH%\bin
if exist "%FILE_PATH%" goto LBL_8_9_1_File_Exist
echo 8.9.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_9_1_Done
:LBL_8_9_1_File_Exist

set FILE_PATH=%PROD_PATH%\bin\nllme32.dll
if not exist "%FILE_PATH%" echo 8.9.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.9.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%PROD_PATH%" /s >> %LOG_FILE%
:LBL_8_9_Done


rem Content of C:\Program Files\NextLabs\Office Communicator Enforcer
echo. >> %LOG_FILE%
rem if %HOST_ARCH% == x86 
set PROD_PATH=%ProgramFiles%\NextLabs\Office Communicator Enforcer
rem if %HOST_ARCH% == x64 set PROD_PATH=%ProgramFiles(x86)%\NextLabs\Office Communicator Enforcer
if exist "%PROD_PATH%" goto LBL_8_10_File_Exist
echo 8.10 FAIL - %PROD_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_10_Done
:LBL_8_10_File_Exist

set FILE_PATH=%PROD_PATH%\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.10.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.10.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Office Communicator Enforcer\bin
set FILE_PATH=%PROD_PATH%\bin
if exist "%FILE_PATH%" goto LBL_8_10_1_File_Exist
echo 8.10.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_10_1_Done
:LBL_8_10_1_File_Exist

set FILE_PATH=%PROD_PATH%\bin\ipcproxy.dll
if not exist "%FILE_PATH%" echo 8.10.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.10.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%PROD_PATH%\bin\uccpPEP32.dll
if not exist "%FILE_PATH%" echo 8.10.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.10.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%PROD_PATH%" /s >> %LOG_FILE%
:LBL_8_10_Done


rem Content of C:\Program Files\NextLabs\Removable Device Enforcer
echo. >> %LOG_FILE%
set PROD_PATH=%ProgramFiles%\NextLabs\Removable Device Enforcer
if exist "%PROD_PATH%" goto LBL_8_11_File_Exist
echo 8.11 FAIL - %PROD_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_11_Done
:LBL_8_11_File_Exist

set FILE_PATH=%PROD_PATH%\ReadMe.txt
if not exist "%FILE_PATH%" echo 8.11.0.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.11.0.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1


rem Content of C:\Program Files\NextLabs\Removable Device Enforcer\bin
set FILE_PATH=%PROD_PATH%\bin
if exist "%FILE_PATH%" goto LBL_8_11_1_File_Exist
echo 8.11.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_8_11_1_Done
:LBL_8_11_1_File_Exist

set FILE_PATH=%PROD_PATH%\bin\NextlabsCredentialProvider.dll
if not exist "%FILE_PATH%" echo 8.11.1.1 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.11.1.1 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%PROD_PATH%\bin\logon_detection_win7.exe
if not exist "%FILE_PATH%" echo 8.11.1.2 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.11.1.2 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

set FILE_PATH=%PROD_PATH%\bin\nl_devenf_plugin.dll
if %HOST_ARCH% == x86 set FILE_PATH=%PROD_PATH%\bin\nl_devenf_plugin32.dll
if not exist "%FILE_PATH%" echo 8.11.1.3 FAIL - %FILE_PATH% does not exist >> %LOG_FILE%
if not exist "%FILE_PATH%" set /a FAIL_COUNT += 1
if exist "%FILE_PATH%" echo 8.11.1.3 PASS - %FILE_PATH% exists >> %LOG_FILE%
if exist "%FILE_PATH%" set /a PASS_COUNT += 1

echo. >> %LOG_FILE%
dir "%PROD_PATH%" /s >> %LOG_FILE%
:LBL_8_11_Done


:LBL_Section9
echo Section 9: Check Add/Remove Programs (or Uninstall or change a program)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 9: Check Add/Remove Programs (or Uninstall or change a program) >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT1=NlInstallCheck09_1.tmp
set TMP_OUT2=NlInstallCheck09_2.tmp

wmic PRODUCT GET > %TMP_OUT1%
type %TMP_OUT1% > %TMP_OUT2%

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Policy Controller
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_1_Product_Exist
echo 9.1 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_1_Done
:LBL_9_1_Product_Exist 
echo 9.1 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_1_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Network Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_2_Product_Exist
echo 9.2 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_2_Done
:LBL_9_2_Product_Exist 
echo 9.2 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_2_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=Nextlabs Key Management Client
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_3_Product_Exist
echo 9.3 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_3_Done
:LBL_9_3_Product_Exist 
echo 9.3 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_3_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Rights Management Client
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_4_Product_Exist
echo 9.4 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_4_Done
:LBL_9_4_Product_Exist 
echo 9.4 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_4_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Outlook Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_5_Product_Exist
echo 9.5 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_5_Done
:LBL_9_5_Product_Exist 
echo 9.5 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_5_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Desktop Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_6_Product_Exist
echo 9.6 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_6_Done
:LBL_9_6_Product_Exist 
echo 9.6 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_6_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Live Meeting Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_7_Product_Exist
echo 9.7 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_7_Done
:LBL_9_7_Product_Exist 
echo 9.7 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_7_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Office Communicator Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_8_Product_Exist
echo 9.8 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_8_Done
:LBL_9_8_Product_Exist 
echo 9.8 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_8_Done

echo. >> %LOG_FILE%
set PRODUCT_NAME=NextLabs Removable Device Enforcer
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_9_9_Product_Exist
echo 9.9 FAIL - "%PRODUCT_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_9_9_Done
:LBL_9_9_Product_Exist 
echo 9.9 PASS - "%PRODUCT_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%PRODUCT_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_9_9_Done


if exist %TMP_OUT1% del /F %TMP_OUT1%
if exist %TMP_OUT2% del /F %TMP_OUT2%


:LBL_Section10
echo Section 10: Check Windows services
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 10: Check Windows services >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT1=NlInstallCheck10_1.tmp
set TMP_OUT2=NlInstallCheck10_2.tmp

wmic SERVICE GET > %TMP_OUT1%
type %TMP_OUT1% > %TMP_OUT2%

echo. >> %LOG_FILE%
set SERVICE_NAME=ComplianceEnforcerService
findstr /I /C:"%SERVICE_NAME%" %TMP_OUT2% > nul
if %ERRORLEVEL% == 0 goto LBL_10_1_Service_Exist
echo 10.1 FAIL - "%SERVICE_NAME%" is not listed >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_10_1_Done
:LBL_10_1_Service_Exist 
echo 10.1 PASS - "%SERVICE_NAME%" is listed >> %LOG_FILE%
findstr /I /C:"%SERVICE_NAME%" %TMP_OUT2% >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_10_1_Done

if exist %TMP_OUT1% del /F %TMP_OUT1%
if exist %TMP_OUT2% del /F %TMP_OUT2%


:LBL_Section11
echo Section 11: Check plug-ins
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 11: Check plug-ins >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck11.tmp

rem iePEP
echo. >> %LOG_FILE%
set REGKEY=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_1_Plugin_Exist
echo 11.1 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.1 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_1_Done
:LBL_11_1_Plugin_Exist
echo 11.1 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_1_Done

if %HOST_ARCH% == x86 goto LBL_11_2_Done
set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\explorer\Browser Helper Objects\{FB159F40-0C40-4480-9A72-71C1D07606B7}
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_2_Plugin_Exist
echo 11.2 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.2 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_2_Done
:LBL_11_2_Plugin_Exist
echo 11.2 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_2_Done

rem enchancement
set REGKEY=HKLM\SOFTWARE\Classes\AppID\{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_3_Plugin_Exist
echo 11.3 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.3 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_3_Done
:LBL_11_3_Plugin_Exist
echo 11.3 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_3_Done

rem NLVLOfficeEnforcer
if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\NLOfficePEP.1
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\NLOfficePEP.1
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_4_Plugin_Exist
echo 11.4 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.4 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_4_Done
:LBL_11_4_Plugin_Exist
echo 11.4 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_4_Done

if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\NLOfficePEP.1
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\NLOfficePEP.1
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_5_Plugin_Exist
echo 11.5 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.5 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_5_Done
:LBL_11_5_Plugin_Exist
echo 11.5 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_5_Done

if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\NLOfficePEP.1
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\PowerPoint\Addins\NLOfficePEP.1
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_6_Plugin_Exist
echo 11.6 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.6 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_6_Done
:LBL_11_6_Plugin_Exist
echo 11.6 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_6_Done

rem OE
if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Outlook\Addins\msoPEP.msoObj.1
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_7_Plugin_Exist
echo 11.7 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.7 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_7_Done
:LBL_11_7_Plugin_Exist
echo 11.7 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_7_Done

rem CE_Explorer
set REGKEY=HKCR\*\shellex\ContextMenuHandlers\CE_Explorer
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_8_Plugin_Exist
echo 11.8 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.8 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_8_Done
:LBL_11_8_Plugin_Exist
echo 11.8 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_8_Done

rem CEOffice
if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Excel\Addins\CEOffice.Office
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Excel\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_9_Plugin_Exist
echo 11.9 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.9 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_9_Done
:LBL_11_9_Plugin_Exist
echo 11.9 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_9_Done

if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\PowerPoint\Addins\CEOffice.Office
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\PowerPoint\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_10_Plugin_Exist
echo 11.10 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.10 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_10_Done
:LBL_11_10_Plugin_Exist
echo 11.10 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_10_Done

if %HOST_ARCH% == x86 set REGKEY=HKLM\SOFTWARE\Microsoft\Office\Word\Addins\CEOffice.Office
if %HOST_ARCH% == x64 set REGKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\Office\Word\Addins\CEOffice.Office
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_11_11_Plugin_Exist
echo 11.11 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 11.11 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_11_11_Done
:LBL_11_11_Plugin_Exist
echo 11.11 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_11_11_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section12
echo Section 12: Check file extension
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 12: Check file extension >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlInstallCheck12.tmp

echo. >> %LOG_FILE%
set REGKEY=HKCR\.nxl
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_12_1_Mapping_Exist
echo 12.1 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 12.1 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_12_1_Done
:LBL_12_1_Mapping_Exist
echo 12.1 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_12_1_Done

set REGKEY=HKCR\nxlfile
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_12_2_Mapping_Exist
echo 12.2 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 12.2 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_12_2_Done
:LBL_12_2_Mapping_Exist
echo 12.2 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_12_2_Done

set REGKEY=HKLM\SOFTWARE\Classes\.nxl
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_12_3_Mapping_Exist
echo 12.3 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 12.3 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_12_3_Done
:LBL_12_3_Mapping_Exist
echo 12.3 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_12_3_Done

set REGKEY=HKLM\SOFTWARE\Classes\nxlfile
reg query "%REGKEY%" > %TMP_OUT%
if %ERRORLEVEL% == 0 goto LBL_12_4_Mapping_Exist
echo 12.4 INFO: Explain error. The specified registry key is "%REGKEY%"
echo 12.4 FAIL - Registry key "%REGKEY%" is missing >> %LOG_FILE%
set /a FAIL_COUNT += 1
goto LBL_12_4_Done
:LBL_12_4_Mapping_Exist
echo 12.4 PASS - Registry key "%REGKEY%" exists >> %LOG_FILE%
set /a PASS_COUNT += 1
type %TMP_OUT% >> %LOG_FILE%
:LBL_12_4_Done

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section13
echo Section 13: List folder permission
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 13: List folder permission >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%ProgramFiles%\NextLabs
echo 13.1 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
icacls "%DIR_PATH%" >> %LOG_FILE%

if "%ProgramFiles(x86)%" == "" goto LBL_13_2_x64_Only
set DIR_PATH=%ProgramFiles(x86)%\NextLabs
echo 13.2 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
icacls "%DIR_PATH%" >> %LOG_FILE%
:LBL_13_2_x64_Only

set DIR_PATH=%windir%\System32\drivers
echo 13.3 INFO - Permissions on directory "%DIR_PATH%" >> %LOG_FILE%
icacls "%DIR_PATH%" >> %LOG_FILE%


:LBL_Section14
echo Section 14: Check MSI log
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 14: Check MSI log >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%windir%\Temp
echo 14.1 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_14_1_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_14_1_Done
:LBL_14_1_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_14_1_Done

set DIR_PATH=%SystemDrive%\Temp
echo 14.2 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_14_2_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_14_2_Done
:LBL_14_2_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_14_2_Done

set DIR_PATH=%TEMP%
echo 14.3 INFO - Checking directory %DIR_PATH% >> %LOG_FILE%
if exist bin\tail.exe goto LBL_14_3_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D >> %LOG_FILE%
goto LBL_14_3_Done
:LBL_14_3_Has_Tail
dir "%DIR_PATH%\MSI*.LOG" /Q /O:D | tail -10 >> %LOG_FILE%
:LBL_14_3_Done

rem DEBUG: Uncomment the following line to skip to end
rem goto LBL_SectionDone


:LBL_Section15
echo Section 15: Check temp folder
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 15: Check temp folder >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
echo 15.0 INFO: Checking temp folder - %TEMP% >> %LOG_FILE%
set FILE_PATH=%TEMP%\installercommon.dll
if exist "%FILE_PATH%" goto LBL_15_1_Found
echo 15.1 PASS - "%FILE_PATH%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_15_1_Done
:LBL_15_1_Found
echo 15.1 WARN - "%FILE_PATH%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_15_1_Done

set FILE_PATH=%TEMP%\PDPStop.dll
if exist "%FILE_PATH%" goto LBL_15_2_Found
echo 15.2 PASS - "%FILE_PATH%" does not exist >> %LOG_FILE%
set /a PASS_COUNT += 1
goto LBL_15_2_Done
:LBL_15_2_Found
echo 15.2 WARN - "%FILE_PATH%" exists >> %LOG_FILE%
set /a WARN_COUNT += 1
:LBL_15_2_Done


:LBL_Section16
echo Section 16: List GPO registry entries (must be reviewed manually)
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 16: List GPO registry entries >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
echo 16.1 INFO - GPO install registry entries does not have searchable pattern, just list them here to help debugging >> %LOG_FILE%
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
