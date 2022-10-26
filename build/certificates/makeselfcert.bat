@echo on

@echo.
@echo Delete existing certificates
del /F NextLabsSelfDebug.pvk
del /F NextLabsSelfDebug.cer
del /F NextLabsSelfDebug.spc
del /F NextLabsSelfDebug.pfx

@echo.
@echo Create Sub Cert
"C:\WinDDK\7600.16385.1\bin\x86\makecert.exe" -n "CN=NextLabsSelfDebug" -e 01/01/2099 -sv NextLabsSelfDebug.pvk NextLabsSelfDebug.cer

@echo.
@echo Create SPC
"C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin\cert2spc.exe" NextLabsSelfDebug.cer NextLabsSelfDebug.spc

@echo.
@echo Create PFX file
"C:\WinDDK\7600.16385.1\bin\x86\pvk2pfx.exe" -pvk NextLabsSelfDebug.pvk -pi 123blue! -spc NextLabsSelfDebug.spc -pfx NextLabsSelfDebug.pfx -f -po 123blue!

@echo.
pause