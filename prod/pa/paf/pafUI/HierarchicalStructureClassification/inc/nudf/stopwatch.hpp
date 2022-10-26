
#ifndef __NUDF_STOPWATCH_HPP__
#define __NUDF_STOPWATCH_HPP__


#include <Windows.h>

namespace nudf
{
namespace util
{

class CStopWatch
{
public:
    CStopWatch();
    CStopWatch(_In_ bool highresolution, _In_ bool autostart);
    virtual ~CStopWatch();

    void Start() throw();
    void Stop() throw();

    inline __int64 GetElapse() const throw() {return _elapse;}

private:
    __int64 _start;
    __int64 _elapse;
    __int64 _frequency;
};

class CHighResolStopWatch : public CStopWatch
{
public:
    CHighResolStopWatch() : CStopWatch(true, false) {}
    virtual ~CHighResolStopWatch() {}
};

class CLowResolStopWatch : public CStopWatch
{
public:
    CLowResolStopWatch() : CStopWatch(false, false) {}
    virtual ~CLowResolStopWatch() {}
};

class CAutoStopWatch : public CStopWatch
{
public:
    CAutoStopWatch();
    CAutoStopWatch(_In_ bool highresolution);
    virtual ~CAutoStopWatch();
};

class CAutoHighResolStopWatch : public CAutoStopWatch
{
public:
    CAutoHighResolStopWatch() : CAutoStopWatch(true) {}
    virtual ~CAutoHighResolStopWatch() {}
};

class CAutoLowResolStopWatch : public CAutoStopWatch
{
public:
    CAutoLowResolStopWatch() : CAutoStopWatch(false) {}
    virtual ~CAutoLowResolStopWatch() {}
};



}   // namespace nudf::util
}   // namespace nudf




#endif  // #ifndef __NUDF_ENCODING_HPP__
