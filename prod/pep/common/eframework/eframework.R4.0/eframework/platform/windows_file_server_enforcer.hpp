/***************************************************************************************************
 *
 * Determine if the WFSE is installed
 *
 * A copy of this code is used by //depot/external/podofo-0.8.1-NextLabs-WFSE/src/windows_file_server_enforcer.hpp, 
 * when you update this code, 
 * please make sure the change is reflected on the other copy.
 *
 **************************************************************************************************/

#ifndef __WINDOWS_FSE_HPP__
#define __WINDOWS_FSE_HPP__

#include <windows.h>

namespace nextlabs
{
	//	if the wfse is installed
	typedef enum
	{
		WFSE_INSTALLED_UNSET,
		WFSE_INSTALLED_NO,
		WFSE_INSTALLED_YES
	}WFSE_INSTALLED_STATUS;

	static WFSE_INSTALLED_STATUS wfse_installed = WFSE_INSTALLED_UNSET;


	class windows_fse
	{

	public:


		/** is_wfse_installed
		*
		*  \brief Determine if the Windows File Server Enforcer is installed.
		*  \return true if the Windows File Server Enforcer is installed, otherwise false.
		*/
		static bool is_wfse_installed(void) throw()
		{
			if(WFSE_INSTALLED_UNSET == wfse_installed)
			{
				//	read reg to determine if wfse is installed
				HKEY hKey = NULL; 
				LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
					"SYSTEM\\CurrentControlSet\\services\\NLFse",
					0,KEY_QUERY_VALUE,&hKey);

				if( result != ERROR_SUCCESS )
				{
					//	wfse_installed will be set to WFSE_INSTALLED_NO
					wfse_installed = WFSE_INSTALLED_NO;
					return false;
				}
				RegCloseKey(hKey);

				//	we have reg key "NLFse", then set wfse_installed to yes
				wfse_installed = WFSE_INSTALLED_YES;
				return true;
			}
			else if(wfse_installed == WFSE_INSTALLED_YES)
			{
				return true;
			}
			else
			{
				return false;
			}
		}/* is_wfse_installed */



	};/* windows_fse */

}/* namespace nextlabs */

#endif /* __WINDOWS_FSE_HPP__ */
