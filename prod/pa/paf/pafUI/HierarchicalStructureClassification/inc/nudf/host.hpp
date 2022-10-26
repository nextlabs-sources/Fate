

#ifndef __NUDF_HOST_HPP__
#define __NUDF_HOST_HPP__

#include <string>

namespace nudf
{
namespace win
{
    
class CHost
{
public:
    CHost();
    virtual ~CHost();
    
    const std::wstring& GetHostName() const throw();
    const std::wstring& GetLocaleName() const throw();
    void Clear() throw();
    CHost& operator = (const CHost& host) throw();

private:
    mutable std::wstring _host;
    mutable std::wstring _locale;
};




}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_HOST_HPP__