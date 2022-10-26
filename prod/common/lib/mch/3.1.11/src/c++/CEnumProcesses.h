// ***************************************************************
//  CEnumProcesses.h          version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  enumerate all running processes
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CENUMPROCESSES_H
#define _CENUMPROCESSES_H

#ifndef _CCOLLECTION_H
  #include "CCollection.h"
#endif

typedef struct tagProcessRecord
{
  DWORD Id;
  LPWSTR ExeFile;
  DWORD SessionId;
  WCHAR UserSID[MAX_PATH];
} PROCESS_RECORD;


// EnumProcesses Class
class SYSTEMS_API CEnumProcesses
{
  public:
    CEnumProcesses(BOOL needFullPath);
    ~CEnumProcesses();

    int GetCount();
    void RemoveAt(int index);
    const PROCESS_RECORD& operator[] (int index) const;

    _declspec(property(get = getIsValid)) bool IsValid;

    bool getIsValid()
    {
      return mIsValid;
    }

  protected:
    void AddRecord(DWORD id, DWORD SessionID, wchar_t *exeFile, LPWSTR sid);
    CCollection<PROCESS_RECORD, CStructureEqualHelper<PROCESS_RECORD>> mArray;

    bool mIsValid;
};

#endif