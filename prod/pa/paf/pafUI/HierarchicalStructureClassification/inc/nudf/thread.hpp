

#ifndef _NUDF_THREAD_HPP__
#define _NUDF_THREAD_HPP__


#include <Windows.h>
#include <assert.h>
#include <string>
#include <vector>
#include <memory>

namespace nudf {
namespace util {
namespace thread {


class CThread
{
public:
    CThread();
    virtual ~CThread();

    virtual void Start(_In_opt_ PVOID Context);
    virtual void Stop() throw();

    virtual DWORD OnStart(_In_opt_ PVOID Context)=0;
    virtual DWORD OnRunning(_In_opt_ PVOID Context)=0;
    
protected:
    HANDLE  _h;
};

class CThreadEx : public CThread
{
public:
    CThreadEx();
    virtual ~CThreadEx();

    virtual void Start(_In_opt_ HANDLE hJobEvent, _In_opt_ PVOID Context);
    virtual void Stop() throw();

    virtual DWORD OnStart(_In_opt_ PVOID Context)=0;
    virtual DWORD OnRunning(_In_opt_ PVOID Context)=0;

    inline HANDLE* GetEvents() throw() {return _hEvents;}
    
protected:
    HANDLE  _hEvents[2];
    BOOL    _bExternalJobEvent;
};


template <class T>
class CThreadPool
{
public:
    CThreadPool()
    {
    }

    virtual ~CThreadPool()
    {
        Stop();
    }

    void Start(_In_ ULONG Size, _In_opt_ LPVOID Context)
    {
        if(Size == 0) {
            throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
        }

        try {
            for(ULONG i=0; i<Size; i++) {
                T* th = new T;
                th->Start(Context);
                _threadobjs.push_back(std::shared_ptr<T>(th));
            }
        }
        catch(const nudf::CException& e) {
            _threadobjs.clear();
            throw e;
        }
    }

    virtual void Stop()
    {
        _threadobjs.clear();
    }

private:
    std::vector<std::shared_ptr<T>> _threadobjs;
};


template <class T>
class CThreadExPool
{
public:
    CThreadExPool() : _hJobEvent(NULL)
    {
    }

    virtual ~CThreadExPool()
    {
        Stop();
    }

    void Start(_In_ ULONG Size, _In_opt_ LPVOID Context)
    {
        if(Size == 0) {
            throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
        }

        _hJobEvent = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        if(NULL == _hJobEvent) {
            throw WIN32ERROR();
        }

        try {
            for(ULONG i=0; i<Size; i++) {
                T* th = new T;
                th->Start(_hJobEvent, Context);
                _threadobjs.push_back(std::shared_ptr<T>(th));
            }
        }
        catch(const nudf::CException& e) {
            _threadobjs.clear();
            CloseHandle(_hJobEvent);
            _hJobEvent = NULL;
            throw e;
        }
    }

    virtual void Stop()
    {
        if(NULL != _hJobEvent) {
            _threadobjs.clear();
            CloseHandle(_hJobEvent);
            _hJobEvent = NULL;
        }
    }

    void SetJobEvent()
    {
        SetEvent(_hJobEvent);
    }

    void ResetJobEvent()
    {
        ResetEvent(_hJobEvent);
    }

private:
    std::vector<std::shared_ptr<T>> _threadobjs;
    HANDLE _hJobEvent;
};

    
}   // namespace nudf::util::thread
}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_THREAD_HPP__