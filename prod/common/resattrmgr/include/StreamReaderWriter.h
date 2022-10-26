
#ifndef STREAM_READER_WRITER_HEAD
#define STREAM_READER_WRITER_HEAD

#include "EncodeSteamMgr.h"

typedef HANDLE (WINAPI *PCreateFileW)(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
typedef BOOL (WINAPI *PCloseHandle)(HANDLE hObject);
typedef BOOL (WINAPI *PReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
typedef BOOL (WINAPI *PWriteFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

class CStreamReaderWriter
{
public:
    CStreamReaderWriter();
    PWCHAR* EnumStream(IN const WCHAR* FileName, IN OUT DWORD* pdwStreamNameCount);
    void ReleaseStreamNameBuffer(IN PWCHAR* StreamNameBuffer, IN DWORD dwStreamNameCount);

    BOOL WriteStreamWithWideChar(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN const WCHAR* StreamData, IN OUT DWORD* pdwWriteSize);
    BOOL ReadStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, _Inout_ EncodeSteamMgr& obEncodeSteamMgrInout);

	BOOL RemoveStream(IN const WCHAR* FileName, IN const WCHAR* StreamName);

    void setCreateFileW(LPVOID pCreateFileW){m_pCreateFileW = (PCreateFileW)pCreateFileW;};
    void setCloseHandle(LPVOID pCloseHandle){m_pCloseHandle = (PCloseHandle)pCloseHandle;};
    void setReadFile(LPVOID pReadFile){m_pReadFile = (PReadFile)pReadFile;};
    void setWriteFile(LPVOID pWriteFile){m_pWriteFile = (PWriteFile)pWriteFile;};

private:
    BOOL InnerWriteStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN const void* StreamData, IN OUT DWORD* pdwWriteSize, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength);
    BOOL InnerReadStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN void* StreamData, IN OUT DWORD* pdwReadSize);

protected:
    BOOL CheckFunctionsValid();
    BOOL CheckKernelFunctionsValid();
    HANDLE CreateStream(IN const WCHAR* FileName, IN const WCHAR* StreamName, IN BOOL ReadOnly, IN BOOL CreateNew);
private:
    PCreateFileW m_pCreateFileW;
    PCloseHandle m_pCloseHandle;
    PReadFile    m_pReadFile;
    PWriteFile   m_pWriteFile;
};

#endif
