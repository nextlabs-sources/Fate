Detail Of these three tools:

1.enforcerStopperV2
  This tool is modified based on "Fate\main\prod\pc\enforcerStopper". main modifications:
  I. make enforcerStopperV2 to be a lib not a exe. because this enforcerStopperV2 would be used in both OEInstallPrep.exe and PCStop.exe. and them must be a single binary.
  II. make enforcerStopperV2 link all dependents statically.
  III. Didn't show "Success" when successed to stop pc.
  IV. Check empty password.
  V. Call NLQuench when failed.
  VI. Modify the return value.
  
2. PCStop
   This tool just call enforcerStopperV2. 

3. OEInstallPrep.
   This is a OE Uninstall Tool, This tool call enforcerStopperV2 to stop PC. then use "WINDOWS INSTALLER CLEAN UP UTILITY"   to clean the installation information about OE. 
   because we need OEInstallPrep.exe to be a independ binary. so we embeded "WINDOWS INSTALLER CLEAN UP UTILITY"(msizap.exe) as resource. when execute, OEInstallPrep extract msizap.exe to temp folder.

4. OEPlugin
   This is a OE plugin register/unregister tool.


How to Complile.
1. All these too can be compiled with makefile.
2. All static library its depends is in lib\ folder. if you need update them. compiled them in original position and copy them to lib\.
3. To compile a library to static library. added "SUBTARGET_STATIC=yes" after SUBTARGET=xxxx in makefile.inc.