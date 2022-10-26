REM
REM This script should only be run as the sole user that has acccess to
REM \\%SIGNING_SERVER%\%SIGNING_SHARE%.  Currently this is NEXTLABS\build-op.
REM

dir
echo $SIGNING_SERVER

SETLOCAL ENABLEDELAYEDEXPANSION

IF "%SIGNING_SERVER%"=="" GOTO USAGE

set SIGNING_SHARE=signing256Only
set SIGNING_REQ_FILENAME=signingReq.txt
set SIGNING_DONE_FILENAME=signingDone.txt



REM
REM Copy the files to a unique directory on the server.
REM

set FILE_LIST=
FOR %%i IN (*.dll *.exe *.sys *.cat *.cab *.msi *.api) DO set FILE_LIST=!FILE_LIST! "%%i"
IF NOT DEFINED FILE_LIST (
  ECHO No supported files are found!
  EXIT /b 1
)

net use "\\%SIGNING_SERVER%\%SIGNING_SHARE%" /persistent:no
set SIGNING_PATH=\\%SIGNING_SERVER%\%SIGNING_SHARE%\%COMPUTERNAME%_%RANDOM%
md "%SIGNING_PATH%"
IF ERRORLEVEL 1 GOTO :EOF

FOR %%i IN (%FILE_LIST%) DO (
  xcopy /k %%i "%SIGNING_PATH%"
  IF ERRORLEVEL 1 GOTO :EOF
)



REM
REM Request the server to start signing by creating the file
REM %SIGNING_REQ_FILENAME%.
REM

copy nul "%SIGNING_PATH%\%SIGNING_REQ_FILENAME%"



REM
REM Wait for the server to finish signing by waiting for the presence of the
REM file %SIGNING_DONE_FILENAME%.
REM

:WAIT

IF NOT EXIST "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" (
  ECHO Waiting for "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" ......

  IF EXIST C:\cygwin\bin\sleep.exe (
    C:\cygwin\bin\sleep.exe 10
  ) ELSE IF EXIST C:\cygwin64\bin\sleep.exe (
    C:\cygwin64\bin\sleep.exe 10
  )

  GOTO WAIT
)

ECHO "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" detected.
del "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%"



REM
REM Move the files back to our directory.
REM

xcopy /k /y "%SIGNING_PATH%"

rd /s /q "%SIGNING_PATH%"

net use "\\%SIGNING_SERVER%\%SIGNING_SHARE%" /del

GOTO :EOF



:USAGE
echo Usage: %0
echo The environment variable SIGNING_SERVER must be defined.
