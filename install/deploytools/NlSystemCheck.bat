@echo off
setlocal

rem ---------------------------------------------------------------
rem	Copyright 2011 NextLabs, Inc. All rights reserved
rem
rem Description:
rem   NextLabs endpoint system check script
rem
rem Written by:
rem	  Poon Fung
rem
rem Version:
rem  0.1.0 - 10/22/2011 First version
rem  0.1.1 - 1/23/2012 Echo more environment variables to help debug running in DOS-32
rem          under x64
rem ---------------------------------------------------------------


echo NextLabs Endpoint System Check v0.1.1


rem --------------------
rem Jump table

if "%1" == "" goto LBL_PrintUsage
goto LBL_Start


rem --------------------
rem Help

:LBL_PrintUsage
echo Usage: NLSystemCheck ^<label^>
echo   label   Label used to make up log file name NLSystemCheck-label.log
echo Example:
echo   NLSystemCheck "WinXP-WAYA"
goto LBL_Exit


rem --------------------
rem Main program

:LBL_Start
set TMP_OUT=NLSystemCheck00.tmp

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
set LOG_FILE=NLSystemCheck-%1.log

rem Remove log file
if exist %LOG_FILE% del /F %LOG_FILE%


rem Print summary
echo NextLabs Endpoint Check Script v0.1.1 >> %LOG_FILE%
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
rem goto LBL_Section4


:LBL_Section1
echo Section 1: Get OS information
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 1: Get OS information >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
systemInfo >> %LOG_FILE%


:LBL_Section2
echo Section 2: List running processes
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 2: List running processes >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
tasklist >> %LOG_FILE%


:LBL_Section3
echo Section 3: List software installed
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 3: List software installed >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
wmic product get >> %LOG_FILE%


:LBL_Section4
echo Section 4: List NextLabs software directory
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 4: List NextLabs software directory >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set DIR_PATH=%ProgramFiles%\NextLabs
echo   - %DIR_PATH%
dir "%DIR_PATH%" /s >> %LOG_FILE%

if not %HOST_ARCH% == x64 goto LBL_4_1_x64
set DIR_PATH=%ProgramFiles(x86)%\NextLabs
echo   - %DIR_PATH%
dir "%DIR_PATH%" /s >> %LOG_FILE%
:LBL_4_1_x64


:LBL_Section5
echo Section 5: List NextLabs registry settings
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Section 5: List NextLabs registry settings >> %LOG_FILE%
echo -- >> %LOG_FILE%

echo. >> %LOG_FILE%
set REGKEY=HKLM\SOFTWARE\NextLabs
echo   - %REGKEY%
reg query "%REGKEY%" >> %LOG_FILE%

if not %HOST_ARCH% == x64 goto LBL_5_1_x64
set REGKEY=HKLM\SOFTWARE\Wow6432Node\NextLabs
echo   - %REGKEY%
reg query "%REGKEY%" >> %LOG_FILE%
:LBL_5_1_x64


:LBL_SectionDone
echo. >> %LOG_FILE%
echo -------------------------------------------------------------- >> %LOG_FILE%
echo -- Done >> %LOG_FILE%
echo -- >> %LOG_FILE%

rem DEBUG: Uncomment the following line to show log file content
rem type %LOG_FILE%

echo Additional output is written to log file %LOG_FILE%


:LBL_Exit
rem --------------------
rem Clean up

endlocal
