

#ifndef __NUDF_KNOWNDIRS_HPP__
#define __NUDF_KNOWNDIRS_HPP__

#include <nudf\sa.hpp>
#include <string>

namespace nudf
{
namespace win
{


class CKnownDirs
{
public:
    CKnownDirs();
    virtual ~CKnownDirs();
    
    virtual void Load(_In_ HANDLE hToken) throw();
    virtual void Clear();

    inline const std::wstring& GetInternetCacheDir() const throw() {return _dirInternetCache;}
    inline const std::wstring& GetLocalAppDataDir() const throw() {return _dirLocalAppData;}
    inline const std::wstring& GetLocalAppDataDirLow() const throw() {return _dirLocalAppDataLow;}
    inline const std::wstring& GetRoamingAppDataDir() const throw() {return _dirRoamingAppData;}
    inline const std::wstring& GetProfileDir() const throw() {return _dirProfile;}
    inline const std::wstring& GetDesktopDir() const throw() {return _dirDesktop;}
    inline const std::wstring& GetDocumentsDir() const throw() {return _dirDocuments;}
    inline const std::wstring& GetCookiesDir() const throw() {return _dirCookies;}
    inline const std::wstring& GetTempDir() const throw() {return _dirTemp;}

private:
    bool CreateTempDir(_In_ HANDLE hToken, _In_ const std::wstring& folder);

private:
    std::wstring    _dirInternetCache;      // FOLDERID_InternetCache
    std::wstring    _dirLocalAppData;       // FOLDERID_LocalAppData
    std::wstring    _dirLocalAppDataLow;    // FOLDERID_LocalAppDataLow
    std::wstring    _dirRoamingAppData;     // FOLDERID_RoamingAppData
    std::wstring    _dirProfile;            // FOLDERID_Profile
    std::wstring    _dirDesktop;            // FOLDERID_Desktop
    std::wstring    _dirDocuments;          // FOLDERID_Documents
    std::wstring    _dirCookies;            // FOLDERID_Cookies
    std::wstring    _dirTemp;               // %LocalAppData%\Temp
};



}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_KNOWNDIRS_HPP__