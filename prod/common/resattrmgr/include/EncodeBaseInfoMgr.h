#pragma once

#include "EncodeTypes.h"
#include "EncodeBomMgr.h"

class EncodeBaseInfoMgr
{
#pragma region Singleton
public:
    static EncodeBaseInfoMgr& GetInstance();
private:
    EncodeBaseInfoMgr(void);
    ~EncodeBaseInfoMgr(void);
#pragma endregion Singleton

#pragma region Init
private:
    void InitEncodeMapping() throw();
    void InitDefaultEncodeType(_In_ bool bForceUpdate) throw();
#pragma endregion Init

#pragma region Tools
public:
    EMEncodeType GetDefaultEncodingType() const throw() { return m_emDefaultEncodeType; };

    // If cannot find, return emEncode_Unknown
    EMEncodeType GetEncodingTypeByString(_In_ const std::wstring& kwstrEncodeNameIn, _In_ const EMEncodeType kemDefaultEncodeTypeIn) const throw();
    EMEncodeType GetEncodingTypeByBOMCode(_In_ const BomCode& knBomCodeIn) const throw();

    // If cannot find return an invalid header with effective length 0
    const STUBomHeader* GetBOMHeaderByEncodeType(_In_ const EMEncodeType& kemEncodeTypeIn) const throw();

    // If cannot find, return CP_ACP
    UINT GetCodePageByEncodeType(_In_ const EMEncodeType& kemEncodeTypeIn) const throw();

#pragma endregion Tools

#pragma region Members
private:
    EMEncodeType m_emDefaultEncodeType;

    std::map<std::wstring, EMEncodeType> m_mapEncodeFormatStringAndEnum;
    std::map<BomCode, EMEncodeType> m_mapEncodeBomCodeAndEnum;
    std::map<EMEncodeType, const STUBomHeader*> m_mapEncodeFormatEnumAndHeader;    
    std::map<EMEncodeType, UINT> m_mapEncodeTypeAndCodePage;
#pragma endregion Members

};