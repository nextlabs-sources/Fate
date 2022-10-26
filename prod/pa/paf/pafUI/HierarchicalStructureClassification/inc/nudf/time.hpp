

#ifndef __NUDF_TIME_HPP__
#define __NUDF_TIME_HPP__

#include <Windows.h>
#include <time.h>

namespace nudf {
namespace time {


// Timedifference between 1601-01-01T00:00:00Z and 1970-01-01T00:00:00Z
#define TIME_DIFF   (116444736000000000ULL)
    

class CTimeZone
{
public:
    CTimeZone() : _status(TIME_ZONE_ID_UNKNOWN)
    {
        memset(&_info, 0, sizeof(_info));
        _status = GetTimeZoneInformation(&_info);
    }

    CTimeZone(_In_ LPCWSTR wzBias) : _status(TIME_ZONE_ID_UNKNOWN)
    {
        BiasFromString(wzBias);
    }

    ~CTimeZone()
    {
    }

    inline DWORD GetStatus() const throw() {return _status;}
    inline bool IsUnknown() const throw() {return (TIME_ZONE_ID_UNKNOWN == _status) ? true : false;}
    inline bool IsStandard() const throw() {return (TIME_ZONE_ID_STANDARD == _status) ? true : false;}
    inline bool IsDaylight() const throw() {return (TIME_ZONE_ID_DAYLIGHT == _status) ? true : false;}

    inline const TIME_ZONE_INFORMATION& GetTimeZoneInfo() const throw() {return _info;}

    inline LONG GetBias() const throw()
    {
        LONG bias = _info.Bias;
        switch (_status)
        {
        case TIME_ZONE_ID_STANDARD:
            bias += _info.StandardBias;
            break;
        case TIME_ZONE_ID_DAYLIGHT:
            bias += _info.DaylightBias;
            break;
        default:
            break;
        }
        return bias;
    }
    inline LONG GetUnsignedBias() const throw()
    {
        LONG bias = GetBias();
        return (bias < 0) ? (0 - bias) : bias;
    }

    CTimeZone& operator = (const CTimeZone& zone) throw()
    {
        if(this != &zone) {
            _status = zone.GetStatus();
            memcpy(&_info, &zone.GetTimeZoneInfo(), sizeof(_info));
        }
        return *this;
    }


    void BiasToString(std::wstring& bias) const throw()
    {
        LONG  ulBias = GetUnsignedBias();
        WCHAR wzBias[32] = {0};
        memset(wzBias, 0, sizeof(wzBias));
        wzBias[0] = (_info.Bias > 0) ? L'-' : L'+';
        swprintf_s(wzBias+1, 31, L"%02d:%02d", ulBias/60, ulBias%60);
        bias = wzBias;
    }

    BOOL BiasFromString(_In_ LPCWSTR wzBias) throw()
    {
        LONG lHour   = 0;
        LONG lMinute = 0;

        memset(&_info, 0, sizeof(_info));
        
        // [+|-]HH:MM
        if(6 != wcslen(wzBias)) {
            return FALSE;
        }

        swscanf_s(wzBias+1, L"%02d:%02d", &lHour, &lMinute);
        lMinute += (lHour * 60);
        if(L'-' == wzBias[0]) {
            _info.Bias = lMinute;
            return TRUE;
        }
        else if(L'+' == wzBias[0]) {
            _info.Bias = (0 - lMinute);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

private:
    TIME_ZONE_INFORMATION   _info;
    DWORD                   _status;
};


class CTime
{
public:
    CTime()
    {
        _time.dwHighDateTime = 0;
        _time.dwLowDateTime = 0;
    }

    explicit CTime(_In_ const SYSTEMTIME* pst)
    {
        FromSystemTime(pst);
    }

    explicit CTime(_In_ const FILETIME* pft)
    {
        memcpy(&_time, pft, sizeof(FILETIME));
    }

    explicit CTime(_In_ ULONGLONG ull)
    {
        _time.dwLowDateTime  = (DWORD)(ull & 0xFFFFFFFF);
        _time.dwHighDateTime = (DWORD)(ull >> 32);
    }

    virtual ~CTime() throw()
    {
    }

    void Now()
    {
        GetSystemTimeAsFileTime(&_time);
    }

    CTime& operator = (const CTime& time) throw()
    {
        if(this != &time) {
            memcpy(&_time, &time.GetTime(), sizeof(FILETIME));
        }
        return *this;
    }

    // Equal To
    bool EqualTo(const CTime& time) const throw()
    {
        return (0 == CompareFileTime(&_time, &time.GetTime())) ? true : false;
    }
    bool operator == (const CTime& time) const throw()
    {
        return EqualTo(time);
    }

    // Earlier than
    bool EarlierThan(const CTime& time) const throw()
    {
        return (-1 == CompareFileTime(&_time, &time.GetTime())) ? true : false;
    }
    bool operator < (const CTime& time) const throw()
    {
        return EarlierThan(time);
    }

    // Later than
    bool LaterThan(const CTime& time) const throw()
    {
        return (1 == CompareFileTime(&_time, &time.GetTime())) ? true : false;
    }
    bool operator > (const CTime& time) const throw()
    {
        return LaterThan(time);
    }

    void ToLocalTime(_In_ const CTimeZone* zone) throw()
    {
        if(NULL == zone) {
            FILETIME tmp = {_time.dwLowDateTime, _time.dwHighDateTime};
            FileTimeToLocalFileTime(&tmp, &_time);
        }
        else {
            SYSTEMTIME st = {0};
            SYSTEMTIME utc = {0};
            ToSystemTime(&st);
            SystemTimeToTzSpecificLocalTime(&zone->GetTimeZoneInfo(), &st, &utc);
            SystemTimeToFileTime(&utc, &_time);
        }
    }

    void ToUtcTime(_In_ const CTimeZone* zone) throw()
    {
        if(NULL == zone) {
            FILETIME tmp = {_time.dwLowDateTime, _time.dwHighDateTime};
            LocalFileTimeToFileTime(&tmp, &_time);
        }
        else {
            SYSTEMTIME st = {0};
            SYSTEMTIME utc = {0};
            ToSystemTime(&st);
            TzSpecificLocalTimeToSystemTime(&zone->GetTimeZoneInfo(), &st, &utc);
            SystemTimeToFileTime(&utc, &_time);
        }
    }

    ULONGLONG ToUll() const throw()
    {
        ULONGLONG ull = _time.dwHighDateTime;
        ull <<= 32;
        ull += _time.dwLowDateTime;
        return ull;
    }

    void FromUll(ULONGLONG ull) throw()
    {
        _time.dwLowDateTime  = (DWORD)(ull & 0xFFFFFFFF);
        _time.dwHighDateTime = (DWORD)(ull >> 32);
    }

    time_t ToSecondsSince1970Jan1st()
    {
        LONGLONG ll = _time.dwHighDateTime;
        ll <<= 32;
        ll += _time.dwLowDateTime;
        ll -= TIME_DIFF;
        ll /= 10000000;
        return (time_t)ll;
    }

    void FromSecondsSince1970Jan1st(time_t t)
    {
        LONGLONG ll = Int32x32To64(t, 10000000) + TIME_DIFF;
        _time.dwLowDateTime = (DWORD)ll;
        _time.dwHighDateTime = ll >>32;
    }

    __int64 ToMilliSecondsSince1970Jan1st()
    {
        LONGLONG ll = _time.dwHighDateTime;
        ll <<= 32;
        ll += _time.dwLowDateTime;
        ll -= TIME_DIFF;
        ll /= 10000;
        return (time_t)ll;
    }

    void FromMilliSecondsSince1970Jan1st(__int64 milliseconds)
    {
        LONGLONG ll = Int32x32To64(milliseconds, 10000) + TIME_DIFF;
        _time.dwLowDateTime = (DWORD)ll;
        _time.dwHighDateTime = ll >>32;
    }

    void ToSystemTime(SYSTEMTIME* pst) const throw()
    {
        FileTimeToSystemTime(&_time, pst);
    }

    void FromSystemTime(const SYSTEMTIME* pst) throw()
    {
        if(!SystemTimeToFileTime(pst, &_time)) {
            _time.dwHighDateTime = 0;
            _time.dwLowDateTime = 0;
        }
    }

    void AddSeconds(ULONGLONG dwSeconds) throw()
    {
        static const ULONGLONG ratio = 10000000;
        ULONGLONG ull = ToUll();
        ull += (dwSeconds * ratio);
        FromUll(ull);
    }

    void AddMinutes(ULONGLONG dwMinutes) throw()
    {
        AddSeconds(dwMinutes * 60);
    }

    void AddHours(ULONGLONG dwHours) throw()
    {
        AddSeconds(dwHours * 3600);
    }

    void AddDays(ULONGLONG dwDays) throw()
    {
        AddSeconds(dwDays * 86400);
    }

    void SubSeconds(ULONGLONG dwSeconds) throw()
    {
        static const ULONGLONG ratio = 10000000;
        ULONGLONG ull = ToUll();
        ull -= (dwSeconds * ratio);
        FromUll(ull);
    }

    void SubMinutes(ULONGLONG dwMinutes) throw()
    {
        SubSeconds(dwMinutes * 60);
    }

    void SubHours(ULONGLONG dwHours) throw()
    {
        SubSeconds(dwHours * 3600);
    }

    void SubDays(ULONGLONG dwDays) throw()
    {
        SubSeconds(dwDays * 86400);
    }

    const FILETIME& GetTime() const throw() {return _time;}
    FILETIME& GetTime() throw() {return _time;}


private:
    FILETIME   _time;
};



}   // namespace time
}   // namespace nudf



#endif