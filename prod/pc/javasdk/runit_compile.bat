rem -------------------------------------------------------------------
rem Policy Studio commandline compilation script
rem -------------------------------------------------------------------

setlocal

if not "%NLBUILDROOT%" == "" goto skip_nlbuildroot_err
echo ERROR: NLBUILDROOT not set
goto endAll
:skip_nlbuildroot_err

if not "%NLEXTERNALDIR%" == "" goto skip_nlexternaldir_err
echo ERROR: NLEXTERNALDIR not set
goto endAll
:skip_nlexternaldir_err

set PATH=%NLEXTERNALDIR%\apache-ant\apache-ant-1.7.0\bin;%PATH%
set ANT_OPTS=-Xmx1024M -XX:PermSize=128m -XX:MaxPermSize=512m
set ANT_ARGS=-Dnlbuildroot=%NLBUILDROOT%
ant -v -f build_compile.xml

endlocal
