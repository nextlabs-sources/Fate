/***************************************************************************************************
 *
 * Determine if the RMC is installed
 *
 **************************************************************************************************/

#ifndef __RIGHTS_MANAGEMENT_CLIENT_HPP__
#define __RIGHTS_MANAGEMENT_CLIENT_HPP__

#include <windows.h>

namespace nextlabs
{
	//	if the RMC is installed
	typedef enum
	{
		RMC_INSTALLED_UNSET,
		RMC_INSTALLED_NO,
		RMC_INSTALLED_YES
	}RMC_INSTALLED_STATUS;

	static RMC_INSTALLED_STATUS rmc_installed = RMC_INSTALLED_UNSET;


	class rmc
	{

	public:


		/** is_rmc_installed
		*
		*  \brief Determine if the Rights Management Client is installed.
		*  \return true if the Rights Management Client is installed, otherwise false.
		*/
		static bool is_rmc_installed(void) throw()
		{
			if(RMC_INSTALLED_UNSET == rmc_installed)
			{
				//	read reg to determine if RMC is installed
				HKEY hKey = NULL; 
				LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
					"SYSTEM\\CurrentControlSet\\services\\NLSysEncryption",
					0,KEY_QUERY_VALUE,&hKey);

				if( result != ERROR_SUCCESS )
				{
					//	rmc_installed will be set to RMC_INSTALLED_NO
					rmc_installed = RMC_INSTALLED_NO;
					return false;
				}
				RegCloseKey(hKey);

				//	we have reg key "NLSysEncryption", then set rmc_installed to yes
				rmc_installed = RMC_INSTALLED_YES;
				return true;
			}
			else if(rmc_installed == RMC_INSTALLED_YES)
			{
				return true;
			}
			else
			{
				return false;
			}
		}/* is_rmc_installed */



	};/* rmc */

}/* namespace nextlabs */

#endif /* __RIGHTS_MANAGEMENT_CLIENT_HPP__ */
