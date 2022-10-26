#ifndef SETUP_HEADER 
#define SETUP_HEADER    

    /*
	 *  Validate Administration Password
	 */
	export prototype INT ValidateAdministrationPassword(HWND);
	export prototype INT BackupKeystoreFiles(HWND);
    export prototype INT RestoreKeystoreFiles(HWND);      
    export prototype INT InsertDateToRegistry(HWND);
    
    
#endif