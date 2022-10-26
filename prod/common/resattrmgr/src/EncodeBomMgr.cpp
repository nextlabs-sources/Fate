#include "EncodeBomMgr.h"
#include "EncodeBaseInfoMgr.h"

#pragma region Standard BOM Headers

const STUBomHeader g_kstuBom_NULL(STUBomUnit(0x00, 0x00, 0x00, 0x00), 0);

const STUBomHeader g_kstuBom_UTF8(STUBomUnit(0xEF, 0xBB, 0xBF, 0x00), 3);

const STUBomHeader g_kstuBom_Unicode_BigEnd(STUBomUnit(0xFE, 0xFF, 0x00, 0x00), 2);
const STUBomHeader g_kstuBom_Unicode(STUBomUnit(0xFF, 0xFE, 0x00, 0x00), 2);

const STUBomHeader g_kstuBom_UTF32_BigEnd(STUBomUnit(0x00, 0x00, 0xFE, 0xFF), 4);
const STUBomHeader g_kstuBom_UTF32(STUBomUnit(0xFF, 0xFE, 0x00, 0x00), 4);

#pragma endregion


STUBomUnit::STUBomUnit(_In_ const unsigned int unFirstIn, _In_ const unsigned int unSecondIn, _In_ const unsigned int unThirdIn, _In_ const unsigned int unFourthIn)
    :unFirst(unFirstIn), unSecond(unSecondIn), unThird(unThirdIn), unFourth(unFourthIn)
{

}

BomCode STUBomUnit::GetBOMCode(_In_ const int knEffectLength) const throw()
{
    BomCode nBomCodeRet = 0;
    if (2 == knEffectLength)
    {
        nBomCodeRet = (unFirst << 24) + (unSecond << 16);
    }
    else if (3 == knEffectLength)
    {
       nBomCodeRet = (unFirst << 24) + (unSecond << 16) + (unThird << 8);
    }
    else if (4 == knEffectLength)
    {
       nBomCodeRet = (unFirst << 24) + (unSecond << 16) + (unThird << 8) + (unFourth);
    }
    else
    {
        nBomCodeRet = 0;
    }
    return nBomCodeRet;
}
const byte* STUBomUnit::GetBomHeaderByteStream(_In_ const int knEffectLength) const throw()
{
    if (IsEffectiveBomLength(knEffectLength))
    {
        return (const byte*)(this);
    }
    else
    {
        return NULL;
    }
}

bool STUBomUnit::IsEffectiveBomLength(_In_ const int knEffectLength) throw()
{
    return ((1 < knEffectLength) && (5 > knEffectLength));
}

STUBomHeader::STUBomHeader(_In_ const STUBomUnit& kstuBomHeaderIn, _In_ const int knBomEffectiveLengthIn)
    :stuBomHeader(kstuBomHeaderIn), nBomEffectiveLength(knBomEffectiveLengthIn),
    nBomCode(kstuBomHeaderIn.GetBOMCode(knBomEffectiveLengthIn)),
    pByteBomStream(kstuBomHeaderIn.GetBomHeaderByteStream(knBomEffectiveLengthIn))
{
}
bool STUBomHeader::IsValidBomHeader() const throw()
{
    return STUBomUnit::IsEffectiveBomLength(nBomEffectiveLength);
}


EncodeBomMgr::EncodeBomMgr(_In_ const STUBomHeader& kstuBomHeaderIn) : m_pstuBOMHeader(NULL), m_emEncodingType(emEncode_Unknown)
{
    Init(kstuBomHeaderIn);
}
EncodeBomMgr::EncodeBomMgr(_In_ const STUBomUnit& kstuBomUnitIn) : m_pstuBOMHeader(NULL), m_emEncodingType(emEncode_Unknown)
{
    const STUBomHeader kstuBomHeader = EstablishBomHeaderFromBomUnit(kstuBomUnitIn); 
    Init(kstuBomHeader);
}
EncodeBomMgr::EncodeBomMgr(_In_ const byte* pByteStreamIn, _In_ const size_t kstLengthIn) : m_pstuBOMHeader(NULL), m_emEncodingType(emEncode_Unknown)
{
    if (NULL != pByteStreamIn)
    {
        const STUBomUnit kstuBomUnit = GetByteStreamBOMHeader(pByteStreamIn, kstLengthIn);
        const STUBomHeader kstuBomHeader = EstablishBomHeaderFromBomUnit(kstuBomUnit);
        Init(kstuBomHeader);
    }
    else
    {
        InitInnerMembers();
    }
}

EncodeBomMgr::~EncodeBomMgr()
{
    if (NULL != m_pstuBOMHeader)
    {
        delete m_pstuBOMHeader;
    }
}

void EncodeBomMgr::Init(_In_ const STUBomHeader& kstuBomHeader) throw()
{
    InitInnerMembers();

    if (kstuBomHeader.IsValidBomHeader())
    {
        m_pstuBOMHeader = new STUBomHeader(kstuBomHeader);

        EncodeBaseInfoMgr& obEncodeBaseInfoMgrIns = EncodeBaseInfoMgr::GetInstance();
        m_emEncodingType = obEncodeBaseInfoMgrIns.GetEncodingTypeByBOMCode(kstuBomHeader.nBomCode);
    }
}
void EncodeBomMgr::InitInnerMembers() throw()
{
    m_pstuBOMHeader = NULL;
    m_emEncodingType = emEncode_Unknown;
}

STUBomHeader EncodeBomMgr::EstablishBomHeaderFromBomUnit(_In_ const STUBomUnit& kstuBomUnitIn) throw()
{
    int nEffectiveBomLength = GetEffectiveBomLengthFromBomUnit(kstuBomUnitIn);
    if ((1 < nEffectiveBomLength) && (5 > nEffectiveBomLength))
    {
        return STUBomHeader(kstuBomUnitIn, nEffectiveBomLength);
    }
    else
    {
        return STUBomHeader(STUBomUnit(0x00, 0x00, 0x00, 0x00), 0);
    }
}

int EncodeBomMgr::GetEffectiveBomLengthFromBomUnit(_In_ const STUBomUnit& kstuBomUnitIn) throw()
{
    int nEffectiveBomLengthRet = 0;

    EMEncodeType emEncodeType = emEncode_Unknown;
    EncodeBaseInfoMgr& obEncodeBaseInfoMgrIns = EncodeBaseInfoMgr::GetInstance();   

    std::list<std::pair<int, BomCode>> lsBomLengthAndCode = GetAllPossibleBOMCodeFromBOMHeader(kstuBomUnitIn);
    for (std::list<std::pair<int, BomCode>>::const_iterator kitr = lsBomLengthAndCode.begin(); kitr != lsBomLengthAndCode.end(); ++kitr)
    {
        emEncodeType = obEncodeBaseInfoMgrIns.GetEncodingTypeByBOMCode(kitr->second);
        if (emEncode_Unknown == emEncodeType)
        {
            // Continue
        }
        else
        {                                                         
            nEffectiveBomLengthRet = kitr->first;
        }
    }
    return nEffectiveBomLengthRet;
}
STUBomUnit EncodeBomMgr::GetByteStreamBOMHeader(_In_ const byte* pByteStreamIn, _In_ const size_t kstLengthIn) throw()
{
    byte szBom[4] = { 0 };
    for (size_t i = 0; (i < 4) && (i < kstLengthIn); ++i)
    {
        szBom[i] = *(pByteStreamIn + i);       
    }

#if 0 // Print byte stream and BOM   
    printf("\nByteStream:\n\t");
    for (size_t i = 0; i < kstLengthIn; ++i)
    {
        printf("%x ", *(pByteStreamIn + i));
    }
    printf("\nBOM:\t%x %x %x %x\n", szBom[0], szBom[1], szBom[2], szBom[3]);
#endif
    
    return STUBomUnit(szBom[0], szBom[1], szBom[2], szBom[3]);
}
std::list<std::pair<int, BomCode>> EncodeBomMgr::GetAllPossibleBOMCodeFromBOMHeader(_In_ const STUBomUnit& stuBomHeader) throw()
{
    std::list<std::pair<int, BomCode>> lsBomLengthAndCodeRet;

    BomCode nItemBomCode = 0;    
    for (int i = 2; i < 5; ++i)
    {
        nItemBomCode = stuBomHeader.GetBOMCode(i);
        lsBomLengthAndCodeRet.push_back(std::pair<int, BomCode>(i, nItemBomCode));
    }
    return lsBomLengthAndCodeRet;
}