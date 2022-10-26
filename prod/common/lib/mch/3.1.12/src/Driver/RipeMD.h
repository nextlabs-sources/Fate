// ***************************************************************
//  RipeMD.h                  version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  RipeMD hash functions
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-01-10 1.0.0 initial release

#ifndef _RipeMD_
#define _RipeMD_

// ********************************************************************

// is used to store volatile hash calculation data
typedef struct
  THashContext_ {
    unsigned int     State [16];
    unsigned __int64 Len;
    unsigned char    Buf [128];
    unsigned int     Index;
} THashContext;

// ********************************************************************

// initialize hash calculation
void InitHash (THashContext *Context);

// ********************************************************************

// update the hash value; can be called multiple times
void UpdateHash (THashContext *Context, void *Buf, int Len);

// ********************************************************************

// finish hash calculation and return the final hash result
void CloseHash (THashContext *Context, unsigned int *Digest);

// ********************************************************************

// InitHash + UpdateHash + CloseHash
void Hash (void *Buf, int Len, unsigned int *Digest);

// ********************************************************************

#endif
