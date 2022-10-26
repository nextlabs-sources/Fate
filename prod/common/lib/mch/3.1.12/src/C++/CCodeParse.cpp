// ***************************************************************
//  CCodeParse.cpp            version: 1.0.2  ·  date: 2012-04-03
//  -------------------------------------------------------------
//  disassemble a single asm instruction
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2012 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2012-04-03 1.0.2 fixed small bug in register tracking
// 2010-07-26 1.0.1 (1) fixed string class usage bug
//                  (2) fixed: x64 disasm output "push rax" instead of "r8"
// 2010-01-10 1.0.0 initial version

#define _CCODEPARSE_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

// ------------ Constructors -----------------

CCodeParse::CCodeParse() : mDisAsm()
{
}

CCodeParse::CCodeParse(LPVOID pCode) : mDisAsm()
{
  InitializeMembers(pCode);
  RawParseCode(pCode, NULL, 0);
}

CCodeParse::CCodeParse(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap) : mDisAsm()
{
  InitializeMembers(pCode);
  RawParseCode(pCode, pRegState, pMemoryMap);
}

void CCodeParse::InitializeMembers(LPVOID pCode)
{
  mpCodeAddress = pCode;      // Address where code begins
  mpCode = pCode;             // Current code address for parsing
  mpNext = NULL;              // Next Code Address

  mTarget = NULL;             // Absolute target address
  mpTarget = NULL;            // Code Pointer to target information
  mppTarget = NULL;           // Pointer to pointer to target information
  mTargetSize = 0;            // Size of target information in bytes

  mIsRelTarget = FALSE;       // is this target relative?  (or absolute?)
  mIsCall = FALSE;            // is the instruction a call?
  mIsEnlargeable = FALSE;     // can the target size of this opcode be extended?
  mIsJmp = FALSE;             // is the instruction a jump?
  mIsValid = FALSE;           // is the code pointer valid?

  mOpCode = 0;                // Opcode, one byte (0x00xx) or two byte (0x0fxx) where xx is the opCode
  mOpCodeFlags = 0;           // flags from one of the const flag tables

  mModRm = 0;                 // ModRm byte, if available, otherwise 0.   
  #ifdef _WIN64
    mIsRipRelative = FALSE;     // is the displacement data RIP relative?
  #endif
  mRegPtr = FALSE;            // do the modrm reg/mem bits address a register?
  mRegBits = 0;               // modrm reg/mem bits
  mMultiBits = 0;             // modrm multi purpose bits

  mSib = 0;                   // Scale-Index-Base
  mSibScaleRegister = 0;      // Sib scale register
  mSibScaleFactor = 0;        // Sib scale factor

  mDisplacementSize = 0;      // size of displacement data
  mpDisplacement = NULL;      // Code pointer to displacement data
  mDisplacementInt = 0;       // displacement data in integer form
  mDisplacementDword = 0;     // displacement data in dword form

  mImmediateDataLength = 0;   // size of immediate data
  mImmediateValue = 0;        // immediate value (in integer form, if available)
  mOperandWordSize = 4;       // 4 or 2, depending on the operand size prefix, DEFAULT is 4

  mPrefixSegment = 0;
  mHasPrefixOPSIZ = FALSE;
  mHasPrefixADRSIZ = FALSE;
  mHasPrefixLOCK = FALSE;
  mHasPrefixREP = FALSE;
  mHasPrefixREPN = FALSE;
  #ifdef _WIN64
    mPrefixREX = 0;
  #endif

  mLabelIndex = 0;            // label index

  mDisAsm.NullString();

  mIsPush = FALSE;
  mIsPop = FALSE;
  mPushParameter = -1;
}
// ------------ Destructor -----------------

CCodeParse::~CCodeParse()
{
}

void CCodeParse::Parse(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap)
{
  InitializeMembers(pCode);
  RawParseCode(pCode, pRegState, pMemoryMap);
}

LPCWSTR CCodeParse::ToString()
{
  if (mDisAsm.IsEmpty())
  {
    SString rs;
    SString ms;
    SString ims;
    if (mpCodeAddress != mpNext)
    {
      if (mIsValid)  // is this opcode/modrm combination valid?
      {
        // it is, first of all let's print out the code address
        mDisAsm.Format(L"%p   ", mpCodeAddress);

        // then we check the prefixes
        if (mHasPrefixLOCK)
          mDisAsm += L"lock ";
        if (mOpCode < 0xff)
        {
          if (mHasPrefixREPN)
            mDisAsm += L"repne ";
          if (mHasPrefixREP)
            mDisAsm += L"rep ";
        }

        GetLabel(mpNext);

        // Add instruction parameters
        if ((mOpCodeFlags & fMod) != 0)
        {
          // ModRm byte present
          // Check for special cases
          if ((mOpCode > 0xff) && (mHasPrefixREPN || mHasPrefixREP))
          {
            // sse2 instruction present with special prefixes
            if ((mOpCode == 0x0f2a) || (mOpCode == 0x0f2c) || (mOpCode == 0x0f2d))
            {
              // These instructions change the mod64 or reg64 flags to 32
              if ((mOpCodeFlags & fMod) == fMod64)
                mOpCodeFlags = (mOpCodeFlags & (~fMod)) + fMod32;
              else
                mOpCodeFlags = (mOpCodeFlags & (~fReg)) + fReg32;
            }
            else
            {
              if ((mOpCode == 0x0f6f) || (mOpCode == 0x0f70) || (mOpCode == 0x0f7e) || (mOpCode == 0x0f7f))
              {
                mOpCodeFlags = (mOpCodeFlags & (~fMod)) + fMod128;
                mOpCodeFlags = (mOpCodeFlags & (~fReg)) + fReg128;
              }
              else
              {
                if (mOpCode == 0x0fd6)
                {
                  if (mHasPrefixREP)
                    mOpCodeFlags = (mOpCodeFlags & (~fMod)) + fMod128;
                  else
                    mOpCodeFlags = (mOpCodeFlags & (~fReg)) + fReg128;
                }
              }
            }
          }

          // Compose the register string, if a register is available
          // A ModRm byte, so the register information is stored there
          switch (mOpCodeFlags & fReg)
          {
            case fReg8:    // byte register
              #ifdef _WIN64
                if ((mPrefixREX != 0) && (mMultiBits < 8))
                  RegStr(rs, 6, mMultiBits);  // byte register amd64 special mode
                else
              #endif
                RegStr(rs, 1, mMultiBits);
              break;
            case fReg16:   // word register
              RegStr(rs, 2, mMultiBits);
              break;
            case fRegxx:   // segment/cr/dr register
              if (mOpCode > 0xff)
              {
                if ((mOpCode % 2) == 1)
                  rs.Format(L"dr%d", mMultiBits);
                else
                  rs.Format(L"cr%d", mMultiBits);
              }
              else 
                rs = CRegisterLabels[3][mMultiBits];
              break;
            case fReg32:   // (d)word register
              RegStr(rs, 4, mMultiBits);
              break;
            case fReg64:   // qword register
              RegStr(rs, 8, mMultiBits);
              break;
            case fRegSt:   // st floating point register
              St(rs);
              break;
            case fReg128:  // oword register
              RegStr(rs, 16, mMultiBits);
              break;
            default:
              rs.NullString();
              break;
          }
          // compose the modrm byte into a string
          DisAsmModRm(ms);
        }
        else
        {
          ms.NullString();
          // we have no modrm byte
          // we compose the register string, if a register is available
          // check the flags for register information
          switch (mOpCodeFlags & fReg)
          {
            case fRegAl:      // al register
              RegStr(rs, 1, 0);
              break;
            case fRegEax:     // (e)ax register
              RegStr(rs, 4, 0);
              break;
            case fRegO8:      // byte register depending on opcode
              RegStr(rs, 1, mOpCode & 0x07);
              break;
            case fRegO32:     // (d)word register depending on opcode
              #ifdef _WIN64
                if ((mPrefixREX & 0x1) != 0)
                  RegStr(rs, 4, mOpCode & 0x07 + 8);
                else
              #endif
                RegStr(rs, 4, mOpCode & 0x07);
              break;
            case fRegEaxO:    // fRegEax + fRegO32
              RegStr(rs, 4, 0);
              RegStr(ms, 4, mOpCode & 0x07);
              break;
            case fRegDxA:     // dx register + (e)ax/al register
              if ((mOpCode % 2) == 1)
                RegStr(rs, 4, 0);
              else
                RegStr(rs, 1, 0);
              ms = L"dx";
              break;
            default:
              rs.NullString();
          }
        }
        LPVOID pCode = (LPVOID) ((ULONG_PTR) mpNext - (DWORD) mImmediateDataLength);
        if (mImmediateDataLength > 0)
        {
          if (mIsRelTarget || (mOpCode == 0x69) || (mOpCode == 0x6b) || ((mOpCode >= 0x80) && (mOpCode <= 0x83)))
            IxToInt(ims);
          else
            IxToDw(ims, pCode);
        }
        // now we have 3 strings: register (rs), modrm (ms) and immediate (ims)
        // let's sort out empty strings, the filled strings are stored into s2-s4
        SString s2, s3, s4;
        if (!rs.IsEmpty())
        {
          s2 = rs;
          if (!ms.IsEmpty())
          {
            s3 = ms;
            s4 = ims;
          }
          else
          {
            s3 = ims;
            s4.NullString();
          }
        }
        else
        {
          if (!ms.IsEmpty())
          {
            s2 = ms;
            s3 = ims;
          }
          else
          {
            s2 = ims;
            s3.NullString();
          }
          s4.NullString();
        }

        if (!s2.IsEmpty())
        {
          int fillup = 10 + sizeof(LPVOID) * 2;
          if ((int) mDisAsm.Length() >= fillup)
            fillup = (int) mDisAsm.Length();

          for (int i = (int) mDisAsm.Length(); i <= fillup; i++)
            mDisAsm += L" ";

          if ((mOpCodeFlags & fOrder) != 0)
          {
            SString t(s2);
            s2 = s3;
            s3 = t;
          }
          mDisAsm += s2;
          if (!s3.IsEmpty())
          {
            mDisAsm += L", ";
            mDisAsm += s3;
            if (!s4.IsEmpty())
            {
              mDisAsm += L", ";
              mDisAsm += s4;
            }
          }
        }
        // the following special cases didn't fit into the flags logic
        // so we handle them here manually
        if ((mOpCode == 0xd0) || (mOpCode == 0xd1))
          mDisAsm += L", 1";
        if ((mOpCode == 0xd2) || (mOpCode == 0xd3) || (mOpCode == 0x0fa5) || (mOpCode == 0x0fad))
          mDisAsm += L", cl";

        if (mTarget != NULL)
          CheckFunctionName(mTarget);
        else
        {
          if (((mOpCodeFlags & fMod) != 0) && (mDisplacementSize == 4))
            CheckStringData(mDisplacementDword, TRUE);
          if (mImmediateDataLength >= 4)
            CheckStringData((ULONG_PTR) mImmediateValue, (mOpCodeFlags & fPtr) != 0);
        }
      } // end IsValid
      else
      {
/*  madshi: this still needs to be converted
      // this opcode/modrm combination is invalid, print the data in "db"s
      for c1 := dword(result.This) to dword(code) + dword(imLen) - 1 do
        disAsm := disAsm + #$d#$a + CodeToHex(pointer(c1)) + '   ' +
                  'db ' + IntToHexEx(dword(byte(pointer(c1)^)), 2);
      Delete(disAsm, 1, 2);
*/
      }
    } // end if mpCodeAddress != mNext

  }
  return mDisAsm.GetBuffer();
}

// Increment code pointer by given number of bytes
inline void CCodeParse::IncCP(DWORD d)
{
  mpCode = (LPVOID) ((LPBYTE) mpCode + d);
}

void CCodeParse::RawParseCode(LPVOID pCode, REG_STATE *pRegState, CMemoryMap *pMemoryMap)
{
  mpCode = pCode;

  if (mpCode != NULL)
  {
    __try
    {
      // now we strip off all prefixes, so we end up with the real opcode
      BOOL morePrefixes = TRUE;

//wprintf(L"pcode before: %p -> %0X\n", mpCode, *((BYTE *) mpCode));

      while (morePrefixes)
      {
        mOpCode = *((BYTE *) mpCode);
        IncCP();
        morePrefixes = CheckPrefix(pRegState);
      }

//wprintf(L"pcode nach prefix: %p -> %0X\n", mpCode, *((BYTE *) mpCode));

      // Check opcode size
      if (mOpCode == 0x0f)
      {
        // Two byte opcode ($0f $xx)
        mOpCode = *((BYTE *) mpCode);
        mOpCodeFlags = COpCodeFlags0f[mOpCode];
        mLabelIndex = COpCodeLabelIndex0f[mOpCode];
        mOpCode += 0x0f00;
        IncCP();
      }
      else
      {
        // One byte opcode
        mOpCodeFlags = COpCodeFlags[mOpCode];
        mLabelIndex = COpCodeLabelIndex[mOpCode];
      }

//wprintf(L"pcode final: %p -> %0X\n", mpCode, *((BYTE *) mpCode));
//wprintf(L"%0X\n", mOpCode);

      if ((mOpCodeFlags & fMod) != 0)
      {
        // this instruction has a modrm byte, so let's parse it
        ParseModRm();

        if ((mOpCodeFlags & fMod) == fModOpc)
        {
          // this is one of the few opcodes, which differ quite much,
          // depending on the modrm opcode extension
          // so have to get the real flags from an additional table
          for (int i = 0; i < OpCodeFlagsExCount; i++)
          {
            if (COpCodeFlagsEx[i].OpCode == mOpCode)
            {
              if (mRegPtr)
                mOpCodeFlags = COpCodeFlagsEx[i].Flags[mMultiBits];
              else
                mOpCodeFlags = COpCodeFlagsEx[i].Flags[mMultiBits + 8];
              break;
            }
          }
        }
      }

      // how long (if available at all) is the immediate data for this opcode?
      mImmediateDataLength = ParseImmediateData();
      // is this opcode/modrm combination valid?
      if (IsValidOpCode())
      {
        mIsValid = TRUE;
        #ifdef _WIN64
          mIsRipRelative = ((mOpCodeFlags & fMod) != 0) && (mRegBits == -1) && (mSibScaleFactor == 0) && (mDisplacementSize == 4) && (mSib == 0);
          if (mIsRipRelative)
          {
            mDisplacementDword = (ULONGLONG) ((LPBYTE) mpCode + (int) mImmediateDataLength + mDisplacementInt);
            mDisplacementInt   = (LONGLONG) mDisplacementDword;
          }
        #endif
        CheckTarget(pRegState, pMemoryMap);
        if (pRegState != NULL)
          CheckRegState(pRegState);
      }
      IncCP(mImmediateDataLength);
    }
    __except (ExceptionFilter(L"RawParseCode", GetExceptionInformation()))
    {
      mIsValid = FALSE;
    }
  }

  mpNext = mpCode;

  if ((mIsJmp || mIsCall) && (mTarget == mpNext))
  {
    mIsJmp = FALSE;
    mIsCall = FALSE;
    mTarget = NULL;
    mpTarget = NULL;
    mppTarget = NULL;
    mTargetSize = 0;
    mIsEnlargeable = FALSE;
  }
}

// Checks for and processes current prefix if found
// Called by RawParseCode
BOOL CCodeParse::CheckPrefix(REG_STATE *pRegState)
{
  BOOL result = TRUE;

  switch (mOpCode)
  {
    #ifdef _WIN64
      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
      case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        mPrefixREX = (BYTE) mOpCode;
        break;
    #endif
    case 0x66:
      mHasPrefixOPSIZ = TRUE;
      mOperandWordSize = 2;
      break;
    case 0x67:
      mHasPrefixADRSIZ = TRUE;
      break;
    case 0xf0:
      mHasPrefixLOCK = TRUE;
      break;
    case 0xf2:
      mHasPrefixREPN = TRUE;
      // f2 is only "repne" in one byte opcodes
      // in two byte opcodes it is used as a special size flag for sse2
      if ((*((BYTE *) mpCode) != 0xf0) && (pRegState))
        (*pRegState)[1] = NULL;
      break;
    case 0xf3:
      mHasPrefixREP = TRUE;
      // f3 is only "rep" in one byte opcodes
      // in two byte opcodes it is used as a special size flag for sse2
      if (*((BYTE *) mpCode) != 0x0f && pRegState)
        (*pRegState)[1] = NULL;
      break;
    default:   // $26, $2e, $36, $3e and $64..$65 are segment override prefixes
      if ((mOpCode & 0xe7) == 0x26)
        mPrefixSegment = (BYTE) (((mOpCode >> 3) & 0x03) + 1);
      else
      {
        if ((mOpCode == 0x64) || (mOpCode == 0x65))
          mPrefixSegment = (BYTE) (mOpCode - 0x60 + 1);
        else
          result = FALSE;
      }
  }
  return result;
}

// Processes ModRm byte
// Called by RawParseCode
void CCodeParse::ParseModRm(void)
{
  mModRm = *((BYTE *) mpCode);
  mSib = 0;
  mRegPtr = (mModRm & 0xc0) != 0xc0;   // 0x11 is NOT a Register Pointer, 0x11 is
  mRegBits = mModRm & 0x07;            // mask is 0111b
  mMultiBits = (mModRm >> 3) & 0x07;   // 0011 1000 >> 3 becomes 0111b
  mSibScaleFactor = 0;

  IncCP();

  switch (mModRm & 0xc0)
  {
    case 0x40:
      mDisplacementSize = 1;
      break;
    case 0x80:
      #ifndef _WIN64
        if (mHasPrefixADRSIZ)
          mDisplacementSize = 2;
        else
      #endif
        mDisplacementSize = 4;
      break;
    default:
      mDisplacementSize = 0;
  }

  #ifndef _WIN64
    if (mRegPtr && mHasPrefixADRSIZ)
    {
      // the address size prefix has serious effect on modrm pointers
      if ((mRegBits >= 0) && (mRegBits <= 3))  // 0x00-0x11
      {
        mSibScaleRegister = 6 + (mRegBits & 0x01);
        mSibScaleFactor = 1;
      }
      if (((mModRm & 0xc0) == 0) && (mRegBits == 6))
      {
        mDisplacementSize = 2;
        mRegBits = -1;
      }
      else
      {
        switch (mRegBits)
        {
          case 0:
          case 1:
          case 7:
            mRegBits = 3;
            break;
          case 2:
          case 3:
          case 6:
            mRegBits = 5;
            break;
          case 4:
            mRegBits = 6;
            break;
          case 5:
            mRegBits = 7;
            break;
        }
      }
    }
    else
    {
  #endif
  if ((mRegBits == 4) && (mRegPtr))
  {
    mSib = *((BYTE *) mpCode);
    IncCP();

    mRegBits = mSib & 0x07;
    #ifdef _WIN64
      if (((mSib & 0x38) != 0x20) || ((mPrefixREX & 0x2) != 0))
    #else
      if ((mSib & 0x38) != 0x20)
    #endif
    {
      mSibScaleFactor = 1 << (mSib >> 6);
      mSibScaleRegister = (mSib >> 3) & 0x07;
    }
  }
  if ((mModRm & 0xc0) == 0 && (mRegBits == 5))
  {
    mDisplacementSize = 4;
    mRegBits = -1;
  }
  #ifndef _WIN64
    }
  #endif

  #ifdef _WIN64
    if ((mPrefixREX & 0x1) != 0)
      mRegBits += 8;
    if ((mPrefixREX & 0x4) != 0)
      mMultiBits += 8;
    if ((mPrefixREX & 0x2) != 0)
      mSibScaleRegister += 8;
  #endif

  mpDisplacement = mpCode;
  switch (mDisplacementSize)
  {
    case 1:
      mDisplacementInt = *((char*) mpCode);
      mDisplacementDword = *((BYTE *) mpCode);
      break;
    case 2:
      mDisplacementInt = *((short *) mpCode);
      mDisplacementDword = *((WORD *) mpCode);
      break;
    case 4:
      mDisplacementInt = *((int *) mpCode);
      mDisplacementDword = *((DWORD *) mpCode);
      break;
    default:
      mDisplacementInt = 0;
      mDisplacementDword = 0;
  }
  IncCP(mDisplacementSize);
}

int CCodeParse::ParseImmediateData(void)
{
  int result;

  switch (mOpCodeFlags & fI)
  {
    case fI8:
      result = 1;
      break;
    case fI16:
      result = 2;
      break;
    case fI32:
      #ifdef _WIN64
        if (((mPrefixREX & 0x8) != 0) && (mOpCode >= 0xb8) && (mOpCode <= 0xbf))
          result = 8;
        else
      #endif
        result = mOperandWordSize;
      break;
    default:
      result = 0;
      switch (mOpCode)
      {
        case 0x9a:
        case 0xea:
          result = mOperandWordSize + 2;
          break;
        case 0xc8:
          result = 3;
          break;
        case 0xa0:
        case 0xa1:
        case 0xa2:
        case 0xa3:
          #ifdef _WIN64
            if (mHasPrefixADRSIZ)
              result = 4;
            else
              result = 8;
          #else
            if (mHasPrefixADRSIZ)
              result = 2;
            else
              result = 4;
          #endif
          break;
      }
  }
  switch (result)
  {
    case 1:
      mImmediateValue = *((char *) mpCode);
      break;
    case 2:
      mImmediateValue = *((short *) mpCode);
      break;
    case 4:
      mImmediateValue = *((int *) mpCode);
      break;
    #ifdef _WIN64
      case 8:
        mImmediateValue = *((LONGLONG *) mpCode);
      break;
    #endif
    default:
      mImmediateValue = 0;
  }
  return result;
}

BOOL CCodeParse::IsValidOpCode(void)
{
  BOOL result = (mOpCodeFlags != fInvalid);

  if (result)
  {
    if (mOpCode > 0xff)
    {
      switch ((BYTE) (mOpCode & 0xff))
      {
        case 0x00:
          result = mMultiBits <= 5;
          break;
        case 0x01:
          result = ((mMultiBits != 5) && (mRegPtr || (mMultiBits > 3)));
          break;
        case 0x0d:
          result = mRegPtr && (mMultiBits <= 1);
          break;
        case 0x0f:
          {
            BYTE code2 = *((BYTE *) mpCode);
            result = ( (code2 == 0x0c) || (code2 == 0x0d) || (code2 == 0x1c) || (code2 == 0x1d) ||
                       (code2 == 0x8a) || (code2 == 0x8e) || (code2 == 0x90) || (code2 == 0x94) ||
                       (code2 == 0x96) || (code2 == 0x97) || (code2 == 0x9a) || (code2 == 0x9e) ||
                       (code2 == 0xa0) || (code2 == 0xa4) || (code2 == 0xa6) || (code2 == 0xa7) ||
                       (code2 == 0xaa) || (code2 == 0xae) || (code2 == 0xb0) || (code2 == 0xb4) ||
                       (code2 == 0xb6) || (code2 == 0xb7) || (code2 == 0xbb) || (code2 == 0xbf)    );
          }
          break;
        case 0x18:
          result = mRegPtr && (mMultiBits <= 3);
          break;
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
          result = !mRegPtr;
          break;
        case 0x6c:
        case 0x6d:
          result = mHasPrefixOPSIZ;
          break;
        case 0x71:
        case 0x72:
          result = (!mRegPtr) && ((mMultiBits == 2) || (mMultiBits == 4) || (mMultiBits == 6));
          break;
        case 0x73:
          result = (!mRegPtr) && ((mMultiBits == 2) || (mMultiBits == 3) || (mMultiBits == 6) || (mMultiBits == 7));
          break;
        case 0x7c:
        case 0x7d:
        case 0xd0:
          result = mHasPrefixOPSIZ || mHasPrefixREPN;
          break;
        case 0xae:
          result = (mMultiBits == 7) || ((mMultiBits < 4) && mRegPtr) || ((mMultiBits > 4) && (!mRegPtr));
          break;
        case 0xb2:
        case 0xb4:
        case 0xb5:
        case 0xc3:
        case 0xc7:
          result = mRegPtr;
          break;
        case 0xba:
          result = mMultiBits >= 4;
          break;
        case 0xd6:
        case 0xe6:  
          result = mHasPrefixOPSIZ || mHasPrefixREP || mHasPrefixREPN;
          break;
        case 0xf0:
          result = mHasPrefixREPN;
          break;
        default:
          // Future ASSERT
          break;
      }
    }
    else
    {
      BYTE mr;
      BOOL modRmInSet;
      switch ((BYTE) (mOpCode & 0xff))
      {
        case 0x62:
        case 0x8d:
        case 0xc4:
        case 0xc5:
          result = mRegPtr;
          break;
        case 0x8c:
        case 0x8e:
          result = mMultiBits <= 5;
          break;
        case 0xd9:
          mr = mModRm;
          modRmInSet = ((mr >= 0xd1) && (mr <= 0xd7)) || (mr == 0xe2) || (mr == 0xe3) || (mr == 0xe6) || (mr == 0xe7) || (mr == 0xef);
          result = mRegPtr || (!modRmInSet);
          break;
        case 0xda:
          mr = mModRm;
          modRmInSet = ((mr >= 0xea) && (mr <= 0xef)) || (mr == 0xe8);
          result = mRegPtr || (!modRmInSet);
          break;
        case 0xdb:
          mr = mModRm;
          modRmInSet = ((mr >= 0xe5) && (mr <= 0xe7));
          result = mRegPtr || (!modRmInSet);
          break;
        case 0xdf:
          mr = mModRm;
          modRmInSet = ((mr >= 0xe1) && (mr <= 0xe7));
          result = mRegPtr || (!modRmInSet);
          break;
        case 0xfe:
          result = mMultiBits <= 1;
          break;
        case 0xff:
          result = (mMultiBits < 7) && (mRegPtr || (!((mMultiBits == 3) || (mMultiBits == 5))));
          break;
      }
    }
  }
  return result;
}

void CCodeParse::CheckTarget(REG_STATE *pRegState, CMemoryMap *pMemoryMap)
{
  // is this a jmp or call instruction?
  if ((mOpCodeFlags & fJmpRel) != 0)
  {
    // this is a relative jmp or call instruction, so we know the target
    mIsRelTarget = TRUE;
    mpTarget = mpCode;
    mTarget = (LPVOID) ((LPBYTE) mpCode + mImmediateDataLength + (int) mImmediateValue);
    mTargetSize = mImmediateDataLength;
    mIsEnlargeable = !((mOpCode >= 0xe0) && (mOpCode <= 0xe3));
    if (mOpCode == 0xe8)
      mIsCall = TRUE;
    else
      mIsJmp = TRUE;
  }
  else
  {
    // We do not track pusha or popa
    //  pusha would by highly unusual for pushing function parameters.
    if ( ((mOpCode >= 0x58) && (mOpCode <= 0x5f)) ||  // pop into register, 0-7 from 8
         (mOpCode == 0x1f) ||                         // pop DS
         (mOpCode == 0x07) ||                         // pop ES
         (mOpCode == 0x17) ||                         // pop SS
         (mOpCode == 0x0fa1) ||                       // pop FS
         (mOpCode == 0x0fa9) )                        // pop GS
    {
      // Popping into a register
      mIsPop = TRUE;
    }
    else if ((mOpCode == 0x8f) && (mMultiBits == 0))
    {
      // Pop w/ModRM determining where to, but we don't care where we pop
      mIsPop = TRUE;
    }
    else if ( (mOpCode == 0x6a) ||                         // push 8bit
              (mOpCode == 0x68) ||                         // push 16/32bit
              ((mOpCode >= 0x50) && (mOpCode <= 0x57)) ||  // push register, 0-7
              (mOpCode == 0x0e) ||                         // push CS
              (mOpCode == 0x06) ||                         // push ES
              (mOpCode == 0x1e) ||                         // push DS
              (mOpCode == 0x16) ||                         // push SS
              (mOpCode == 0x0fa0) ||                       // push FS
              (mOpCode == 0x0fa8) )                        // push GS
    {
      // push a literal or register, not a paramater we track
      mIsPush = TRUE;
      mPushParameter = -1;
    }
    else if ((mOpCode == 0xff) && (mMultiBits == 6))
    {
      // For mPushParameter, we assume the standard stack frame
      // callee pushes parameters on stack            0x08...0xXX
      // call pushes return address                   0x04
      // prolog pushes ebp, and moves esp into ebp    0x00
      mIsPush = TRUE;
      mPushParameter = -1;
      // We are interested in tracking parameters like: [ebp + disp8]
      // ModR/M of 0x75 = 
      //  01         Mod = 1
      //    110      Register bits = 6, extends the OpCode to a Push
      //       101   R/M = 5, ModR/M of 01 and 101 determines Register and disp, EBP+disp8 (mod of 10 is disp32)
      if (((mModRm & 0xC0) == 0x40) && (mRegBits == 0x05))
      {
        // Per above, subtract 8 (ebp and return address) and divide by 4 to convert disp into parameter position
        if (mDisplacementDword >= 0x08)
          mPushParameter = ((DWORD) mDisplacementDword - 0x08) / 4;
      }
    }
    else if ((mOpCode == 0xff) && ((mMultiBits >= 2) && (mMultiBits <= 5)))
    {
      // jmp or call, target known or unknown, depending on the modrm byte
      if ((mMultiBits == 2) || (mMultiBits == 3))
        mIsCall = TRUE;
      else
        mIsJmp = TRUE;

      if ((mRegBits == -1) && (mSibScaleFactor == 0) && (mDisplacementSize == 4))
      {
        // just a plain jmp/call [$xxxxxxxx], so we know the target
        mppTarget = (LPVOID*) mpDisplacement;
        mIsValid = (mPrefixSegment) || (CMemoryMap::TryRead((LPVOID) mDisplacementDword, &mTarget, sizeof(mTarget), pMemoryMap));
//        mpTarget = (LPVOID) mDisplacementDword;  // doesn't match mTargetSize in 64bit
        mTargetSize = sizeof(mTarget);
        mIsEnlargeable = TRUE;
      }
      else
      {
        if ( (pRegState != NULL) && (mRegBits != -1) && ((*pRegState)[mRegBits] != NULL) &&
             (mSibScaleFactor == 0) && (mDisplacementSize == 0) )
        {
          // "jmp/call exx" or "jmp/call [exx]" with known register value
          if (mRegPtr)
          {
            mppTarget = (LPVOID *) ((*pRegState)[mRegBits]);
            mIsValid = (mPrefixSegment) || (CMemoryMap::TryRead(*mppTarget, &mTarget, sizeof(mTarget), pMemoryMap));
          }
          else
          {
            mpTarget = (*pRegState)[mRegBits];
            mTarget = *((PVOID*) mpTarget);
          }
          mTargetSize = sizeof(mTarget);
        }
      }
    }
    #ifndef _WIN64
      else
      {
        if (mOpCode == 0x9a)
          mIsCall = TRUE;  // unknown call
        else if (mOpCode == 0xea)
          mIsJmp = TRUE;  // unknown jmp
      }
    #endif
  }
}

// Keep track on which value the registers have
void CCodeParse::CheckRegState(REG_STATE *pRegState)
{
  DWORD clr = mOpCodeFlags & fClr;
  // we clear all the register states that the flags tell us
  if ((clr == fClrA) || (clr == fClrMA) || (clr == fClrOA))
    (*pRegState)[0] = NULL;
  if ((!mRegPtr) && ((clr == fClrM) || (clr == fClrRM) || (clr == fClrMA)))
    (*pRegState)[mRegBits] = NULL;
  if ((clr == fClrR) || (clr == fClrRM))
    (*pRegState)[mMultiBits] = NULL;
  if ((clr == fClrO) || (clr == fClrOA))
    (*pRegState)[mOpCode & 0x07] = NULL;

   // a lot of special cases need to be handled manually
  if (mOpCode > 0xff)
  {
    // 2 byte cases
    switch ((BYTE) (mOpCode & 0xff))
    {
      case 0x00:
        if ((mMultiBits <= 1) && (!mRegPtr))
          (*pRegState)[mRegBits] = NULL;
        break;
      case 0x01:
        if ((mMultiBits == 4) && (!mRegPtr))
          (*pRegState)[mRegBits] = NULL;
        break;
      case 0x2c:
      case 0x2d:
        if (mHasPrefixREPN || mHasPrefixREP)
          (*pRegState)[mMultiBits] = NULL;
        break;
      case 0x31:
      case 0x32:
      case 0x33:
      case 0xc7:
        (*pRegState)[2] = NULL;
        break;
      case 0x7e:
      case 0x7f:
        if ((!mHasPrefixREP) && (!mRegPtr))
          (*pRegState)[mRegBits] = NULL;
        break;
      case 0xa2:
        for (int i = 0; i < 4; i++)
          (*pRegState)[i] = NULL;
        break;
      case 0xba:
        if ((mMultiBits >= 5) && (!mRegPtr))
          (*pRegState)[mRegBits] = NULL;
        break;
      default:
        // Valid
        break;
    }
  }
  else
  {
    switch (mOpCode)
    {
      #ifndef _WIN64
        case 0x61:
          for (int i = 0; i < 8; i++)
            (*pRegState)[i] = NULL;
          break;
      #endif
      case 0x6c: case 0x6d: case 0xaa: case 0xab: case 0xae: case 0xaf:
        (*pRegState)[7] = NULL;
        break;
      case 0x6e: case 0x6f: case 0xac: case 0xad:
        (*pRegState)[6] = NULL;
        break;
      case 0x80: case 0x81: case 0x82: case 0x83:
        if ((!mRegPtr) && (mMultiBits != 7))
          (*pRegState)[mRegBits] = NULL;
        break;
      case 0x8b:
        if (!mRegPtr)
          (*pRegState)[mMultiBits] = (*pRegState)[mRegBits];
        break;
      case 0x99:
        (*pRegState)[2] = NULL;
        break;
      case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        (*pRegState)[6] = NULL;
        (*pRegState)[7] = NULL;
        break;
      case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
        #ifdef _WIN64
          if ((!mHasPrefixOPSIZ) && (mImmediateDataLength == 8))
        #else
          if (!mHasPrefixOPSIZ)
        #endif
          (*pRegState)[ mOpCode & 0x07] = mpCode;
        break;
      case 0xc8: case 0xc9:
        (*pRegState)[5] = NULL;
        break;
      #ifndef _WIN64
        case 0xc7:
          if ((!mRegPtr) && (!mHasPrefixOPSIZ))
            (*pRegState)[mRegBits] = mpCode;
          break;
      #endif
      case 0xdf:
        if (mModRm == 0xe0)
          (*pRegState)[0] = NULL;
        break;
      case 0xe0: case 0xe1: case 0xe2:
        (*pRegState)[1] = NULL;
        break;
      case 0xf7:
        if (mMultiBits >= 4)
          (*pRegState)[2] = NULL;
        break;
      case 0xff:
        if ((mMultiBits <= 1) && (!mRegPtr))
          (*pRegState)[mRegBits] = NULL;
        break;
      default:
        // Valid
        break;
    }
  }
}

void CCodeParse::RegStr(SString& rs, int len, int reg)
{
  if ((mOpCode < 0xd8) || (mOpCode > 0xdf))
  {
    // this is a non floating point register
    if (mHasPrefixOPSIZ && (((mOpCodeFlags & f66) == f66R) || ((mOpCodeFlags & f66) == f66RM)))
    {
      if (((mOpCodeFlags & fMod) != 0) && ((mOpCodeFlags & fReg) > fReg32 || (mOpCodeFlags & fMod) > fMod32))
        len = 16;   // 66 prefix set for sse2 instructions means -> oword (16 bytes)
      else
        len = 2;    // 66 prefix set for other instructions means -> word (2 bytes)
    }
    #ifdef _WIN64
      else 
        if ((len == 4) && (((mPrefixREX & 0x8) != 0) || ((mOpCode >= 0x50) && (mOpCode <= 0x5f))))
          len = 5;
    #endif

    switch (len)
    {
      case 1: case 2: case 4:
      #ifdef _WIN64
        case 5: case 6:
      #endif
        rs = CRegisterLabels[len][reg];
        break;
      case 8:
        rs.Format(L"mm%d", reg);
        break;
      case 16:
        rs.Format(L"xmm%d", reg);
        break;
    }
  }
  else
  {
    St(rs, reg);
  }
}

// return the "reg" floating point register as a string
void CCodeParse::St(SString& st, int reg)
{
  if (reg != 0)
    st.Format(L"st(%d)", reg);
  else
    st.Format(L"st");
}

// return the immediate data as a string in integer form
void CCodeParse::IxToInt(SString& ims)
{
  // first of all lets print out the relative immediate value
  if ((mImmediateValue > -10) && (mImmediateValue < 10))
  {
    if (mImmediateValue < 0)
      ims.Format(L"-%d", 0 - mImmediateValue);
    else if ((mOpCodeFlags & fJmpRel) != 0)
      ims.Format(L"+%d", mImmediateValue);
    else
      ims.Format(L"%d", mImmediateValue);
  }
  else
  {
    if (mImmediateValue < 0)
      ims.Format(L"-%0Xh", 0 - mImmediateValue);
    else if ((mOpCodeFlags & fJmpRel) != 0)
      ims.Format(L"+%0Xh", mImmediateValue);
    else
      ims.Format(L"%0Xh", mImmediateValue);
  }
  if (mIsRelTarget)  // this is a relative call or jmp, let's add the absolute target
  {
    SString t;
    t.Format(L"%s (%0Xh)", ims.GetBuffer(), mTarget);
    ims = t;
  }
}

// return the immediate data as a string in dword form
void CCodeParse::IxToDw(SString& ims, LPVOID pCode)
{
  DWORD ic;
  ims.NullString();
  // 3dnow instructions' ($0f0f) immediate data is hidden
  // same with aam/aad, if the immediate data is the default value of $0a
  if ( (mOpCode != 0x0f0f) &&
       (((mOpCode != 0xd4) && (mOpCode != 0xd5)) || (*((LPBYTE) pCode - 1) != 0x0a)) )
  {
    // in all other cases we show the immediate data
    DWORD ic2 = 0;
    switch (mImmediateDataLength)
    {
      case 1:
        if (mOpCode == 0x6a)
        {
          // byte immediate data sign extended
          ic = (DWORD) (*((char*) pCode));
          if (mHasPrefixOPSIZ)
            ic = (WORD) ic;
        }
        else
        {
          // byte immediate data unsigned
          ic = *(LPBYTE(pCode));
        }
        break;
      case 2:   // word immediate data unsigned
        ic = *((WORD *) pCode);
        break;
      case 3: // "word, byte" immediate data unsigned
        ic = *((WORD *) pCode);
        ic2 =  *((LPBYTE) pCode + 2);
        break;
      case 4:   // "word:word" immediate data unsigned
        if ((mOpCode == 0x9a) || (mOpCode == 0xea))
        {
          ic = *((WORD *) pCode);
          ic2 = *((WORD *) ((ULONG_PTR) pCode + 2));
        }
        else // dword immediate data unsigned
          ic = *((DWORD *) pCode);
        break;
      default:
        ic = *((DWORD *) pCode);
        ic2 =  *((WORD *) ((ULONG_PTR) pCode + 4));
        break;
    }
    #ifdef _WIN64
      if ((ic2 == 0) && (mImmediateDataLength != 8))
    #else
      if (ic2 == 0)
    #endif
    {
      // we have the usual immediate form, just one value
      if (ic < 10)
        ims.Format(L"%d", ic);
      else
        ims.Format(L"%0Xh", ic);
    }
    else
    {
      if (mImmediateDataLength == 3)
        ims.Format(L"%0Xh, %0Xh", ic, ic2);
      else
        #ifdef _WIN64
          if (mImmediateDataLength == 8)
          {
            ULONGLONG immData = *((ULONGLONG *) pCode);
            DWORD immLow = (DWORD) immData;
            DWORD immHigh = (DWORD) (immData >> 32);
            if (immHigh != 0)
            {
              ims.Format(L"%X%08Xh", immHigh, immLow);
            }
            else
              ims.Format(L"%Xh", immLow);
          }
          else
        #endif
          ims.Format(L"%0Xh:%0Xh", ic2, ic);
    }
    if (((mOpCodeFlags & fPtr) != 0) && ((mOpCodeFlags & fMod) == 0))
    {
      SString t;
      t.Format(L"[%s]", ims.GetBuffer());
      ims = t;
      if (mPrefixSegment != 0)
      {
        t.NullString();
        t.Format(L"%s:%s", CRegisterLabels[3][mPrefixSegment - 1], ims.GetBuffer());
        ims = t;
      }
    }
  }
  else
    ims.NullString();
}

// how big is the register/memory, the modrm byte refers to?
int CCodeParse::CalculateMSize(void)
{
  int result;
  // If the prefix is OPSIZ and changes size of ModRm || Reg and ModRm
  if (mHasPrefixOPSIZ && (((mOpCodeFlags & f66) == f66M) || ((mOpCodeFlags & f66) == f66RM)))
  {
    if ( ((mOpCodeFlags & fMod) != 0) && 
         (((mOpCodeFlags & fReg) > fReg32) || ((mOpCodeFlags & fMod) > fMod32)) )
    {
      // 66 prefix set for sse2 instructions means -> oword (16 bytes)
      result = 16;
    }
    else
    {
      // 66 prefix set for other instructions means -> word (2 bytes)
      result = 2;
    }
  }
  else
  {
    // 66 prefix is not set, or doesn't have any effect this time
    switch (mOpCodeFlags & fMod)
    {
      case fMod8:
        result = 1;
        break;
      case fMod16:
        result = 2;
        break;
      case fMod32:
        #ifdef _WIN64
          if ( (((mPrefixREX & 0x8) != 0) && (mOpCode != 0x63)) ||
               (mOpCode == 0x8f) || 
               ((mOpCode == 0xff) && (mMultiBits == 2 || mMultiBits == 4 || mMultiBits == 6)) ||
               ((mOpCode >= 0x0f20) && (mOpCode <= 0x0f23)) )
            result = 5;
          else
        #endif
          result = 4;
        break;
      case fMod64:
        result = 8;
        break;
      case fMod80:
        result = 10;
        break;
      default:
        result = 16;
        break;
    }
  }
  return result;
}

// compose the modrm byte (plus sib byte, if available) into a string
void CCodeParse::DisAsmModRm(SString& modRmString)
{
  modRmString.NullString();
  if (mRegPtr)
  {
    int regSize;
    // this modrm byte references memory
    // now let's begin to set up the modrm result string
    // a modrm memory location is always printed in "[]" brackets
    modRmString = L"[";

    #ifdef _WIN64
      if (mHasPrefixADRSIZ)
        regSize = 4;
      else
        regSize = 5;
    #else
      if (mHasPrefixADRSIZ)
        regSize = 2;
      else
        regSize = 4;
    #endif

    if (mRegBits != -1)
      modRmString += CRegisterLabels[regSize][mRegBits];

    if (mSibScaleFactor != 0)
    {
      modRmString += L"+";
      modRmString += CRegisterLabels[regSize][mSibScaleRegister];
      if (mSibScaleFactor > 1)
      {
        SString scaleStr;
        scaleStr.Format(L"*%d", mSibScaleFactor);
        modRmString += scaleStr;
      }
    }
    if (modRmString.Length() > 1)
    {
      // we do have address registers
      // so print the displacement data (if available) in integer form
      if (mDisplacementInt != 0)
      {
        if (mDisplacementInt > 0)
        {
          if (mDisplacementInt >= 10)
          {
            SString dispStr;
            dispStr.Format(L"+%0Xh", mDisplacementInt);
            modRmString += dispStr;
          }
          else
          {
            SString dispStr;
            dispStr.Format(L"+%d", mDisplacementInt);
            modRmString += dispStr;
          }
        }
        else
        {
          if (mDisplacementInt <= -10)
          {
            SString dispStr;
            dispStr.Format(L"-%0Xh", 0 - mDisplacementInt);
            modRmString += dispStr;
          }
          else
          {
            SString dispStr;
            dispStr.Format(L"-%d", 0 - mDisplacementInt);
            modRmString += dispStr;
          }
        }
      }
    }
    else
    {
      // we don't have any address registers, just our displacement data
      // so print the displacement data in dword form
      // if no displacement data is available (strange), print out "0"
      SString dispDwordStr;
      if (mDisplacementDword < 10)
        dispDwordStr.Format(L"%d", mDisplacementDword);
      else
        #ifdef _WIN64
          if (mDisplacementDword > 0xffffffff)
            dispDwordStr.Format(L"%X%08Xh", (DWORD) (mDisplacementDword >> 32), (DWORD) mDisplacementDword);
          else
        #endif
          dispDwordStr.Format(L"%0Xh", mDisplacementDword);

       modRmString += dispDwordStr;
    }
    modRmString += L"]";

    modRmString.Replace(L"[+", L"[");

    if (mPrefixSegment != 0)
    {
      SString t;
      t.Format(L"%s:%s", CRegisterLabels[3][mPrefixSegment - 1], modRmString.GetBuffer());
      modRmString = t;
    }
    if ((mOpCodeFlags & fPtr) != 0)
    {
      SString t;
      switch (CalculateMSize())
      {
        case 1:
          t.Format(L"byte ptr %s", modRmString.GetBuffer());
          break;
        case 2:
          t.Format(L"word ptr %s", modRmString.GetBuffer());
          break;
        case 4:
          t.Format(L"dword ptr %s", modRmString.GetBuffer());
          break;
        #ifdef _WIN64
          case 5:
            t.Format(L"qword ptr %s", modRmString.GetBuffer());
            break;
        #endif
        case 8:
          t.Format(L"qword ptr %s", modRmString.GetBuffer());
          break;
        case 10:
          t.Format(L"tbyte ptr %s", modRmString.GetBuffer());
          break;
        case 16:
          t.Format(L"oword ptr %s", modRmString.GetBuffer());
          break;
        default:
          t.Format(L"BAD ptr %s", modRmString.GetBuffer());
          break;
      }
      modRmString = t;
    }
  }
  else
  {
    // the modrm byte doesn't refer to memory
    // it addresses a pure register, so print out the register
    if ((mOpCode < 0xd8) || (mOpCode > 0xdf))
    {
      // here we have the usual case, namely non floating point registers
      int mdRmSize = CalculateMSize();
      switch (mdRmSize)
      {
        case 1:
        case 2:
        case 3:
        case 4:
        #ifdef _WIN64
          case 5:
        #endif
          modRmString = CRegisterLabels[mdRmSize][mRegBits];
          break;
        case 8:
          modRmString.Format(L"mm%d", mRegBits);
          break;
        case 16:
          modRmString.Format(L"xmm%d", mRegBits);
          break;
        default:
          modRmString = L"BAD";
          break;
      }
    }
    else
    {
      St(modRmString, mRegBits);
    }
  }
}

void CCodeParse::GetLabel(LPVOID pCode)
{
  SString result;

  result = L"";
  if (mLabelIndex > 0)
  {
    SString s1 = COpCodeLabels[mLabelIndex];

    s1.Replace(L"%cc", CConditionalLabels[mOpCode & 0x0F]);
    s1.Replace(L"%seg", CRegisterLabels[3][(mOpCode >> 3) & 0x07]);

    LPWSTR temp = NULL;
    if (mHasPrefixOPSIZ)
      temp = s1.SubStr(1, L'/');
    else if (mHasPrefixREPN)
      temp = s1.SubStr(2, L'/');
    else if (mHasPrefixREP)
      temp = s1.SubStr(3, L'/');

    // if still empty
    if (temp == NULL)
      temp = s1.SubStr(0, L'/');

    if (temp != NULL)
    {
      result = temp;
      SString::DeallocateBuffer(temp);
    }
    // pick the sub string for the modrm multi purpose value (if available)
    int count = s1.SubStrCount(L'|');
    if (count > 1)
    {
      if ((count > 8) && (!mRegPtr))
      {
        result = result.SubStr(8 + mMultiBits, L'|');
        if (result.PosStr(L"-") > 0)
          result = result.SubStr(mRegBits, L'-');
      }
      else
      {
        result = result.SubStr(mMultiBits);
      }
    }
    // does the label depend on whether the modrm byte addresses memory?
    if (result.PosStr(L":") > 0)
    {
      if (mRegPtr)
        result = result.SubStr(1, L':');
      else
        result = result.SubStr(0, L':');
    }
    // a lot of sse2 labels need to be adjusted according to the prefixes
    if (mHasPrefixOPSIZ || mHasPrefixREPN)
      result.Replace(L"S", L"d");
    else
      result.Replace(L"S", L"s");
    if (mHasPrefixREP || mHasPrefixREPN)
      result.Replace(L"P", L"s");
    else
      result.Replace(L"P", L"p");

    // add the leading "f" for floating point instructions
    if ((mOpCode >= 0xd8) && (mOpCode <= 0xdf))
    {
      SString temp;
      temp.Format(L"f%s", result.GetBuffer());
      result = temp;
    }
  }
  else
  {
    if (this->mOpCode == 0x0f0f)
    {
      // This is 3dnow instruction, let's search for label
      for (int i = 0; i < NowLabelArrayLength; i++)
      {
        if (CNowLabels[i].Code == *(BYTE *) (((ULONG_PTR) pCode) - 1))
        {
          result = CNowLabels[i].Label;
          break;
        }
      }
    }
  }
  mDisAsm += result;
}

BOOL CCodeParse::CheckFunctionName(LPVOID pCode)
{
  BOOL result = FALSE;
  SString s;

  HMODULE hModule;
  SString moduleName;
  char arrCh[MAX_PATH + 1];
  moduleName.NullString();
  // to which module does the target belong?
  s.NullString();
  if (FindModule(pCode, &hModule, arrCh, MAX_PATH))
  {
    moduleName = arrCh;
    // try to find the target name
    char procName[MAX_PATH];
    if (GetImageProcName(hModule, pCode, procName, MAX_PATH))
    {
      s = procName;
    }

    if (s.IsEmpty())
    {
      // no name found, maybe this is a static linking?
      IMAGE_NT_HEADERS *pNtHeaders = GetImageNtHeaders(hModule);
      if ((pNtHeaders != NULL) && (pCode >= hModule) &&
         ((ULONG_PTR) pCode <= ((ULONG_PTR) hModule + GetSizeOfImage(pNtHeaders))))
      {
        // target is inside of code area
        CCodeParse c(pCode, NULL, NULL);
        if ((c.mTarget != NULL) && FindModule(c.mTarget, &hModule, arrCh, MAX_PATH))
        {
          moduleName = arrCh;
          // and the target is itself a jmp or call again
          // let's see whether we can find the name of the target's target
          if (GetImageProcName(hModule, c.mTarget, procName, MAX_PATH))
            s = procName;
        }
      }
    }
    if (!s.IsEmpty())
    {
      if ((hModule != gHModule) && (s.PosStr(L"(") == -1))
      {
        for (int i = (int) moduleName.Length() - 2; i > 0; i--)
        {
          if (moduleName[i] == L'\\')
          {
            SString temp(moduleName);
            moduleName = &temp[i + 1];
            break;
          }
        }
        s += L" (";
        s += moduleName;
        s += L")";
      }
      // finally let's add the function name to the output string
      for (int i = (int) mDisAsm.Length(); i < 32 + sizeof(LPVOID) * 2; i++)
        mDisAsm += L" ";
      mDisAsm += L"  ; ";
      mDisAsm += s;
      result = TRUE;
    }
  }
  return result;
}

void CCodeParse::CheckStringData(ULONG_PTR data, BOOL isPointer)
{
  HMODULE hModule;
  char moduleName[MAX_PATH];

  if (FindModule(mpCode, &hModule, moduleName, MAX_PATH))
  {
    IMAGE_NT_HEADERS *pNtHeaders = GetImageNtHeaders(hModule);
    if (pNtHeaders != NULL)
    {
      if ((data >= (ULONG_PTR) hModule) && (data <= (ULONG_PTR) hModule + GetSizeOfImage(pNtHeaders) - 100))
      {
        #ifdef _WIN64
          if ( isPointer && (*(ULONG_PTR*) data > (ULONG_PTR) hModule) &&
               (*(ULONG_PTR*) data <= (ULONG_PTR) hModule + GetSizeOfImage(pNtHeaders) - 100) )
            data = *(ULONG_PTR*) data;
          else
        #endif
          if ( isPointer && (*(DWORD*) data > (ULONG_PTR) hModule) &&
               (*(DWORD*) data <= (ULONG_PTR) hModule + GetSizeOfImage(pNtHeaders) - 100) )
            data = *(DWORD*) data;
        if (!CheckFunctionName((LPVOID) data))
        {
          char* pc = (char*) data;
          int i1 = 0;
          int i2 = 0;
          int i3 = 0;
          int i4 = 0;
          int i5 = 0;
          for (i1 = 1; i1 <= 100; i1++)
          {
            if ((*pc == (char) 0) || (*pc == (char) 10) || (*pc == (char) 12))
              break;
            if (((*pc >= 'A') && (*pc <= 'Z')) || ((*pc >= 'a') && (*pc <= 'z')) || ((*pc >= '0') && (*pc <= '9')) || (*pc == ' '))
              i2++;
            else
            {
              if ((*pc == ':') || (*pc == '\\') || (*pc == '.') || (*pc == ','))
                i3++;
              else
              {
                if ((*pc >= (char) 33) && (*pc <= (char) 93))
                  i4++;
                else
                {
                  if (*pc < (char) 32)
                    return;
                  i5++;
                }
              }
            }
            pc++;
          }
          if ( (i1 > 4) &&
               (i2 * 2 > i3 * 5) && (i2 + (i3 / 2) > i4 * 8) &&
               (i2 + (i3 / 2) + (i4 / 4) > i5 * 10) )
          {
            char* buf;
            buf = (char*) LocalAlloc(LPTR, i1);
            memmove(buf, (LPVOID) data, i1 - 1);

            for (int i = (int) mDisAsm.Length(); i < 32 + sizeof(LPVOID) * 2; i++)
              mDisAsm += L" ";

            mDisAsm += L"  ; \"";
            mDisAsm += buf;
            mDisAsm += L"\"";

            LocalFree((HLOCAL) buf);
          }
        }
      }
    }
  }
}
