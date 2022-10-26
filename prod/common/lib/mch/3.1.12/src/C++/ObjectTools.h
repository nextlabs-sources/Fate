// ***************************************************************
//  ObjectTools.h             version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  wraps the creation of global windows objects
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _OBJECTTOOLS_H
#define _OBJECTTOOLS_H

#undef EXTERN
#ifdef _OBJECTTOOLS_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void ApiSpecialName(LPCSTR prefix1, LPCSTR prefix2, LPVOID api, BOOL shared, LPSTR specialName);
EXTERN void InitSecurityAttributes(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd, bool limited);
EXTERN bool IsMetro(void);
EXTERN bool CreateMetroSd(SECURITY_DESCRIPTOR *sd);
EXTERN void InitSas();
EXTERN void FinalSas();
EXTERN PSECURITY_ATTRIBUTES GetLimitedSa();
EXTERN PSECURITY_ATTRIBUTES GetUnlimitedSa();

#endif