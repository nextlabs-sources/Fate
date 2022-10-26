

#ifndef __NUDF_LOG_HPP__
#define __NUDF_LOG_HPP__



#include <string>
#include <queue>
#include <nudf\shared\logdef.h>

namespace nudf
{
namespace util
{
namespace log
{


#define MIN_LOG_QUEUE_SIZE      256
#define MAX_LOG_QUEUE_SIZE      4096
#define DEFAULT_LOG_QUEUE_SIZE  1024
#define MIN_LOG_FILE_SIZE       16384       // (1024 * 16, 16 KB)
#define MAX_LOG_FILE_SIZE       67108864    // (1024 * 1024 * 64, 64 MB)
#define DEFAULT_LOG_FILE_SIZE   1048576     // (1024 * 1024, 1 MB)
#define MAX_LOG_ROTATION        10
#define DEFAULT_LOG_ROTATION    0

class CLogEntry
{
public:
    CLogEntry();
    CLogEntry(_In_ LOGLEVEL Level, _In_opt_ LPCSTR Module, _In_ LPCWSTR Info, ...);
    CLogEntry(_In_ LOGLEVEL Level, _In_opt_ LPCSTR Module, _In_ LONG Error, _In_ LPCSTR File, _In_ LPCSTR Function, _In_ LONG Line, _In_ LPCWSTR Info, ...);
    virtual ~CLogEntry();

    inline const std::wstring& GetData() const throw() {return _Data;}
    inline CLogEntry& operator = (const CLogEntry& le) throw() {if(this != &le) {_Data = le.GetData();} return *this;}
    
private:
    std::wstring _Data;
};

class CLog
{
public:
    CLog();
    virtual ~CLog();

    virtual VOID Start();
    virtual VOID Stop();

    inline LOGLEVEL GetAcceptLevel() const throw() {return _AcceptLevel;}
    inline BOOL AcceptLevel(_In_ LOGLEVEL Level) const throw() {return (_AcceptLevel < Level) ? FALSE : TRUE;}
    inline ULONG GetQueueSize() const throw() {return _QueueSize;}
    inline VOID SetAcceptLevel(_In_ LOGLEVEL Level) throw() {_AcceptLevel = Level;}
    inline VOID SetQueueSize(_In_ ULONG Size) throw() {_QueueSize = (Size < MIN_LOG_QUEUE_SIZE) ? MIN_LOG_QUEUE_SIZE : ((Size > MAX_LOG_QUEUE_SIZE) ? MAX_LOG_QUEUE_SIZE : Size);}
    inline BOOL IsRunning() const throw() {return _Active;}

    BOOL Push(_In_ const CLogEntry& le) throw();
    BOOL Push(_In_ LPCWSTR Info) throw();
    DWORD WorkerThread() throw();

protected:
    virtual VOID LogEntry(_In_ const std::wstring& s) throw() = 0;
    
private:
    LOGLEVEL        _AcceptLevel;
    ULONG           _QueueSize;
    std::queue<std::wstring> _Queue;
    CRITICAL_SECTION        _QueueLock;
    HANDLE          _Events[2];
    HANDLE          _Thread;
    BOOL            _Active;
};

class CFileLog : public CLog
{
public:
    CFileLog();
    virtual ~CFileLog();

    virtual VOID Start();
    virtual VOID Stop();

    VOID SetLogDir(_In_opt_ LPCWSTR Dir) throw();
    VOID SetLogName(_In_ LPCWSTR Name) throw();

    inline VOID SetFileSizeLimit(_In_ ULONG Size) throw() {_FileSize = (Size < MIN_LOG_FILE_SIZE) ? MIN_LOG_FILE_SIZE : ((Size > MAX_LOG_FILE_SIZE) ? MAX_LOG_FILE_SIZE : Size);}
    inline VOID SetRotation(_In_ ULONG Rotate) throw() {_Rotation = (Rotate > MAX_LOG_ROTATION) ? MAX_LOG_ROTATION : Rotate;}
    inline const std::wstring& GetLogDir() const throw() {return _LogDir;}
    inline const std::wstring& GetLogName() const throw() {return _LogName;}
    inline ULONG GetFileSizeLimit() const throw() {return _FileSize;}
    inline ULONG GetRotation() const throw() {return _Rotation;}

protected:
    VOID Rotate();
    virtual VOID LogEntry(_In_ const std::wstring& s) throw();

private:
    std::wstring    _LogDir;
    std::wstring    _LogName;
    const std::wstring _LogExt;
    HANDLE          _FileHandle;
    ULONG           _FileSize;
    ULONG           _Rotation;
};



}   // namespace nudf::util::log
}   // namespace nudf::util
}   // namespace nudf




#endif  // #ifndef __NUDF_LOG_HPP__