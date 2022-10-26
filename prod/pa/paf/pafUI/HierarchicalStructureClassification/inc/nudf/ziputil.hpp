
#ifndef _NUDF_ZIP_HPP__
#define _NUDF_ZIP_HPP__

#include <string>
#include <vector>

namespace nudf {
namespace util {



class CZip
{
public:
    CZip();
    ~CZip();

public:
    bool Zip(_In_ const std::wstring& source, _In_ const std::wstring& zipfile, _In_opt_ LPSECURITY_ATTRIBUTES sa);
    bool Unzip(_In_ const std::wstring& zipfile, _In_ const std::wstring& targetdir);

protected:
    bool CreateEmptyZip(_In_ const std::wstring& zipfile, _In_opt_ LPSECURITY_ATTRIBUTES sa);
    bool VerifyFileOpIsDone(_In_ const std::wstring& file, _In_ DWORD dwWait=INFINITE);
};


    
}   // namespace nudf::util
}   // namespace nudf

#endif  // _NUDF_ZIP_HPP__