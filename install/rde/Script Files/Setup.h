#ifndef SETUP_HEADER 
#define SETUP_HEADER    
  
  	#define ENFORCER_DEVICE_DRIVER_INSTALLER_DLL_NAME "nlPnpDriverInstaller.dll"
    #define ENFORCER_DEVICE_DRIVER_HARDWARD_ID "root\\NLDevEnf"
    
    /*
	 *  Validate Administration Password
	 */
	export prototype INT ValidateAdministrationPassword(HWND);
	
	/*
	 * Install driver
	 */              
	export prototype INT InstallEnforcerDeviceDriver(HWND);
	
	/*
	 * Uninstall driver
	 */
	export prototype INT UninstallEnforcerDeviceDriver(HWND);
	          
	/**
	 * Load enforcerDeviceDriverInstall DLL
	 */
	export prototype INT LoadEnforcerDeviceDriverInstaller(HWND);
	
	/**
	 * Unload enforcerDeviceDriverInstall DLL
	 */
	export prototype INT UnloadEnforcerDeviceDriverInstaller(HWND);
	
	/*
	 *  DLL function to install/uninstall driver
	 */                                         
	export prototype cdecl INT nlPnpDriverInstaller.nlPnpDriverInstall(BOOL, BYVAL WSTRING, BYVAL WSTRING);	
	                                             
	export prototype INT deleteKey(HWND, STRING);
	                                         
	export prototype INT clearRegistry(HWND);
	
	export prototype INT InsertDeleteInRunonce(HWND);       
	export prototype INT DeleteKeyByControl(HWND,STRING,STRING);    
	export prototype INT InsertDateToRegistry(HWND);
	
	                                       
#endif