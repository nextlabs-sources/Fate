#include <windows.h>
#include <stdio.h>
#include <wdfinstaller.h>
#include <newdev.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <strsafe.h>

extern int nlPnpDriverInstall(BOOLEAN bInstall, LPCTSTR infFile, LPCTSTR hardwareID);

namespace {
/////////////////////
// Private GLOBALS //
/////////////////////
LPCTSTR   infFile;
LPCTSTR   hardwareID;
BOOLEAN   IsInstall;
WCHAR     INFFileWithPath[255];


///////////////
// CONSTANTS //
///////////////
#define USAGE_STRING "Usage: nlDevenfInstaller {-i|-r} inf hwid\n\n"  \
                     "  {-i|-r} -- Indicates whether this is an "  \
                     "installation or a removal. As to today, the " \
					 "remove functionality is not implemented.\n" \
					 "  inf -- Specifies an INF file with installation " \
					 "information for the device\n " \
                     " hwid -- Specifies a hardware ID for the device\n"

#define INSTALL_OR_REMOVE_OPT   1
#define INF_FILE_NAME_OPT       2
#define HWID_OPT                3
#define ARGUMENT_COUNT          4

BOOLEAN
ParseParameters(
    int argc, 
    WCHAR **argv
    ) 
{
    if (argc != ARGUMENT_COUNT) {
        printf(USAGE_STRING);
        return FALSE;
    }

    //
    // Install or remove?
    //
    if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-i") == 0) {
        IsInstall = TRUE;
    } else if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-r") == 0) {
        IsInstall = FALSE;
    } else {
        printf("Unknown option %ls\n", argv[1]);
        return FALSE;
    }

    //
    // INF file name ...
    //
    infFile = argv[INF_FILE_NAME_OPT];

    //
    // And the SYS...
    // 
    hardwareID = argv[HWID_OPT];

    return TRUE;
}
}
void __cdecl wmain(int argc, WCHAR **argv) 
{

    //
    // Do all of the necessary initialization
    //
    if (!ParseParameters(argc, argv)) {
        //
        // Whoever failed printed out an appropriate error,
        //  just return.
        //
        return;
    }

    //
    // Let the user know what she's doing...
    //
    printf("You want to %s %ls. The INF file is located at %ls\n",
        IsInstall ? "INSTALL" : "REMOVE", 
        hardwareID,
        infFile);

    //
    // Take the appropriate action...
    //
    if (IsInstall) {
      int ret=nlPnpDriverInstall(IsInstall, infFile, hardwareID);
      printf("nlPnpDriverInstall: %d\n", ret);
    } else {
      int ret=nlPnpDriverInstall(IsInstall, infFile, hardwareID);
      printf("nlPnpDriverInstall (uninstall): %d\n", ret);
    }

    return;

}

