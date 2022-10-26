#include "EncodeSteamMgr.h"
#include "EncodeBaseInfoMgr.h"
#include "EncodeBomMgr.h"


EncodeSteamMgr::EncodeSteamMgr(_In_opt_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemDefaultEncodeTypeIn)
    : m_pByteEncodeStreamNoBom(NULL), m_stEncodeStreamLength(0), m_emEncodeType(emEncode_Unknown), m_pstuOriginalBomHeader(NULL)
{
    if (NULL != pByteEncodeStreamIn)
    {
        SetEncodeStream(pByteEncodeStreamIn, kstEncodeStreamLengthIn, kemDefaultEncodeTypeIn);
    }
}
EncodeSteamMgr::~EncodeSteamMgr()
{
    ReInitStoredResouce();
}

bool EncodeSteamMgr::SetEncodeStream(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemDefaultEncodeTypeIn)
{
    bool bRet = false;

    // Parameters check
    if ((NULL != pByteEncodeStreamIn) && (0 < kstEncodeStreamLengthIn))
    {
        // Check BOM
        EncodeBomMgr obEncodeBomMgr(pByteEncodeStreamIn, kstEncodeStreamLengthIn);
        EMEncodeType emEncodeType = obEncodeBomMgr.GetEncodeType();
        if (emEncode_Unknown == emEncodeType)
        {
            // The stream no BOM or no support BOM
            if (emEncode_Unknown == kemDefaultEncodeTypeIn)
            {
                emEncodeType = EncodeBaseInfoMgr::GetInstance().GetDefaultEncodingType();
            }
            else
            {
                emEncodeType = kemDefaultEncodeTypeIn;
            }
        }
        ReEstablishStreamContent(pByteEncodeStreamIn, kstEncodeStreamLengthIn, emEncodeType, obEncodeBomMgr.GetBomHeader());
        bRet = true;
    }
    return bRet;
}
void EncodeSteamMgr::ReEstablishStreamContent(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemEncodeTypeIn, _In_ const STUBomHeader& kstuBomHeaderIn) throw()
{
    ReInitStoredResouce();
    InnerEstablishStreamContent(pByteEncodeStreamIn, kstEncodeStreamLengthIn, kemEncodeTypeIn, kstuBomHeaderIn);
}

void EncodeSteamMgr::ReInitStoredResouce() throw()
{
    if (NULL != m_pByteEncodeStreamNoBom)
    {
        delete[] m_pByteEncodeStreamNoBom;
        m_pByteEncodeStreamNoBom = NULL;
    }
    m_stEncodeStreamLength = 0;

    m_emEncodeType = emEncode_Unknown;

    if (NULL != m_pstuOriginalBomHeader)
    {
        delete m_pstuOriginalBomHeader;
        m_pstuOriginalBomHeader = NULL;
    }
}

void EncodeSteamMgr::InnerEstablishStreamContent(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemEncodeTypeIn, _In_ const STUBomHeader& kstuBomHeaderIn) throw()
{
    m_emEncodeType = kemEncodeTypeIn;

    int nBomLength = 0;
    if (kstuBomHeaderIn.IsValidBomHeader())
    {
        m_pstuOriginalBomHeader = new STUBomHeader(kstuBomHeaderIn);
        nBomLength = m_pstuOriginalBomHeader->nBomEffectiveLength;
    }
    else
    {
        m_pstuOriginalBomHeader = NULL;
    }

    m_stEncodeStreamLength = kstEncodeStreamLengthIn - nBomLength;
    m_pByteEncodeStreamNoBom = new byte[m_stEncodeStreamLength];
    memset(m_pByteEncodeStreamNoBom, 0, m_stEncodeStreamLength);
    memcpy(m_pByteEncodeStreamNoBom, pByteEncodeStreamIn + nBomLength, m_stEncodeStreamLength);
}

byte* EncodeSteamMgr::GetSpeicifyEncodeByteStream(_In_ EMEncodeType emTargetEncodeTypeIn, _In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutByteLength) const throw()
{
    byte* kpTargetByteSteamRet = NULL;

    // Standard input parameters
    EncodeBaseInfoMgr& obEncodeBaseInfoMgrIns = EncodeBaseInfoMgr::GetInstance();
    if (emEncode_Unknown == emTargetEncodeTypeIn)
    {
        emTargetEncodeTypeIn = obEncodeBaseInfoMgrIns.GetDefaultEncodingType();
    }

    if (emTargetEncodeTypeIn == m_emEncodeType)
    {
          kpTargetByteSteamRet = CloneEncodeByteStream(kbIncludeBom, kbForceEndWithZero, pnOutByteLength);
    }
    else
    {
        // Convert to Unicode
        bool bNeedFreeWideStream = false;
        size_t stWideCharCount = 0;
        const wchar_t* kpwchTargetByteSteamNoBom = InnerGetWideCharNoBomBuffer(bNeedFreeWideStream, &stWideCharCount);

        // Convert from unicode to specify byte stream
        if ((NULL != kpwchTargetByteSteamNoBom) && (0 < stWideCharCount))
        {
            UINT unCodePage = obEncodeBaseInfoMgrIns.GetCodePageByEncodeType(emTargetEncodeTypeIn);
            const STUBomHeader* kpstuTargetBomHeader = kbIncludeBom ? (obEncodeBaseInfoMgrIns.GetBOMHeaderByEncodeType(emTargetEncodeTypeIn)) : NULL;
            if (NULL == kpstuTargetBomHeader)
            {
                kpTargetByteSteamRet = HZConvertWideCharToByteStream(kpwchTargetByteSteamNoBom, stWideCharCount, unCodePage, NULL, 0, kbForceEndWithZero, pnOutByteLength);
            }
            else
            {
                kpTargetByteSteamRet = HZConvertWideCharToByteStream(kpwchTargetByteSteamNoBom, stWideCharCount, unCodePage, kpstuTargetBomHeader->pByteBomStream, kpstuTargetBomHeader->nBomEffectiveLength, kbForceEndWithZero, pnOutByteLength);
            }

            if (bNeedFreeWideStream)
            {
                FreeResource((void**)(&kpwchTargetByteSteamNoBom));
            }
        }
    }

    return kpTargetByteSteamRet;
}

const wchar_t* EncodeSteamMgr::InnerGetWideCharNoBomBuffer(_Out_ bool& bNeedFreeWideSteamOut, _Inout_opt_ size_t* pnOutWCharLength) const throw()
{
    bNeedFreeWideSteamOut = false;
    size_t stWideCharCount = 0;
    const wchar_t* kpwchTargetByteSteamNoBomRet = NULL;
    if ((emEncode_Unicode == m_emEncodeType) || (emEncode_Unicode_BigEnd == m_emEncodeType))
    {
        kpwchTargetByteSteamNoBomRet = (wchar_t*)(m_pByteEncodeStreamNoBom);
        stWideCharCount = (m_stEncodeStreamLength / sizeof(wchar_t));
    }
    else
    {
        kpwchTargetByteSteamNoBomRet = GetWideCharBuffer(false, false, &stWideCharCount);
        bNeedFreeWideSteamOut = true;
    }

    if (NULL != pnOutWCharLength)
    {
        *pnOutWCharLength = stWideCharCount;
    }
    return kpwchTargetByteSteamNoBomRet;
}

wchar_t* EncodeSteamMgr::GetWideCharBuffer(_In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutWCharLength) const throw()
{
    wchar_t* kpwchTargetByteSteamRet = NULL;
    if ((NULL != m_pByteEncodeStreamNoBom) && (0 < m_stEncodeStreamLength) && (emEncode_Unknown != m_emEncodeType))
    {
        if ((emEncode_Unicode == m_emEncodeType) || (emEncode_Unicode_BigEnd == m_emEncodeType))
        {
            size_t stByteLength = 0;
            kpwchTargetByteSteamRet = (wchar_t*)CloneEncodeByteStream(kbIncludeBom, kbForceEndWithZero, &stByteLength);
            if (NULL != pnOutWCharLength)
            {
                *pnOutWCharLength = stByteLength / sizeof(wchar_t);
            }
        } 
        else
        {
            EncodeBaseInfoMgr& obEncodeBaseInfoMgrIns = EncodeBaseInfoMgr::GetInstance();
            UINT unCodePage = obEncodeBaseInfoMgrIns.GetCodePageByEncodeType(m_emEncodeType);
            if (kbIncludeBom)
            {
                kpwchTargetByteSteamRet = HZConvertByteStreamToWideChar(m_pByteEncodeStreamNoBom, m_stEncodeStreamLength, unCodePage, g_kstuBom_Unicode.pByteBomStream, g_kstuBom_Unicode.nBomEffectiveLength, kbForceEndWithZero, pnOutWCharLength);
            }
            else
            {
                kpwchTargetByteSteamRet = HZConvertByteStreamToWideChar(m_pByteEncodeStreamNoBom, m_stEncodeStreamLength, unCodePage, NULL, 0, kbForceEndWithZero, pnOutWCharLength);
            }
        }
    }
    return kpwchTargetByteSteamRet;
}

void EncodeSteamMgr::FreeResource(_Inout_ void** ppResourceIn) const throw()
{
    if (NULL != ppResourceIn)
    {
        void* pResource = *ppResourceIn;
        if (NULL != pResource)
        {
            delete[] pResource;
            pResource = NULL;
        }
    }
}


byte* EncodeSteamMgr::CloneEncodeByteStream(_In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutByteLength) const throw()
{
    int nSteamLength = m_stEncodeStreamLength + (kbForceEndWithZero ? 2 : 0);

    EncodeBaseInfoMgr& obEncodeBaseInfoMgrIns = EncodeBaseInfoMgr::GetInstance();
    const STUBomHeader* kpstuTargetBomHeader = NULL;
    if (kbIncludeBom)
    {
        if (NULL == m_pstuOriginalBomHeader)
        {
            kpstuTargetBomHeader = obEncodeBaseInfoMgrIns.GetBOMHeaderByEncodeType(m_emEncodeType);
        }
        else
        {
            kpstuTargetBomHeader = m_pstuOriginalBomHeader;
        }
    }

    int nBomLength = (NULL == kpstuTargetBomHeader) ? 0 : kpstuTargetBomHeader->nBomEffectiveLength;

    byte* pByteTargetSteamRet = new byte[nSteamLength + nBomLength];
    memset(pByteTargetSteamRet, 0, nSteamLength + nBomLength);

    if (NULL != kpstuTargetBomHeader)
    {
        memcpy(pByteTargetSteamRet, kpstuTargetBomHeader->pByteBomStream, nBomLength);
    }
    memcpy(pByteTargetSteamRet + nBomLength, m_pByteEncodeStreamNoBom, nSteamLength);

    if (NULL != pnOutByteLength)
    {
        *pnOutByteLength = nSteamLength + nBomLength;
    }

    return pByteTargetSteamRet;
}

wchar_t* EncodeSteamMgr::HZConvertByteStreamToWideChar(_In_ const byte* kpchSourceIn, _In_ const size_t knCharCount, _In_ const UINT kunSourceCodePage, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutWCharLength)
{
    int nTargetBufLen = MultiByteToWideChar(kunSourceCodePage, 0, (LPCSTR)(kpchSourceIn), (int)knCharCount, NULL, 0) + (kbForceEndWithZero ? 1 : 0);
    int nTargetFullBufWithBomLength = nTargetBufLen;
    bool bNeedBom = ((NULL != pByteBomHeader) && (STUBomUnit::IsEffectiveBomLength(nBomHeaderLength)));
    if (bNeedBom)
    {
        nTargetFullBufWithBomLength += nBomHeaderLength;
    }
    else
    {
        nBomHeaderLength = 0;
    }

    wchar_t* pwchTargetBufferRet = new wchar_t[nTargetFullBufWithBomLength];
    wmemset(pwchTargetBufferRet, '\0', nTargetFullBufWithBomLength);

    if (bNeedBom)
    {
        memcpy(pwchTargetBufferRet, pByteBomHeader, nBomHeaderLength);
    }
    MultiByteToWideChar(kunSourceCodePage, 0, (LPCSTR)kpchSourceIn, (int)knCharCount, (wchar_t*)((byte*)(pwchTargetBufferRet) + nBomHeaderLength), nTargetBufLen);

    if (NULL != pnOutWCharLength)
    {
        *pnOutWCharLength = nTargetFullBufWithBomLength;
    }
    return pwchTargetBufferRet;
}
byte* EncodeSteamMgr::HZConvertWideCharToByteStream(_In_ const wchar_t* kpwchSourceIn, _In_ const size_t knWCharCount, _In_ const UINT kunTargetCodePage, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutByteLength)
{
    int nTargetBufLen = ::WideCharToMultiByte(kunTargetCodePage, NULL, kpwchSourceIn, (int)knWCharCount, NULL, NULL, NULL, NULL) + (kbForceEndWithZero ? 1 : 0);
    int nFullBufWithBomLength = nTargetBufLen;
    bool bNeedBom = ((NULL != pByteBomHeader) && (STUBomUnit::IsEffectiveBomLength(nBomHeaderLength)));
    if (bNeedBom)
    {
        nFullBufWithBomLength += nBomHeaderLength;
    }
    else
    {
        nBomHeaderLength = 0;
    }

    byte* pByteTargetBufferRet = new byte[nFullBufWithBomLength];
    memset(pByteTargetBufferRet, '\0', nFullBufWithBomLength);

    if (bNeedBom)
    {
        memcpy(pByteTargetBufferRet, pByteBomHeader, nBomHeaderLength);
    }
    ::WideCharToMultiByte(kunTargetCodePage, NULL, kpwchSourceIn, (int)knWCharCount, (LPSTR)(pByteTargetBufferRet + nBomHeaderLength), nTargetBufLen, NULL, NULL);

    if (NULL != pnOutByteLength)
    {
        *pnOutByteLength = nFullBufWithBomLength;
    }
    return pByteTargetBufferRet;
}