

#ifndef _NUDF_RESUTIL_HPP__
#define _NUDF_RESUTIL_HPP__


#include <Windows.h>
#include <assert.h>
#include <string>

namespace nudf {
namespace util {
namespace res {

#define MAX_MESSAGE_LENGTH  2048

std::wstring LoadMessage(_In_ HMODULE module, _In_ UINT id, _In_ ULONG max_length, _In_ DWORD langid=LANG_NEUTRAL, _In_opt_ LPCWSTR default_msg=NULL);
std::wstring LoadMessageEx(_In_ HMODULE module, _In_ UINT id, _In_ ULONG max_length, _In_ DWORD langid, _In_opt_ LPCWSTR default_msg, ...);


}   // namespace nudf::util::res
}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_RESUTIL_HPP__