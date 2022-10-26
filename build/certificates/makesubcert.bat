@echo on

@echo.
@echo Delete existing certificates
del /f NextLabsDebug.pvk
del /f NextLabsDebug.cer
del /f NextLabsDebug.spc
del /f NextLabsDebug.pfx

@echo.
@echo Create Sub Cert
"C:\WinDDK\7600.16385.1\bin\x86\makecert.exe" -n "CN=NextLabsDebug" -e 01/01/2099 -ic NextLabs.cer -iv NextLabs.pvk -sv NextLabsDebug.pvk NextLabsDebug.cer

@echo.
@echo Create SPC
"C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin\cert2spc.exe" NextLabsDebug.cer NextLabs.cer NextLabsDebug.spc

@echo.
@echo Create PFX file
"C:\WinDDK\7600.16385.1\bin\x86\pvk2pfx.exe" -pvk NextLabsDebug.pvk -pi 123blue! -spc NextLabsDebug.spc -pfx NextLabsDebug.pfx -f -po 123blue!

@echo.
pause