

#ifndef __NUDF_CERTIFICATE_HPP__
#define __NUDF_CERTIFICATE_HPP__

#include <Bcrypt.h>
#include <Wincrypt.h>

#include <string>
#include <vector>
#include <nudf\crypto.hpp>

namespace nudf
{
namespace crypto
{

class CCertDecoder
{
public:
    static bool DecodeSubject(_In_ PCCERT_CONTEXT context, _Out_ std::wstring& subject);
    static bool DecodeIssuer(_In_ PCCERT_CONTEXT context, _Out_ std::wstring& issuer);
    static bool DecodeSerial(_In_ PCCERT_CONTEXT context, _Out_ std::wstring& serial);
    static bool DecodeThumbprint(_In_ PCCERT_CONTEXT context, _Out_ std::vector<unsigned char>& thumbprint);
    static bool DecodeAlgorithm(_In_ PCCERT_CONTEXT context, _Out_ std::wstring& sign_alg, _Out_ std::wstring& thumbprint_alg);
    static bool DecodeValidDate(_In_ PCCERT_CONTEXT context, _Out_ SYSTEMTIME* validfrom, _Out_ SYSTEMTIME* validthrough);
    static bool DecodeKey(_In_ PCCERT_CONTEXT context, _Out_ std::vector<unsigned char>& key);
    static std::wstring DecodeNameInfo(_In_ unsigned char* pb, _In_ unsigned long cb, _In_ const char* id);
    static std::wstring DecodeRdnAttribute(_In_ PCERT_RDN_ATTR attr);
};

class CCertContext
{
public:
    CCertContext();
    virtual ~CCertContext();

    inline bool IsValid() const throw() {return (NULL != _context);}
    inline PCCERT_CONTEXT& GetContext() throw() {return _context;}
    inline operator PCCERT_CONTEXT() const throw() {return _context;}
    bool operator == (_In_ const CCertContext& context) const throw();

    PCCERT_CONTEXT Attach(_In_ PCCERT_CONTEXT context) throw();
    PCCERT_CONTEXT Detach() throw();
    void Close() throw();

    HRESULT Create(_In_ LPCWSTR names, _In_ bool signature, _In_ bool strong, _In_ int valid_years=1);
    HRESULT Create(_In_ LPCWSTR names, _In_ bool signature, _In_ bool strong, _In_opt_ PCCERT_CONTEXT root, _In_ int valid_years=1);

    // Get Properties
    HRESULT GetPropAccessState(_Out_ PDWORD state) const throw();
    HRESULT GetPropCrossCertDistPoints(_Out_ std::vector<UCHAR>& dist_points) const throw();
    HRESULT GetPropCtlUsage(_Out_ std::vector<UCHAR>& usage) const throw();
    HRESULT GetPropDateStamp(_Out_ FILETIME* date) const throw();
    HRESULT GetPropDescription(_Out_ std::vector<UCHAR>& desc) const throw();
    HRESULT GetPropEnhKeyUsage(_Out_ std::vector<UCHAR>& usage) const throw();
    HRESULT GetPropFriendlyName(_Out_ std::wstring& name) const throw();
    HRESULT GetPropHash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropIssuerPubKeyMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropIssuerSnMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropKeyProvHandle(_Out_ HCRYPTPROV* handle) const throw();
    HRESULT GetPropKeyProvInfo(_Out_ std::vector<UCHAR>& info) const throw();
    HRESULT GetPropKeySpec(_Out_ PDWORD spec) const throw();
    HRESULT GetPropMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropSha1Hash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropSignHashCngAlgorithm(_Out_ std::wstring& sign_alg, _Out_ std::wstring& hash_alg) const throw();
    HRESULT GetPropSignatureHash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropSubjectInfoAccess(_Out_ std::vector<UCHAR>& access) const throw();
    HRESULT GetPropSubjectNameMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw();
    HRESULT GetPropSubjectPubKeyMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw();

    HRESULT GetSubjectName(_Out_ std::wstring& name) const throw();
    HRESULT GetIssuerName(_Out_ std::wstring& name) const throw();
    HRESULT GetValidFromDate(_Out_ PSYSTEMTIME date) const throw();
    HRESULT GetValidThruDate(_Out_ PSYSTEMTIME date) const throw();

    DWORD GetPublicKeyLength() const throw();
    HRESULT GetPublicKeyBlob(_Out_ std::vector<UCHAR>& keyblob) const throw();
    HRESULT GetPublicKeyBlob(_Out_ CRsaPubKeyBlob& keyblob) const throw();
    HRESULT GetPublicKeyBlob(_Out_ CLegacyRsaPubKeyBlob& keyblob) const throw();
    HRESULT GetPrivateKeyBlob(_Out_ std::vector<UCHAR>& keyblob) const throw();
    HRESULT GetPrivateKeyBlob(_Out_ CLegacyRsaPriKeyBlob& keyblob) const throw();

    // Compare
    bool IsSelfSign() const throw();
    bool EqualPublicKey(_In_ const CCertContext& context) const throw();
    bool HasPrivateKey() const throw();

protected:
    HRESULT GetProperty(_In_ DWORD id, _Out_ std::vector<UCHAR>& data) const throw();

private:
    PCCERT_CONTEXT  _context;
};

class CX509CertContext : public CCertContext
{
public:
    CX509CertContext();
    virtual ~CX509CertContext();

    virtual HRESULT Create(_In_ const UCHAR* pb, _In_ ULONG cb);
    virtual HRESULT Create(_In_ LPCWSTR base64);
    virtual HRESULT Create(_In_ LPCSTR base64);
    virtual HRESULT CreateFromFile(_In_ LPCWSTR file);
};

class CPkcs12CertContext : public CCertContext
{
public:
    CPkcs12CertContext();
    virtual ~CPkcs12CertContext();

    virtual HRESULT Create(_In_ const UCHAR* pb, _In_ ULONG cb, _In_ LPCWSTR password);
    virtual HRESULT Create(_In_ LPCWSTR base64, _In_ LPCWSTR password);
    virtual HRESULT Create(_In_ LPCSTR base64, _In_ LPCWSTR password);
    virtual HRESULT CreateFromFile(_In_ LPCWSTR file, _In_ LPCWSTR password);
};


class CCertStore
{
public:
    CCertStore();
    CCertStore(_In_ LPCWSTR location);
    virtual ~CCertStore();

    virtual HCERTSTORE Detach() throw();
    virtual void Close() throw();

    inline bool IsValid() const throw() {return (NULL != _store);}
    inline operator HCERTSTORE() const throw() {return _store;}
    inline const std::wstring& GetLocation() const throw() {return _location;}
    
    HRESULT ExportToPkcs12Package(_In_ LPCWSTR file, _In_ LPCWSTR password, _In_ bool with_privatekey=true) throw();
    HRESULT ExportToX509Package(_In_ LPCWSTR file) throw();

    HRESULT AddCert(_In_ PCCERT_CONTEXT context);
    HRESULT AddCert(_In_ PCCERT_CONTEXT context, _Out_ CCertContext& new_context);

    template<class _Pr1>
    bool FindCert(_Out_ CCertContext& result, _In_ _Pr1 _Pred) throw()
    {
        CCertContext prev_context;

        do {

            CCertContext cur_context;

            cur_context.Attach(CertFindCertificateInStore(*this, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, prev_context));
            prev_context.Close();
            if(!cur_context.IsValid()) {
                break;
            }

            // Compare
            if(_Pred(cur_context)) {
                // Found
                result.Attach(cur_context.Detach());
                break;
            }

            // Move to next
            prev_context.Attach(cur_context.Detach());

        } while(prev_context.IsValid());

        return result.IsValid();
    }
    
    template<class _Pr1>
    void ForEach(_In_ _Pr1 _Pred) throw()
    {
        CCertContext prev_context;

        do {

            CCertContext cur_context;

            cur_context.Attach(CertFindCertificateInStore(*this, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, prev_context));
            prev_context.Close();
            if(!cur_context.IsValid()) {
                break;
            }

            // if _Pred return false, then don't continue
            if(!_Pred(cur_context)) {
                break;
            }

            // Move to next
            prev_context.Attach(cur_context.Detach());

        } while(prev_context.IsValid());
    }
    

protected:
    HCERTSTORE Attach(_In_ HCERTSTORE store) throw();
    void SetLocation(const std::wstring& location) throw() {_location = location;}

private:
    HCERTSTORE      _store;
    std::wstring    _location;
};


class CSysCertStore : public CCertStore
{
public:
    CSysCertStore();
    CSysCertStore(_In_ LPCWSTR location);
    virtual ~CSysCertStore();
    virtual HRESULT Open() throw();
};

class CMemCertStore : public CCertStore
{
public:
    CMemCertStore();
    virtual ~CMemCertStore();
    virtual HRESULT Open() throw();
};

class CFileCertStore : public CCertStore
{
public:
    CFileCertStore();
    CFileCertStore(_In_ LPCWSTR file);
    virtual ~CFileCertStore();
    virtual HRESULT Open() throw();
    virtual HRESULT Create() throw();
};

class CPkcs12CertStore : public CCertStore
{
public:
    CPkcs12CertStore();
    CPkcs12CertStore(_In_ LPCWSTR file);
    virtual ~CPkcs12CertStore();
    virtual HRESULT Open() throw();
    virtual HRESULT Open(_In_opt_ LPCWSTR password) throw();
    virtual HRESULT Open(_In_ const UCHAR* pb, _In_ ULONG cb, _In_opt_ LPCWSTR password) throw();
};



}   // namespace nudf::crypto
}   // namespace nudf



#endif  // #ifndef __NUDF_CERTIFICATE_HPP__