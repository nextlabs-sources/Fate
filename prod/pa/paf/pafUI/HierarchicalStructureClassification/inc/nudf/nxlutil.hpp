

#ifndef __NUDF_NXLUTIL_HPP__
#define __NUDF_NXLUTIL_HPP__


#include <string>
#include <vector>
#include <nudf\shared\nxlfmt.h>


namespace nudf
{
namespace util
{
namespace nxl
{
  
//
//  Add Implementation Here
//

HRESULT __stdcall NxrmEncryptFile(const WCHAR *FileName);

HRESULT __stdcall NxrmEncryptFileEx(const WCHAR *FileName, const char *tag, UCHAR *data, USHORT datalen);

HRESULT __stdcall NxrmEncryptFileEx2(HANDLE hFile, const char *tag, UCHAR *data, USHORT datalen);

HRESULT __stdcall NxrmDecryptFile(const WCHAR *FileName);

HRESULT __stdcall NxrmDecryptFile2(HANDLE hFile);

HRESULT __stdcall NxrmCheckRights(const WCHAR *FileName, ULONGLONG *RightMask, ULONGLONG *CustomRightsMask, ULONGLONG *EvaluationId);

HRESULT __stdcall NxrmCheckRightsNoneCache(const WCHAR *FileName, ULONGLONG *RightMask, ULONGLONG *CustomRightsMask, ULONGLONG *EvaluationId);

HRESULT __stdcall NxrmSyncNXLHeader(HANDLE hFile, const char *tag, UCHAR *data, USHORT datalen);

HRESULT __stdcall NxrmReadTags(const WCHAR *FileName, UCHAR *data, USHORT *datalen);

HRESULT __stdcall NxrmReadTagsEx(HANDLE hFile, UCHAR *data, USHORT *datalen);

HRESULT __stdcall NxrmIsDecryptedFile(const WCHAR *FileName);

HRESULT __stdcall NxrmSetSourceFileName(const WCHAR *FileName, const WCHAR *SrcNTFileName);

typedef enum {
    NXL_UNKNOWN = -1,
    NXL_OK = 0,
    NXL_BAD_SIGNATURE,
    NXL_BAD_VERSION,
    NXL_BAD_ALIGNMENT,
    NXL_BAD_DATAOFFSET,
    NXL_BAD_ENCRYPT_ALG,
    NXL_BAD_CBC_SIZE,
    NXL_BAD_KEK_ALG,
    NXL_BAD_KEK_ID_SIZE,
    NXL_BAD_SECTION_COUNT
} NXLSTATUS;

LPCWSTR NxlStatus2Message(_In_ NXLSTATUS status);

class CFile
{
public:
    CFile();
    CFile(_In_ LPCWSTR path);
    virtual ~CFile();


    inline bool IsValid() const throw() {return _header.empty()?false:true;}
    inline bool IsOpened() const throw() {return (INVALID_HANDLE_VALUE!=_handle);}

    inline PCNXL_HEADER GetNHeader() const throw() {return ((_header.size() < sizeof(NXL_HEADER)) ? NULL : ((PCNXL_HEADER)(&_header[0])));}
    inline PNXL_HEADER GetNHeader() throw() {return ((_header.size() < sizeof(NXL_HEADER)) ? NULL : ((PNXL_HEADER)(&_header[0])));}

    NXLSTATUS Validate() throw();
    void Open(_Out_opt_ NXLSTATUS* status);
    void OpenEx(_In_ bool readonly, _Out_opt_ NXLSTATUS* status);
    void Close() throw();
    void Reset() throw();

    void GetNAttributes(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& attributes);
    void SetNAttributes(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& attributes, _Out_opt_ PULONG paires_written);
    void GetNTags(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& tags);
    void SetNTags(_Out_ std::vector<std::pair<std::wstring, std::wstring>>& tags, _Out_opt_ PULONG paires_written);


protected:
    inline HANDLE GetFileHandle() {return _handle;}

protected:
    NXLSTATUS ValidateHeader() const throw();
    bool FindSection(_In_ LPCSTR name, _Out_opt_ PULONG offset, _Out_opt_ PULONG size) const throw();
    void GetNPairDataW(_In_ LPCSTR name, _Out_ std::vector<std::pair<std::wstring, std::wstring>>& data);
    void SetNPairDataW(_In_ LPCSTR name, _In_ const std::vector<std::pair<std::wstring, std::wstring>>& data, _Out_opt_ PULONG paires_written);
    void GetNPairDataUtf8(_In_ LPCSTR name, _Out_ std::vector<std::pair<std::wstring, std::wstring>>& data);
    void SetNPairDataUtf8(_In_ LPCSTR name, _In_ const std::vector<std::pair<std::wstring, std::wstring>>& data, _Out_opt_ PULONG paires_written);

private:
    std::wstring        _path;
    std::vector<UCHAR>  _header;
    HANDLE              _handle;
};


}   // namespace nudf::util::nxl
}   // namespace nudf::util
}   // namespace nudf



#endif  // #ifndef __NUDF_NXLUTIL_HPP__