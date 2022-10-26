@echo
startlocal

set REPO_DIR=\\nextlabs.com\share\data\mis\helpdesk\ALPO_Cleanup\Install_Check

if not "%PROCESSOR_ARCHITEW6432%" == "" goto lbl_invokeDos32In64
rem We are runing DOS-32 on x86 or DOS-64 on x64
call %REPO_DIR%\AlpoDesktopInstallCheck.bat
goto lbl_invokeDone
:lbl_invokeDos32In64
rem We are running DOS-32 on x64. Start DOS prompt in 64-bit mode.
%windir%\sysnative\cmd.exe /c %REPO_DIR%\AlpoDesktopInstallCheck.bat
:lbl_invokeDone

endlocal
exit