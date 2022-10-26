@echo off
setlocal

rem ---------------------------------------------------------------
rem	Copyright 2011 NextLabs, Inc. All rights reserved
rem
rem Description:
rem   NextLabs script to collect log files related to installation
rem
rem Written by:
rem	  Poon Fung
rem
rem Version:
rem  0.1 - 7/31/2011 Initial checkin
rem
rem Notes:
rem  1.	There should be a bin directory in the same directory as NlCollectInstallLog.bat.
rem		However, the bin directory is optional. If bin directory exists, this script
rem		will run additional command to collect more info.
rem  2.	The bin directory contains GNUWin32 and Windows Resource Kit tools.
rem  3. FINDSTR is used because GNU find does not set ERRORLEVEL.
rem  4. PATH is altered at the being of the script to add bin directory. As a result,
rem		this script will pick up bin/find.exe instead of C:/Windows/System32/find.exe.
rem ---------------------------------------------------------------


echo NextLabs Installation Log Collector v0.1.1


rem --------------------
rem Jump table

if "%1" == "" goto LBL_PrintUsage
goto LBL_Start


rem --------------------
rem Help

:LBL_PrintUsage
echo Usage: NlCollectInstallLog ^<label^>
echo   label   Label used to make up log file name NlCollectInstallLog-label.log
echo Example:
echo   NlCollectInstallLog "WinXP-WAYA-first"
goto LBL_Exit


rem --------------------
rem Main program

:LBL_Start
set TMP_OUT=NlCollectInstallLog00.tmp

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
set LOG_FILE=NlCollectInstallLog-%1.log
set ZIP_FILE=NlCollectInstallLog-%1.zip
set STAGING_DIR=NlCollectInstallLog-%1
set PASS_COUNT=0
set FAIL_COUNT=0

rem Add bin directory to path
set PATH=bin;%PATH%

rem Remove log file and create staging directory
if exist %LOG_FILE% del /F %LOG_FILE%
if exist %ZIP_FILE% del /F %ZIP_FILE%
if not exist %STAGING_DIR% mkdir %STAGING_DIR%
if exist %STAGING_DIR% del /F %STAGING_DIR%\*.log

rem Print summary
echo NextLabs Installation Log Collector v0.1.1 >> %LOG_FILE%
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo TIME = %DATE% %TIME% >> %LOG_FILE%
echo COMPUTERNAME = %COMPUTERNAME% >> %LOG_FILE%
echo PROCESSOR_ARCHITECTURE = %PROCESSOR_ARCHITECTURE% >> %LOG_FILE%
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
rem goto LBL_Section2


:LBL_Section1
echo Section 1: Collect event logs
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 1: Collect event logs >> %LOG_FILE%
echo -- >> %LOG_FILE%

if not exist bin\dumpel.exe goto LBL_1_No_Dumpel
echo. >> %LOG_FILE%
echo 1.1 INFO: Dump system event log >> %LOG_FILE%
bin\dumpel -f %STAGING_DIR%\eventLog_system.log -l system -d 1 2>&1 >> %LOG_FILE%
set /a PASS_COUNT += 1
echo 1.2 INFO: Dump application event log >> %LOG_FILE%
bin\dumpel -f %STAGING_DIR%\eventLog_app.log -l application -d 1 2>&1 >> %LOG_FILE%
set /a PASS_COUNT += 1
:LBL_1_No_Dumpel


:LBL_Section2
echo Section 2: Collect MSI logs
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 2: Collect MSI logs >> %LOG_FILE%
echo -- >> %LOG_FILE%

set TMP_OUT=NlCollectInstallLog02.tmp

if not exist bin\head.exe goto LBL_2_No_Head
rem Directory pointed to by TEMP variables
if not exist %TEMP% goto LBL_2_1_Done
dir "%TEMP%\MSI*.log" /B /O:-D | bin\head -10 > %TMP_OUT%
for /f "delims=" %%f in ('type %TMP_OUT%') DO (
	echo 2.1 INFO: Collecting %TEMP%\%%f >> %LOG_FILE%
	copy /y %TEMP%\%%f %STAGING_DIR% 2>&1 >> %LOG_FILE%
)
set /a PASS_COUNT += 1
:LBL_2_1_Done

rem C:\Temp
set DIR_PATH=%SystemDrive%\Temp
if not exist %DIR_PATH% goto LBL_2_2_Done
dir "%DIR_PATH%\MSI*.log" /B /O:-D | bin\head -10 > %TMP_OUT%
for /f "delims=" %%f in ('type %TMP_OUT%') DO (
	echo 2.2 INFO: Collecting %DIR_PATH%\%%f >> %LOG_FILE%
	copy /y %DIR_PATH%\%%f %STAGING_DIR% 2>&1 >> %LOG_FILE%
)
set /a PASS_COUNT += 1
:LBL_2_2_Done

rem C:\Windows\Temp
set DIR_PATH=%windir%\Temp
if not exist %DIR_PATH% goto LBL_2_3_Done
dir "%DIR_PATH%\MSI*.log" /B /O:-D | bin\head -10 > %TMP_OUT%
for /f "delims=" %%f in ('type %TMP_OUT%') DO (
	echo 2.2 INFO: Collecting %DIR_PATH%\%%f >> %LOG_FILE%
	copy /y %DIR_PATH%\%%f %STAGING_DIR% 2>&1 >> %LOG_FILE%
)
set /a PASS_COUNT += 1
:LBL_2_3_Done
:LBL_2_No_Head

if exist %TMP_OUT% del /F %TMP_OUT%


:LBL_Section3
echo Section 3: Zip log directory
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 3: Zip log directory >> %LOG_FILE%
echo -- >> %LOG_FILE%

if not exist bin\zip.exe goto LBL_3_No_Zip
bin\zip -j %ZIP_FILE% %STAGING_DIR%/*.log
set /a PASS_COUNT += 1
:LBL_3_No_Zip

del /F %STAGING_DIR%\*.log
rmdir %STAGING_DIR%


:LBL_SectionDone
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Done >> %LOG_FILE%
echo -- >> %LOG_FILE%

rem DEBUG: Uncomment the following line to show log file content
rem type %LOG_FILE%

echo Additional output is written to log file %LOG_FILE%
echo %PASS_COUNT% passed, %FAIL_COUNT% failed
echo. >> %LOG_FILE%
echo %PASS_COUNT% passed, %FAIL_COUNT% failed >> %LOG_FILE%


:LBL_Exit
rem --------------------
rem Clean up

endlocal
