// ***************************************************************
//  CFunctionParse.h          version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  disassembling a whole function
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CFUNCTIONPARSE_H
#define _CFUNCTIONPARSE_H

typedef struct tagCodeArea
{
  LPVOID AreaBegin;
  LPVOID AreaEnd;
  BOOL CaseBlock;
  BOOL OnExceptBlock;
  LPVOID CalledFrom;
  REG_STATE Registers;
} CODE_AREA;

typedef struct tagFarCall
{
  BOOL Call;               // call or jmp
  LPVOID CodeAddress1;     // beginning of call instruction
  LPVOID CodeAddress2;     // beginning of next instruction
  LPVOID Target;
  BOOL RelTarget;
  LPVOID pTarget;
  LPVOID *ppTarget;
  #ifdef _WIN64
    BOOL mIsRipRelative;   // is the call/jmp target data RIP relative?
  #endif
} FAR_CALL;

typedef struct tagUnknownTarget
{
  BOOL Call;
  LPVOID CodeAddress1;
  LPVOID CodeAddress2;
} UNKNOWN_TARGET;

typedef struct tagCopy
{
  BOOL IsValid;
  int BufferLength;
  LPVOID LastErrorAddress;
  DWORD LastErrorNo;
  LPWSTR LastErrorString;
} COPY;

class SYSTEMS_API CFunctionParse
{
  public:

    CFunctionParse(LPVOID pFunction);
    CFunctionParse(LPVOID pFunction, REG_STATE *pRegState, CMemoryMap *pMemoryMap);

    ~CFunctionParse(void);

    SString ToString(LPVOID exceptAddr, int maxLines, bool autoDelimiters);

    BOOL IsInterceptable();

    LPVOID mpFunction;
    REG_STATE mRegState;
    CMemoryMap *mpMemoryMap;

    void InitializeMembers(LPVOID pFunction, REG_STATE *pRegState = NULL, CMemoryMap *pMemoryMap = NULL);

    void RawParseFunction();
    void AddCodeArea(LPVOID newAreaBegin, LPVOID newCaller);
    void AddSpecialBlock(LPVOID newAreaBegin, DWORD areaLength, BOOL isCaseBlock, BOOL isOnExceptBlock);
    void CheckAddTarget(CCodeParse& cp);
    void ParseCodeArea(CCodeParse& cp);
    void CalcCodeBegin(ULONG_PTR *ce);
    void FindCodeArea(void);

    void FindCodeArea(int& codeAreaIndex);
    void AddLine(LPVOID pCode, SString& line, BOOL justCount, BOOL addCodePosition = TRUE);
    void ToStringInternal(const SString& functionName, int *pCodeAreaIndex, BOOL justCount, BOOL bAutoDelimiters);

    int mCurrentCodeArea;
    LPVOID mpModuleCodeBegin;
    LPVOID mpModuleCodeEnd;
    LPVOID mpModuleDataBegin;
    LPVOID mpModuleDataEnd;

    int mLines;
    int mFirst;
    int mMaxLines;
    bool bBefore;
    LPVOID mpExceptAddr;
    LPVOID mpMsvcrtThrowExceptionAddr;

    BOOL mIsValid;
    BOOL mIsInterceptable;

    LPVOID mpEntryPoint;
    LPVOID mpCodeBegin;
    int mCodeLength;
    LPVOID mpLastErrorAddress;
    DWORD mLastErrorNo;
    SString *mLastErrorString;

    COPY mCopy;

    void AddCodeAreaItem(LPVOID areaBegin, LPVOID areaEnd, BOOL caseBlock, BOOL onExceptBlock, LPVOID calledFrom, REG_STATE *registers);
    void AddFarCallItem(const CCodeParse& c);
    void AddUnknownTargetItem(const CCodeParse& c);

    SString mDisAsm;
    CCollection<CODE_AREA, CStructureEqualHelper<CODE_AREA>> mCodeAreas;
    CCollection<FAR_CALL, CStructureEqualHelper<FAR_CALL>> mFarCalls;
    CCollection<UNKNOWN_TARGET, CStructureEqualHelper<UNKNOWN_TARGET>> mUnknownTargets;
};

#endif