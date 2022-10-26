/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * Main Routine
 *
 * Phase 1:   January 2012 by H.K.
 *    File and directory, Service, and Registry Cleanup
 *
 ***************************************************************************************/
#include "stdafx.h"
#include "nl_cleanup.h"
#include "nl_baseCleanup.h"
#include "nl_fileCleanup.h"
#include "nl_serviceCleanup.h"
#include "nl_registryCleanup.h"
#include "nl_configFile.h"

// item list from config file
ItemsList itemsList;

#if 1
// testing module
int test_tmain(int argc, wchar_t* argv[], FILE* fp, bool flag)
{
	printf("\nTesting with config file : %ls ", argv[1]);

	itemList items;
	items.push_back(L"c:\\Temp\\docutest");
	
	// testing files and directories cleanup
	FileCleanup fileclean(fp,flag);
    fileclean.deleteItems(items);

	// testing services cleanup
	//ServiceCleanup serviceclean(fp,flag);
    //serviceclean.deleteItems(filepaths);
	return 0;
}
#endif

void Display_help(void)
{
		fprintf(stdout, "usage: NlUninstallCleanup -c <config-file> [-n]\n");
		fprintf(stdout, "       NlUninstallCleanup -h\n");
		fprintf(stdout, "\tOptions:\n");
		fprintf(stdout, "\t  -c <file>  Specify a configuration file\n");
		fprintf(stdout, "\t  -h:/h:/?   Print this message\n");
		fprintf(stdout, "\t  -n         Dry run only. Do not make any modification.\n");
		fprintf(stdout, "\n");
		fprintf(stdout, "Steps to use this utility:\n");
		fprintf(stdout, "1. Stop Policy Controller.\n");
		fprintf(stdout, "2. Open a DOS Prompt using Run as Administrator (right click on icon).");
		fprintf(stdout, "3. Enter 'NlUninstallCleanup -c <config-file> -n' to do a dry run.\n");
		fprintf(stdout, "4. Check output file NlUninstallCleanup.log in current directory for error.\n");
		fprintf(stdout, "5. Enter 'NlUninstallCleanup -c <config-file> to perform cleanup.\n");
		fprintf(stdout, "6. Reboot you computer to complete removal of loaded driver.\n");
		fprintf(stdout, "7. Repeat steps 1 and 4 to cleanup remaining files or check if system is clean.\n");
		fprintf(stdout, "\n");
}
// main routine
int _tmain(int argc, _TCHAR* argv[])
{
	fprintf(stdout, "\nNextLabs Installation Cleanup Utility v 0.1 (Built %s %s)\n", __DATE__, __TIME__);

	int argc_i = 1;
	if( argc < 3 || argc > 4 
		|| ((argc == 2)&& ( (wcscmp(argv[1], L"-h") == 0) || (wcscmp(argv[1], L"/h") == 0) || (wcscmp(argv[1], L"/?") == 0)) ))
	{
		Display_help();
		return 1;
	}
	//
	std::wstring filename;
    bool realDelete = true;
	int numErrors =0;

	if( argc >2 )
	{
		// read command line options
		while( argc_i < argc )
		{
			if( wcscmp(argv[argc_i], L"-c") == 0)
			{
				 filename = wstring(argv[argc_i+1]);
				 argc_i += 2;
			} else 
			if( wcscmp(argv[argc_i], L"-n") == 0)
			{	 realDelete = false;
				 argc_i++;
			} else
				 argc_i++;
		}
	}

	// define log file
	FILE *logFile;
	int errorCode2 = _wfopen_s (&logFile,L"NlUninstallCleanup.log",L"w");
	if (logFile == NULL)
	{
		printf("\nERROR: Create log file NlUninstallCleanup.log failed (error code %d)\n", errorCode2);
		exit(1);
	}
	ConfigFile configfile(logFile, realDelete);

	int errorCode;
	if( (errorCode = configfile.read_config_file(filename, logFile, itemsList)) != 0 )
	{
		wprintf(L"\nERROR: Read configuration file %s failed (eError code %d)\n", filename.c_str(), errorCode);
		exit(errorCode);
	};
	// define mode: simulation, or real deletion

#ifdef TEST_DETAIL
	// testing with detailed info
	printf("\n file name : %s ", filename.c_str());
	test_tmain(argc, argv, logFile, realDelete);
    return 0;
#endif

	// delete services
	ServiceCleanup cleanservice(logFile, realDelete);
    numErrors += cleanservice.deleteItems(itemsList.services);

	// delete files and directories
	FileCleanup cleanfile(logFile, realDelete);
    numErrors +=  cleanfile.deleteItems(itemsList.filepaths);

	// delete registries
	RegistryCleanup cleanregistry(logFile, realDelete);
    numErrors +=  cleanregistry.deleteItems(itemsList.registries);

	fprintf(logFile, "\nTotal number of removal errors = %d\n", numErrors);
	printf("\nTotal number of removal errors = %d\n", numErrors);

	return numErrors;
}