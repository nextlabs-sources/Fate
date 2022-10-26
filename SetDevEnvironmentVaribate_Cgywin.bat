@echo off

echo 请以管理员身份运行

rem NLEXTERNALDIR 需要提前设置好
rem 需要设置的环境变量: WORKSPACE, NLBUILDROOT, BUID_OUTPUT, BUILD_NUMBER

set BAT_DIR=%~dp0
set BAT_DIR=%BAT_DIR:~0,-1%
set BAT_DIR=%BAT_DIR:\=/%

set WORKSPACE_NEW=%BAT_DIR%
set NLBUILDROOT_NEW=%BAT_DIR%
set BUID_OUTPUT_NEW=%BAT_DIR%/output
set BUILD_NUMBER_NEW=1001
set VERSION_BUILD_NEW=1001

setx /m WORKSPACE %WORKSPACE_NEW%
setx /m NLBUILDROOT %NLBUILDROOT_NEW%
setx /m BUID_OUTPUT %BUID_OUTPUT_NEW%
setx /m BUILD_NUMBER %BUILD_NUMBER_NEW%
setx /m VERSION_BUILD %VERSION_BUILD_NEW%

subst p: "c:\Program Files"
setx /m JDKDIR "p:/Java/jdk1.8.0_261"

echo BAT_DIR=%BAT_DIR%
echo WORKSPACE FROM %WORKSPACE% TO %WORKSPACE_NEW%
echo NLBUILDROOT FROM %NLBUILDROOT% TO %NLBUILDROOT_NEW%
echo BUID_OUTPUT FROM %BUID_OUTPUT% TO %BUID_OUTPUT_NEW%
echo BUILD_NUMBER FROM %BUILD_NUMBER% TO %BUILD_NUMBER_NEW%
echo VERSION_BUILD FROM %VERSION_BUILD% TO %VERSION_BUILD_NEW%


echo -----------------------
echo 请在新打开的dev.exe中编译
echo 命令提示符窗口可能无法退出，请手动关闭
echo -----------------------

rem set VC_VARS_ALL_BAT=%VS140COMNTOOLS%
rem set VC_VARS_ALL_BAT=%VC_VARS_ALL_BAT:Tools=IDE%devenv.exe

rem mkdir "%BAT_DIR%\.git\hooks"
rem copy /y "%BAT_DIR%\pre-commit" "%BAT_DIR%\.git\hooks\pre-commit"

pause