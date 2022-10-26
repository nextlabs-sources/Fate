#ifndef SETUP_FILES_HEADER
#define SETUP_FILES_HEADER 

#define COMMON_INSTALL_DLL_KEY "installer.dll"
#define COMMON_INSTALL_DLL_FILE FOLDER_TEMP ^ "installercommon.dll"     

#define ONLINE_HELP_INDEX_TEMPLATE_KEY "index.html.tmplte"
#define ONLINE_HELP_INDEX_TEMPLATE_FILE FOLDER_TEMP ^ "Index.template.html"
  
#define ONLINE_HELP_INFORMATION_TEMPLATE_KEY "information.html.tmplte"
#define ONLINE_HELP_INFORMATION_TEMPLATE_FILE FOLDER_TEMP ^ "Information.template.html"

#define ONLINE_HELP_NOTIFICATIONS_TEMPLATE_KEY "notifications.html.tmplte"
#define ONLINE_HELP_NOTIFICATIONS_TEMPLATE_FILE FOLDER_TEMP ^ "Notifications.template.html"

#define PDP_STOP_DLL_KEY "pdpstop.dll"
#define PDP_STOP_DLL_FILE FOLDER_TEMP ^ "PDPStop.dll"    

#define MCH_SETUP_KEY "mch_install_test64.exe"
#define MCH_SETUP_FILE FOLDER_TEMP ^ "mch_install_test.exe"

export prototype INT ExtractSetupFiles(HWND); 
export prototype INT InstallNLCC(HWND);
export prototype INT UninstallNLCC(HWND);
export prototype INT DisableServiceStartByName(HWND,STRING);
export prototype INT DisableServiceStart(HWND);    
export prototype INT EnableServiceStartByName(HWND,STRING,STRING);
export prototype INT EnablePCServiceStart(HWND);  
export prototype INT StartPCService(HWND);
export prototype INT DeleteServiceByName(HWND,STRING);
export prototype INT DeleteServices(HWND); 

export prototype INT CheckVersion(HWND, STRING, STRING);
export prototype INT CheckInstalled(HWND);   

export prototype INT MCH_install(HWND, STRING);
export prototype INT Uninstall_nlinjection(HWND);
export prototype INT Install_nlinjection(HWND);  

export prototype INT StopEDPManager(HWND);    
export prototype INT StartEDPManager(HWND); 

export prototype INT DeleteProcDetectService(HWND);
export prototype INT StartProcDetect(HWND);
export prototype INT DisableProcDetect(HWND);  
export prototype INT CreateProcDetectService(HWND);    
export prototype INT EnableWDEProcDetect(HWND);   

export prototype INT CreatePCService(HWND);     
export prototype INT RegisterPlugin(HWND);
export prototype INT UnregisterPlugin(HWND);    

export prototype INT NE_installSPI32(HWND);
export prototype INT NE_installSPI64(HWND);
export prototype INT NE_uninstallSPI32(HWND);
export prototype INT NE_uninstallSPI64(HWND);      

export prototype INT RDE_uninstalldriver(HWND);
export prototype INT RDE_installdriver(HWND);   

export prototype INT CheckEP(HWND, STRING);    

export prototype INT SE_unregisterIcon(HWND);   
export prototype INT SE_registerIcon(HWND);
export prototype INT SE_startService(HWND);    
export prototype INT WDE_disableEDP(HWND);    

export prototype INT PC_deleteUnused(HWND);  

export prototype INT StopPC(HWND);      
export prototype INT CheckServiceByName(HWND, STRING);
export prototype INT CheckService(HWND);

 
#endif
