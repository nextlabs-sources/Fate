@echo on
setlocal

rem Notes: LOG_FILE must match the same definition in NlInstallCheck.bat.
rem		set LOG_FILE=NlInstallCheck-%1.log

rem Set variables
set REPO_DIR=\\nextlabs.com\share\data\mis\helpdesk\ALPO_Cleanup\Uninstall_Check
set LABEL=%COMPUTERNAME%-%DATE:~10,4%%DATE:~4,2%%DATE:~7,2%-%TIME:~0,2%%TIME:~3,2%

rem Call install check script
pushd %TEMP%
copy /y %REPO_DIR%\NlInstallCheck.bat .
call NlInstallCheck.bat %LABEL%
@echo on
dir NlInstallCheck-*.log
popd

rem Copy log file to repo
copy /Y %TEMP%\NlInstallCheck-%LABEL%.log %REPO_DIR%\%LOG_FILE_NAME%
endlocal
