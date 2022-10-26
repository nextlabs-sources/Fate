##########################################################################
#
# Policy Controller Plug-in Framework
#
##########################################################################

The Policy Controller Plug-in Framework is a plug&play solution to extend policy controller's services. It allows policy controller to load additional modules that perform some services (e.g. tamper resistance) other than build-in services (e.g. evaluation). Besides, an enforcer (e.g. Removable Device Enforcer) can use this framework to invoke its functionality within policy controller process's context. Thus, its enforcement will stay with the running life of policy controller. 

plug-in configuration file
---------------------------
Starting from policy controller 4.0, in order to load plug-in library(s), what need to be done is,
1. Create a ANSI text type file under the folder "Policy Controller\config\plugin".
2. Add the library(s) with the path line by line. You can use the maro [NextLabs] in your path. For example, for removable device enforcer, it is sepcified as,
[NextLabs]\Removable Device Enforcer\bin\nl_devenf_plugin.dll  

For policy controller 3.5, this is done through a build-in table of policy controller. 

plug-in DLL
-----------
Every plug-in DLL needs to export the following functions which are called 
by the policy controller. The function "PluginEntry" will be called when policy controller starts running; the function "PluginUnload" will be called when policy controller is going to stop. 

/** PluginEntry
 *
 *  Entry point where plug-in required resources can be claimed.  The
 *  context parameter permits the plug-in to store plug-in defined
 *  information which is provided when the PluginUnload callback is
 *  called.
 */
extern "C" int PluginEntry( void** context )

/** PluginUnload
 *
 *  Entry point where a plug-in required to release resources that were
 *  claimed in the Policy Controller's call to PluginEntry.  The context
 *  parameter is given as the content to PluginEntry's context which may
 *  have been set by the plug-in.
 */
extern "C" int PluginUnload( void* context )

plugin Example
--------------
The plugin_example.dll demonstrates how a plug-in can be written.  

plugin tester 
-------------
The executable plugin_driver.exe allows you to test your plug-in before integrate with policy controller. It will load and call PluginEntry for a given plug-in library.  It waits for console input and then calls PluginUnload. The syntax is, 
  plugin_driver.exe <plugin-library with full path>

This is a very useful tool for plug-in libary running with policy controller 3.5.


