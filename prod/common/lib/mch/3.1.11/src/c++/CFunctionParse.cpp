// ***************************************************************
//  CFunctionParse.cpp        version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  disassembling a whole function
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#define _CFUNCTIONPARSE_C

#include "SystemIncludes.h"
#include "Systems.h"
#include "SystemsInternal.h"

CFunctionParse::CFunctionParse(LPVOID pFunction) :
mDisAsm(), mCodeAreas(), mFarCalls(), mUnknownTargets()
{
  InitializeMembers(pFunction);
  RawParseFunction();
}

CFunctionParse::CFunctionParse(LPVOID pFunction, REG_STATE *pRegState, CMemoryMap *pMemoryMap) :
mDisAsm(), mCodeAreas(), mFarCalls(), mUnknownTargets()
{
  InitializeMembers(pFunction, pRegState, pMemoryMap);
  RawParseFunction();
}

void CFunctionParse::InitializeMembers(LPVOID pFunction, REG_STATE *pRegState, CMemoryMap *pMemoryMap)
{
  mDisAsm.Empty();
  mCodeAreas.RemoveAll();
  mFarCalls.RemoveAll();
  mUnknownTargets.RemoveAll();

  mpFunction = pFunction;
  if (pRegState == NULL)
    memset(mRegState, 0, sizeof(REG_STATE));
  else
    memcpy(mRegState, pRegState, sizeof(REG_STATE));
  mpMemoryMap = pMemoryMap;

  mIsValid = FALSE;
  mIsInterceptable = FALSE;

  mpCodeBegin = NULL;
  mCodeLength = 0;

  mCopy.BufferLength = 0;
  mCopy.IsValid = FALSE;
  mCopy.LastErrorAddress = NULL;
  mCopy.LastErrorNo = 0;
  mCopy.LastErrorString = NULL;

  mCurrentCodeArea = 0;
  mpModuleCodeBegin = NULL;
  mpModuleCodeEnd = NULL;
  mpModuleDataBegin = NULL;
  mpModuleDataEnd = NULL;

  mpLastErrorAddress = NULL;
  mLastErrorNo = 0;
  mLastErrorString = NULL;
}

CFunctionParse::~CFunctionParse()
{
}

BOOL CFunctionParse::IsInterceptable()
{
  return mIsInterceptable;
}

SString CFunctionParse::ToString(LPVOID exceptAddr, int maxLines, bool autoDelimiters)
{
  mDisAsm = L"<Invalid>";
  if (mIsValid)
  {
    mDisAsm.Empty();
    // Get name of function
    MEMORY_BASIC_INFORMATION mbi;
    SString mName;
    mName.NullString();

    if ( (VirtualQuery(mpFunction, &mbi, sizeof(mbi)) == sizeof(mbi)) &&
         (mbi.State == MEM_COMMIT) && (mbi.AllocationBase) )
    {
      char procName[MAX_PATH];
      if (GetImageProcName((HMODULE) mbi.AllocationBase, mpFunction, procName, MAX_PATH))
        mName = procName;
    }

    if (mName.IsEmpty())
      mName.Format(L"sub_%0X", mpFunction);
    else
    {
      SString temp(mName);
      mName.Format(L"public %s", temp.GetBuffer());
    }

    // Create array of code areas
    int *pCodeAreaIndex;
    pCodeAreaIndex = (int *) LocalAlloc(LPTR, mCodeAreas.GetCount() * sizeof(int));
    int i2 = -1;
    for (int i1 = 0; i1 < mCodeAreas.GetCount(); i1++)
    {
      // i2 is updated by FindCodeArea
      FindCodeArea(i2);
      if (i2 == -1)
      {
        // MessageBox(0, L"test", L"test", 0);
        mDisAsm = L"Internal error while composing the disassembling string";
        goto EXIT;
      }
      pCodeAreaIndex[i1] = i2;
    }

    mFirst = 0;
    mMaxLines = maxLines;
    mpExceptAddr = exceptAddr;
    // If MaxLines set, just count
    if (mMaxLines > 0)
      ToStringInternal(mName, pCodeAreaIndex, TRUE, autoDelimiters);

    ToStringInternal(mName, pCodeAreaIndex, FALSE, autoDelimiters);

EXIT:
    LocalFree(pCodeAreaIndex);
  }
  return mDisAsm;
}

void CFunctionParse::FindCodeArea(int& codeAreaIndex)
{
  ULONG_PTR min, cur;
  if (codeAreaIndex == -1)
    min = 0;
  else
    min = (ULONG_PTR) (mCodeAreas[codeAreaIndex].AreaEnd) + 1;

  cur = (ULONG_PTR) -1;
  codeAreaIndex = -1;

  for (int i = 0; i < mCodeAreas.GetCount(); i++)
  {
    if ( ((ULONG_PTR) mCodeAreas[i].AreaBegin >= min) &&
         ((ULONG_PTR) mCodeAreas[i].AreaBegin <  cur)    )
    {
      cur = (ULONG_PTR) mCodeAreas[i].AreaBegin;
      codeAreaIndex = i;
    }
  }
}

void CFunctionParse::AddLine(LPVOID pCode, SString& line, BOOL justCount, BOOL addCodePosition)
{
  if ((!justCount) && (mLines >= mFirst))
  {
    if (addCodePosition)
    {
      if (!line.IsEmpty())
      {
        SString temp(line);
        line.Format(L" %s", temp.GetBuffer());
      }
      SString temp(line);
      line.Format(L"%p%s", pCode, temp.GetBuffer());
    }
    line += L"\r\n";
    mDisAsm += line;
  }

  if (mMaxLines > 0)
  {
    mLines++;
    if (((ULONG_PTR) mpExceptAddr <= (ULONG_PTR) pCode) && bBefore)
    {
      if (mLines > mMaxLines * 12 / 10)
      {
        if (justCount)
        {
          mFirst = mLines - mMaxLines - 2;
          if (mFirst < 0)
            mFirst = 0;
        }
        else
        {
/*          mFirst = 0;
          int i2 = mLen;
          for (int i1 = 0; i1 < mMaxLines; i1++)
            i2 = mDisAsm.PosStr(L"\r\n", i2 - 3, 0);
          if (i2 >= 0)
          {
            SString temp(mDisAsm);
            mDisAsm.Format(L"[...]\r\n%s", temp.GetBuffer() + i2 + 2);
            mLen = mLen - i2 - 2 + 7;
          } */
        }
      }
      mLines = 0;
      bBefore = FALSE;
    }
  }
}

void CFunctionParse::ToStringInternal(const SString& functionName, int *pCodeAreaIndex, BOOL justCount, BOOL bAutoDelimiters)
{
  if (pCodeAreaIndex == NULL)
    return;

  mLines = 0;
  bBefore = TRUE;
  bool b1 = false;
  bool b2 = false;
  LPVOID lastCi = NULL;

  CCodeParse ci;
  ci.mIsValid = FALSE;

  for (int i1 = 0; i1 < mCodeAreas.GetCount(); i1++)
  {
    CODE_AREA ca = mCodeAreas[pCodeAreaIndex[i1]];

    if ((ca.AreaBegin == mpFunction) || (bAutoDelimiters && (ca.CalledFrom != NULL)))
    {
      if (b1)
      {
        SString emptyStr = L"";
        AddLine(lastCi, emptyStr, justCount, TRUE);
      }
      if ((!justCount) && (mLines >= mFirst))
      {
        SString temp;
        if (ca.AreaBegin == mpFunction)
        {
          temp.Format(L"%s:", ((SString*) &functionName)->GetBuffer());
          for (int i2 = (int) temp.Length(); i2 < 31; i2++)
          temp += L" ";
          temp += L"  ; function entry point";
        }
        else
          temp.Format(L"loc_%0X:", ca.AreaBegin);
        AddLine(ca.AreaBegin, temp, justCount, TRUE);
      }
      else
      {
        SString emptyStr = L"";
        AddLine(ca.AreaBegin, emptyStr, justCount, TRUE);
      }
    }

    if (ca.CaseBlock)
    {
/*      if autoDelimiters then begin
        if b1 then
          AddLine(lastCi);
        if b2 then begin
          AddLine(ci.This, '; ---------------------------------------------------------');
          AddLine(ci.This);
        end;
      end;
      for i2 := 0 to (dword(AreaEnd) + 1 - dword(AreaBegin)) div 4 - 1 do begin
        if (not justCount) and (lines >= first) then begin
          s1 := '  dd loc_' + CodeToHex(TPAPointer(AreaBegin)^[i2], false);
          if i2 = 0 then
            s1 := FillStr(s1, -31) + '  ; case jump table';
          AddLine(pointer(dword(AreaBegin) + dword(i2) * 4), s1);
        end else
          AddLine(pointer(dword(AreaBegin) + dword(i2) * 4), '');
        if (not before) and (justCount or (lines >= maxLines)) then
          break;
      end;
      if autoDelimiters and (before or (lines < maxLines)) then begin
        AddLine(pointer(dword(AreaEnd) + 1));
        AddLine(pointer(dword(AreaEnd) + 1), '; ---------------------------------------------------------');
        AddLine(pointer(dword(AreaEnd) + 1));
      end;
      b1 := false;
      b2 := false; */
    }
    else
    {
      if (ca.OnExceptBlock)
      {
/*        // this is a Delphi style "exception on E: Exception do ..." block
        i3 := 1;
        if not justCount then
          for i2 := 1 to TPCardinal(AreaBegin)^ do
            if TPAPointer(AreaBegin)^[i2 * 2 - 1] <> nil then begin
              try
                clss := TClass(TPAPointer(AreaBegin)^[i2 * 2 - 1]^);
                if Length(clss.ClassName) > i3 then
                  i3 := Length(clss.ClassName);
              except end;
            end;
        for i2 := 1 to TPCardinal(AreaBegin)^ do begin
          if (not justCount) and (lines >= first) then begin
            if TPAPointer(AreaBegin)^[i2 * 2 - 1] <> nil then begin
              try
                s1 := TClass(TPAPointer(AreaBegin)^[i2 * 2 - 1]^).ClassName;
              except
                s1 := 'EUnknown';
              end;
              s1 := '  on ' + FillStr(s1, -i3) + ' do';
            end else
              s1 := '  else' + FillStr('', i3 + 2);
            s1 := s1 + ' loc_' + CodeToHex(TPAPointer(AreaBegin)^[i2 * 2], false);
          end else
            s1 := '';
          AddLine(pointer(dword(AreaBegin) + dword(i2) * 8 - 4), s1);
          if (not before) and (justCount or (lines >= maxLines)) then
            break;
        end;
        if autoDelimiters and (before or (lines < maxLines)) then begin
          AddLine(pointer(dword(AreaEnd) + 1));
          AddLine(pointer(dword(AreaEnd) + 1), '; ---------------------------------------------------------');
          AddLine(pointer(dword(AreaEnd) + 1));
        end;
        b1 := false;
        b2 := false;   */
      }
      else
      {
        REG_STATE rs;
        ci.mpNext = ca.AreaBegin;
        memmove(rs, ca.Registers, sizeof(REG_STATE));
        do
        {
          b1 = TRUE;
          b2 = TRUE;
          if (ci.mIsValid)
            lastCi = ci.mpCodeAddress;
          else
            lastCi = ca.AreaBegin;
          if ((!justCount) && (mLines >= mFirst))
          {
            ci.Parse(ci.mpNext, &rs, NULL);
            SString temp = ci.ToString();
            if (ci.mIsRelTarget)
            {
              for (int i2 = 0; i2 < mCodeAreas.GetCount(); i2++)
              {
                if (ci.mTarget == mCodeAreas[i2].AreaBegin)
                {
                  int i3 = temp.PosStr(L" ", 3 + sizeof(LPVOID) * 2);
                  if (i3 >= 0)
                  {
                    SString temp2(temp);
                    temp2[i3] = (wchar_t) 0;
                    temp = temp2.GetBuffer();
                  }
                  for (i3 = (int) temp.Length(); i3 < 11 + sizeof(LPVOID) * 2; i3++)
                    temp += L" ";
                  wchar_t* pc = temp.GetBuffer() + temp.Length() - 1;
                  if (pc[0] != L" "[0])
                    temp += L" ";
                  if (ci.mTarget == mpFunction)
                    temp += functionName;
                  else
                  {
                    SString temp2(temp);
                    temp.Format(L"%sloc_%0X", temp2.GetBuffer(), ci.mTarget);
                  }
                  break;
                }
              }
            }
            AddLine(ci.mpCodeAddress, temp, justCount, FALSE);
          }
          else
          {
            ci.Parse(ci.mpNext, &rs, NULL);
            SString emptyStr = L"";
            AddLine(ci.mpCodeAddress, emptyStr, justCount, FALSE);
          }

          if ( bAutoDelimiters &&
               (ci.mIsJmp || ci.mIsCall || ((ci.mOpCode == 0xc2) || (ci.mOpCode == 0xc3) || (ci.mOpCode == 0xca) || (ci.mOpCode == 0xcb) || (ci.mOpCode == 0xcf))) &&
               (((ULONG_PTR) ci.mpNext <= (ULONG_PTR) ca.AreaEnd) || (i1 < mCodeAreas.GetCount() - 1)) )
          {
            SString emptyStr = L"";
            AddLine(ci.mpCodeAddress, emptyStr, justCount, TRUE);
            if (((!(ci.mIsJmp || ci.mIsCall)) || (ci.mIsJmp && ((ci.mOpCode == 0xe9) || (ci.mOpCode == 0xea) || (ci.mOpCode == 0xeb) || (ci.mOpCode == 0xff)))) )
            {
              SString delimiter = L"; ---------------------------------------------------------";
              SString emptyStr = L"";
              AddLine(ci.mpCodeAddress, delimiter, justCount, TRUE);
              AddLine(ci.mpCodeAddress, emptyStr,  justCount, TRUE);
              b2 = false;
            }
            b1 = false;
          }
        } while (((ULONG_PTR) ci.mpNext <= (ULONG_PTR) ca.AreaEnd) && (bBefore || (!justCount && (mLines < mMaxLines))));
      }
    }

    if ((!bBefore) && (mLines >= mMaxLines) && (((ULONG_PTR) ci.mpNext <= (ULONG_PTR) ca.AreaEnd) || (i1 < mCodeAreas.GetCount() - 1)))
    {
      if (!justCount)
      {
   /*        mDisAsm.SetBufferLen(mLen);
         mDisAsm += L"[...]";
         mLen += 7; */
      }
      break;
    }
  } // end for loop
}

void CFunctionParse::RawParseFunction(void)
{
  MEMORY_BASIC_INFORMATION mbi;

  mpMsvcrtThrowExceptionAddr = GetProcAddress(GetModuleHandleA("msvcrt.dll"), "_CxxThrowException");

  // Can we read this address?
  if ((VirtualQuery(mpFunction, &mbi, sizeof(mbi)) == sizeof(mbi)) && (mbi.State == MEM_COMMIT))
  {
    // AllocationBase is HMODULE of module that contains the function
    IMAGE_NT_HEADERS *pNtHeaders = GetImageNtHeaders((HMODULE) mbi.AllocationBase);
    if (pNtHeaders != NULL)
    {
      // IMAGE_DOS_HEADER
      // MS-DOS Stub Program
      // IMAGE_NT_HEADERS
      // - Signature "PE"
      // - IMAGE_FILE_HEADER
      // - IMAGE_OPTIONAL_HEADER
      // IMAG_SECTION_HEADER[0..n]
      IMAGE_SECTION_HEADER *pSectionHeader;  // Right after IMAGE_NT_HEADERS
      pSectionHeader = (IMAGE_SECTION_HEADER *) ((ULONG_PTR) pNtHeaders + sizeof(IMAGE_NT_HEADERS));
      if ((pSectionHeader->Characteristics & IMAGE_SCN_CNT_CODE) != 0)
      {
        mpModuleCodeBegin = (LPVOID) ((ULONG_PTR) mbi.AllocationBase + pSectionHeader->VirtualAddress);
        mpModuleCodeEnd   = (LPVOID) ((ULONG_PTR) mpModuleCodeBegin + pSectionHeader->Misc.VirtualSize - 1);
        pSectionHeader = (IMAGE_SECTION_HEADER *) ((ULONG_PTR) pSectionHeader + sizeof(IMAGE_SECTION_HEADER));
        if ((pSectionHeader->Characteristics & IMAGE_SCN_CNT_CODE) != 0)
          mpModuleCodeEnd = (LPVOID) ((ULONG_PTR) mbi.AllocationBase + pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize - 1);
      }
      else
      {
        mpModuleCodeBegin = (LPVOID) ((ULONG_PTR) mbi.AllocationBase + pNtHeaders->OptionalHeader.BaseOfCode);
        mpModuleCodeEnd   = (LPVOID) ((ULONG_PTR) mpModuleCodeBegin  + pNtHeaders->OptionalHeader.SizeOfCode);
      }
      #ifdef _WIN64
        mpModuleDataBegin = mpModuleCodeEnd;
        pSectionHeader = (IMAGE_SECTION_HEADER *) ((ULONG_PTR) pNtHeaders + sizeof(IMAGE_NT_HEADERS));
        for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
        {
          if ( ((pSectionHeader->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA  ) != 0) ||
               ((pSectionHeader->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) != 0)    )
          {
            mpModuleDataBegin = (LPVOID) ((ULONG_PTR) mbi.AllocationBase + pSectionHeader->VirtualAddress);
            break;
          }
        }
      #else
        mpModuleDataBegin = (LPVOID) ((ULONG_PTR) mbi.AllocationBase + pNtHeaders->OptionalHeader.BaseOfData);
      #endif
      mpModuleDataEnd = (LPVOID) ((ULONG_PTR) mpModuleDataBegin + pNtHeaders->OptionalHeader.SizeOfUninitializedData +
                                  pNtHeaders->OptionalHeader.SizeOfInitializedData - 1);
      if (mpModuleCodeBegin > mpModuleDataBegin)
        mpModuleCodeBegin = mpModuleDataBegin;
      if (mpModuleCodeEnd < mpModuleDataEnd)
        mpModuleCodeEnd = mpModuleDataEnd;
    }
    else
    {
      mpModuleCodeBegin = mbi.BaseAddress;
      mpModuleCodeEnd = (LPVOID) ((ULONG_PTR) mpModuleCodeBegin + mbi.RegionSize);
    }

    CCodeParse cp;
    mpEntryPoint = mpFunction;
    mIsInterceptable = FALSE;
    mCopy.IsValid = TRUE;
    ZeroMemory(&mRegState, sizeof(REG_STATE));
    AddCodeArea(mpEntryPoint, NULL);

    while (TRUE)
    {
      // Find the first code area with a NULL end... hasn't been processed yet
      mCurrentCodeArea = -1;
      for (int i = 0; i < mCodeAreas.GetCount(); i++)
      {
        if (mCodeAreas[i].AreaEnd == NULL)
        {
          mCurrentCodeArea = i;
          break;
        }
      }

      // All current code areas have been processed, search UnknownTargets
      BOOL b1 = FALSE;
      if (mCurrentCodeArea == -1)
      {
        for (int i = mUnknownTargets.GetCount() - 1; i > -1; i--)
        {
          b1 = FALSE;
          for (int j = 0; j < mCodeAreas.GetCount(); j++)
          {
            if ( (mUnknownTargets[i].CodeAddress1 >= mCodeAreas[j].AreaBegin) &&
                 (mUnknownTargets[i].CodeAddress2 <= mCodeAreas[j].AreaEnd  )    )
            {
              cp.Parse(mUnknownTargets[i].CodeAddress1, &mCodeAreas[j].Registers, NULL);
              if (cp.mTarget != NULL)
              {
                CheckAddTarget(cp);
                if (mCodeAreas[mCodeAreas.GetCount() - 1].AreaEnd == NULL)
                {
                  mCurrentCodeArea = mCodeAreas.GetCount() - 1;
                  b1 = TRUE;
                }
              }
              break;
            }
          }
          if (b1)
            break;
        }
      }

      if (mCurrentCodeArea == -1)
      {
        b1 = FALSE;
        for (int i = 0; i < mUnknownTargets.GetCount(); i++)
        {
          if (!mUnknownTargets[i].Call)
          {
            b1 = TRUE;
            break;
          }
        }
        if (b1)
        {
          ULONG_PTR codeEnd;
          CalcCodeBegin(&codeEnd);
          FindCodeArea();
          if (((ULONG_PTR) mpCodeBegin - 1 != codeEnd) && (codeEnd - (ULONG_PTR) mpCodeBegin < 0x400))
          {
            // some bytes in the middle of our function are not disassembled yet
            // plus there are jump instructions to unknown targets in the code
            // so we guess that these are not yet disassembled parts of the code too
            mCurrentCodeArea = mCodeAreas.GetCount();
            memset(mRegState, 0, sizeof(REG_STATE));
            AddCodeArea(mpCodeBegin, NULL);

            ParseCodeArea(cp);
            if (!cp.mIsValid)
            {
              // oooops, we guessed wrong, let's pretend we didn't even try...
              mCodeAreas.RemoveAt(mCodeAreas.GetCount() - 1); // Count = mCurrentCodeArea;
              cp.mIsValid = TRUE;
              break;
            }
          }
          else
            break;
        }
        else
          break;
      }
      else
      {
        ParseCodeArea(cp);
        if (!cp.mIsValid)
          break;
      }
    } // end while (TRUE)

    if (!cp.mIsValid)
    {
      mIsValid = FALSE;
      mpLastErrorAddress = cp.mpNext;
      // mLastErrorNo =  invalid code
      // mLastErrorStr = invalid code
    }
    else
    {
      mIsValid = TRUE;
      ULONG_PTR codeEnd;
      CalcCodeBegin(&codeEnd);
      mCopy.BufferLength+= mCodeLength + sizeof(LPVOID);
      // We start at 0
      int currentIndex = 0;
      int functionLength = 0;
      BOOL loop;
      do
      {
        loop = FALSE;
        // If we find a case block, exception block, or called from this function, go ahead and break
        if ( mCodeAreas[currentIndex].CaseBlock || mCodeAreas[currentIndex].OnExceptBlock || 
             (mCodeAreas[currentIndex].CalledFrom != NULL) )
          break;
        // Increment length by value of current code area
        functionLength += (DWORD) ((ULONG_PTR) mCodeAreas[currentIndex].AreaEnd - (ULONG_PTR) mCodeAreas[currentIndex].AreaBegin + 1);
        // The we loop through code areas looking for next area
        for (int i = 1; i < mCodeAreas.GetCount(); i++)
        {
          // if i is directly behind sortedIndex directly, then i is next in list
          if ((ULONG_PTR) mCodeAreas[i].AreaBegin == ((ULONG_PTR) mCodeAreas[currentIndex].AreaEnd + 1))
          {
            // Update current index
            currentIndex = i;
            // We found another one, keep looping
            loop = TRUE;
            break;
          }
          // if we fall through the loop, then we are done
        }
      } while (loop);

      if (functionLength >= 6)
      {
        mIsInterceptable = TRUE;
        // Start at first instruction
        LPVOID codeAddress = mpEntryPoint;
        // loop through instructions for first 6 bytes, not much

        do
        {
          CCodeParse codeParse(codeAddress);
          codeAddress = codeParse.mpNext;

          if (codeParse.mIsCall || codeParse.mIsJmp)
          {
            // If the target is bad...
            if ( ((codeParse.mpTarget != NULL) || (codeParse.mppTarget != NULL)) &&
                 // ... or the target size is too small to be moved and instruction won't allow it to be enlarged
                 (!codeParse.mIsEnlargeable) && (codeParse.mTargetSize < sizeof(LPVOID)) )
            {
              // we can't intercept
              mIsInterceptable = FALSE;
              break;
            }
          }
        } while ((ULONG_PTR) codeAddress - (ULONG_PTR) mpEntryPoint < 6);
      }
    }
  }
  else
  {
    mIsValid = FALSE;
    mpLastErrorAddress = this->mpFunction;
    // error no
      // string
  }
}

void CFunctionParse::AddCodeArea(LPVOID newAreaBegin, LPVOID newCaller)
{
  LPVOID newAreaEnd = NULL;
  BOOL doAdd = TRUE;

  for (int i = 0; i < this->mCodeAreas.GetCount(); i++)
  {
    if (mCodeAreas[i].AreaBegin == newAreaBegin)
    {
      // Update the matching code area

      // if newCaller is greater than current called from but less than AreaBegin, update CalledFrom
      if ( (mCodeAreas[i].CalledFrom == NULL) ||
           ( (newCaller > mCodeAreas[i].CalledFrom) &&
             (newCaller < mCodeAreas[i].AreaBegin )    ) )
        mCodeAreas[i].CalledFrom = newCaller;

      // if current REG_STATE doesn't match code area REG_STATE, set to NULL
      for (int j = 0; j < sizeof(LPVOID) * 2; j++)
      {
        if (mRegState[j] != mCodeAreas[i].Registers[j])
          mCodeAreas[i].Registers[j] = NULL;
      }
      // Just an update
      doAdd = FALSE;
      break;
    }
    else // Area begin does not match
    {
      if ( (newAreaBegin >  mCodeAreas[i].AreaBegin) &&
           (newAreaBegin <= mCodeAreas[i].AreaEnd  )    )
      {
        // This is in middle of current area, so update this for insert
        newAreaEnd = mCodeAreas[i].AreaEnd;  // madshi: corrected
        mCodeAreas[i].AreaEnd = (LPVOID) ((ULONG_PTR) newAreaBegin - 1);
        for (int j = 0; j < sizeof(LPVOID) * 2; j++)
        {
          if (mRegState[j] != mCodeAreas[i].Registers[j])
            mRegState[j] = NULL;
        }
        if (i == this->mCurrentCodeArea)
          mCurrentCodeArea = mCodeAreas.GetCount();
        break;
      }
      // else keep looping
    }
  }
  // If we are adding this item, do it, but newAreaEnd could be NULL?
  if (doAdd)
  {
    this->AddCodeAreaItem(newAreaBegin, newAreaEnd, FALSE, FALSE, newCaller, &mRegState);
  }
}

void CFunctionParse::AddSpecialBlock(LPVOID newAreaBegin, DWORD areaLength, BOOL isCaseBlock, BOOL isOnExceptBlock)
{
  REG_STATE registers;
  LPVOID newAreaEnd = (LPVOID) ((ULONG_PTR) newAreaBegin + areaLength - 1);
  ZeroMemory(&registers, sizeof(REG_STATE));
  AddCodeAreaItem(newAreaBegin, newAreaEnd, isCaseBlock, isOnExceptBlock, NULL, &registers);
}

void CFunctionParse::CheckAddTarget(CCodeParse& cp)
{
  if (cp.mIsCall || cp.mIsJmp)
  {
    if ((cp.mpTarget != NULL) || (cp.mppTarget != NULL))
    {
      // Delphi & Borland specific exception material skipped
      if ( (cp.mTargetSize >= 4) &&
           (cp.mIsCall || (cp.mTarget < mpModuleCodeBegin) || (cp.mTarget > mpModuleCodeEnd)) )
      {
        BYTE byteRead;
        if ( (cp.mTarget!= NULL) &&
             CMemoryMap::TryRead(cp.mTarget, &byteRead, 1, mpMemoryMap) &&
             ((byteRead == 0xe9) || (byteRead == 0xeb) || (byteRead == 0xff)) )
        {
          // statically linked APIs are normally realized by a "call" to a "jmp"
          // we want to have the *real* target, so we take the "jmp" target
          CCodeParse cp2(cp.mTarget);
          HMODULE hMod;
          char arrChA[MAX_PATH];
          if ( cp2.mIsValid && (cp2.mTarget != NULL) &&
               ( (!FindModule(cp.mTarget, &hMod, arrChA, MAX_PATH)     ) ||
                 (!GetImageProcName(hMod, cp.mTarget, arrChA, MAX_PATH))    ) &&
               (FindModule(cp2.mTarget, &hMod, arrChA, MAX_PATH)) &&
               (GetImageProcName(hMod, cp2.mTarget, arrChA, MAX_PATH)) )
            cp.mTarget = cp2.mTarget;
        }
        BOOL b1 = TRUE;
        for (int i = 0; i < this->mFarCalls.GetCount(); i++)
          if (mFarCalls[i].CodeAddress2 == cp.mpNext)
          {
            b1 = FALSE;
            break;
          }
        if (b1)
        {
          AddFarCallItem(cp);
        }
      }
      else
        AddCodeArea(cp.mTarget, cp.mpCodeAddress);

      for (int i = 0; i < this->mUnknownTargets.GetCount(); i++)
      {
        if (mUnknownTargets[i].CodeAddress1 == cp.mpCodeAddress)
        {
          // remove the unknown target cause it's no longer unknown
          mUnknownTargets[i] = mUnknownTargets[mUnknownTargets.GetCount() - 1];
          mUnknownTargets.RemoveAt(mUnknownTargets.GetCount() - 1);
          //ZeroMemory(&mUnknownTargets[mUnknownTargets.GetCount() - 1], sizeof(UNKNOWN_TARGET));
          //mUnknownTargets.GetCount()--;
          break;
        }
      }
    }
    else
    {
      AddUnknownTargetItem(cp);
    }
  }
}

typedef struct tagCmpRecord
{
  BYTE Switches;
  BYTE Register;
  BOOL JumpFound;
} CMP_RECORD;

void CFunctionParse::ParseCodeArea(CCodeParse &cp)
// walks the current code area instruction by instruction
{
  CMP_RECORD cmp;
  cmp.Switches = 0;
  cmp.Register = 0;
  cmp.JumpFound = FALSE;

  LPVOID pCurrent = mCodeAreas[mCurrentCodeArea].AreaBegin;
  memmove(mCodeAreas[mCurrentCodeArea].Registers, this->mRegState, sizeof(REG_STATE));

  // Initialize pEnd 
  // It starts at the end of Module
  //  Then, if a code area begins after current code area and its end is less than pEnd,
  //    we set pEnd to the end if its area.  
  LPVOID pEnd = this->mpModuleCodeEnd;
  for (int i = 0; i < mCodeAreas.GetCount(); i++)
  {
    if ( (mCodeAreas[i].AreaBegin > mCodeAreas[mCurrentCodeArea].AreaBegin) &&
         (mCodeAreas[i].AreaBegin < pEnd                                  )    )
      pEnd = mCodeAreas[i].AreaBegin;
  }
  while (TRUE)
  {
    REG_STATE rs2;
    memmove(rs2, mRegState, sizeof(REG_STATE));

    cp.Parse(pCurrent, &mRegState, NULL);
    if (!cp.mIsValid)
      break;

    // mCurrentCodeArea gets its AreaEnd updated as each new instruction is processed
    mCodeAreas[mCurrentCodeArea].AreaEnd = (LPVOID) ((ULONG_PTR) (cp.mpNext) - 1);

    // Is it ret/iret?
    if ((cp.mOpCode == 0xc2) || (cp.mOpCode == 0xc3) || (cp.mOpCode == 0xca) || (cp.mOpCode == 0xcb) || (cp.mOpCode == 0xcf))
      break;

/*   // madshi: I think the try..catch detection might also work for MSVC++, so I think we should better add it, same with case/switch blocks
    if( (cp->mOpCode == 0x68) && ((ULONG_PTR) cp->mpNext - (ULONG_PTR) cp->mpCodeAddress == 5) &&
        ( *((LPBYTE) cp->mpNext) == 0x64 ) )
    {
      // This is a "push dword" instruction, followed by a "fs:" prefix
      //  we store it because it can be part of a try..except/finally block
      push = *((DWORD *) ((ULONG_PTR) cp->mpCodeAddress + 1));
    }
    else
    {
      if( push != 0 )
      {
        if( cp->mOpCode == 0xff )
        {
          if( ( ( (cp->mModRm & 0xf8) == 0x30 ) &&                                  // (1) push dword ptr fs:[register]
                ( (ULONG_PTR) cp->mpNext - (ULONG_PTR) cp->mpCodeAddress == 3 ) &&  //     instruction == 3 bytes
                ( rs2[cp->mModRm & 0x07] == NULL ) ) ||                             //     register == 0
              ( ( cp->mModRm == 0x35 ) &&                                         // (2) push dword ptr fs:[dword]
                ( (ULONG_PTR) cp->mpNext - (ULONG_PTR) cp->mpCodeAddress == 7 ) &&  //     instruction == 7 bytes
                ( *((DWORD *)( (ULONG_PTR) cp->mpCodeAddress + 3 )) == 0 ) ) )     //     dword = 0
            // some exception handler is being installed
            // we add this pointer to this handle to our list of code areas
            AddCodeArea( (LPVOID) push, NULL );
        }
        push = 0;
      }
    }
    if( cmp.Switches == 0 )
    {
      if(  cp->mOpCode == 0x83 && ( cp->mModRm >= 0xf8 && cp->mModRm <= 0xff ) )
      {
        // found cmp reg, byteValue
        // may be beginning of a case statement
        cmp.Switches = *(LPBYTE)( (ULONG_PTR) cc + 2 );
        cmp.Register = cp->mModRm & 0x07;
        cmp.JumpFound = FALSE;
      }
    }
    else if(!cmp.JumpFound)
    {
      if( ( (cp->mOpCode == 0x77) || (cp->mOpCode == 0x7f) ) ||
         ( cp->mOpCode == 0xf87 ) || ( cp->mOpCode == 0x8f ) )
      {
        // this may still be a case statement with ja/jg
        cmp.JumpFound = TRUE;
        cmp.Switches++;
      }
      else if( (cp->mOpCode == 0x73 || cp->mOpCode == 0x7d ) ||   // this may still be case with jae/jge
            ( cp->mOpCode == 0x0f83 ) || (cp->mOpCode == 0x0f8d ) )
        cmp.JumpFound = TRUE;
      else
        cmp.Switches = 0;
    }
    else if(  (cp->mOpCode == 0xff) && (cp->mModRm == 0x24 ) &&
           (  (*((LPBYTE)( (ULONG_PTR) cc + 2 )) & 0xc7) == 0x85 ) &&
           (  (((*((LPBYTE)( (ULONG_PTR) cc + 2 )) >> 3) & 0x07) == cmp.Register ) ) &&
           (  *(DWORD *)((ULONG_PTR) cc + 3 ) == (ULONG_PTR) cp->mpNext )   )
    {
      // it *is* a case statement!
      // Fill the code areas for the branches
      for( int i = 0; i < cmp.Switches; i++ )
        AddCodeArea( ((LPVOID *)(cp->mpNext))[i], cc );
      AddSpecialBlock( cp->mpNext, cmp.Switches*4, TRUE, FALSE );
      break;
    }
    else
      cmp.Switches = 0;
*/
    CheckAddTarget(cp);

    if ( (mCodeAreas[mCodeAreas.GetCount() - 1].AreaBegin > mCodeAreas[mCurrentCodeArea].AreaBegin) &&
         (mCodeAreas[mCodeAreas.GetCount() - 1].AreaBegin < pEnd                                  )    )
      pEnd = mCodeAreas[mCodeAreas.GetCount() - 1].AreaBegin;

    if ((cp.mpCodeAddress) && (cp.mpCodeAddress == mpMsvcrtThrowExceptionAddr))  // ThrowException?
      break;

    if ( cp.mIsJmp &&
         ( ((cp.mOpCode >= 0xe9) && ( cp.mOpCode        <= 0xeb)) ||
           ((cp.mOpCode == 0xff) && ((cp.mModRm & 0x30) == 0x20))    ) )
      break;  // jmp instruction

    // Update location in code area
    pCurrent = cp.mpNext;
    BOOLEAN b1 = FALSE;
    for (int i = 0; i < sizeof(LPVOID) * 2; i++)
    {
      if (this->mRegState[i] != rs2[i])
      {
        // Why add a code area because registers changed?
        b1 = TRUE;
        break;
      }
    }
    if (b1 || (pCurrent >= pEnd))
    {
      AddCodeArea(pCurrent, NULL);
      break;
    }
  }
  if (pCurrent > pEnd)
    cp.mIsValid = FALSE;
}

void CFunctionParse::CalcCodeBegin(ULONG_PTR *ce)
{
  mpCodeBegin = (LPVOID) -1;
  *ce = 0;
  for (int i = 0; i < mCodeAreas.GetCount(); i++)
  {
    if (mCodeAreas[i].AreaBegin < mpCodeBegin)
      mpCodeBegin = mCodeAreas[i].AreaBegin;
    if ((ULONG_PTR) mCodeAreas[i].AreaEnd > *ce)
      *ce = (ULONG_PTR) mCodeAreas[i].AreaEnd;
  }
  mCodeLength = (int) (*ce - (ULONG_PTR) mpCodeBegin + 1);
}

void CFunctionParse::FindCodeArea(void)
{
  for (int i = 0; i < mCodeAreas.GetCount(); i++)
  {
    if (mCodeAreas[i].AreaBegin == mpCodeBegin)
    {
      mpCodeBegin = (LPVOID) ((ULONG_PTR) mCodeAreas[i].AreaEnd + 1);
      FindCodeArea();
      break;
    }
  }
}

void CFunctionParse::AddCodeAreaItem(LPVOID areaBegin, LPVOID areaEnd, BOOL caseBlock, BOOL onExceptBlock,
                                     LPVOID calledFrom, REG_STATE *registers)
{
  CODE_AREA c;
  c.AreaBegin = areaBegin;
  c.AreaEnd = areaEnd;
  c.CaseBlock = caseBlock;
  c.OnExceptBlock = onExceptBlock;
  c.CalledFrom = calledFrom;
  memmove(c.Registers, registers, sizeof(REG_STATE));
  mCodeAreas.Add(c);
}

void CFunctionParse::AddFarCallItem(const CCodeParse& c)
{
  FAR_CALL f;

  f.Call = c.mIsCall;
  f.CodeAddress1 = c.mpCodeAddress;
  f.CodeAddress2 = c.mpNext;
  f.Target = c.mTarget;
  f.RelTarget = c.mIsRelTarget;
  f.pTarget = c.mpTarget;
  f.ppTarget = c.mppTarget;
  if (f.ppTarget != NULL)
    mCopy.BufferLength += sizeof(LPVOID);
  #ifdef _WIN64
    f.mIsRipRelative = c.mIsRipRelative;
    if (f.RelTarget)
      mCopy.BufferLength += 6 + sizeof(LPVOID);
  #endif

  mFarCalls.Add(f);
}

void CFunctionParse::AddUnknownTargetItem(const CCodeParse& c)
{
  UNKNOWN_TARGET u;

  u.Call = c.mIsCall;
  u.CodeAddress1 = c.mpCodeAddress;
  u.CodeAddress2 = c.mpNext;

  mUnknownTargets.Add(u);
}
