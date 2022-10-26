

#ifndef __NUDF_PE_HPP__
#define __NUDF_PE_HPP__

#include <string>
#include <vector>

namespace nudf
{
namespace win
{
    

class CPECert
{
public:
    CPECert() {}
    ~CPECert(){}

    inline bool IsValid() const throw() {return (!_subject.empty());}
    inline const std::wstring& GetSubject() const throw() {return _subject;}
    inline const std::wstring& GetIssuer() const throw() {return _issuer;}
    inline const std::wstring& GetSerial() const throw() {return _serial;}
    inline const std::vector<UCHAR>& GetThumbprint() const throw() {return _thumbprint;}
    inline const std::wstring& GetThumbprintAlg() const throw() {return _thumbprintAlg;}
    inline const std::wstring& GetSignatureAlg() const throw() {return _signatureAlg;}
    inline const SYSTEMTIME& GetValidFromDate() const throw() {return _validfrom;}
    inline const SYSTEMTIME& GetValidThruDate() const throw() {return _validthrough;}

    CPECert& operator = (const CPECert& cert) throw()
    {
        if(this != &cert) {
            _subject = cert.GetSubject();
        }
        return *this;
    }

    bool Load(_In_ LPCWSTR file) throw();
    void Clear() throw();


private:
    std::wstring    _subject;
    std::wstring    _issuer;
    std::wstring    _serial;
    std::vector<UCHAR> _thumbprint;
    std::wstring    _thumbprintAlg;
    std::wstring    _signatureAlg;
    SYSTEMTIME      _validfrom;
    SYSTEMTIME      _validthrough;
};

class CPEFile
{
public:
    CPEFile();
    virtual ~CPEFile();

    void Load(_In_ LPCWSTR file) throw();
    void Clear() throw();
    CPEFile& operator = (const CPEFile& pe) throw();

    inline bool IsValid() const throw() {return (0x00004550 == _ntHeader.Signature);}
    inline const IMAGE_NT_HEADERS* GetNtHeaders() const throw() {return (&_ntHeader);}
    inline const IMAGE_FILE_HEADER* GetFileHeader() const throw() {return (&_ntHeader.FileHeader);}
    inline const IMAGE_OPTIONAL_HEADER* GetOptionalHeader() const throw() {return (&_ntHeader.OptionalHeader);}
    inline const CPECert& GetCert() const throw() {return _cert;}

    inline bool IsX86Image() const throw() {return (IsValid() && _ntHeader.FileHeader.Machine==IMAGE_FILE_MACHINE_I386);}
    inline bool IsX64Image() const throw() {return (IsValid() && _ntHeader.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64);}
    inline bool IsIA64Image() const throw() {return (IsValid() && _ntHeader.FileHeader.Machine==IMAGE_FILE_MACHINE_IA64);}

    inline bool IsExeImage() const throw() {return (IsValid() && (0 != (_ntHeader.FileHeader.Characteristics==IMAGE_FILE_EXECUTABLE_IMAGE)));}
    inline bool IsDllImage() const throw() {return (IsValid() && (0 != (_ntHeader.FileHeader.Characteristics==IMAGE_FILE_DLL)));}
    inline bool IsSysImage() const throw() {return (IsValid() && (0 != (_ntHeader.FileHeader.Characteristics==IMAGE_FILE_SYSTEM)));}

    inline DWORD GetChecksum() const throw() {return _ntHeader.OptionalHeader.CheckSum;}

protected:
    bool load_pe_header(LPCWSTR file);

private:
    IMAGE_NT_HEADERS    _ntHeader;
    CPECert             _cert;
};




}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_PE_HPP__