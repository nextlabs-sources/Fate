#pragma once

#include <string>
#include <map>

#include <sal.h>

#include <EncodeBaseInfoMgr.h>
#include <EncodeBomMgr.h>

class EncodeSteamMgr
{
public:
    EncodeSteamMgr(_In_opt_ byte* pByteEncodeStreamIn = NULL, _In_ const size_t kstEncodeStreamLengthIn = 0, _In_ const EMEncodeType kemDefaultEncodeTypeIn = emEncode_Unknown);
    ~EncodeSteamMgr();

private:  // emphasize the following members are private
    EncodeSteamMgr(_In_opt_ const EncodeSteamMgr&);
    const EncodeSteamMgr& operator=(_In_opt_ const EncodeSteamMgr&);

public:
    bool SetEncodeStream(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemDefaultEncodeTypeIn) throw();

    byte* GetSpeicifyEncodeByteStream(_In_ EMEncodeType emTargetEncodeTypeIn, _In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutBufferLength) const throw();
    wchar_t* GetWideCharBuffer(_In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutWCharLength) const throw();
    void FreeResource(_Inout_ void** ppResourceIn) const throw();  

private:
    void ReEstablishStreamContent(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemEncodeTypeIn, _In_ const STUBomHeader& kstuBomHeaderIn) throw();
    void ReInitStoredResouce() throw();
    void InnerEstablishStreamContent(_In_ byte* pByteEncodeStreamIn, _In_ const size_t kstEncodeStreamLengthIn, _In_ const EMEncodeType kemEncodeTypeIn, _In_ const STUBomHeader& kstuBomHeaderIn) throw();
    
    const wchar_t* InnerGetWideCharNoBomBuffer(_Out_ bool& kbNeedFreeWideSteamOut, _Inout_opt_ size_t* pnOutWCharLength) const throw();
    byte* CloneEncodeByteStream(_In_ const bool kbIncludeBom, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutByteLength) const throw();

private:
    static wchar_t* HZConvertByteStreamToWideChar(_In_ const byte* kpchSourceIn, _In_ const size_t knCharCount, _In_ const UINT kunSourceCodePage, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutWCharLength) throw();
    static byte* HZConvertWideCharToByteStream(_In_ const wchar_t* kpwchSourceIn, _In_ const size_t knWCharCount, _In_ const UINT kunTargetCodePage, _In_opt_ const byte* pByteBomHeader, _In_ int nBomHeaderLength, _In_ const bool kbForceEndWithZero, _Inout_opt_ size_t* pnOutByteLength) throw();

private:
    byte* m_pByteEncodeStreamNoBom;
    size_t m_stEncodeStreamLength;
    EMEncodeType m_emEncodeType;
    // The pass in stream original BOM info
    // If the stream do not contains BOM this pointer is NULL
    STUBomHeader* m_pstuOriginalBomHeader;
};
