####################################################################
#
# ceInjection - Dynamic Injection
#
# This document describes the implementation of dynamic injection
# and it use with adapters.  An example of an adapter is provided.
#
####################################################################

Change Log
----------
03-06-2008 Initial check-in after merge to main.
03-11-2008 Install level information.
03-17-2008 Open issues: call of hooked function from within a hooked
           function.

Open Issues
-----------
If a target function called from within a hooked function calls a
hooked function the adapter state will become corrupt.

Example: MoveFileW is implemented as MoveFileWithProgressW

myMoveFileW                    (hook function)
  MoveFileW                    (call of target function)
    MoveFileWithProressW       (target function calls hooked function)
      myMoveFileWithProgressW  (state becomes corrupt)

Definitions
-----------
  Enforcer   Collection of adapter(s) that compose an enforcement
             product.
  Adapter    Policy Adapter used to enforce actions by a process.
  End-Point  Adapter that resides at the bottom of the adapter
             stack.
  Target     The function being hooked (e.g. CreateFileW from
             kernel32.dll).
  Level      Level to install the adapter at.  See CEAdapter.h for
             level definition.

Configuration
-------------
Adapters are configured to load and install in a given process when
a configration file for that process exists.  Configuration supports
injection of adapter(s) into an application and specifying a hook
library that should be injected for all applications.

If two adapters are specified to install at the same level their order
is not defined.  The general method to describe install order is as
follows.  Level param is optional.  Default is used when it is not
present.

hook=[library],[level]

For example, the LPR and PRINT commands might share a common
configration:

Example File:

  ; Files: lpr.exe.ini, print.exe.ini
  ; LPR/PRINT common adapter to map OPEN -> PRINT action
  hook=adapter_lpr.dll

There may be more than one hook library specified for a process.

Example File (winword.exe,excel.exe,etc.):

  ; MS Application with Office and Messenger support
  hook=adapter_msoffice2k3.dll
  hook=adapter_messenger.dll

There may also be a set of adapters that are injected for all
processes.  For example, the tamper proofing adapter may be
injected into all applications.  In addition some applications
may be injected with additional libraries as examples above show.

The below example shows that adapter_fs should be installed for
all processes at (lowest) level 0.

Example File (all):

  ; Tamper proofing/File system control installed at level 0
  hook=adapter_fs,0

####################################################################
#
# Example Enforcer
#
# Handle LPR/PRINT
#
#   The OPEN/READ actions imply PRINT action, so to avoid a document
#   from being printed the adapter must prohibit OPEN/READ.  This
#   action is trapped in CreateFileW() provided by kernel32.dll.
#
####################################################################

(1) Handle initialization.  The AdapterEntry callback from the
    dynamic injection framework occurs.

    int AdapterEntry(void)
    {
      /* Install hook for CreateFileW in kernel32.dll */

      bool result;

      restul = adapter::AddHook("kernel32.dll",      // base library
	  	                "CreateFileW",       // base function
		                myCreateFileW);      // address of hook

      /* The return value indicates whether the adapter started
         successfully.  When an error occurs in AdapterEntry it
         should return non-zero.
      */
      return result;

    }/* AdapterEntry */

(2) Handle hooked target function.  When CreateFileW is called from
    an application the hooked function myCreateFileW will be called.

    The hook function should evaluate policy, set cancel state if
    needed and check for the end-point state with cancel state.  If
    the current adapter is the end-point and the cancel state has
    been set, the adapter must return to the caller without calling
    the next adapter via CEDI_Next.  Calling the next adapter will
    result in the target function being called.

    /* Trap CreateFileW calls for policy evaluation. */
    HANDLE myCreateFileW( LPCTSTR lpFileName,
  			  DWORD dwDesiredAccess,
   			  DWORD dwShareMode,
			  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			  DWORD dwCreationDisposition,
			  DWORD dwFlagsAndAttributes,
			  HANDLE hTemplateFile )
    {
      /* Evaluate policy for requested CreateFile() operation. */

      deny = my_file_action_check(lpFileName,PRINT);

      /* If the file should not be printed it must be cancelled. */
      if( lpFileName && deny == true )
      {
        /* Set return value and deny state.  This will prevent the
	   target function from being called.
        */
        adapter::SetDenyReturnValue<HANDLE>(INVALID_HANDLE_VALUE);
      }

      /* Call into next adapter */
      return = adapter::Next<HANDLE>(lpFileName,
	  	  	             dwDesiredAccess,
		  	             dwShareMode,
			             lpSecurityAttributes,
			             dwCreationDisposition,
			             dwFlagsAndAttributes,
			             hTemplateFile);
    }/* myCreateFileW */
