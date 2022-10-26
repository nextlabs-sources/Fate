#ifndef SERVICE_CONTROL_HEADER
#define SERVICE_CONTROL_HEADER 
       
    #define PDP_CONTROLLER_DLL "PDPStop.dll"
       
	//Possible return code from StopService
	#define ERROR_INCORRECT_PASSWORD -8
	#define ERROR_AGENT_NOT_STOPPED 2
	#define ERROR_AGENT_NOT_RUNNING 3
	#define ERROR_AGENT_UNKOWN 4 
    #define ERROR_SDK_CONNECTION_FAILED -2

    #define UNINSTALL_PASSWORD_PROP_NAME "UNINSTALL_PASSWORD"
    #define NEED_AGENT_STOP_PROPERTY_NAME "_NeedAgentStop"

    /*
	 *  Stop Enforcer Service
	 */
	export prototype INT StopService(HWND); 
	
	/*
	 *  Start Enforcer Service
	 */
	export prototype INT StartService(HWND);

	//////////////////////////////////////////////////////////////////////////////
	// Function: IsServiceInstalled
	//
	// This function returns TRUE if the service is installed, FALSE otherwise
	//////////////////////////////////////////////////////////////////////////////	           
	export prototype BOOL IsServiceInstalled(HWND);

	//////////////////////////////////////////////////////////////////////////////
	// Function: IsServiceRunning
	//
	// This function returns TRUE if the service is running, FALSE otherwise
	//////////////////////////////////////////////////////////////////////////////	                                     
	export prototype BOOL IsServiceRunning(HWND);  
	
	//////////////////////////////////////////////////////////////////////////////
	// Function: IsCurrentuserSystem
	//
	// This function returns TRUE if the installer is run from the system user, 
	// FALSE otherwise
	//
	//  FIX ME - This should be in the common code
	//////////////////////////////////////////////////////////////////////////////
	prototype BOOL IsCurrentUserSystem(HWND);   
	

	//////////////////////////////////////////////////////////////////////////////
	// Function: StopServiceWithPassword
	//
	// This function shuts down the agent with a challenge and a shared secret
	/////////////////////////////////////////////////////////////////////////////
	prototype INT StopServiceWithPassword(HWND);

	//////////////////////////////////////////////////////////////////////////////
	// Function: StopServiceWithChallenge
	//
	// This function shuts down the agent with a challenge and a shared secret
	/////////////////////////////////////////////////////////////////////////////
	prototype INT StopServiceWithChallenge(HWND);  
	
	//////////////////////////////////////////////////////////////////////////////
	// Function: LoadAgentControllerDLL
	//
	// This function loads the agent controller DLL.
	// Returns ERROR_SUCCESS if succeeded, -1 otherwise
	//////////////////////////////////////////////////////////////////////////////	                                            
	prototype INT LoadAgentControllerDLL(HWND); 
	
	//////////////////////////////////////////////////////////////////////////////
	// Function: UnloadAgentControllerDLL
	//
	// This function unloads the agent controller DLL.
	// Returns ERROR_SUCCESS if succeeded, -1 otherwise
	//////////////////////////////////////////////////////////////////////////////
	prototype INT UnloadAgentControllerDLL(HWND);     

	prototype INT GetPolicyControllerInstallDir(HWND, BYREF STRING);

	//External DLL functions
	prototype INT PDPStop.stopAgentService(BYREF WSTRING);

#endif
