/***************************************************************************************************
 *
 * Determine if the RMS is installed
 *
 **************************************************************************************************/

#ifndef __RIGHTS_MANAGEMENT_SERVER_HPP__
#define __RIGHTS_MANAGEMENT_SERVER_HPP__

#include <windows.h>

namespace nextlabs
{
	//	if the RMS is installed
	typedef enum
	{
		RMS_INSTALLED_UNSET,
		RMS_INSTALLED_NO,
		RMS_INSTALLED_YES
	}RMS_INSTALLED_STATUS;

	static RMS_INSTALLED_STATUS rms_installed = RMS_INSTALLED_UNSET;


	class rms
	{

	public:


		/** is_rms_installed
		*
		*  \brief Determine if the Rights Management Server is installed.
		*  \return true if the Rights Management Server is installed, otherwise false.
		*/
		static bool is_rms_installed(void) throw()
		{
			if(RMS_INSTALLED_UNSET == rms_installed)
			{
				//	read reg to determine if RMS is installed
				HKEY hKey = NULL; 
				LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
					"SYSTEM\\CurrentControlSet\\services\\Rights Management Server",
					0,KEY_QUERY_VALUE,&hKey);

				if( result != ERROR_SUCCESS )
				{
					//	rms_installed will be set to RMS_INSTALLED_NO
					rms_installed = RMS_INSTALLED_NO;
					return false;
				}
				RegCloseKey(hKey);

				//	we have reg key "NLSysEncryption", then set rms_installed to yes
				rms_installed = RMS_INSTALLED_YES;
				return true;
			}
			else if(rms_installed == RMS_INSTALLED_YES)
			{
				return true;
			}
			else
			{
				return false;
			}
		}/* is_rms_installed */



	};/* rms */

}/* namespace nextlabs */

#endif /* __RIGHTS_MANAGEMENT_SERVER_HPP__ */
