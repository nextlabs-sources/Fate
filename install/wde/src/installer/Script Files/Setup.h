#ifndef SETUP_HEADER 
#define SETUP_HEADER    

    /*
	 *  Validate Administration Password
	 */
	export prototype INT ValidateAdministrationPassword(HWND);

        /* 
         *  Start EDP Manager
         */
    export prototype INT StartEDPManagerSetup(HWND);
	export prototype INT StartEDPManager(HWND);
	export prototype INT StartProcDetect(HWND);
	export prototype INT CreateProcDetectService(HWND);

        /* 
         *  Stop EDP Manager
         */
	export prototype INT StopEDPManager(HWND);  
	export prototype INT DeleteProcDetectService(HWND);
	export prototype INT ConfigureDesktopEnforcer(HWND);
	export prototype INT UnconfigureDesktopEnforcer(HWND);
	export prototype INT ClearVerboseRegistryEntry(HWND);  
	
	export prototype INT checkAndDelete(HWND, STRING);  
	export prototype INT DeleteRegistry(HWND);    
	
	export prototype INT InsertDeleteInRunonce(HWND);
	export prototype INT InsertDateToRegistry(HWND);
	export prototype INT AddFirewallExceptionList(HWND);   

#endif
