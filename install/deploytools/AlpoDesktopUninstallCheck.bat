@echo on
setlocal

rem Notes: LOG_FILE must match the same definition in NlUninstallCheck.bat.
rem		set LOG_FILE=NlUninstallCheck-%1.log

rem Set variables
set REPO_DIR=\\ts01\transfer\poon\alpo_logs
set LABEL=%COMPUTERNAME%-%DATE:~10,4%%DATE:~4,2%%DATE:~7,2%-%TIME:~0,2%%TIME:~3,2%

rem Call uninstall check script
pushd %TEMP%
copy /y %REPO_DIR%\NlUninstallCheck.bat .
call NlUninstallCheck.bat %LABEL%
@echo on
dir NlUninstallCheck-*.log
popd

rem Copy log file to repo
copy /Y %TEMP%\NlUninstallCheck-%LABEL%.log %REPO_DIR%\%LOG_FILE_NAME%
endlocal
