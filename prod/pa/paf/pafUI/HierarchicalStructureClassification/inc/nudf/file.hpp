
#ifndef __NUDF_FILE_HPP__
#define __NUDF_FILE_HPP__


#include <string>
#include <map>

namespace nudf
{
namespace win
{
namespace file
{


class CFile
{
public:
    CFile();
    virtual ~CFile();

    VOID Create();
    VOID Open();
    VOID Close();

    inline HANDLE GetHandle() const throw() {return _Handle;}
    inline const std::wstring& GetPath() const throw() {return _Path;}
    inline BOOL Opened() const throw() {return (INVALID_HANDLE_VALUE != _Handle);}

private:
    HANDLE          _Handle;
    std::wstring    _Path;
};


}   // namespace nudf::win::file
}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_FILE_HPP__