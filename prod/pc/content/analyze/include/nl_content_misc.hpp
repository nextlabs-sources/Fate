/*******************************************************************************
 *
 * NextLabs Content Analysis Misc Functions
 *
 * This file provides a couple of miscellaneous functions used by various parts of
 * the CA code.
 *
 */

#ifndef __NL_CONTENT_ANALYSIS_MISC_HPP__
#define __NL_CONTENT_ANALYSIS_MISC_HPP__

#include <KtmW32.h>
#include "eframework/platform/windows_file_server_enforcer.hpp"

namespace NLCA
{
    typedef HANDLE (APIENTRY* CreateTransactionPtr) (IN LPSECURITY_ATTRIBUTES lpTransactionAttributes OPTIONAL,
                                                     IN LPGUID UOW OPTIONAL,
                                                     IN DWORD CreateOptions OPTIONAL,
                                                     IN DWORD IsolationLevel OPTIONAL,
                                                     IN DWORD IsolationFlags OPTIONAL,
                                                     IN DWORD Timeout OPTIONAL,
                                                     _In_opt_ LPWSTR Description);

    CreateTransactionPtr CreateTransactionFn = NULL;

    typedef BOOL (APIENTRY* CopyFileTransactedPtr) (IN LPCWSTR lpExistingFileName,
                                                    IN LPCWSTR lpNewFileName,
                                                    _In_opt_ LPPROGRESS_ROUTINE lpProgressRoutine,
                                                    _In_opt_ LPVOID lpData,
                                                    _In_opt_ LPBOOL pbCancel,
                                                    IN DWORD dwCopyFlags,
                                                    IN HANDLE hTransaction);

    CopyFileTransactedPtr CopyFileTransactedFn = NULL;

    typedef BOOL (APIENTRY* CommitTransactionPtr) (IN HANDLE TransactionHandle);

    CommitTransactionPtr CommitTransactionFn = NULL;

    static void InitializeTransactionFunctions()
    {
        static bool initialized = false;
    
        if (initialized)
        {
            return;
        }

        HMODULE hKtmW32 = LoadLibraryW(L"KtmW32.dll");
        if (hKtmW32 != NULL)
        {
            CreateTransactionFn = (CreateTransactionPtr)GetProcAddress(hKtmW32, "CreateTransaction");

            CommitTransactionFn = (CommitTransactionPtr)GetProcAddress(hKtmW32, "CommitTransaction");
        }

        HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
        if (hKernel32 != NULL)
        {
            CopyFileTransactedFn = (CopyFileTransactedPtr) GetProcAddress(hKernel32, "CopyFileTransactedW");
        }

        if (CreateTransactionFn == NULL || CommitTransactionFn == NULL || CopyFileTransactedFn == NULL )
        {
            if (hKtmW32 != NULL)
            {
                FreeLibrary(hKtmW32);
            }

            CreateTransactionFn = NULL;
            CommitTransactionFn = NULL;
            CopyFileTransactedFn = NULL;
        }

        initialized = true;
    }

    /**
     * WFSE has a bug which causes a deadlock for some file operations.  One way around
     * this is to make a backup copy of the file before doing content analysis
     */
    static BOOL SafeCopyFile(const wchar_t *from, const wchar_t *to, BOOL failIfExists)
    {
        InitializeTransactionFunctions();

        // If we are WFSE and the transaction functions loaded correctly (we only need to
        // check one, it's all or nothing) then use the transaction API
        if (nextlabs::windows_fse::is_wfse_installed() && CreateTransactionFn != NULL)
        {
            HANDLE hTransaction = CreateTransactionFn(NULL, 0, 0, 0, 0, INFINITE, NULL);

            if (hTransaction != INVALID_HANDLE_VALUE)
            {
                BOOL ret = CopyFileTransactedFn(from, to,
                                                NULL,
                                                NULL,
                                                NULL,
                                                failIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0,
                                                hTransaction);
                CommitTransactionFn(hTransaction);
                CloseHandle(hTransaction);

                return ret;
            }
        }

        return CopyFile(from, to, failIfExists);
    }

    /**
     * Utility function used by ifilter cracker and text cracker to make a tmp
     * copy
     */
    bool CopyFileToTemp(std::wstring &file_name)
    {
        wchar_t temp_path[MAX_PATH] = { 0 };
        DWORD temp_path_len = 0;
        
        temp_path_len = GetTempPath(_countof(temp_path),temp_path);
        if( temp_path_len > 0 && temp_path_len < _countof(temp_path) )
        {
            wchar_t temp_file[MAX_PATH] = { 0 };
            if( GetTempFileName(temp_path,L"NLCA",0,temp_file) != 0 )
            {
                // We aren't actually going to use this file
                DeleteFile(temp_file);
                
                /* Append true extension to the temporary file name */
                std::wstring new_file_name;
                new_file_name = temp_file;
                std::wstring::size_type i = file_name.find_last_of(L'.'); // offset to ".[extension]"
                if( i != std::wstring::npos )
                {
                    new_file_name.append(file_name,i,file_name.length() - i);
                }
                
                /* Copy existing file to temporary file */
                if( SafeCopyFile(file_name.c_str(),new_file_name.c_str(),TRUE) != FALSE )
                {
                    file_name = new_file_name;                              // accept new file name

                    return true;
                }
            }
        }

        return false;
    }

    
}
#endif
