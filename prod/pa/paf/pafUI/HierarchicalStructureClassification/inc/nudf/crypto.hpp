

#ifndef __NUDF_CRYPTO_HPP__
#define __NUDF_CRYPTO_HPP__

#include <Bcrypt.h>
#include <Wincrypt.h>

#include <string>
#include <vector>

namespace nudf
{
namespace crypto
{


typedef struct _CRYPTO_RSAKEY_BLOB {
    PUBLICKEYSTRUC  Header;
    RSAPUBKEY       PubKey;
} CRYPTO_RSAKEY_BLOB, *PCRYPTO_RSAKEY_BLOB;
typedef const CRYPTO_RSAKEY_BLOB* PCCRYPTO_RSAKEY_BLOB;



template<class T>
class CKeyBlob
{
public:
    CKeyBlob() {}
    virtual ~CKeyBlob() {}

    inline void Clear() throw() {_data.clear();}
    inline const T* GetBlob() const throw() {return _data.empty() ?  NULL : ((const T*)(&_data[0]));}
    inline T* GetBlob() throw() {return _data.empty() ?  NULL : ((T*)(&_data[0]));}
    inline ULONG GetBlobSize() const throw() {return (ULONG)_data.size();}
    inline CKeyBlob<T>& operator = (const CKeyBlob<T>& blob)
    {
        if(this != &blob) {
            _data.clear();
            if(blob.GetBlobSize() != 0) {
                _data.resize(blob.GetBlobSize(), 0);
                memcpy(&_data[0], blob.GetBlob(), blob.GetBlobSize());
            }
        }
        return *this;
    }
    
    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
    {
        _data.clear();
        _data.resize(size, 0);
        memcpy(&_data[0], blob, size);
        return S_OK;
    }

    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw() = 0;
    virtual const UCHAR* GetKey() const throw() = 0;
    virtual ULONG GetKeySize() const throw() = 0;
    virtual ULONG GetKeyBitsLength() const throw() = 0;
    
protected:
    const std::vector<UCHAR>& GetData() const throw() {return _data;}
    std::vector<UCHAR>& GetData() throw() {return _data;}

private:
    std::vector<UCHAR> _data;
};

class CAesKeyBlob : public CKeyBlob<BCRYPT_KEY_DATA_BLOB_HEADER>
{
public:
    CAesKeyBlob();
    virtual ~CAesKeyBlob();

    CAesKeyBlob& operator = (const CAesKeyBlob& blob);

    virtual HRESULT Generate(_In_ ULONG bitslen) throw();
    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw();
    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw();
    virtual const UCHAR* GetKey() const throw();
    virtual ULONG GetKeySize() const throw();
    virtual ULONG GetKeyBitsLength() const throw();
};

typedef enum {
    UNKNWONRSAKEYBLOBTYPE = 0,
    RSAPUBKEYBLOBTYPE,
    RSAPRIKEYBLOBTYPE,
    RSAFULLPRIKEYBLOBTYPE,
    LEGACYRSAPUBKEYBLOBTYPE,
    LEGACYRSAPRIKEYBLOBTYPE
} RSAKEYBLOBTYPE;

class CRsaPubKeyBlob : public CKeyBlob<BCRYPT_RSAKEY_BLOB>
{
public:
    CRsaPubKeyBlob();
    virtual ~CRsaPubKeyBlob();

    CRsaPubKeyBlob& operator = (const CRsaPubKeyBlob& blob);

    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw();
    virtual ULONG GetKeyBitsLength() const throw();    

    // Get Attributes
    const UCHAR* GetPubexp() const throw();
    ULONG GetPubexpLen() const throw();
    const UCHAR* GetModulus() const throw();
    ULONG GetModulusLen() const throw();

    static RSAKEYBLOBTYPE GetBlobType(_In_ const UCHAR* blob, _In_ ULONG size) throw();

protected:
    virtual HRESULT SetBlob(_In_ const BCRYPT_RSAKEY_BLOB* blob, _In_ ULONG size) throw();
    virtual HRESULT SetBlob(_In_ PCCRYPTO_RSAKEY_BLOB blob, _In_ ULONG size) throw();
    
private:
    virtual HRESULT Generate(_In_ ULONG size) throw() {UNREFERENCED_PARAMETER(size); return HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);}
    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw() {UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(size);}
    virtual const UCHAR* GetKey() const throw() {return NULL;}
    virtual ULONG GetKeySize() const throw() {return 0UL;}
};

class CRsaPriKeyBlob : public CRsaPubKeyBlob
{
public:
    CRsaPriKeyBlob();
    virtual ~CRsaPriKeyBlob();

    CRsaPriKeyBlob& operator = (const CRsaPriKeyBlob& blob);

    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw();
    virtual HRESULT Generate(_In_ ULONG bitslen) throw();

    // Get Attributes
    bool IsRsaFullPriKeyBlob() const throw();
    const UCHAR* GetPrime1() const throw();
    ULONG GetPrime1Len() const throw();
    const UCHAR* GetPrime2() const throw();
    ULONG GetPrime2Len() const throw();
    const UCHAR* GetExponent1() const throw();
    ULONG GetExponent1Len() const throw();
    const UCHAR* GetExponent2() const throw();
    ULONG GetExponent2Len() const throw();
    const UCHAR* GetCoefficient() const throw();
    ULONG GetCoefficientLen() const throw();
    const UCHAR* GetPrivateExponent() const throw();
    ULONG GetPrivateExponentLen() const throw();

    void GetPublicKeyBlob(_Out_ CRsaPubKeyBlob& blob) throw();
    
protected:
    virtual HRESULT SetBlob(_In_ const BCRYPT_RSAKEY_BLOB* blob, _In_ ULONG size) throw();
    virtual HRESULT SetBlob(_In_ PCCRYPTO_RSAKEY_BLOB blob, _In_ ULONG size) throw();

private:
    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw() {UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(size);}
    virtual const UCHAR* GetKey() const throw() {return NULL;}
    virtual ULONG GetKeySize() const throw() {return 0UL;}
};


class CLegacyRsaPubKeyBlob : public CKeyBlob<CRYPTO_RSAKEY_BLOB>
{
public:
    CLegacyRsaPubKeyBlob();
    virtual ~CLegacyRsaPubKeyBlob();

    CLegacyRsaPubKeyBlob& operator = (const CLegacyRsaPubKeyBlob& blob);

    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw();
    virtual ULONG GetKeyBitsLength() const throw();    

    // Get Attributes
    const UCHAR* GetPubexp() const throw();
    ULONG GetPubexpLen() const throw();
    const UCHAR* GetModulus() const throw();
    ULONG GetModulusLen() const throw();
    
private:
    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw() {UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(size);}
    virtual const UCHAR* GetKey() const throw() {return NULL;}
    virtual ULONG GetKeySize() const throw() {return 0UL;}
};

class CLegacyRsaPriKeyBlob : public CLegacyRsaPubKeyBlob
{
public:
    CLegacyRsaPriKeyBlob();
    virtual ~CLegacyRsaPriKeyBlob();

    CLegacyRsaPriKeyBlob& operator = (const CLegacyRsaPriKeyBlob& blob);

    virtual HRESULT SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw();

    // Get Attributes
    const UCHAR* GetPrime1() const throw();
    ULONG GetPrime1Len() const throw();
    const UCHAR* GetPrime2() const throw();
    ULONG GetPrime2Len() const throw();
    const UCHAR* GetExponent1() const throw();
    ULONG GetExponent1Len() const throw();
    const UCHAR* GetExponent2() const throw();
    ULONG GetExponent2Len() const throw();
    const UCHAR* GetCoefficient() const throw();
    ULONG GetCoefficientLen() const throw();
    const UCHAR* GetPrivateExponent() const throw();
    ULONG GetPrivateExponentLen() const throw();

    void GetPublicKeyBlob(_Out_ CRsaPubKeyBlob& blob) const throw();

private:
    virtual void SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw() {UNREFERENCED_PARAMETER(key); UNREFERENCED_PARAMETER(size);}
    virtual const UCHAR* GetKey() const throw() {return NULL;}
    virtual ULONG GetKeySize() const throw() {return 0UL;}
};


bool Initialize() throw();

bool AesEncrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec) throw();
bool AesEncrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec, _Out_ std::vector<unsigned char>& cipher) throw();
bool AesDecrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec) throw();
bool AesDecrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec, _Out_ std::vector<unsigned char>& plain) throw();


HRESULT RsaSign(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw();
HRESULT RsaSign(_In_ const CRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw();
HRESULT RsaSign(_In_ const CLegacyRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw();

HRESULT RsaVerifySignature(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw();
HRESULT RsaVerifySignature(_In_ const CRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw();
HRESULT RsaVerifySignature(_In_ const CLegacyRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw();

HRESULT RsaEncrypt(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw();
HRESULT RsaEncrypt(_In_ const CRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw();
HRESULT RsaEncrypt(_In_ const CLegacyRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw();

HRESULT RsaDecrypt(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw();
HRESULT RsaDecrypt(_In_ const CRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw();
HRESULT RsaDecrypt(_In_ const CLegacyRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw();

bool ToMd5(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& md5) throw();
bool ToSha1(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& sha1) throw();
bool ToSha256(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& sha256) throw();

unsigned long ToCrc32(_In_ unsigned long init_crc, _In_ const void* pb, _In_ unsigned long cb) throw();
unsigned __int64 ToCrc64(_In_ unsigned __int64 init_crc, _In_ const void* pb, _In_ unsigned long cb) throw();




}   // namespace nudf::crypto
}   // namespace nudf




#endif  // #ifndef __NUDF_CRYPTO_HPP__