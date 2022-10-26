// ***************************************************************
//  SystemsTest.h             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  declares all the various test functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _SYSTEMS_TEST_H
#define _SYSTEMS_TEST_H

#ifdef _WIN64
  #define _WIN32_WINNT 0x501
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

bool TestAllocEx(void);
bool TestProtect(void);
bool TestCopyFunction(void);
bool TestGetSmssHandle(void);
bool TestHandleLiveForeverWithProcess(void);
bool TestHandleLiveForever(void);
bool TestHandleLiveForeverCode(void);
bool TestRemoteExecute(void);

bool TestHooking(void);
bool TestSafeHooking(void);
bool TestNoSafeUnhooking(void);
bool TestMixtureModeHooking(void);
bool TestInUse(void);
bool TestImportExportTablePatching(void);
bool TestFindInternalCreateWindow(void);

bool TestInjection(void);
bool TestDriverInjection(void);

bool TestIpc(void);

#endif