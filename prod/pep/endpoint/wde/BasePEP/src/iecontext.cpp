#include "iecontext.h"
#include "iepmapi.h"

HANDLE gSharedFileMutex;
LPCTSTR SHARED_MUTEX_NAME = L"5AA70967-8661-424F-BA00-978CBC35A6DE";

namespace nextlabs
{
    void CIEContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
    {
        table[kHMCoCreateInstance] = true;
        table[kHMCOMNewItem] = true;
        table[kHMCOMRenameItem] = true;
        table[kHMCOMRenameItems] = true;
        table[kHMCOMCopyItems] = true;
        table[kHMCOMMoveItems] = true;
        table[kHMCOMPerformOperations] = true;

        table[kHMDeviceIoControl] = true;
        table[kHMSetFileAttributesW] = true;
        table[kHMAddUsersToEncryptedFile] = true;
        table[kHMSetNamedSecurityInfoW] = true;

        table[kHMGetOpenFileName] = true;
    }

    EventResult CIEContext::EventAfterGetOpenFileName(LPOPENFILENAMEW lpofn)
    {
        boost::scoped_ptr<CEventHander> handler(new CEventHander());
        std::wstring strDest;
        HRESULT hRet;
        LPWSTR pwszCacheDir = NULL;
        hRet = IEGetWriteableFolderPath(FOLDERID_InternetCache, &pwszCacheDir);

        if (SUCCEEDED(hRet))
        {
            DWORD dwResult = WaitForSingleObject(gSharedFileMutex, INFINITE);
            std::wstring tempFile;
            tempFile.append(pwszCacheDir);
            tempFile.append(L"\\low\\nxIETemp.tmp");
            HANDLE hFileHandle = CreateFile(tempFile.c_str(), GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
            if (hFileHandle != INVALID_HANDLE_VALUE)
            {
                DWORD dwBytesReaded = 0; 
                WCHAR strBuf[4096] = {0};

                BOOL bSuccess = ReadFile(hFileHandle, strBuf, 4096, &dwBytesReaded, NULL);
                strDest.append(strBuf);
                CloseHandle(hFileHandle);
            }
            CoTaskMemFree(pwszCacheDir);
            switch (dwResult)
            {
            case WAIT_OBJECT_0:
                if (!ReleaseMutex(gSharedFileMutex))
                {
                    OutputDebugStringW(L"Release gSharedFileMutex error");
                }
                break;
            default:
                OutputDebugStringW(L"Wait error");
                break;
            }
        }
        return handler->HandleUploadAction(lpofn, strDest);
    }
}  // namespace nextlabs