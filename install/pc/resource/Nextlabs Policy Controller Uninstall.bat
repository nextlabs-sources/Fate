@echo off
sc query complianceenforcerservice | findstr /i "running"
if %ERRORLEVEL% == 0 bin\stopenforcer.exe
msiexec /x ProductCode /qb
