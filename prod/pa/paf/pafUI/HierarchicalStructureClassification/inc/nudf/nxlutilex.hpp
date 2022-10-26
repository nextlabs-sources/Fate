

#ifndef __NUDF_NXLUTILEX_HPP__
#define __NUDF_NXLUTILEX_HPP__

#include <nudf\nxlutil.hpp>



namespace nudf
{
namespace util
{
namespace nxl
{
  
class CFileEx : public CFile
{
public:
    CFileEx();
    CFileEx(_In_ LPCWSTR path);
    virtual ~CFileEx();
    
    bool DecodeContentKey(_In_ const std::vector<UCHAR>& kekey, _Out_ std::vector<UCHAR>& cekey);
    bool VerifySectionChecksum(_In_ int index, _Out_opt_ PULONG crc_in_header, _Out_opt_ PULONG crc_calculated);
    bool VerifySectionTableChecksum(_In_reads_(16) const UCHAR* key, _Out_opt_ PULONG crc_in_table, _Out_opt_ PULONG crc_calculated);
};


}   // namespace nudf::util::nxl
}   // namespace nudf::util
}   // namespace nudf


#endif