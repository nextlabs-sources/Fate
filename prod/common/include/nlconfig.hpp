
#ifndef __NLCONFIG_HPP__
#define __NLCONFIG_HPP__

#include <windows.h>
#include <string>
#include <cassert>

/** NLConfig
*
*  \brief Simple interface for NextLabs software runtime configuration.
*/
class NLConfig
{
public:

    /** IsDebugMode
    *
    *  \brief Determine if the system is in a debug mode state.
    *  \return true if debug mode is on, otherwise false.
    *
    *  \note The key 'DebugMode' (DWORD) under 'SOFTWARE\NextLabs'
    *        contrils behavior for binaries build in release mode.  When 'DebugMode' is
    *        non-zero IsDebugMode() will return true.
    */
    static bool IsDebugMode(void) throw()
    {
#if defined(DEBUG) || defined(_DEBUG)
        return true;
#else
		int debug_mode = 0;
		return ReadKey(L"SOFTWARE\\NextLabs\\DebugMode",&debug_mode) && debug_mode != 0;
#endif
    }/* IsDebugMode */

    static bool ReadKey( const wchar_t* in_key , int* in_value )
    {
        return ReadKey(in_key,(void*)in_value,sizeof(int));
    }

    static bool ReadKey( const wchar_t* in_key , wchar_t* in_value , size_t in_value_count )
    {
        return ReadKey(in_key,(void*)in_value,in_value_count*sizeof(wchar_t));
    }

    /** ReadKey
    *
    *  \brief Read a configuration key.
    *
    *  \param in_key (in)   NULL terminated string with full name of key to read.
    *  \param in_value (in) Point to store value of type DWORD.
    *
    *  \return true if the value of the key was successfully read, otherwise false.
    */
    static bool ReadKey( const wchar_t* in_key , void* in_value , size_t in_value_size)
    {
        return ReadKey(in_key, in_value, in_value_size, KEY_QUERY_VALUE);
    }
    static bool ReadKey( const wchar_t* in_key , void* in_value , size_t in_value_size, REGSAM samDesired)
    {
        assert( in_key != NULL );
        assert( in_value != NULL );
        if( in_key == NULL || in_value == NULL )
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

        /* Parse out key path and key name */
        std::wstring key_path(in_key);
        std::wstring::size_type i = key_path.find_last_of(L"\\//");
        if( i == std::wstring::npos )
        {
            return false;
        }
        std::wstring key_name(key_path,i+1); // capture key name
        key_path.erase(i);                   // trim tail which includes key name.

        HKEY hKey = NULL;
        LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0, samDesired, &hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }

        DWORD result_size = (DWORD)in_value_size;
        rstatus = RegQueryValueExW(hKey,key_name.c_str(),NULL,NULL,(LPBYTE)in_value,&result_size);
        RegCloseKey(hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }
        SetLastError(ERROR_SUCCESS);
        return true;
    }/* ReadKey */

    static bool WriteKey( const wchar_t* in_key , int in_value )
    {
        assert( in_key != NULL );
        if( in_key == NULL )
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

        /* Parse out key path and key name */
        std::wstring key_path(in_key);
        std::wstring::size_type i = key_path.find_last_of(L"\\//");
        if( i == std::wstring::npos )
        {
            return false;
        }
        std::wstring key_name(key_path,i+1); // capture key name
        key_path.erase(i);                   // trim tail which includes key name.

        HKEY hKey = NULL;
        LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0,KEY_SET_VALUE,&hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }

        rstatus = RegSetValueExW(hKey,key_name.c_str(),0,REG_DWORD,(const BYTE*)&in_value,sizeof(DWORD));
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }
        return true;
    }/* WriteKey */

    static bool WriteKey( const wchar_t* in_key , const wchar_t* in_value , size_t in_value_count )
    {
        assert( in_key != NULL );
        if( in_key == NULL )
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

        /* Parse out key path and key name */
        std::wstring key_path(in_key);
        std::wstring::size_type i = key_path.find_last_of(L"\\//");
        if( i == std::wstring::npos )
        {
            return false;
        }
        std::wstring key_name(key_path,i+1); // capture key name
        key_path.erase(i);                   // trim tail which includes key name.

        HKEY hKey = NULL;
        LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0,KEY_SET_VALUE,&hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }

        rstatus = RegSetValueExW(hKey,key_name.c_str(),0,REG_SZ,(const BYTE*)in_value,(DWORD)in_value_count*sizeof(wchar_t));
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }
        return true;
    }/* WriteKey */

    /** GetComponentInstallPath
    *
    *  \brief Get the install root for a component (i.e. Policy Controller)
    *
    *  \param in_component (in)  Component to get install path for.  When this parameter
    *                            is NULL the install path is the NextLabs root install path.
    *  \param in_path (out)      Path component is installed at.
    *  \param in_path_count (in) Size in chars of parameter in_path.
    *
    *  \return true if successful, otherwise false.  Use GetLastError() for
    *          more information.
    *
    *  \note When GetComponentInstallPath fails the value of in_path is undefined.  Parameter
    *        in_path will always be NULL terminated.
    */
    _Check_return_ static bool GetComponentInstallPath( _In_opt_ const wchar_t* in_component ,
        _Out_cap_(in_path_count) wchar_t* in_path ,
        _In_ size_t in_path_count )
    {
        assert( in_path != NULL );
        if( in_path == NULL )
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return false;
        }

        /*******************************************************************************************
        * Policy Controller is a special case for component install path.  The follow keys are
        * defined:
        *
        *   InstallDir = c:\Program Files\NextLabs\
        *   PolicyControllerDir = c:\Program Files\NextLabs\Policy Controller
        *
        ******************************************************************************************/

        std::wstring key_path(L"SOFTWARE\\NextLabs\\"); // NextLabs root
        if( in_component != NULL )
        {
            key_path.append(in_component);              /* Real component */
        }
        else
        {
            key_path.append(L"Policy Controller");      /* NextLabs install path via Policy Controller */
        }
        HKEY hKey = NULL;
        LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key_path.c_str(),0,KEY_QUERY_VALUE,&hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false;
        }

        const wchar_t* key_name = L"InstallDir";
        if( in_component != NULL && _wcsicmp(in_component,L"Compliant Enterprise\\Policy Controller") == 0 )
        {
            key_name = L"PolicyControllerDir";
        }

        DWORD path_size = (DWORD)in_path_count * sizeof(wchar_t);
        rstatus = RegQueryValueExW(hKey,key_name,NULL,NULL,(LPBYTE)in_path,&path_size);
        RegCloseKey(hKey);
        if( rstatus != ERROR_SUCCESS )
        {
            return false; // last error for RegQueryValueEx is used
        }
        SetLastError(ERROR_SUCCESS);
        return true;
    }/* GetComponentInstallPath */

};/* NLConfig */

#endif /* __NLCONFIG_HPP__*/
