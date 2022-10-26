// ***************************************************************
//  AtomicMove.h              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  move up to 8 bytes atomically (multi cpu safe)
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _ATOMICMOVE_H
#define _ATOMICMOVE_H

#undef EXTERN
#ifdef _ATOMICMOVE_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

// EXTERN BOOL AtomicMove(LPVOID source, LPVOID destination, int length);

#endif