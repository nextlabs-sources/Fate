#pragma once

#include <vector>
#include <map>
#include <list>

#include "EncodeTypes.h"

#pragma pack (1)
struct STUBomUnit
{
    unsigned int unFirst : 8;
    unsigned int unSecond : 8;
    unsigned int unThird : 8;
    unsigned int unFourth : 8;

    STUBomUnit(_In_ const unsigned int unFirstIn, _In_ const unsigned int unSecondIn, _In_ const unsigned int unThirdIn, _In_ const unsigned int unFourthIn);
    
    BomCode GetBOMCode(_In_ const int knEffectLength) const throw();
    const byte* GetBomHeaderByteStream(_In_ const int knEffectLength) const throw();

    static bool IsEffectiveBomLength(_In_ const int knEffectLength) throw();
};
#pragma pack ()


#pragma warning(push)
#pragma warning(disable: 4512) // warning C4512: 'STUBomHeader' : assignment operator could not be generated
struct STUBomHeader
{
    const STUBomUnit stuBomHeader;
    const int nBomEffectiveLength;

    const BomCode nBomCode;
    const byte* pByteBomStream;

    STUBomHeader(_In_ const STUBomUnit& kstuBomHeaderIn, _In_ const int knBomEffectiveLengthIn);
    bool IsValidBomHeader() const throw();
};
#pragma warning(pop)

#pragma region Standard BOM Headers

extern const STUBomHeader g_kstuBom_NULL;

extern const STUBomHeader g_kstuBom_UTF8;

extern const STUBomHeader g_kstuBom_Unicode_BigEnd;
extern const STUBomHeader g_kstuBom_Unicode;

extern const STUBomHeader g_kstuBom_UTF32_BigEnd;
extern const STUBomHeader g_kstuBom_UTF32;

#pragma endregion

class EncodeBomMgr
{
public:
    EncodeBomMgr(_In_ const STUBomUnit& kstuBomUnitIn);
    EncodeBomMgr(_In_ const STUBomHeader& kstuBomHeaderIn);
    EncodeBomMgr(_In_ const byte* pByteStreamIn, _In_ const size_t kstLengthIn);
    ~EncodeBomMgr();

public:
    EMEncodeType GetEncodeType() const throw() { return m_emEncodingType; }
    STUBomHeader GetBomHeader() const throw() { return (NULL == m_pstuBOMHeader) ? g_kstuBom_NULL : *m_pstuBOMHeader; }

private:
    void Init(_In_ const STUBomHeader& stuBomHeader) throw();
    void InitInnerMembers() throw();

private:
    static STUBomHeader EstablishBomHeaderFromBomUnit(_In_ const STUBomUnit& kstuBomUnitIn) throw();
    static int GetEffectiveBomLengthFromBomUnit(_In_ const STUBomUnit& kstuBomUnitIn) throw();
    static STUBomUnit GetByteStreamBOMHeader(_In_ const byte* pByteStreamIn, _In_ const size_t kstLengthIn) throw();
    static std::list<std::pair<int, BomCode>> GetAllPossibleBOMCodeFromBOMHeader(_In_ const STUBomUnit& stuBomHeader) throw();

private:
    EMEncodeType m_emEncodingType;  
    STUBomHeader* m_pstuBOMHeader;
};
