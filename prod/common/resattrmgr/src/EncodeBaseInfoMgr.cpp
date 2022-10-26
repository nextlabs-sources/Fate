#include "EncodeBaseInfoMgr.h"

#include "nlconfig.hpp"

#include <algorithm>

template<typename tyKey, typename tyValue, typename tyCmp>
tyValue GetValueFromMapByKey(_In_ const std::map<tyKey, tyValue, tyCmp> kmapIn, tyKey tyKeyIn, tyValue tyDefault)
{
    std::map<tyKey, tyValue>::const_iterator kitr = kmapIn.find(tyKeyIn);
    if (kmapIn.end() == kitr)
    {
        // Do not find, return default
        return tyDefault;
    } 
    else
    {
        // Find
        return kitr->second;
    }
}

EncodeBaseInfoMgr& EncodeBaseInfoMgr::GetInstance()
{
    static EncodeBaseInfoMgr obEncodeBaseInfoMgrIns;
    return obEncodeBaseInfoMgrIns;
}
EncodeBaseInfoMgr::EncodeBaseInfoMgr(void)
{
    InitEncodeMapping();
    InitDefaultEncodeType(true);
}
EncodeBaseInfoMgr::~EncodeBaseInfoMgr(void)
{

}

void EncodeBaseInfoMgr::InitEncodeMapping() throw()
{
    {
        // String and encode types
        m_mapEncodeFormatStringAndEnum.insert(std::pair<std::wstring, EMEncodeType>(L"UTF8", emEncode_UTF8));
        m_mapEncodeFormatStringAndEnum.insert(std::pair<std::wstring, EMEncodeType>(L"UNICODE_BE", emEncode_Unicode_BigEnd));
        m_mapEncodeFormatStringAndEnum.insert(std::pair<std::wstring, EMEncodeType>(L"UNICODE", emEncode_Unicode));
        m_mapEncodeFormatStringAndEnum.insert(std::pair<std::wstring, EMEncodeType>(L"UTF32_BE", emEncode_UTF32_BigEnd));
        m_mapEncodeFormatStringAndEnum.insert(std::pair<std::wstring, EMEncodeType>(L"UTF32", emEncode_UTF32));
    }

    {
        // BomHeader and BomCode with encode types
        m_mapEncodeFormatEnumAndHeader.insert(std::pair<EMEncodeType, const STUBomHeader*>(emEncode_UTF8, &g_kstuBom_UTF8));
        m_mapEncodeFormatEnumAndHeader.insert(std::pair<EMEncodeType, const STUBomHeader*>(emEncode_Unicode_BigEnd, &g_kstuBom_Unicode_BigEnd));
        m_mapEncodeFormatEnumAndHeader.insert(std::pair<EMEncodeType, const STUBomHeader*>(emEncode_Unicode, &g_kstuBom_Unicode));
        m_mapEncodeFormatEnumAndHeader.insert(std::pair<EMEncodeType, const STUBomHeader*>(emEncode_UTF32_BigEnd, &g_kstuBom_UTF32_BigEnd));
        m_mapEncodeFormatEnumAndHeader.insert(std::pair<EMEncodeType, const STUBomHeader*>(emEncode_UTF32, &g_kstuBom_UTF32));

        m_mapEncodeBomCodeAndEnum.insert(std::pair<BomCode, EMEncodeType>(g_kstuBom_UTF8.nBomCode, emEncode_UTF8));
        m_mapEncodeBomCodeAndEnum.insert(std::pair<BomCode, EMEncodeType>(g_kstuBom_Unicode_BigEnd.nBomCode, emEncode_Unicode_BigEnd));
        m_mapEncodeBomCodeAndEnum.insert(std::pair<BomCode, EMEncodeType>(g_kstuBom_Unicode.nBomCode, emEncode_Unicode));
        m_mapEncodeBomCodeAndEnum.insert(std::pair<BomCode, EMEncodeType>(g_kstuBom_UTF32_BigEnd.nBomCode, emEncode_UTF32_BigEnd));
        m_mapEncodeBomCodeAndEnum.insert(std::pair<BomCode, EMEncodeType>(g_kstuBom_UTF32.nBomCode, emEncode_UTF32));
    }

    {
        // Code page and encode types
        m_mapEncodeTypeAndCodePage.insert(std::pair<EMEncodeType, UINT>(emEncode_UTF8, 65001));
        m_mapEncodeTypeAndCodePage.insert(std::pair<EMEncodeType, UINT>(emEncode_Unicode_BigEnd, 1201));
        m_mapEncodeTypeAndCodePage.insert(std::pair<EMEncodeType, UINT>(emEncode_Unicode, 1200));
        m_mapEncodeTypeAndCodePage.insert(std::pair<EMEncodeType, UINT>(emEncode_UTF32_BigEnd, 12001));
        m_mapEncodeTypeAndCodePage.insert(std::pair<EMEncodeType, UINT>(emEncode_UTF32, 12000));
    }
}

void EncodeBaseInfoMgr::InitDefaultEncodeType(_In_ bool bForceUpdate) throw()
{
    if (bForceUpdate || (emEncode_Unknown == m_emDefaultEncodeType))
    {
        m_emDefaultEncodeType = emEncode_UTF8;

        // Read default encoding type from register
        // HKEY_LOCAL_MACHINE\SOFTWARE\Nextlabs\Compliant Enterprise\Desktop Enforcer\DefaultNTFSTagEncoding       

        wchar_t wszDefaultEncodeType[64] = {0}; 
        std::wstring wstrDefaultEncodingKeyPath = L"SOFTWARE\\Nextlabs\\Compliant Enterprise\\Desktop Enforcer\\DefaultNTFSTagEncoding";
        bool bRet = NLConfig::ReadKey(wstrDefaultEncodingKeyPath.c_str(), wszDefaultEncodeType, sizeof(wszDefaultEncodeType), KEY_QUERY_VALUE | KEY_WOW64_64KEY);
        if (bRet)
        {
            m_emDefaultEncodeType = GetEncodingTypeByString(wszDefaultEncodeType, emEncode_UTF8);
        }
    }

}

EMEncodeType EncodeBaseInfoMgr::GetEncodingTypeByString(_In_ const std::wstring& kwstrEncodeNameIn, _In_ const EMEncodeType kemDefaultEncodeTypeIn) const throw()
{
    return GetValueFromMapByKey(m_mapEncodeFormatStringAndEnum, kwstrEncodeNameIn, kemDefaultEncodeTypeIn);
}
EMEncodeType EncodeBaseInfoMgr::GetEncodingTypeByBOMCode(_In_ const BomCode& knBomCodeIn) const throw()
{
    return GetValueFromMapByKey(m_mapEncodeBomCodeAndEnum, knBomCodeIn, emEncode_Unknown);
}

const STUBomHeader* EncodeBaseInfoMgr::GetBOMHeaderByEncodeType(_In_ const EMEncodeType& kemEncodeTypeIn) const throw()
{
    return GetValueFromMapByKey(m_mapEncodeFormatEnumAndHeader, kemEncodeTypeIn, (const STUBomHeader*)(NULL));
}
UINT EncodeBaseInfoMgr::GetCodePageByEncodeType(_In_ const EMEncodeType& kemEncodeTypeIn) const throw()
{
    return GetValueFromMapByKey(m_mapEncodeTypeAndCodePage, kemEncodeTypeIn, (UINT)(CP_ACP));
}