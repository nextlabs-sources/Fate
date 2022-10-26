#ifndef POLICY_CONTROLLER_HEADER
#define POLICY_CONTROLLER_HEADER 
  
  	/*
  	 * Constant for ICENet location property name
  	 */
 	#define ICENET_SERVER_LOCATION_PROP_NAME "ICENET_SERVER_LOCATION"
 	         
 	/*
 	 *  Property name for performance improvement.  Only discover servers once
 	 */
 	#define NEED_DISCOVERY_PROP_NAME "NEED_ICENET_LOCATION_DISCOVERY"
 	 
 	/*
 	 * Text constants.  Because merge modules can't be i18n'd, define label here 	 
 	 */                                                                         
 	#define IDS_ICENET_LOCATION_LABEL "ICENet Server location"
 	#define ERR_INVALID_ICENET_SERVER_LOCATION_MESSAGE "The Icenet server location you specified does not exist or does not seem to be running now. Do you want to keep this value? \nTo confirm your input, please click Yes, otherwise please click No."
 	#define ERR_INVALID_PORT_NUMBER_MESSAGE "The port number you have entered is invalid. Please enter a valid port number."
 

	#define CONFIG_PATH "config" 

 	/*
 	 * Constants for creating and configuring comm profile
 	 */	   
	#define COMM_PROFILE_FILE "config\\commprofile.xml" 
	#define ICENET_HOST_TOKEN "[ICENET_HOST]"
	#define ICENET_PORT_TOKEN "[ICENET_PORT]"                           
	                                                   
 	/*
 	 * Constants for creating and configuring online help
 	 */	   
	#define ONLINE_HELP_INDEX_FILE "help\\Index.html"
	#define ONLINE_HELP_INFORMATION_FILE "help\\Information.html"
	#define ONLINE_HELP_NOTIFICATIONS_FILE "help\\Notifications.html"		                                                   
	/*
 	 * Constants for creating and logging properties file
 	 */	   
	#define LOGGING_PROPERTIES_FILE "config\\logging.properties" 
    #define BLUEJUNGLE_HOME_TOKEN "[BLUEJUNGLE_HOME]" 
       
    #define NEED_AGENT_STOP_PROPERTY_NAME "_NeedAgentStop"
    /*
	 *  Validate specified ICENet Server Location
	 */
	export prototype INT ValidateIcenetServerLocation(HWND);   

    /*
	 *  Request a password from the end user before uninstallation can take place
	 */
	export prototype INT ValidateUninstallPassword(HWND);
	 

 

	/*
	 * Start the Obligation Manager process
	 */                                  
	 export prototype INT StartObligationManager(HWND);
	 	 
	/*
	 * Stop the Obligation Manager process
	 */                                  
	 export prototype INT StopObligationManager(HWND); 
	 
	 /*
	  *  Retrieve the Policy Controller Directory
	  */                                         
	  export prototype STRING getPolicyControllerDirectory(STRING);
	  

#endif
