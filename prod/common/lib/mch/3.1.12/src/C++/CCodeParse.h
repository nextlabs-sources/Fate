// ***************************************************************
//  CCodeParse.h              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  disassemble a single asm instruction
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CCODEPARSE_H
#define _CCODEPARSE_H

#ifndef _SString_H
  #include "SString.h"
#endif

#ifdef _WIN64
  typedef LPVOID REG_STATE[16];
#else
  typedef LPVOID REG_STATE[8];
#endif

const DWORD DisAsmErrorBase           = 0x770000;
const DWORD UnknownTargetError        = DisAsmErrorBase + 0;
const DWORD InvalidCodeError          = DisAsmErrorBase + 1;
const DWORD CodeNotInterceptableError = DisAsmErrorBase + 2;
const DWORD BadFunctionError          = DisAsmErrorBase + 3;
const DWORD DoubleHookError           = DisAsmErrorBase + 4;

const LPWSTR UnknownTargetErrorStr        = L"This target can't be seen in the assembler code.";
const LPWSTR InvalidCodeErrorStr          = L"Invalid code!";
const LPWSTR CodeNotInterceptableErrorStr = L"This code is not interceptable due to it's design.";
const LPWSTR BadFunctionErrorStr          = L"The specified function is bad.";
const LPWSTR DoubleHookErrorStr           = L"This code was already hooked by another hooking library.";

class SYSTEMS_API CCodeParse
{
  public:
    CCodeParse();
    CCodeParse(LPVOID pCode);
    CCodeParse(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap);

    ~CCodeParse(void);

    void Parse(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap);

    LPCWSTR ToString(void);

    LPVOID mpCodeAddress;          // Address where code begins
    BOOL mIsValid;                 // is the code pointer valid?
    BOOL mIsCall;                  // is the instruction a call?
    BOOL mIsJmp;                   // is the instruction a jump?
    BOOL mIsRelTarget;             // is this target relative?  (or absolute?)
    BOOL mIsEnlargeable;           // can the target size of this opcode be extended?

    WORD mOpCode;                  // Opcode, one byte (0x00xx) or two byte (0x0fxx) where xx is the opCode
    BYTE mModRm;                   // ModRm byte, if available, otherwise 0.
    #ifdef _WIN64
      BOOL mIsRipRelative;         // is the displacement data RIP relative?
    #endif
    ULONG_PTR mDisplacementDword;  // displacement data in dword form
    LPVOID mpNext;                 // Next code address

    LPVOID mTarget;                // Absolute target address = mpTarget + DataLength (2 or 4) + Data Value
    LPVOID mpTarget;               // Pointer to target information in the code
    LPVOID *mppTarget;             // Pointer to pointer to target information

    BOOL mIsPush;                  // is the instruction a push?
    int mPushParameter;            // -1 if unknown, 0-xx is passed in DWORD parameter
    BOOL mIsPop;                   // is the instruction a pop?
    int mTargetSize;               // Size of target information in bytes

    int mDisplacementSize;          // size of displacement data
    LPVOID mpDisplacement;          // at which address is the displacement data stored?
    LONG_PTR mDisplacementInt;      // displacement data in integer form

    int mImmediateDataLength;       // size of immediate data
    LONG_PTR mImmediateValue;       // immediate value (in integer form, if available)
    int mOperandWordSize;           // 4 or 2, depending on the operand size prefix

  private:
    void InitializeMembers(LPVOID pCode);

    void IncCP(DWORD d = 1);

    void RawParseCode(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap);

    BOOL CheckPrefix(REG_STATE *pRegState);
    void ParseModRm(void);
    int ParseImmediateData(void);
    BOOL IsValidOpCode(void);
    void CheckTarget(REG_STATE *pRegState, CMemoryMap *pMemoryMap);
    void CheckRegState(REG_STATE *pRegState);

    void St(SString& st, int reg = 0);
    void RegStr(SString& rs, int len, int reg);
    void IxToInt(SString& ims);
    void IxToDw(SString& ims, LPVOID pCode);
    int CalculateMSize(void);
    void DisAsmModRm(SString& modRmString);
    void GetLabel(LPVOID pCode);
    BOOL CheckFunctionName(LPVOID pCode);
    void CheckStringData(ULONG_PTR data, BOOL isPointer);

    SString mDisAsm;

    BYTE mPrefixSegment;            // 26, 2e, 36, 3e, 64..65 segment override prefix
    BOOL mHasPrefixOPSIZ;           // 66 - operand size prefix
    BOOL mHasPrefixADRSIZ;          // 67 - address size prefix
    BOOL mHasPrefixLOCK;            // f0 - lock prefix
    BOOL mHasPrefixREPN;            // f2 - repne (or sse2 special size) prefix
    BOOL mHasPrefixREP;             // f3 - rep   (or sse2 special size) prefix
    #ifdef _WIN64
      BYTE mPrefixREX;              // 40..4f - REX prefix
    #endif

    LPVOID mpCode;                  // Current code address
    DWORD mOpCodeFlags;             // flags from one of the const flag tables
    BYTE mSib;                      // sib byte

    BOOL mRegPtr;                   // do the modrm reg/mem bits address a register?
    int mRegBits;                   // modrm reg/mem bits
    int mMultiBits;                 // modrm multi purpose bits
    int mSibScaleRegister;          // sib scale register
    int mSibScaleFactor;            // sib scale factor

    int mLabelIndex;                // label index
};

#endif