@echo off
rem -------------------------------------------------------------------------
rem Description
rem		This script runs NlUninstallCleanup.exe to cleanup files, registry settings,
rem		device drivers and services left behind by uninstall. It can also be used
rem		to check what needs to be cleanup.
rem
rem		Currently, NlUninstallCleanup.exe cannot stop tamper resistance, remove
rem		Add/Remove Program entries and remove GPO entries.
rem
rem		This script must be run from InstallCleanup directory. In addition, the 
rem		directory structure under InstallCleanup must be maintained.
rem
rem Instructions
rem		1. To do a dry run (check what needs to be deleted):
rem			- Stop Policy Controller
rem			- Start a DOS Prompt by right clicking Run as Administrator
rem			- cd <resource-kit-root>/InstallCleanup
rem			- InstallCleanup -n
rem			- Check output file NlUninstallCleanup.log for error
rem		2. To cleanup files, registry settings, device drivers and Windows services:
rem			- Stop Policy Controller
rem			- Start a DOS Prompt by right clicking Run as Administrator
rem			- cd <resource-kit-root>/InstallCleanup
rem			- InstallCleanup.bat
rem			- Check output file NlUninstallCleanup.log for error
rem			- Reboot computer to remove loaded device driver and running services
rem			- Start a DOS Prompt by right clicking Run as Administrator
rem			- cd <resource-kit-root>/InstallCleanup
rem			- InstallCleanup.bat
rem			- Check output file NlUninstallCleanup.log for error
rem -------------------------------------------------------------------------


setlocal

rem Execute command
if "%PROCESSOR_ARCHITECTURE%" == "x86" goto lblCleanupX86
@echo on
pushd x64
NlUninstallCleanup -c NlUninstallCleanup_x64.cfg %1
popd
@echo off
goto lblCleanupDone
:lblCleanupX86
@echo on
pushd x86
NlUninstallCleanup -c NlUninstallCleanup_x86.cfg %1
popd
@echo off
:lblCleanupDone

endlocal