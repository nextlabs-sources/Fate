@echo off
setlocal

set CURRENTDIR=%CD%

if not "%WDKDIR%"=="" goto _skip_set_winddkdir
	set WDKDIR=C:\WinDDK\7600.16385.1
:_skip_set_winddkdir

if "%NLBUILDROOT_DOS%"=="" goto _buildroot_not_set
if "%NLCERTDIR%"=="" goto _nlcertdir_not_set

set SIGNTOOL=%WDKDIR%\bin\x86\signtool.exe
set INFOTOOL=%WDKDIR%\bin\selfsign\Inf2Cat.exe
set NEXTLABSCERT=%NLCERTDIR%\NextLabs.pfx
set NEXTLABSPSWD=IiVf1itvOrqJ
set CROSSCERT=%NLCERTDIR%\MSCV-VSClass3.cer

set INF_FILE=%CURRENTDIR%\%1.inf

set X86_DRV_BIN=%CURRENTDIR%\objfre_wxp_x86
set X86_DRV_FILE=%X86_DRV_BIN%\i386\%1.sys
set X64_DRV_BIN=%CURRENTDIR%\objfre_win7_amd64
set X64_DRV_FILE=%X64_DRV_BIN%\amd64\%1.sys

set INSTALL_DIR=%CURRENTDIR%\drivers
set X86_CAT_FILE=%INSTALL_DIR%\%1.x86.cat
set X64_CAT_FILE=%INSTALL_DIR%\%1.x64.cat

set WDF_COINSTALLER_DLL_NAME=wdfcoinstaller01009.dll
set X86_WDF_COINSTALLER_DLL=%WDKDIR%\redist\wdf\x86\%WDF_COINSTALLER_DLL_NAME%
set X64_WDF_COINSTALLER_DLL=%WDKDIR%\redist\wdf\amd64\%WDF_COINSTALLER_DLL_NAME%

rem check arguments
if NOT "%1"=="" goto _ArgOk
echo Usage:
echo drvbuild.bat DrvName
echo   example: drvbuild.bat nxefs " - build nxefs.sys, 32 bit and 64 bit"
echo drvbuild.bat DrvName 32
echo   example: drvbuild.bat nxefs yes " - build nxefs.sys, 32 bit only"
echo drvbuild.bat DrvName 64
echo   example: drvbuild.bat nxefs yes " - build nxefs.sys, 64 bit only"
echo NOTICE:
echo   "NLBUILDROOT_DOS" must be set before call this batch file
set ERRORLEVEL=1
goto _exit


:_ArgOk


rem inspect tools/certs
if not Exist "%SIGNTOOL%" goto _signtool_not_exist
if not Exist "%INFOTOOL%" goto _infotool_not_exist
if not Exist "%NEXTLABSCERT%" goto _nextlabscert_not_exist
if not Exist "%CROSSCERT%" goto _scrosscert_not_exist

rem inspect .inf file
if not Exist "%INF_FILE%" goto _inffile_not_exist



rem Deleting build logs ...
del /F /Q *.log
del /F /Q *.wrn
del /F /Q *.err

rem Deleting build/install dirs ...
if Exist "%X86_DRV_BIN%" rmdir /Q /S "%X86_DRV_BIN%"
if Exist "%X64_DRV_BIN%" rmdir /Q /S "%X64_DRV_BIN%"
if Exist "%INSTALL_DIR%" rmdir /Q /S "%INSTALL_DIR%"

rem build amd64 for windows7
if /I "%2"=="32" goto _build_x64_end
cmd.exe /k "%NLBUILDROOT_DOS%\build\drvbuildsingle.bat" x64
if not Exist "%X64_DRV_FILE%" goto _error_build_x64
:_build_x64_end

rem build amd64 for windows xp
if /I "%2"=="64" goto _build_x86_end
cmd.exe /k "%NLBUILDROOT_DOS%\build\drvbuildsingle.bat" x86
if not Exist "%X86_DRV_FILE%" goto _error_build_x86
:_build_x86_end

rem sign binaries
if /I "%2"=="32" goto _sign_x64_end
%SIGNTOOL% sign /ac "%CROSSCERT%" /f "%NEXTLABSCERT%" /p %NEXTLABSPSWD% /n "NextLabs, Inc." /t http://timestamp.verisign.com/scripts/timestamp.dll "%X64_DRV_FILE%"
if %ERRORLEVEL% == 0 goto _sign_x64_end
if %ERRORLEVEL% == 1 goto _sign_x64_end
goto _error_sign_x64
:_sign_x64_end

if /I "%2"=="64" goto _sign_x86_end
%SIGNTOOL% sign /ac "%CROSSCERT%" /f "%NEXTLABSCERT%" /p %NEXTLABSPSWD% /n "NextLabs, Inc." /t http://timestamp.verisign.com/scripts/timestamp.dll "%X86_DRV_FILE%"
if %ERRORLEVEL% == 0 goto _sign_x86_end
if %ERRORLEVEL% == 1 goto _sign_x86_end
goto _error_sign_x86
:_sign_x86_end

rem prepare installer disk
md "%INSTALL_DIR%"
md "%INSTALL_DIR%\i386"
md "%INSTALL_DIR%\amd64"
copy "%INF_FILE%" "%INSTALL_DIR%\%1.inf"
copy "%X64_DRV_FILE%" "%INSTALL_DIR%\amd64\%1.sys"
if Exist "%X86_DRV_FILE%" copy "%X86_DRV_FILE%" "%INSTALL_DIR%\i386\%1.sys"
rem copy wdfcoinstaller dll
copy "%X86_WDF_COINSTALLER_DLL%" "%INSTALL_DIR%\i386\"
copy "%X64_WDF_COINSTALLER_DLL%" "%INSTALL_DIR%\amd64\"

rem create cat file
set SUPPORTOS=XP_X86,XP_X64,Vista_X86,Vista_X64,7_X86,7_X64,Server2003_X86,Server2003_X64,Server2008_X86,Server2008_X64,Server2008R2_X64
if /I "%2"=="32" set SUPPORTOS=XP_X86,Vista_X86,7_X86,Server2003_X86,Server2008_X86
if /I "%2"=="64" set SUPPORTOS=XP_X64,Vista_X64,7_X64,Server2003_X64,Server2008_X64,Server2008R2_X64

if /I "%2"=="32" goto _make_cat_x64_end
%INFOTOOL% /driver:"%INSTALL_DIR%" /os:%SUPPORTOS%
if not Exist "%X64_CAT_FILE%" goto _error_make_cat
:_make_cat_x64_end

if /I "%2"=="64" goto _make_cat_x86_end
if not Exist "%X86_CAT_FILE%" goto _error_make_cat
:_make_cat_x86_end

rem Sign the cat file
if /I "%2"=="32" goto _sign_cat_x64_end
%SIGNTOOL% sign /ac "%CROSSCERT%" /f "%NEXTLABSCERT%" /p %NEXTLABSPSWD% /n "NextLabs, Inc." /t http://timestamp.verisign.com/scripts/timestamp.dll "%CURRENTDIR%\drivers\%1.x64.cat"
:_sign_cat_x64_end

if /I "%2"=="64" goto _sign_cat_x86_end
%SIGNTOOL% sign /ac "%CROSSCERT%" /f "%NEXTLABSCERT%" /p %NEXTLABSPSWD% /n "NextLabs, Inc." /t http://timestamp.verisign.com/scripts/timestamp.dll "%CURRENTDIR%\drivers\%1.x86.cat"
:_sign_cat_x86_end


:_exit
rem exit /b %ERRORLEVEL%
goto _end



rem
rem ERROR
rem

:_buildroot_not_set
echo drvbuild: Variable "NLBUILDROOT_DOS" not set - it must be set before call this batch file
set ERRORLEVEL=1
goto _exit

:_nlcertdir_not_set
echo drvbuild: Variable "NLCERTDIR" not set - it must be set before call this batch file
set ERRORLEVEL=1
goto _exit

:_signtool_not_exist
echo drvbuild: Cannot find signtool.exe - "%SIGNTOOL%"
set ERRORLEVEL=1
goto _exit

:_infotool_not_exist
echo drvbuild: Cannot find info2cat.exe - "%INFOTOOL%"
set ERRORLEVEL=1
goto _exit

:_nextlabscert_not_exist
echo drvbuild: Cannot find NextLabs cert - "%NEXTLABSCERT%"
set ERRORLEVEL=1
goto _exit

:_scrosscert_not_exist
echo drvbuild: Cannot find NextLabs cert - "%CROSSCERT%"
set ERRORLEVEL=1
goto _exit

:_inffile_not_exist
echo drvbuild: Cannot find .inf file - "%INF_FILE%"
set ERRORLEVEL=1
goto _exit

:_error_build_x64
echo drvbuild: Fail to build x64 driver
set ERRORLEVEL=1
goto _exit

:_error_build_x86
echo drvbuild: Fail to build x86 driver
set ERRORLEVEL=1
goto _exit

:_error_sign_x64
echo drvbuild: Fail to sign x64 driver, error=%ERRORLEVEL%
set ERRORLEVEL=1
goto _exit

:_error_sign_x86
echo drvbuild: Fail to sign x86 driver, error=%ERRORLEVEL%
set ERRORLEVEL=1
goto _exit

:_error_make_cat
echo drvbuild: Fail to make cat file
set ERRORLEVEL=1
goto _exit


rem
rem END
rem
:_end
endlocal
