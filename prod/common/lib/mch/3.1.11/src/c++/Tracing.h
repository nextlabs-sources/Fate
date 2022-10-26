// ***************************************************************
//  Tracing.h                 version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  defines for debug tracing output
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _TRACING_H
#define _TRACING_H

#define Trace
#define TraceVerbose

   // DebugTrace macro requires double parentheses 
   // DebugTrace( (Format, ...) )
   #ifdef _DEBUG
      #define DebugTrace( _x_ )  Trace _x_ 
   #else
      #define DebugTrace( _x_ )
   #endif

#endif