

#ifndef __NUDF_SERVICE_HPP__
#define __NUDF_SERVICE_HPP__

#include <Windows.h>
#include <Dbt.h>
#include <assert.h>

#include <string>
#include <vector>


namespace nudf
{
namespace win
{
namespace svc
{


class CRefPtr
{
public:
    CRefPtr() : _ref(0), _p(NULL)
    {
        ::InitializeCriticalSection(&_lock);
    }

    virtual ~CRefPtr() throw()
    {
        ::DeleteCriticalSection(&_lock);
    }

    PVOID Get() throw()
    {
        if(NULL != _p) {
            _ref++;
        }
        return _p;
    }

    VOID Set(PVOID p) throw()
    {
        assert(0 == _ref);
        _p = p;
        _ref = p ? 1 : 0;
    }

    ULONG Release() throw()
    {
        if(_ref != 0) {
            _ref--;
            if(0 == _ref) {
                _p = NULL;
            }
        }
        return _ref;
    }

    VOID Lock() throw()
    {
        EnterCriticalSection(&_lock);
    }

    VOID Unlock() throw()
    {
        LeaveCriticalSection(&_lock);
    }

private:
    PVOID   _p;
    ULONG   _ref;
    CRITICAL_SECTION _lock;
};

extern nudf::win::svc::CRefPtr _RefIServicePtr;

template <class T>
class IServiceInstance
{
public:
    IServiceInstance() : p(NULL)
    {
        _RefIServicePtr.Lock();
        p = (T*)_RefIServicePtr.Get();
        if(NULL == p) {
            p = new T;
            if(NULL != p) {
                _RefIServicePtr.Set(p);
            }
        }
        _RefIServicePtr.Unlock();
    }

    ~IServiceInstance()
    {
        _RefIServicePtr.Lock();
        if(p) {
            if(0 == _RefIServicePtr.Release()) {
                delete p;
            }
            p = NULL;
        }
        _RefIServicePtr.Unlock();
    }

    T* operator ->() const {return p;}

private:
    T* p;

private:
    IServiceInstance<T> operator = (const IServiceInstance<T>& inst);
};


class IService
{
public:
    IService();
    IService(_In_ LPCWSTR Name);
    virtual ~IService();

    VOID Run();
    LONG Start(_In_ int Argc, _In_ LPCWSTR* Argv) throw();
    

    inline BOOL IsRunning() const throw() {return (SERVICE_RUNNING == m_Status.dwCurrentState);}
    inline BOOL IsStopped() const throw() {return (SERVICE_STOPPED == m_Status.dwCurrentState);}
    inline BOOL IsPaused() const throw() {return (SERVICE_PAUSED == m_Status.dwCurrentState);}
    inline BOOL IsPending() const throw() {return (SERVICE_START_PENDING == m_Status.dwCurrentState ||
                                                   SERVICE_STOP_PENDING == m_Status.dwCurrentState ||
                                                   SERVICE_PAUSE_PENDING == m_Status.dwCurrentState ||
                                                   SERVICE_CONTINUE_PENDING == m_Status.dwCurrentState);}
    inline BOOL IsStartPending() const throw() {return (SERVICE_START_PENDING == m_Status.dwCurrentState);}
    inline BOOL IsStopPending() const throw() {return (SERVICE_STOP_PENDING == m_Status.dwCurrentState);}
    inline BOOL IsPausePending() const throw() {return (SERVICE_PAUSE_PENDING == m_Status.dwCurrentState);}
    inline BOOL IsContinuePending() const throw() {return (SERVICE_CONTINUE_PENDING == m_Status.dwCurrentState);}
    
    
    // service events
    virtual void OnStart(){}
    virtual void OnStop() throw() {}
    virtual void OnPause(){}
    virtual void OnResume(){}
    virtual void OnPreshutdown() throw() {}
    virtual void OnShutdown() throw() {}
    virtual void OnParamChange() throw() {}
    virtual void OnNetbindAdd() throw() {}
    virtual void OnNetbindRemove() throw() {}
    virtual void OnNetbindEnable() throw() {}
    virtual void OnNetbindDisable() throw() {}

    // device events
    virtual LONG OnDeviceArrival(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnDeviceRemoveComplete(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnDeviceQueryRemove(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnDeviceQueryRemoveFailed(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnDeviceRemovePending(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnDeviceCustomEvent(_In_ DEV_BROADCAST_HDR* dbch) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }

    // hardware profile events
    virtual LONG OnHwprofileChanged() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnHwprofileQueryChange() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnHwprofileChangeCanceled() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }

    // power events
    virtual LONG OnPowerStatusChanged() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnPowerResumeAuto() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnPowerResumeSuspend() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnPowerSuspend() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnPowerSettingChanged(_In_ POWERBROADCAST_SETTING* pbs) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnPowerLowbattery() throw() { return ERROR_CALL_NOT_IMPLEMENTED; }

    // session events
    virtual LONG OnSessionConn(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionDisconn(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionRemoteConn(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionRemoteDisconn(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionLogon(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionLogoff(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionLock(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionUnlock(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }
    virtual LONG OnSessionRemoteControl(_In_ WTSSESSION_NOTIFICATION* wtsn) throw() { return ERROR_CALL_NOT_IMPLEMENTED; }

public:
    LONG Control(_In_ ULONG ControlCode, _In_ ULONG EvtType, _In_opt_ LPVOID EvtData, _In_opt_ LPVOID Context) throw();

private:
    // Control
    VOID Stop() throw();
    LONG Pause() throw();
    LONG Resume() throw();
    VOID SetStatus(_In_ ULONG State, _In_ ULONG ExitCode, _In_ ULONG Hint) throw();
    
private:
    std::wstring            m_Name;
    SERVICE_STATUS          m_Status;
    SERVICE_STATUS_HANDLE   m_hStatus;
    HANDLE                  m_hStopEvt;
    ULONG                   m_StopHint;

private:
};

class CConfig
{
public:
    CConfig() throw();
    ~CConfig() throw();

    VOID Load(_In_ SC_HANDLE hService);
    VOID Clear() throw();
    CConfig& operator= (const CConfig& Config) throw();

    inline ULONG GetServiceType() const throw() {return m_dwServiceType;}
    inline ULONG GetStartType() const throw() {return m_dwStartType;}
    inline ULONG GetErrorControl() const throw() {return m_dwErrorControl;}
    inline ULONG GetTagId() const throw() {return m_dwTagId;}
    inline const std::wstring& GetBinaryPathName() const throw() {return m_wsBinaryPathName;}
    inline const std::wstring& GetLoadOrderGroup() const throw() {return m_wsLoadOrderGroup;}
    inline const std::wstring& GetServiceStartName() const throw() {return m_wsServiceStartName;}
    inline const std::wstring& GetDisplayName() const throw() {return m_wsDisplayName;}
    inline const std::vector<std::wstring>& GetDependencies() const throw() {return m_vDependencies;}
    
    inline VOID SetServiceType(_In_ ULONG Type) throw() {m_dwServiceType=Type;}
    inline VOID SetStartType(_In_ ULONG Type) throw() {m_dwStartType=Type;}
    inline VOID SetErrorControl(_In_ ULONG Type) throw() {m_dwErrorControl=Type;}
    inline VOID SetTagId(_In_ ULONG Id) throw() {m_dwTagId=Id;}
    inline VOID SetBinaryPathName(_In_ LPCWSTR Name) throw() {m_wsBinaryPathName=Name;}
    inline VOID SetLoadOrderGroup(_In_ LPCWSTR Name) throw() {m_wsLoadOrderGroup=Name;}
    inline VOID SetServiceStartName(_In_ LPCWSTR Name) throw() {m_wsServiceStartName=Name;}
    inline VOID SetDisplayName(_In_ LPCWSTR Name) throw() {m_wsDisplayName=Name;}
    inline VOID ClearDependency(_In_ LPCWSTR Name) throw() {m_vDependencies.clear();}
    inline BOOL AddDependency(_In_ LPCWSTR Name) throw()
    {
        if(NULL == Name || L'\0'==Name[0]) {
            return FALSE;
        }
        for(std::vector<std::wstring>::const_iterator it=m_vDependencies.begin(); it!=m_vDependencies.end(); ++it) {
            if(0 == _wcsicmp(Name, (*it).c_str())) {
                return FALSE;
            }
        }
        m_vDependencies.push_back(Name);
        return TRUE;
    }
    inline VOID RemoveDependency(_In_ LPCWSTR Name) throw()
    {
        if(NULL == Name || L'\0'==Name[0]) {
            return;
        }
        for(std::vector<std::wstring>::const_iterator it=m_vDependencies.begin(); it!=m_vDependencies.end(); ++it) {
            if(0 == _wcsicmp(Name, (*it).c_str())) {
                m_vDependencies.erase(it);
                return;
            }
        }
    }
    inline VOID SetDependencies(_In_ const std::vector<std::wstring>& Dependencies) throw() {m_vDependencies = Dependencies;}

private:
  DWORD  m_dwServiceType;
  DWORD  m_dwStartType;
  DWORD  m_dwErrorControl;
  DWORD  m_dwTagId;
  std::wstring m_wsBinaryPathName;
  std::wstring m_wsLoadOrderGroup;
  std::wstring m_wsServiceStartName;
  std::wstring m_wsDisplayName;
  std::vector<std::wstring> m_vDependencies;
};

class CService
{
public:
    CService();
    virtual ~CService();    

    virtual VOID Create(_In_ LPCWSTR Name, _In_ const CConfig& Config);
    virtual VOID Open(_In_ LPCWSTR Name, _In_ BOOL ReadOnly);
    VOID Close() throw();

    VOID Start(_In_ BOOL StartDependencies, _In_ ULONG WaitTime=INFINITE);
    VOID Stop(_In_ ULONG WaitTime=INFINITE);
    VOID Pause(_In_ ULONG WaitTime=INFINITE);
    VOID Resume(_In_ ULONG WaitTime=INFINITE);
    VOID Enable(_In_ ULONG StartType);
    VOID Disable();
    VOID Delete(_In_ ULONG WaitTime=INFINITE);
    ULONG GetStatus(_Out_ LPSERVICE_STATUS Status, _In_ ULONG PendingWaitTime=0) throw();

    inline const CConfig& GetConfig() const throw() {return m_Config;}
    inline CConfig& GetConfig() throw() {return m_Config;}
    inline BOOL IsDisabled() const throw() {return (SERVICE_DISABLED == m_Config.GetStartType());}

    VOID SetBinaryPathName(_In_ const std::wstring& BinaryPathName);
    VOID SetLoadOrderGroup(_In_ const std::wstring& LoadOrderGroup);
    VOID SetDisplayName(_In_ const std::wstring& DisplayName);
    VOID SetDescription(_In_ const std::wstring& Description);
    VOID SetServiceType(_In_ ULONG ServiceType);
    VOID SetStartType(_In_ ULONG StartType);
    VOID SetErrorControl(_In_ ULONG ErrorControl);
    VOID SetDependencies(_In_ const std::vector<std::wstring>& Dependencies);

private:
    std::wstring    m_Name;
    SC_HANDLE       m_hSvcMgr;
    SC_HANDLE       m_hSvc;
    CConfig         m_Config;
};

__forceinline
BOOL
IsPendingState(_In_ ULONG State)
{
    return (State == SERVICE_START_PENDING ||
            State == SERVICE_STOP_PENDING ||
            State == SERVICE_PAUSE_PENDING ||
            State == SERVICE_CONTINUE_PENDING) ? TRUE : FALSE;
}

BOOL Exist(_In_ LPCWSTR Name) throw();
VOID Delete(_In_ LPCWSTR Name);


}   // namespace nudf::win::svc
}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_SERVICE_HPP__