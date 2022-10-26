#ifndef SETUP_HEADER 
#define SETUP_HEADER    

    /*
	 *  Validate Administration Password
	 */
	export prototype INT ValidateAdministrationPassword(HWND);
	export prototype INT ModifyRegistry(HWND);
    export prototype INT RemoveRunOnceRegistry(HWND);
    export prototype INT ConfigureNLSE(HWND);
    export prototype INT UnconfigureNLSE(HWND);   
    export prototype INT InsertDateToRegistry(HWND);      
    export prototype INT StartSEServices(HWND);
    export prototype INT backupFiles(HWND);
    export prototype INT RestoreBackup(HWND);
    
#endif
