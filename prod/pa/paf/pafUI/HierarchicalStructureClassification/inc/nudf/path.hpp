
#ifndef __NUDF_PATH_HPP__
#define __NUDF_PATH_HPP__


#include <string>
#include <map>

namespace nudf
{
namespace win
{
    
class CPath
{
public:
    CPath() {}
    CPath(_In_ LPCWSTR wzPath) : _Path(wzPath) {}
    virtual ~CPath() {}

    virtual VOID SetPath(_In_ LPCWSTR wzPath) throw() {_Path = wzPath;}
    virtual const std::wstring& GetPath() const throw() {return _Path;}

    inline CPath& operator = (_In_ LPCWSTR wzPath) throw() {_Path = wzPath; return *this;}
    inline CPath& operator = (_In_ const std::wstring& wsPath) throw() {_Path = wsPath; return *this;}
    inline CPath& operator = (_In_ const CPath& Path) throw() {if(this != (&Path)) {_Path = Path.GetPath();} return *this;}
    inline BOOL operator == (_In_ LPCWSTR wzPath) throw() {return (0 == _wcsicmp(_Path.c_str(), wzPath));}
    inline BOOL operator == (_In_ const std::wstring& wsPath) throw() {return (0 == _wcsicmp(_Path.c_str(), wsPath.c_str()));}
    inline BOOL operator == (_In_ const CPath& Path) throw() {if(this != (&Path)) {return (0 == _wcsicmp(_Path.c_str(), Path.GetPath().c_str()));} else return TRUE;}
    inline BOOL Empty() const throw() {return _Path.empty()?TRUE:FALSE;}

private:
    std::wstring    _Path;
};
 
class CFilePath : public CPath
{
public:
    CFilePath();
    CFilePath(_In_ LPCWSTR wzPath);
    CFilePath(_In_ const std::wstring& wsPath);
    virtual ~CFilePath();

    virtual VOID SetPath(_In_ LPCWSTR wzPath);
    
    CFilePath& operator = (_In_ LPCWSTR wzPath);
    CFilePath& operator = (_In_ const std::wstring& wsPath);
    CFilePath& operator = (_In_ const CPath& Path);
    CFilePath& operator = (_In_ const CFilePath& Path) throw();

    BOOL IsUncPath() const throw();         // \\Server\path\file
    BOOL IsDosPath() const throw();         // C:\path\file
    BOOL IsRoot() const throw();

    std::wstring GetParentDir() const throw();
    std::wstring GetFileName() const throw();
    INT CompareFileName(_In_ LPCWSTR FileName) const throw();
};

class CFileName : public CPath
{
public:
    CFileName();
    CFileName(_In_ LPCWSTR wzFileName);
    ~CFileName();

    virtual VOID SetPath(_In_ LPCWSTR wzPath) throw();
    VOID SetFileName(_In_ LPCWSTR wzPath) throw();
    const std::wstring& GetFileName() const throw();
    std::wstring GetExtension() const throw();
    INT CompareExtension(_In_ LPCWSTR Extension) const throw(); // Extension must be start with L'.'

private:
    std::wstring    _Name;
};

class CModulePath : public CFilePath
{
public:
    CModulePath();
    CModulePath(_In_opt_ HMODULE hModule);
    VOID SetModule(_In_opt_ HMODULE hModule) throw();
};


//
//  Inline functions
//

// C:\path\file
__forceinline
BOOL IsDosPath(_In_ LPCWSTR wzPath)
{
    return (NULL != wzPath
            && ((L'a' <= wzPath[0] && L'z' >= wzPath[0]) || (L'A' <= wzPath[0] && L'Z' >= wzPath[0]))
            && L':' == wzPath[1]
            && L'\\' == wzPath[2]
            );
}

// \??\C:\path\file
__forceinline
BOOL IsGlobalDosPath(_In_ LPCWSTR wzPath)
{
    return (NULL != wzPath
            && L'\\' == wzPath[0]
            && L'?' == wzPath[1]
            && L'?' == wzPath[2]
            && L'\\' == wzPath[3]
            && ((L'a' <= wzPath[4] && L'z' >= wzPath[4]) || (L'A' <= wzPath[4] && L'Z' >= wzPath[4]))
            && L':' == wzPath[5]
            && L'\\' == wzPath[6]
            );
}

// \\Server\path\file
__forceinline
BOOL IsUncPath(_In_ LPCWSTR wzPath)
{
    return (NULL != wzPath
            && L'\\' == wzPath[0]
            && L'\\' == wzPath[1]
            && L'\0' != wzPath[2]
            && L'\\' != wzPath[2]
            && NULL != wcschr(&wzPath[2], L'\\')
            );
}

// \??\UNC\Server\path\file
__forceinline
BOOL IsGlobalUncPath(_In_ LPCWSTR wzPath)
{
    return (NULL != wzPath
            && L'\\' == wzPath[0]
            && L'?' == wzPath[1]
            && L'?' == wzPath[2]
            && L'\\' == wzPath[3]
            && (L'u' == wzPath[4] || L'U' == wzPath[4])
            && (L'n' == wzPath[5] || L'N' == wzPath[5])
            && (L'c' == wzPath[6] || L'C' == wzPath[6])
            && L'\\' == wzPath[7]
            && L'\0' != wzPath[8]
            && L'\\' != wzPath[8]
            && NULL != wcschr(&wzPath[8], L'\\')
            );
}

}   // namespace nudf::ipc
}   // namespace nudf



#endif  // #ifndef __NUDF_PATH_HPP__