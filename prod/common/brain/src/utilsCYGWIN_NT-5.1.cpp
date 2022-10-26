// This is the win32 abstraction layer for the brain
//
// All sources, binaries and HTML pages (C) copyright 2007 by NextLabs Inc, 
// Redwood City CA, Ownership remains with NextLabs Inc, 
// All rights reserved worldwide. 
// 
// Author : Dominic Lam
// Date   : 1/30/2007
// Note   : A collection of utilities for Win32
//          
//
// Best viewed by 80-column terminal
// <-------------------------------------------------------------------------->

#include <windows.h>
#include <Sddl.h>
#include <jni.h>
#include <imagehlp.h>
#include <Aclapi.h>
#include "brain.h"
#include "sha1.h"
#include <Lmshare.h>
#include <Lm.h>
#include <WtsApi32.h>
#include <stdio.h>
#define SECURITY_WIN32
#include <security.h>
#include <ShlObj.h>
#include <RegStr.h>
#include <Windows.h>
#include <process.h>
#include "nlDevice.h"

using namespace std;

namespace {
#define USER_FOLDER_KEY _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define MY_DOCUMENTS_VALUE_NAME _T("Personal")
#define MY_DESKTOP_VALUE_NAME _T("Desktop")
#define CD_BURNING_NAME _T("CD Burning")
#define SZ_EVENT_VIEWER_SOURCE _T("Compliant Enterprise")

    //Some constants
    enum {FILE_ACCESS_TIME = 1, FILE_CREATE_TIME = 2,
          FILE_MODIFICATION_TIME = 4};

    //A global variable scoped in this file only
    DeviceDetc deviceDetc; //handling device driver


    static bool IsPrintableString(const nlchar *str)
    {
        size_t i;

        if (str == NULL)
            return false;

        for (i = 0; i < nlstrlen(str); i++)
        {
            if (_istprint(str[i]) == 0)
            {
                return false;
            }
        }

        return true;
    }


    typedef
    DWORD
    (WINAPI * Kernel32_WTSGetActiveConsoleSessionId)();
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    BOOL GetConsoleSessionId(int pid, PDWORD pdwSessionId)
    {
        static HMODULE hKernel32 = LoadLibraryA("Kernel32.dll");
        static Kernel32_WTSGetActiveConsoleSessionId _WTSGetActiveConsoleSessionId = (hKernel32 == NULL) ? NULL : (Kernel32_WTSGetActiveConsoleSessionId)GetProcAddress( hKernel32,"WTSGetActiveConsoleSessionId");

        if (pid == 0)
        {
            // This will work okay on pre-Vista systems, but not on Vista
            // and Windows 7.  On those systems the PC service runs in its
            // own session and we can't get information about the user session
            // from that.  That's not true for XP, so using the PC pid will
            // work for that and it certainly isn't *worse* on Vista/Win7
            pid = GetCurrentProcessId();
        }

        if(ProcessIdToSessionId(pid, pdwSessionId))
        {
            if(_WTSGetActiveConsoleSessionId)   // XP, 2k3, and later systems
            {
                if(*pdwSessionId == _WTSGetActiveConsoleSessionId())
                {
                    return FALSE;
                }
            }
            else                                // Win2k
            {
                if(*pdwSessionId == 0)
                {
                    return FALSE;
                }
            }
        }
        else
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL GetSessionProtocolType(DWORD dwSessionId, USHORT* pusProtocolType)
    {
        LPTSTR pBuffer = NULL;
        DWORD  BytesReturned;
        BOOL   Ret;
        Ret = WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
        dwSessionId,
        WTSClientProtocolType,
        &pBuffer,
        &BytesReturned
            );
        if(Ret && pBuffer)
        {
            *pusProtocolType = *((USHORT*)(pBuffer));
            WTSFreeMemory(pBuffer);
            return TRUE;
        }

        return FALSE;
    }

    BOOL GetSessionRemoteIp(DWORD dwSessionId, PDWORD pdwIp)
    {
        LPTSTR pBuffer = NULL;
        DWORD  BytesReturned;
        BOOL   Ret;

        Ret = WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
        dwSessionId,
        WTSClientAddress,
        &pBuffer,
        &BytesReturned
            );
        if(Ret && pBuffer)
        {
            PWTS_CLIENT_ADDRESS Address = (PWTS_CLIENT_ADDRESS)pBuffer;
            *pdwIp   = ((Address->Address[2]<<24)&0xFF000000) | ((Address->Address[3]<<16)&0xFF0000) | ((Address->Address[4]<<8)&0xFF00) | Address->Address[5];
            WTSFreeMemory(pBuffer); pBuffer = NULL;
            return TRUE;
        }

        return FALSE;
    }

    /* Copy the SHA1 code from 
     * main/src/platform/win32/agent/IPC/IPCJNI/oswrapper.cpp
     * in order to get fingerprint for SDK on Windows platform
     *****************************************************************************
     *
     *  Description:
     *      This file implements the Secure Hashing Standard as defined
     *      in FIPS PUB 180-1 published April 17, 1995.
     *
     *      The Secure Hashing Standard, which uses the Secure Hashing
     *      Algorithm (SHA), produces a 160-bit message digest for a
     *      given data stream.  In theory, it is highly improbable that
     *      two messages will produce the same message digest.  Therefore,
     *      this algorithm can serve as a means of providing a "fingerprint"
     *      for a message.
     *
     *  Portability Issues:
     *      SHA-1 is defined in terms of 32-bit "words".  This code was
     *      written with the expectation that the processor has at least
     *      a 32-bit machine word size.  If the machine word size is larger,
     *      the code should still function properly.  One caveat to that
     *      is that the input functions taking characters and character
     *      arrays assume that only 8 bits of information are stored in each
     *      character.
     *
     *  Caveats:
     *      SHA-1 is designed to work with messages less than 2^64 bits
     *      long. Although SHA-1 allows a message digest to be generated for
     *      messages of any number of bits less than 2^64, this
     *      implementation only works with messages with a length that is a
     *      multiple of the size of an 8-bit character.
     *
     */

    /*
     *  Define the circular shift macro
     */

    extern "C" {
#define SHA1CircularShift(bits,word) \
                ((((word) << (bits)) & 0xFFFFFFFF) | \
                ((word) >> (32-(bits))))

        /* Function prototypes */
        void SHA1ProcessMessageBlock(SHA1Context *);
        void SHA1PadMessage(SHA1Context *);

        /*  
         *  SHA1Reset
         *
         *  Description:
         *      This function will initialize the SHA1Context in preparation
         *      for computing a new message digest.
         *
         *  Parameters:
         *      context: [in/out]
         *          The context to reset.
         *
         *  Returns:
         *      Nothing.
         *
         *  Comments:
         *
         */
        void SHA1Reset(SHA1Context *context)
        {
            context->Length_Low             = 0;
            context->Length_High            = 0;
            context->Message_Block_Index    = 0;

            context->Message_Digest[0]      = 0x67452301;
            context->Message_Digest[1]      = 0xEFCDAB89;
            context->Message_Digest[2]      = 0x98BADCFE;
            context->Message_Digest[3]      = 0x10325476;
            context->Message_Digest[4]      = 0xC3D2E1F0;

            context->Computed   = 0;
            context->Corrupted  = 0;
        }

        /*  
         *  SHA1Result
         *
         *  Description:
         *      This function will return the 160-bit message digest into the
         *      Message_Digest array within the SHA1Context provided
         *
         *  Parameters:
         *      context: [in/out]
         *          The context to use to calculate the SHA-1 hash.
         *
         *  Returns:
         *      1 if successful, 0 if it failed.
         *
         *  Comments:
         *
         */
        int SHA1Result(SHA1Context *context)
        {

            if (context->Corrupted)
            {
                return 0;
            }

            if (!context->Computed)
            {
                SHA1PadMessage(context);
                context->Computed = 1;
            }

            return 1;
        }

        /*  
         *  SHA1Input
         *
         *  Description:
         *      This function accepts an array of octets as the next portion of
         *      the message.
         *
         *  Parameters:
         *      context: [in/out]
         *          The SHA-1 context to update
         *      message_array: [in]
         *          An array of characters representing the next portion of the
         *          message.
         *      length: [in]
         *          The length of the message in message_array
         *
         *  Returns:
         *      TRUE on success, FALSE on failure (per windows DIGEST_FUNCTION requirements).
         *
         *  Comments:
         *
         */
        BOOL WINAPI SHA1Input(     PVOID win_Context,
                                   PBYTE win_message_array,
                                   DWORD win_length)
        {

            SHA1Context         *context = (SHA1Context *) win_Context;
            const unsigned char *message_array = (const unsigned char *) win_message_array;
            unsigned            length = (unsigned) win_length;

            /* length in (0,Message_Block elements) - Bound check */
            if (length == 0) 
            {
                return FALSE;
            }

            if (context->Computed || context->Corrupted)
            {
                context->Corrupted = 1;
                return FALSE;
            }

            while(length-- && !context->Corrupted)
            {
#pragma warning (push)
#pragma warning (disable:6386)
                context->Message_Block[context->Message_Block_Index++] =
                    (*message_array & 0xFF);
#pragma warning (pop)

                context->Length_Low += 8;
                /* Force it to 32 bits */
                context->Length_Low &= 0xFFFFFFFF;
                if (context->Length_Low == 0)
                {
                    context->Length_High++;
                    /* Force it to 32 bits */
                    context->Length_High &= 0xFFFFFFFF;
                    if (context->Length_High == 0)
                    {
                        /* Message is too long */
                        context->Corrupted = 1;
                    }
                }

                if (context->Message_Block_Index == 64)
                {
                    // This function resets Message_Block_Index to 0
                    SHA1ProcessMessageBlock(context);
                }

                message_array++;
            }
            return TRUE;
        }

        /*  
         *  SHA1ProcessMessageBlock
         *
         *  Description:
         *      This function will process the next 512 bits of the message
         *      stored in the Message_Block array.
         *
         *  Parameters:
         *      None.
         *
         *  Returns:
         *      Nothing.
         *
         *  Comments:
         *      Many of the variable names in the SHAContext, especially the
         *      single character names, were used because those were the names
         *      used in the publication.
         *         
         *
         */
        void SHA1ProcessMessageBlock(SHA1Context *context)
        {
            const unsigned K[] =            /* Constants defined in SHA-1   */      
                {
                    0x5A827999,
                    0x6ED9EBA1,
                    0x8F1BBCDC,
                    0xCA62C1D6
                };
            int         t;                  /* Loop counter                 */
            unsigned    temp;               /* Temporary word value         */
            unsigned    W[80];              /* Word sequence                */
            unsigned    A, B, C, D, E;      /* Word buffers                 */

            /*
             *  Initialize the first 16 words in the array W
             */
            for(t = 0; t < 16; t++)
            {
                W[t] = ((unsigned) context->Message_Block[t * 4]) << 24;
                W[t] |= ((unsigned) context->Message_Block[t * 4 + 1]) << 16;
                W[t] |= ((unsigned) context->Message_Block[t * 4 + 2]) << 8;
                W[t] |= ((unsigned) context->Message_Block[t * 4 + 3]);
            }

            for(t = 16; t < 80; t++)
            {
                W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
            }

            A = context->Message_Digest[0];
            B = context->Message_Digest[1];
            C = context->Message_Digest[2];
            D = context->Message_Digest[3];
            E = context->Message_Digest[4];

            for(t = 0; t < 20; t++)
            {
                temp =  SHA1CircularShift(5,A) +
                        ((B & C) | ((~B) & D)) + E + W[t] + K[0];
                temp &= 0xFFFFFFFF;
                E = D;
                D = C;
                C = SHA1CircularShift(30,B);
                B = A;
                A = temp;
            }

            for(t = 20; t < 40; t++)
            {
                temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
                temp &= 0xFFFFFFFF;
                E = D;
                D = C;
                C = SHA1CircularShift(30,B);
                B = A;
                A = temp;
            }

            for(t = 40; t < 60; t++)
            {
                temp = SHA1CircularShift(5,A) +
                       ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
                temp &= 0xFFFFFFFF;
                E = D;
                D = C;
                C = SHA1CircularShift(30,B);
                B = A;
                A = temp;
            }

            for(t = 60; t < 80; t++)
            {
                temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
                temp &= 0xFFFFFFFF;
                E = D;
                D = C;
                C = SHA1CircularShift(30,B);
                B = A;
                A = temp;
            }

            context->Message_Digest[0] =
                (context->Message_Digest[0] + A) & 0xFFFFFFFF;
            context->Message_Digest[1] =
                (context->Message_Digest[1] + B) & 0xFFFFFFFF;
            context->Message_Digest[2] =
                (context->Message_Digest[2] + C) & 0xFFFFFFFF;
            context->Message_Digest[3] =
                (context->Message_Digest[3] + D) & 0xFFFFFFFF;
            context->Message_Digest[4] =
                (context->Message_Digest[4] + E) & 0xFFFFFFFF;

            context->Message_Block_Index = 0;
        }

        /*  
         *  SHA1PadMessage
         *
         *  Description:
         *      According to the standard, the message must be padded to an even
         *      512 bits.  The first padding bit must be a '1'.  The last 64
         *      bits represent the length of the original message.  All bits in
         *      between should be 0.  This function will pad the message
         *      according to those rules by filling the Message_Block array
         *      accordingly.  It will also call SHA1ProcessMessageBlock()
         *      appropriately.  When it returns, it can be assumed that the
         *      message digest has been computed.
         *
         *  Parameters:
         *      context: [in/out]
         *          The context to pad
         *
         *  Returns:
         *      Nothing.
         *
         *  Comments:
         *
         */
        void SHA1PadMessage(SHA1Context *context)
        {
            /*
             *  Check to see if the current message block is too small to hold
             *  the initial padding bits and length.  If so, we will pad the
             *  block, process it, and then continue padding into a second
             *  block.
             */
            if (context->Message_Block_Index > 55)
            {
                context->Message_Block[context->Message_Block_Index++] = 0x80;
                while(context->Message_Block_Index < 64)
                {
                    context->Message_Block[context->Message_Block_Index++] = 0;
                }

                SHA1ProcessMessageBlock(context);

                while(context->Message_Block_Index < 56)
                {
                    context->Message_Block[context->Message_Block_Index++] = 0;
                }
            }
            else
            {
                context->Message_Block[context->Message_Block_Index++] = 0x80;
                while(context->Message_Block_Index < 56)
                {
                    context->Message_Block[context->Message_Block_Index++] = 0;
                }
            }

            /*
             *  Store the message length as the last 8 octets
             */
            context->Message_Block[56] = (context->Length_High >> 24) & 0xFF;
            context->Message_Block[57] = (context->Length_High >> 16) & 0xFF;
            context->Message_Block[58] = (context->Length_High >> 8) & 0xFF;
            context->Message_Block[59] = (context->Length_High) & 0xFF;
            context->Message_Block[60] = (context->Length_Low >> 24) & 0xFF;
            context->Message_Block[61] = (context->Length_Low >> 16) & 0xFF;
            context->Message_Block[62] = (context->Length_Low >> 8) & 0xFF;
            context->Message_Block[63] = (context->Length_Low) & 0xFF;

            SHA1ProcessMessageBlock(context);
        }
        /**
         * resolves cszVarString and replaces all environment variable references 
         * (delimited by %) with the value of the variable.
         * @param cszVarString string with environment variable references delimited by %
         *                     e.g., %USERPROFILE%\Desktop
         * @param szResolvedString the resolved string is copied to this variable
         */
        void ResolveEnvironmentVariables (LPTSTR cszVarString,
                                          TCHAR szResolvedString[MAX_PATH])
        {
            TCHAR envVar [MAX_PATH] = {0};
            BOOL bVar = FALSE;

            if (cszVarString [0] == '%')
            {
                bVar = TRUE;
            }
            TCHAR* currentPosition = NULL;
            TCHAR* token = _tcstok_s (cszVarString, _T("%"), &currentPosition);
    
            if( token == NULL )
            {
                return;
            }

            do 
            {
                if (bVar)
                {
                    if (::GetEnvironmentVariable (token, envVar, MAX_PATH))
                    {
                        _tcsncat_s (szResolvedString, MAX_PATH, envVar, _TRUNCATE);
                    }
                }
                else
                {
                    _tcsncat_s (szResolvedString, MAX_PATH, token, _TRUNCATE);
                }
                bVar = !bVar;
            } while ((token = _tcstok_s (NULL, _T("%"), &currentPosition)) != NULL);
        }
    }

    bool GetFingerprintWithJavaObj(JNIEnv *env, jobject obj,
                                   const nlchar *appNameWithPath, 
                                   nlchar *fingerPrintBuf,
                                   size_t bufSize) 
    {
        LOADED_IMAGE loaded_img;
        char  szExePath [MAX_PATH*2] = {0};
        CHAR *internalName = NULL, *exeName = NULL;
        //12 for 2 DWORDs (32 bits) 50 for 160 bit SHA1 hash, 
        //4 for miscellany like semicolons, terminating NUL byte etc. 
        char  data [MAX_PATH * 4 + 2 * 12 + 50 + 4] = {0}; 

        memset (&loaded_img, '\0', sizeof(loaded_img));
        ::WideCharToMultiByte(CP_ACP, 0, appNameWithPath, 
                              -1, szExePath, MAX_PATH*2, NULL, NULL);

        if (::MapAndLoad (szExePath, NULL, &loaded_img, FALSE, TRUE) != TRUE) {
            TRACE(0, _T("MapAndLoad failed: error code=%d\n"), ::GetLastError());
            return false;
        }

        ULONG deSize;
        IMAGE_NT_HEADERS *pImageNtHeaders=::ImageNtHeader(loaded_img.MappedAddress);
        PIMAGE_SECTION_HEADER pImageSectionHeader = NULL;
        IMAGE_EXPORT_DIRECTORY *exportDirectory;
        exportDirectory=(IMAGE_EXPORT_DIRECTORY *) ::ImageDirectoryEntryToDataEx (
            loaded_img.MappedAddress, 
        TRUE, 
        IMAGE_DIRECTORY_ENTRY_EXPORT, 
        &deSize, 
        &pImageSectionHeader);

        if (pImageSectionHeader != NULL && exportDirectory != NULL) {
            internalName = (CHAR *) ImageRvaToVa (pImageNtHeaders, 
                                                  loaded_img.MappedAddress, 
                                                  exportDirectory -> Name, NULL);
        } else {
            internalName = "(null)";
        }

        exeName = strrchr (loaded_img.ModuleName, '\\');

        SHA1Context sha1Context;
        SHA1Reset (&sha1Context);
        HANDLE hFile = ::CreateFileA (szExePath, FILE_READ_DATA, FILE_SHARE_READ, 
                                      NULL, OPEN_EXISTING, 
                                      FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hFile)
        {
            if (::ImageGetDigestStream (hFile, CERT_PE_IMAGE_DIGEST_DEBUG_INFO, 
                                        SHA1Input, &sha1Context) == FALSE) {
                ::CloseHandle (hFile);
                LPWSTR lpszTemp = NULL;
                DWORD  lastErr  = GetLastError ();
#pragma warning(push)
#pragma warning(disable: 6387)
                ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                FORMAT_MESSAGE_FROM_SYSTEM | 
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL, lastErr, LANG_NEUTRAL, 
                                lpszTemp, 0, NULL);
#pragma warning(pop)
                GlobalFree ((HGLOBAL) lpszTemp);
                return false;
            }
        
            _snprintf_s (data, _countof(data), _TRUNCATE, "%s:%u:%u:%08x%08x%08x%08x%08x", 
                         internalName, 
                         (unsigned int)pImageNtHeaders -> OptionalHeader.MajorImageVersion, 
                         (unsigned int)pImageNtHeaders -> OptionalHeader.MinorImageVersion, 
                         sha1Context.Message_Digest [0], 
                         sha1Context.Message_Digest [1], 
                         sha1Context.Message_Digest [2], 
                         sha1Context.Message_Digest [3], 
                         sha1Context.Message_Digest [4]);
            ::CloseHandle (hFile);
        }
  
        if (::UnMapAndLoad (&loaded_img) != TRUE) {
            return false;
        }
    
        TCHAR wcData [4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)] = {0};
        ::MultiByteToWideChar(CP_ACP, 0, data, 
                              (int)strlen (data) + 1, wcData, 
                              (4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)) - 1); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
        //jstr = env -> NewString (wcData, _tcslen (wcData));
        size_t len=nlstrlen(wcData);
        if(len >= bufSize)
            return false; //pre-allocated buffer is too small
        nlstrncpy_s(fingerPrintBuf, bufSize, wcData, _TRUNCATE);

        return true;
    }

    bool GetFingerprint(const nlchar *appNameWithPath, 
                        nlchar *fingerPrintBuf,
                        size_t bufSize) 
    {
        LOADED_IMAGE loaded_img;
        char  szExePath [MAX_PATH*2];
        CHAR *internalName = NULL, *exeName = NULL;
        //12 for 2 DWORDs (32 bits) 50 for 160 bit SHA1 hash, 
        //4 for miscellany like semicolons, terminating NUL byte etc. 
        char  data [MAX_PATH * 4 + 2 * 12 + 50 + 4] = ""; 

        memset (&loaded_img, '\0', sizeof(loaded_img));
        ::WideCharToMultiByte(CP_ACP, 0, appNameWithPath, 
                              -1, szExePath, MAX_PATH*2, NULL, NULL);

        if (::MapAndLoad (szExePath, NULL, &loaded_img, FALSE, TRUE) != TRUE) {
            TRACE(0, _T("MapAndLoad failed: error code=%d\n"), ::GetLastError());
            return false;
        }

        ULONG deSize;
        IMAGE_NT_HEADERS *pImageNtHeaders=::ImageNtHeader(loaded_img.MappedAddress);
        PIMAGE_SECTION_HEADER pImageSectionHeader = NULL;
        IMAGE_EXPORT_DIRECTORY *exportDirectory;
        exportDirectory=(IMAGE_EXPORT_DIRECTORY *) ::ImageDirectoryEntryToDataEx (
            loaded_img.MappedAddress, 
        TRUE, 
        IMAGE_DIRECTORY_ENTRY_EXPORT, 
        &deSize, 
        &pImageSectionHeader);

        if (pImageSectionHeader != NULL && exportDirectory != NULL) {
            internalName = (CHAR *) ImageRvaToVa (pImageNtHeaders, 
                                                  loaded_img.MappedAddress, 
                                                  exportDirectory -> Name, NULL);
        } else {
            internalName = "(null)";
        }

        exeName = strrchr (loaded_img.ModuleName, '\\');

        SHA1Context sha1Context;
        SHA1Reset (&sha1Context);
        HANDLE hFile = ::CreateFileA (szExePath, FILE_READ_DATA, FILE_SHARE_READ, 
                                      NULL, OPEN_EXISTING, 
                                      FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hFile)
        {
            if (::ImageGetDigestStream (hFile, CERT_PE_IMAGE_DIGEST_DEBUG_INFO, 
                                        SHA1Input, &sha1Context) == FALSE) {
          
                ::CloseHandle (hFile);
                LPWSTR lpszTemp = NULL;
                DWORD  lastErr  = GetLastError ();
#pragma warning(push)
#pragma warning(disable: 6387)
                ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                FORMAT_MESSAGE_FROM_SYSTEM | 
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL, lastErr, LANG_NEUTRAL, 
                                lpszTemp, 0, NULL);
#pragma warning(pop)
                GlobalFree ((HGLOBAL) lpszTemp);
                return false;
            }
        
            _snprintf_s (data, _countof (data), _TRUNCATE, "%s:%u:%u:%08x%08x%08x%08x%08x", 
                         internalName, 
                         (unsigned int)pImageNtHeaders -> OptionalHeader.MajorImageVersion, 
                         (unsigned int)pImageNtHeaders -> OptionalHeader.MinorImageVersion, 
                         sha1Context.Message_Digest [0], 
                         sha1Context.Message_Digest [1], 
                         sha1Context.Message_Digest [2], 
                         sha1Context.Message_Digest [3], 
                         sha1Context.Message_Digest [4]);
            ::CloseHandle (hFile);
        }
  
        if (::UnMapAndLoad (&loaded_img) != TRUE) {
            return false;
        }
    
        TCHAR wcData [4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)];
        ::MultiByteToWideChar(CP_ACP, 0, data, 
                              (int)strlen (data) + 1, wcData, 
                              (4 * (MAX_PATH * 4 + 2 * 12 + 50 + 4)) - 1); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
        //jstr = env -> NewString (wcData, _tcslen (wcData));
        size_t len=nlstrlen(wcData);
        if(len >= bufSize)
            return false; //pre-allocated buffer is too small
        nlstrncpy_s(fingerPrintBuf, bufSize, wcData, _TRUNCATE);

        return true;
    }

    /*
     * Utility time conversion routine.  Converts FILETIME (from FILETIME
     * epoch) -> Java time (from the the standard Java/Unix epoch).
     *
     */
    static const NLint64 SECS_BETWEEN_EPOCHS = 11644473600;
    static const NLint64 SECS_TO_100NS       = 10000000;
    NLint64 FileTimeToJavaTimeMills (FILETIME FileTime)
    {
        NLint64 javaTime;

        javaTime=((NLint64)FileTime.dwHighDateTime << 32) + FileTime.dwLowDateTime;
        javaTime -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

        return javaTime / 10000;
    }

    NLint64 GetFileTime (const nlchar *str, int type, HANDLE processTokenHandle) 
    {
        NLint64 retTime = 0;
        WIN32_FILE_ATTRIBUTE_DATA  fileAttrData;
        volatile int dummy;

        if (processTokenHandle) {
            if (!ImpersonateLoggedOnUser (processTokenHandle)) {
                dummy = 0;                // just to keep Veracode happy
            }
        }
        if (GetFileAttributesEx (str, GetFileExInfoStandard, &fileAttrData) == 0) {
            if (processTokenHandle) {
                if (!RevertToSelf ()) {
                    dummy = 0;              // just to keep Veracode happy
                }
            }
            return -1;
        }
        if (processTokenHandle) {
            if (!RevertToSelf ()) {
                dummy = 0;                // just to keep Veracode happy
            }
        }
        if (type == FILE_ACCESS_TIME) {
            retTime = FileTimeToJavaTimeMills (fileAttrData.ftLastAccessTime);
        }
        if (type == FILE_CREATE_TIME) {
            retTime = FileTimeToJavaTimeMills (fileAttrData.ftCreationTime);
        }
        if (type == FILE_MODIFICATION_TIME) {
            retTime = FileTimeToJavaTimeMills (fileAttrData.ftLastWriteTime);
        }

        return retTime;
    }
}

DeviceDetc::DeviceDetc() 
{
    HKEY hKey;
    TCHAR folder [MAX_PATH];
    TCHAR returnFolder [MAX_PATH];
    DWORD size = MAX_PATH * sizeof (TCHAR);

    //Get CD Burning folder 
    cdBurningFolder=_T("");
    if (::RegOpenKeyEx (HKEY_CURRENT_USER, REGSTR_PATH_SPECIAL_FOLDERS, 
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (::RegQueryValueEx (hKey, CD_BURNING_NAME, NULL, NULL, 
                               (LPBYTE) folder, &size) == ERROR_SUCCESS) {
            returnFolder[0]=0;
            ResolveEnvironmentVariables (folder, returnFolder);
            cdBurningFolder=returnFolder;
        }
        ::RegCloseKey (hKey);
    }
  
    nlthread_cs_init(&csRMCache);
}

DeviceDetc::~DeviceDetc()
{
    nlthread_cs_delete(&csRMCache);
}

////////////////////////////////////////////////////////////////
// Returns the bus type of a particular disk drive
// disk [in] drive letter 
////////////////////////////////////////////////////////////////
STORAGE_BUS_TYPE DeviceDetc::GetBusType(const nlchar disk)
{
    WCHAR szDriveName[MAX_PATH];
    _snwprintf_s (szDriveName, MAX_PATH, _TRUNCATE,L"\\\\.\\%c:", disk);
    HANDLE hDevice;

    hDevice = CreateFile (szDriveName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        DWORD e=GetLastError();
        TRACE(0, _T("hDevice: %d\n"), e);
        return BusTypeUnknown;
    } else {
        DWORD dwBytesReturned, dwRetCode;
        int buf_size = 4096;
        char buf[4096];

        STORAGE_PROPERTY_QUERY query;

        memset ((void *)&query, 0, sizeof (query));
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        dwRetCode = DeviceIoControl (hDevice,
                                     IOCTL_STORAGE_QUERY_PROPERTY,
                                     &query,
                                     sizeof (query),
                                     (STORAGE_DEVICE_DESCRIPTOR *) buf,
                                     buf_size,
                                     &dwBytesReturned,
                                     NULL);
        if(!dwRetCode) {
            CloseHandle (hDevice);
            return BusTypeUnknown;
        }
        STORAGE_DEVICE_DESCRIPTOR *info= (STORAGE_DEVICE_DESCRIPTOR *)buf;

        CloseHandle (hDevice);
        return info->BusType;
    }
}
/////////////////////////////////////////////////////////
// Given a file name, returns if the file is stored on a 
// removable media.
// fileName[in] : name of the file
// Returns TRUE if the file is on a removable media
/////////////////////////////////////////////////////////
BOOL DeviceDetc::IsRemovableMedia(const nlchar *fileName)
{
    if(fileName == NULL)
        return FALSE;

    //First check if this is a CD burner folder
    if( cdBurningFolder.size() > 0 &&
        !_tcsncicmp(fileName, cdBurningFolder.c_str(),cdBurningFolder.size()) )
    {
        return (TRUE);
    }

    //The file name is too short
    if(nlstrlen(fileName) < nlstrlen(_T("C:\\")))
        return FALSE;

    //The file name doesn't include driver
    if(!nlstrstr(fileName, _T(":\\")))
        return FALSE;
  
    //Get driver root
    nlchar root[4] = {0};
    wcsncpy_s (root, fileName, _TRUNCATE);

    //Check if we have cache for this driver
    nlthread_cs_enter(&csRMCache);
    BOOL result;
    if (removableMediaCache.find(root) != removableMediaCache.end()) {
        result = removableMediaCache[root];
        nlthread_cs_leave(&csRMCache);
        return result;
    }
    nlthread_cs_leave(&csRMCache);
  
  
    //Try use GetDriveType to check if it is CDROM, etc.
    //USB is treated as "FIXED_DRIVE", not "removable" in user sense.
    int driveType = ::GetDriveType (root);  
    switch (driveType) {
        case DRIVE_REMOVABLE:
        case DRIVE_CDROM:
            nlthread_cs_enter(&csRMCache);
            removableMediaCache[root]=TRUE;
            nlthread_cs_leave(&csRMCache);
            return TRUE;
            break;
        default:
            break;
    }

    //Now we need to get down to the busType to detect removable device
    if(IsRemovableBusType(root)==TRUE) {
        nlthread_cs_enter(&csRMCache);
        removableMediaCache[root]=TRUE;
        nlthread_cs_leave(&csRMCache);
        return TRUE;
    }

    nlthread_cs_enter(&csRMCache);
    removableMediaCache[root]=FALSE;
    nlthread_cs_leave(&csRMCache);

    return FALSE;
}

/* Rely on bus type to determine if a device is removable. */
BOOL DeviceDetc::IsRemovableBusType(const nlchar *driver)
{
    BOOL bRemovable=FALSE;
    const STORAGE_BUS_TYPE lBusType = GetBusType(driver[0]);

    switch (lBusType) {
        case BusTypeAtapi:
        case BusTypeUsb:
        case BusType1394:
            bRemovable=TRUE;
        default:
            break;
    }
    return bRemovable;
}
/* ------------------------------------------------------------------------- */
/* NL_getUserId                                                              */
/* Getting the user ID of the CURRENT process                                */
/*                                                                           */
/* Input : id_buf: user supply storage for the id                            */
/*         size  : size of id_buf, in NLCHAR count                           */
/* ------------------------------------------------------------------------- */

BJ_error_t NL_getUserId (_Out_z_cap_(size) nlchar * id_buf, _In_ size_t size)
{

    BJ_error_t ret       = BJ_ERROR;

    HANDLE hCurProcess     = INVALID_HANDLE_VALUE;
    HANDLE hProcessToken   = INVALID_HANDLE_VALUE;
    PTOKEN_USER pTokenUser = NULL;
    nlchar * pSid          = NULL;

    if (!id_buf)    return ret;
    if (size <= 0)  return ret;

    memset (id_buf, 0x0, size * sizeof(nlchar));

    hCurProcess = ::GetCurrentProcess();

    // According to MSDN. GetCurrentProcess actually return (HANDLE)-1
    // if (hCurProcess == INVALID_HANDLE_VALUE) goto failed_and_cleanup;

    if (!OpenProcessToken (hCurProcess, TOKEN_QUERY, &hProcessToken)) {
        goto failed_and_cleanup;
    }

    // From MSDN, we should make it fail first, to get the right size
    DWORD len;
    GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &len);

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

        pTokenUser = (PTOKEN_USER) malloc (len);
        if (!pTokenUser) goto failed_and_cleanup;

        GetTokenInformation (hProcessToken, TokenUser, pTokenUser, len, &len);

        ConvertSidToStringSidW (pTokenUser->User.Sid, &pSid);

        size_t sidLen = nlstrlen (pSid);

        // Bail if we don't have enough buffer
        if ((sidLen + 1) > size) {
            goto failed_and_cleanup;
        }
    
        nlstrcpy_s (id_buf, size, pSid);
        ret = BJ_OK;
    }

 failed_and_cleanup:
    if (hProcessToken != INVALID_HANDLE_VALUE) CloseHandle (hProcessToken);
    if (pTokenUser) free (pTokenUser);
    if (pSid)       LocalFree (pSid);
                    
    return ret;
}

//Cause the current process to be suspended for 'milliseconds' milliseconds
void NL_sleep(_In_ unsigned long milliseconds)
{
    Sleep(milliseconds);
}

/* ------------------------------------------------------------------------- */
/* NL_GetCurrentTimeInMillisec
   /* On Linux, this function will return the current time in milliseconds.     *
   /* On Window, this function will return the number of milliseconds that have */
/* elapsed since the system was started, up to 49.7 days.                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */
_Check_return_ double NL_GetCurrentTimeInMillisec()
{
    return GetTickCount();
}

/* ------------------------------------------------------------------------- */
/* NL_GetFingerPrint                                                         */
/* On Linux, this function currently is NOP.                                 *
   /* On Window, this function will return the application finger print.        */
/* ------------------------------------------------------------------------- */
bool NL_GetFingerprint(_In_ const nlchar *appNameWithPath, 
		       _Out_z_cap_(bufSize) nlchar *fingerprintBuf,
		       _In_ int bufSize) 
{
    return GetFingerprint(appNameWithPath, fingerprintBuf, bufSize);
}
/* ------------------------------------------------------------------------- */
/* NL_GetFQDN                                                         */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the fully qualified DNS name that    */
/* uniquely identifies the local computer. application finger print.         */
/* Parameter:                                                                */
/* fqdnBuf (OUT): the buffer for the FQDN string                             */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetFQDN(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fqdnBuf, _Inout_ int *bufSize)
{
    DWORD size=*bufSize;
    if (!GetComputerNameEx(ComputerNameDnsFullyQualified, fqdnBuf, &size)) {
        *fqdnBuf = 0;
        *bufSize = 0;
        return false;
    }
    *bufSize=size;
    return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetMyDesktopFolder                                                     */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the name of my desktop folder        */
/* Parameter:                                                                */
/* fBuf (OUT): the buffer for the FQDN string                                */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDesktopFolder(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fBuf, _Inout_ int *bufSize)
{
    HKEY hKey;
    TCHAR folder [MAX_PATH];
    TCHAR returnFolder [MAX_PATH];
    DWORD size = MAX_PATH * sizeof (TCHAR);

    if(fBuf == NULL)
        return false;

    if (::RegOpenKeyEx (HKEY_CURRENT_USER, REGSTR_PATH_SPECIAL_FOLDERS, 
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (::RegQueryValueEx (hKey, MY_DESKTOP_VALUE_NAME, NULL, 
                               NULL, (LPBYTE) folder, &size) == ERROR_SUCCESS) {
            returnFolder [0] = 0;
            ResolveEnvironmentVariables (folder, returnFolder);
            size=(DWORD)_tcslen(returnFolder); // cast to DWORD is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
            if(size <= 0 || (DWORD)(*bufSize) <= size) {
                ::RegCloseKey (hKey);
                return false; 
            }
            nlstrncpy_s(fBuf, *bufSize, returnFolder, size);
            *bufSize=(int)size; // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
            ::RegCloseKey (hKey);
        } else {
            ::RegCloseKey (hKey);
            return false;
        }
    } else
        return false;
    return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetMyDocumentsFolder                                                   */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will return the name of my documents folder      */
/* Parameter:                                                                */
/* fBuf (OUT): the buffer for the FQDN string                                */
/* bufSize (INOUT): On input, size of the buffer.                            */
/*                  On output, size of the string copied to buffer, not      */
/*                  including null-terminator.                               */
/* Return value: true if success                                             */
/*               false if error                                              */
/* ------------------------------------------------------------------------- */
bool NL_GetMyDocumentsFolder(_Out_z_cap_post_count_(*bufSize, *bufSize) nlchar *fBuf, _Inout_ int *bufSize)
{
    HKEY hKey;
    TCHAR folder [MAX_PATH];
    TCHAR returnFolder [MAX_PATH];
    DWORD size = MAX_PATH * sizeof (TCHAR);

    if(fBuf == NULL)
        return false;

    if (::RegOpenKeyEx (HKEY_CURRENT_USER, USER_FOLDER_KEY, 
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (::RegQueryValueEx (hKey, MY_DOCUMENTS_VALUE_NAME, NULL, 
                               NULL, (LPBYTE) folder, &size) == ERROR_SUCCESS) {
            returnFolder [0] = 0;
            ResolveEnvironmentVariables (folder, returnFolder);
            size= (DWORD)_tcslen(returnFolder); // cast to DWORD is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
            if(size <= 0 || (DWORD)(*bufSize)<= size) {
                ::RegCloseKey (hKey);
                return false;
            }
            nlstrncpy_s(fBuf, *bufSize, returnFolder, size);
            *bufSize=size;
            ::RegCloseKey (hKey);
        } else {
            ::RegCloseKey (hKey);
            return false;
        }
    } else
        return false;

    return true;
}
/* ------------------------------------------------------------------------- */
/* NL_IsRemovableMedia                                                       */
/* On Linux, this function currently is NOP.                                 */
/* On Window, this function will check if the file is on removable media     */
/*                                                                           */
/* ------------------------------------------------------------------------- */
_Check_return_ bool NL_IsRemovableMedia(_In_ const nlchar *fileName)
{
    if(deviceDetc.IsRemovableMedia(fileName))
        return true;
    return false;
}
/* ------------------------------------------------------------------------- */
/* NL_LogEvent                                                               */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will log the event                               */
/* ------------------------------------------------------------------------- */
void NL_LogEvent(_In_ int type, _In_ int eventId, _In_ int paramSize, _In_opt_count_(paramSize)nlchar **ppStrArray)
{
    HANDLE  hEventSource;

    hEventSource = RegisterEventSource(NULL, SZ_EVENT_VIEWER_SOURCE);

    if (hEventSource != NULL) {
        ReportEvent(hEventSource, 
                    (WORD) type,  
                    0,                    
                    (DWORD) eventId,                    
                    NULL,                 
                    (WORD)paramSize,                    
                    0,                    
                    (LPCTSTR*) ppStrArray,          
                    NULL);                

        DeregisterEventSource(hEventSource);
    }
}
/* ------------------------------------------------------------------------- */
/* NL_GetUserName                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the user name.                       */
/* ------------------------------------------------------------------------- */
bool NL_GetUserName(_In_ const nlchar *userSid, _Out_ nlchar **userName)
{
    PSID    sid;
    WCHAR name [MAX_PATH] = {0};
    WCHAR domain [MAX_PATH] = {0};
    SID_NAME_USE sidNameUse;
    DWORD cchName = MAX_PATH;
    DWORD cchDomain = MAX_PATH;    
    WCHAR* pName = name;
    WCHAR* pDomain = domain;
    BOOL bRet = FALSE;

    *userName = NULL;

    if (::ConvertStringSidToSid(userSid, &sid)) {
        bRet = LookupAccountSid(NULL, sid, pName, &cchName, 
                                    pDomain, &cchDomain, &sidNameUse);

        if (!bRet &&
            (cchName > MAX_PATH || cchDomain > MAX_PATH)) {
            if (cchName > MAX_PATH)
                pName = new WCHAR [cchName];
            if (cchDomain > MAX_PATH)
                pDomain = new (nothrow) WCHAR [cchDomain];
            bRet = LookupAccountSid(NULL, sid, pName, &cchName, 
                                        pDomain, &cchDomain, &sidNameUse); 
        }

        if (bRet && sidNameUse == SidTypeUser) {
            DWORD dwLen = MAX_PATH;
            WCHAR* pFullName = new WCHAR [wcslen (pDomain) + wcslen (pName) + 2];
            WCHAR* pBuf = new (nothrow) WCHAR [dwLen];
            _snwprintf_s (pFullName, wcslen (pDomain) + wcslen (pName) + 2, _TRUNCATE, L"%s\\%s",pDomain, pName);
            bRet = ::TranslateName (pFullName, NameSamCompatible, 
                                        NameUserPrincipal, pBuf, &dwLen);
            
            if (!bRet && dwLen > MAX_PATH) {
                delete [] pBuf;
                pBuf = new WCHAR [dwLen];
                bRet = ::TranslateName (pFullName, NameSamCompatible, 
                                            NameUserPrincipal, pBuf, &dwLen);
            }

            if (bRet) {
                *userName = pBuf;
                delete [] pFullName;
            }else {
                _snwprintf_s (pFullName, wcslen (pDomain) + wcslen (pName) + 2, _TRUNCATE, L"%s@%s", pName, pDomain);
                *userName=pFullName;
                delete [] pBuf;
            }
        }
        ::LocalFree (sid);
    }

    if (pName != name) 
        delete [] pName;
    if (pDomain != domain)
        delete [] pDomain;

    return (*userName != NULL);
}

/* ------------------------------------------------------------------------- */
/* NL_GetUserName_Free                                                       */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will free the memory allocated for the user name.*/
/* ------------------------------------------------------------------------- */
void NL_GetUserName_Free(_In_ nlchar *userName)
{
    if(userName)
        delete [] userName;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileModifiedTime                                                    */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last modified time of a file.       */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileModifiedTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken)
{
    NLint64 ret = -1;
    if (fileName) {
        ret = GetFileTime (fileName, FILE_MODIFICATION_TIME, (HANDLE)processToken);
    }
    return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileAccessTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the last access time of a file.         */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileAccessTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken)
{
    NLint64 ret = -1;
    if (fileName) {
        ret = GetFileTime (fileName, FILE_ACCESS_TIME, (HANDLE)processToken);
    }
    return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFileCreateTime                                                      */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the create time of a file.              */
/* ------------------------------------------------------------------------- */
_Check_return_ NLint64 NL_GetFileCreateTime(_In_z_ const nlchar *fileName, _In_opt_ void *processToken)
{
    NLint64 ret = -1;
    if (fileName) {
        ret = GetFileTime (fileName, FILE_CREATE_TIME, (HANDLE)processToken);
    }
    return ret;
}
/* ------------------------------------------------------------------------- */
/* NL_OpenProcessToken                                                       */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will opens the access token associated with a    */
/*            process.                                                       */
/* ------------------------------------------------------------------------- */
void *NL_OpenProcessToken()
{
    HANDLE token = NULL;
    OpenProcessToken(GetCurrentProcess (), TOKEN_QUERY | TOKEN_DUPLICATE, 
                     &token);
    return token;
}
/* ------------------------------------------------------------------------- */
/* NL_GetOwnerSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the file owner's sid.                   */
/* ------------------------------------------------------------------------- */
bool NL_GetOwnerSID(_In_ const nlchar *fileName, _In_ int agentType, 
		    _In_ void *processTokenHandle, _Out_z_cap_(bufSize) nlchar *sidBuf, _In_ int bufSize)
{
    PSID  sidOwner;
    PSECURITY_DESCRIPTOR securityDescriptor;
    TCHAR* pszUSID;
    WCHAR  fileNameWithStream[MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b")];
    volatile int dummy;

    if (fileName == NULL ||
        nlstrlen(fileName) == 0 ||
        nlstrlen(fileName) >= MAX_PATH ||
        !IsPrintableString(fileName)) {
        return false;
    }

    if (agentType == 1) {
        wcsncpy_s (fileNameWithStream, MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b"), fileName, _TRUNCATE);
        wcsncat_s (fileNameWithStream, MAX_PATH + sizeof (L":db172189-cf42-4433-933d-103368dd5f2b"), L":db172189-cf42-4433-933d-103368dd5f2b", _TRUNCATE);
        HANDLE hfile = ::CreateFileW(fileName, FILE_READ_ATTRIBUTES, 
                                     0x7, NULL, OPEN_EXISTING, 0, NULL);
    
        if (hfile != INVALID_HANDLE_VALUE) {
            CloseHandle (hfile);
            hfile = ::CreateFileW (fileNameWithStream, READ_CONTROL, 
                                   0x7, NULL, OPEN_ALWAYS, 0, NULL);

            if (hfile == INVALID_HANDLE_VALUE || 
                GetSecurityInfo (hfile, SE_FILE_OBJECT, 
                                 OWNER_SECURITY_INFORMATION, 
                                 &sidOwner, NULL, NULL, NULL, 
                                 &securityDescriptor) != 0) {
                if (hfile != INVALID_HANDLE_VALUE) {
                    CloseHandle (hfile);
                }
                return false;
            }
            CloseHandle (hfile);
        } else {
            return false;
        }
    } else {
        if (processTokenHandle) {
            if (!ImpersonateLoggedOnUser((HANDLE)processTokenHandle)) {
                dummy = 0;              // just to keep Veracode happy
            }
        }
        if (GetNamedSecurityInfo ((TCHAR *)fileName, SE_FILE_OBJECT, 
                                  OWNER_SECURITY_INFORMATION, 
                                  &sidOwner, NULL, NULL, NULL, 
                                  &securityDescriptor) != 0) {
            if (processTokenHandle) {
                if (!RevertToSelf ()) {
                    dummy = 0;  // just to keep Veracode happy
                }
            }
            return false;
        }
        if (processTokenHandle) {
            if (!RevertToSelf ()) {
                dummy = 0;              // just to keep Veracode happy
            }
        }
    }

    pszUSID = NULL;
    ::ConvertSidToStringSid((PSID) sidOwner, &pszUSID);

    if(_tcslen(pszUSID) >= (size_t)bufSize) {
        ::LocalFree (securityDescriptor);
        ::LocalFree (pszUSID);
        TRACE(0, _T("The buffer (size=%d) for getting file owner's sid (size=%d) is too small.\n"), bufSize, _tcslen(pszUSID)); 
        return false;
    }

    nlstrcpy_s(sidBuf, bufSize, pszUSID);
    ::LocalFree (securityDescriptor);
    ::LocalFree (pszUSID);
    return true;
}
/* ------------------------------------------------------------------------- */
/* NL_GetGroupSID                                                            */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will get the group sid.                          */
/* ------------------------------------------------------------------------- */
bool NL_GetGroupSID(_In_ const nlchar *fileName, _Out_z_cap_(bufSize) nlchar *sidBuf, _In_ int bufSize)
{
    PSID  sidGroup;
    PSECURITY_DESCRIPTOR securityDescriptor;
    TCHAR* pszGSID;

    if(GetNamedSecurityInfo((TCHAR *)fileName, SE_FILE_OBJECT, 
                            GROUP_SECURITY_INFORMATION, 
                            NULL, &sidGroup, NULL, NULL, 
                            &securityDescriptor) != 0) {
        return false;
    }

    pszGSID = NULL;
    ::ConvertSidToStringSid((PSID) sidGroup, &pszGSID);

    if(bufSize <= (int)_tcslen (pszGSID)) {
        TRACE(0, _T("The buffer (size=%d) for getting group sid (size=%d) is too small.\n"), bufSize, _tcslen(pszGSID)); 
        ::LocalFree (securityDescriptor);
        ::LocalFree (pszGSID);
        return false;
    }

    nlstrcpy_s(sidBuf, bufSize, pszGSID);
    ::LocalFree (securityDescriptor);
    ::LocalFree (pszGSID);
    return true;
}

/* ------------------------------------------------------------------------- */
/* NL_GetRdpSessionAddress                                                   */
/* On Linux, this function is NOP.                                           */
/* On Window, this function will return the RDP (remot desktop) session      */
/* address.                                                                  */
/* ------------------------------------------------------------------------- */
long NL_GetRdpSessionAddress(_In_ int pid)
{
    DWORD   dwSessionId = (DWORD)-1;
    USHORT  usProtocolType = 0;
    DWORD   dwIp        = 0;

    static DWORD dwIpStart = ((192<<24)&0xFF000000) | ((168<<16)&0xFF0000) | ((187<<8)&0xFF00) | 80;
    static DWORD dwIpEnd   = ((192<<24)&0xFF000000) | ((168<<16)&0xFF0000) | ((187<<8)&0xFF00) | 85;

    if(!GetConsoleSessionId(pid, &dwSessionId)) {
        return 0;
    }

    if(!GetSessionProtocolType(dwSessionId, &usProtocolType)) {
        return 0;
    }

    if(WTS_PROTOCOL_TYPE_RDP != usProtocolType) {
        return 0;
    }

    if(!GetSessionRemoteIp(dwSessionId, &dwIp)) {
        return 0;
    }

    if(dwIp>=dwIpStart && dwIp<=dwIpEnd) {
        return 0;
    }

    return dwIp;
}
/* ------------------------------------------------------------------------- */
/* NL_GetFilePhysicalPath                                                    */
/* On Linux, this function is NOP currently.                                 */
/* On Window, this function will get the physical path of a file. For        */
/* example,  if file "T:\heidiz\SDK\eval.cpp" is actually located at         */
/* \\bluejungle.com\share\public\heidiz\SDK\eval.cpp. Then, this fucntion    */
/* will return the latter as ouput.                                          */
/* Parameter:                                                                */
/* input (IN): the logical path and name of the file                         */
/* output (OUT): the returned physical path                                  */
/* outputBufLen (IN): the length of output buffer                            */
/* return value: true, if the physical path is different from the logical one*/
/*               false, if the physical path is same as the logical one.     */
/* ------------------------------------------------------------------------- */
bool NL_GetFilePhysicalPath(_In_ const nlchar *input, _Out_z_cap_(outputBufLen) nlchar *output, 
			    _In_ size_t outputBufLen)
{
    if(input == NULL || (input && nlstrlen(input) < 3)) {
        //If input is shorter than the string like "v:\",
        //it is invalid input
        TRACE(0, _T("NL_GetFilePhysicalPath: invalid input\n"));
        return false;
    }

    if(output == NULL) {
        TRACE(0, _T("NL_GetFilePhysicalPath: invalid output\n"));
        return false;
    }
  
    bool bRet = false;
    bool bResolved = false;

    if (input[1] == ':') {
        // Try QueryDosDevice first to see if the drive has been substed.
        nlstring prefix(input, 2);
        nlstring suffix(input, 2, SIZE_MAX);
        DWORD len = 0;

        if(outputBufLen < 8+suffix.length()) {
            TRACE(0, _T("NL_GetFilePhysicalPath: output buffer is too small\n"));
            return false;
        }

        do {
            nlchar buf[MAX_PATH] = {0};
            len = ::QueryDosDevice (prefix.c_str(), buf, MAX_PATH);
            if (len > 0 && nlstrncmp(buf, _T("\\??\\UNC"), 7) == 0) {
                // This resolved to a network file. Copy to buffer and exit loop
                nlstrcpy_s (output, outputBufLen, _T("\\"));
                nlstrcat_s (output, outputBufLen, buf + 7);
                nlstrcat_s (output, outputBufLen, suffix.c_str());
                bResolved = true;
                bRet = true;
            } else if (len > 0 && 
                       wcsncmp(buf, L"\\??\\", 4) == 0 && 
                       buf[5] == ':') {
                // Resolved to a dos drive. Continue with loop since
                // this may be a reference to another subst
                nlstrcpy_s(output, outputBufLen,
                           (std::wstring(buf, 4, SIZE_MAX) + suffix).c_str());
                bResolved=true;
                bRet = true;
            } else {
                // did not resolve to anything. append suffix to prefix and return
                nlstrcpy_s (output, outputBufLen, (prefix + suffix).c_str());
                bResolved = true;
            }
        } while (!bResolved);

        nlchar *pBuf=NULL;
        nlchar drivePrefix [4] = {0};
        nlstrncpy_s (drivePrefix, _countof(drivePrefix), output, 3);

        if (!bRet && ::GetDriveType (drivePrefix) == DRIVE_REMOTE) {
            //May be a mapped network drive.
            DWORD size = MAX_PATH;
            pBuf = new nlchar[size]; 
            //wcscpy (pszwRealFileName, L"s:\\development\\temp\\generator.zip");
            DWORD ret = ::WNetGetUniversalName(output, UNIVERSAL_NAME_INFO_LEVEL, 
                                               pBuf, &size);
            if (ret == ERROR_MORE_DATA) {
                delete [] pBuf;
                pBuf = new nlchar[size];
                ret = ::WNetGetUniversalName(output, UNIVERSAL_NAME_INFO_LEVEL, 
                                             pBuf, &size);
            }
            if (ret == NO_ERROR) {
                UNIVERSAL_NAME_INFO* pInfo = (UNIVERSAL_NAME_INFO*) pBuf;
                nlstrncpy_s (output, outputBufLen, pInfo->lpUniversalName, 
                             nlstrlen(pInfo->lpUniversalName));
                output[nlstrlen(pInfo->lpUniversalName)]=0;
                bRet = true;
            }
            delete [] pBuf;
        } 
    }
  
    return (bRet);
}
