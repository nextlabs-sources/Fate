


#ifndef __NUDF_ERROR_CATEGORY_HPP__
#define __NUDF_ERROR_CATEGORY_HPP__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>


#include <nudf\web\conversions.hpp>

namespace NX {
namespace utility {




/// <summary>
/// Category error type for Windows OS errors.
/// </summary>
class windows_category_impl : public std::error_category
{
public:
    virtual const char *name() const throw() { return "windows"; }

    virtual std::string message(int errorCode) const throw();

    virtual std::error_condition default_error_condition(int errorCode) const throw();
};

/// <summary>
/// Gets the one global instance of the windows error category.
/// </summary>
/// </returns>An error category instance.</returns>
const std::error_category & __cdecl windows_category();


/// <summary>
/// Gets the one global instance of the current platform's error category.
/// <summary>
const std::error_category & __cdecl platform_category();

/// <summary>
/// Creates an instance of std::system_error from a OS error code.
/// </summary>
inline std::system_error __cdecl create_system_error(unsigned long errorCode)
{
    std::error_code code((int)errorCode, platform_category());
    return std::system_error(code, code.message());
}

/// <summary>
/// Creates a std::error_code from a OS error code.
/// </summary>
inline std::error_code __cdecl create_error_code(unsigned long errorCode)
{
    return std::error_code((int)errorCode, platform_category());
}

/// <summary>
/// Creates the corresponding error message from a OS error code.
/// </summary>
inline std::wstring __cdecl create_error_message(unsigned long errorCode)
{
    return utility::conversions::to_string(create_error_code(errorCode).message());
}


}   // namespace utitlity
}   // namespace NX


#endif
