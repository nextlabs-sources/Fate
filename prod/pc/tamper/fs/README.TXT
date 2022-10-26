################################################################################
#
# NextLabs Tamper Resistance File System Filter (NLTamper)
#
################################################################################

The NLTamper driver filters CREATE (IRP_MJ_CREATE) in disk based file systems.
When a CREATE occurs the file attributes such as file name and create
permissions are compared against a set of NLTamper file access policies.  If a
policy matches the policies actions are taken.  For example, a CREATE may be
denied or a CREATE for write access may be denied.

NLTamper policies describe permissions to a set of files on a file system.

Use cases:
  (1) Policy Controller protects all contents from non-Policy Controller
      processes.
  (2) Installed Enfrocer protects its install directory from all processes.

Examples
--------
  Protect: C:\Program Files\NextLabs\Policy Controller FOR ALL ACTION(S)
  Exempt : Process ID of current cepdpdman.exe instance.
  Result : Any process which is not the Policy Controller (cepdpman.exe) will
           be denied when a file system CREATE is issued.

  Protect: C:\Program Files\NextLabs FOR WRITE ACTIONS(S)
  Result : Any process which is not exempted is denied from issuing a CREATE
           which requires write access.

Interface (API)
---------------
  Name                            Description
  -----------------------------------------------------------------------------
  NLTamper_ProtectFile            Protect a file from any CREATE operation.
  NLTamper_ExemptProcessName      Exempt a process image path from protection.
  NLTamper_ExemptProcessId        Exempt a process ID from protection.
  NLTamper_TranslatePath          Translate letter-based path to volume based
                                  path for NLTamper_XXX interfaces.

Targets
  nl_tamper.sys      NLTamper service and file system filter driver.
  tamper_plugin.dll  Tamper Resistance Policy Controller plug-in.

Installation
  rundll32.exe SETUPAPI.DLL,InstallHinfSection DefaultInstall 128 .\nl_tamper.inf

Uninstallation
  rundll32.exe SETUPAPI.DLL,InstallHinfSection DefaultUninstall 128 .\nl_tamper.inf
