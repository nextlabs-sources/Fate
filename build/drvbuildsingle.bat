@echo off
@set CurrentDir=%CD%

@rem inspect arguments
@if "%1"=="" goto _ArgOk
@if /I "%1"=="x86" goto _ArgOk
@if /I "%1"=="x64" goto _ArgOk

@rem Usage:
@echo Usage:
@echo drvbuildsingle.bat [x86 | x64]
@set ERRORLEVEL=1
@goto _exit

:_ArgOk

@rem Clean old files
@rem if exist %BUILD_ALT_DIR% rmdir /Q /S %BUILD_ALT_DIR%
@rem IF %ERRORLEVEL% NEQ 0 goto _error_delete_target

@rem goto proper build command
@if "%1"=="" goto _build_x86
@if /I "%1"=="x86" goto _build_x86
@if /I "%1"=="x64" goto _build_x64


:_build_x64
@call %WDKDIR%\bin\setenv.bat %WDKDIR%\ fre x64 WIN7
@cd /D %CurrentDir%
@build -ceZ
@rem IF %ERRORLEVEL% NEQ 0 goto _error_build_x64
@goto _exit

:_build_x86
@call %WDKDIR%\bin\setenv.bat %WDKDIR%\ fre x86 WXP
@cd /D %CurrentDir%
@build -ceZ
@rem IF %ERRORLEVEL% NEQ 0 goto _error_build_x86
@goto _exit


:_exit
@rem exit /b %ERRORLEVEL%
@exit



rem
rem ERROR
rem

:_error_delete_target
@echo Fail to delete target directory - %BUILD_ALT_DIR%
@set ERRORLEVEL=1
@goto _exit

:_error_build_x64
@echo Fail to build x64 driver, error=%ERRORLEVEL%
@set ERRORLEVEL=1
@goto _exit

:_error_build_x86
@echo Fail to build x86 driver, error=%ERRORLEVEL%
@set ERRORLEVEL=1
@goto _exit