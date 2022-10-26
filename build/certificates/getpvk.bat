@echo on

@echo.
@echo Delete existing certificates
del /f NextLabs.pem
del /f NextLabs.pvk
del /f NextLabsCert.pem

@echo.
@echo Get PEM
"C:\Program Files\GnuWin32\bin\openssl.exe" pkcs12 -in NextLabs.pfx -nocerts -nodes -out NextLabs.pem

@echo.
@echo Get pvk file
pvk -in NextLabs.pem -topvk -out NextLabs.pvk

@echo.
@echo Get pem-certs file
"C:\Program Files\GnuWin32\bin\openssl.exe" pkcs12 -in NextLabs.pfx -nokeys -out NextLabsCert.pem

@echo.
pause