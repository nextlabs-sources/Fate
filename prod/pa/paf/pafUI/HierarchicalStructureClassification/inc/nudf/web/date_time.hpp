

#ifndef __NUDF_DATE_TIME_HPP__
#define __NUDF_DATE_TIME_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono> 

namespace NX {
namespace utility {



// Left over from VS2010 support, remains to avoid breaking.
typedef std::chrono::seconds seconds;

/// Functions for converting to/from std::chrono::seconds to xml string.
namespace timespan
{
    /// <summary>
    /// Converts a timespan/interval in seconds to xml duration string as specified by
    /// http://www.w3.org/TR/xmlschema-2/#duration
    /// </summary>
    std::wstring __cdecl seconds_to_xml_duration(utility::seconds numSecs);

    /// <summary>
    /// Converts an xml duration to timespan/interval in seconds
    /// http://www.w3.org/TR/xmlschema-2/#duration
    /// </summary>
    utility::seconds __cdecl xml_duration_to_seconds(std::wstring timespanString);
}


class datetime
{
public:
    typedef uint64_t interval_type;

    /// <summary>
    /// Defines the supported date and time string formats.
    /// </summary>
    enum date_format { RFC_1123, ISO_8601 };

    /// <summary>
    /// Returns the current UTC time.
    /// </summary>
    static datetime __cdecl utc_now();

    /// <summary>
    /// An invalid UTC timestamp value.
    /// </summary>
    enum:interval_type { utc_timestamp_invalid = static_cast<interval_type>(-1) };

    /// <summary>
    /// Returns seconds since Unix/POSIX time epoch at 01-01-1970 00:00:00.
    /// If time is before epoch, utc_timestamp_invalid is returned.
    /// </summary>
    static interval_type utc_timestamp()
    {
        const auto seconds = utc_now().to_interval() / _secondTicks;
        if (seconds >= 11644473600LL) {
            return seconds - 11644473600LL;
        }
        else {
            return utc_timestamp_invalid;
        }
    }

    datetime() : m_interval(0)
    {
    }

    /// <summary>
    /// Creates <c>datetime</c> from a string representing time in UTC in RFC 1123 format.
    /// </summary>
    static datetime __cdecl from_string(const std::wstring& timestring, date_format format = RFC_1123);

    /// <summary>
    /// Returns a string representation of the <c>datetime</c>.
    /// </summary>
    std::wstring to_string(date_format format = RFC_1123) const;

    /// <summary>
    /// Returns the integral time value.
    /// </summary>
    interval_type to_interval() const
    {
        return m_interval;
    }

    datetime operator- (interval_type value) const
    {
        return datetime(m_interval - value);
    }

    datetime operator+ (interval_type value) const
    {
        return datetime(m_interval + value);
    }

    bool operator== (datetime dt) const
    {
        return m_interval == dt.m_interval;
    }

    bool operator!= (const datetime& dt) const
    {
        return !(*this == dt);
    }

    static interval_type from_milliseconds(unsigned int milliseconds)
    {
        return milliseconds*_msTicks;
    }

    static interval_type from_seconds(unsigned int seconds)
    {
        return seconds*_secondTicks;
    }

    static interval_type from_minutes(unsigned int minutes)
    {
        return minutes*_minuteTicks;
    }

    static interval_type from_hours(unsigned int hours)
    {
        return hours*_hourTicks;
    }

    static interval_type from_days(unsigned int days)
    {
        return days*_dayTicks;
    }

    bool is_initialized() const
    {
        return m_interval != 0;
    }

private:

    friend int operator- (datetime t1, datetime t2);

    static const interval_type _msTicks = static_cast<interval_type>(10000);
    static const interval_type _secondTicks = 1000*_msTicks;
    static const interval_type _minuteTicks = 60*_secondTicks;
    static const interval_type _hourTicks   = 60*60*_secondTicks;
    static const interval_type _dayTicks    = 24*60*60*_secondTicks;

    // void* to avoid pulling in windows.h
    static bool __cdecl datetime::system_type_to_datetime(/*SYSTEMTIME*/ void* psysTime, uint64_t seconds, datetime * pdt);

    // Private constructor. Use static methods to create an instance.
    datetime(interval_type interval) : m_interval(interval)
    {
    }

    interval_type m_interval;
};

inline int operator- (datetime t1, datetime t2)
{
    auto diff = (t1.m_interval - t2.m_interval);
    // Round it down to seconds
    diff /= 10 * 1000 * 1000;
    return static_cast<int>(diff);
}


}   // namespace utitlity
}   // namespace NX


#endif
