// ***************************************************************
//  HookingTypes.h            version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  structures needed for hooking
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _HOOKINGTYPES_H
#define _HOOKINGTYPES_H

#pragma pack(1)

#define PROCESS_QUERY_LIMITED_INFORMATION (0x1000)
#define THREAD_QUERY_LIMITED_INFORMATION (0x0800)

typedef struct tagOpcodePointer
{
  BYTE opcode;
  LPVOID pointer;
} OPCODE_POINTER;

typedef struct tagOpcodePointer32
{
  BYTE opcode;
  VOID * POINTER_32 pointer;
} OPCODE_POINTER32;

typedef struct tagOpcodeDword
{
  BYTE opcode;
  DWORD data;
} OPCODE_DWORD;

typedef struct tagOpcodeWord
{
  BYTE opcode;
  WORD data;
} OPCODE_WORD;

typedef struct tagOpcodeByte
{
  BYTE opcode;
  BYTE data;
} OPCODE_BYTE;

typedef struct tagOpcodeShort
{
  BYTE opcode;
  __int8 data;
} OPCODE_SHORT;

typedef struct tagWordOpcodeByte
{
  WORD opcode;
  BYTE data;
} WORD_OPCODE_BYTE;

typedef struct tagWordOpcodePointer
{
  WORD opcode;
  LPVOID data;
} WORD_OPCODE_POINTER;

typedef struct tagDwordOpcodeByte
{
  DWORD opcode;
  BYTE data;
} DWORD_OPCODE_BYTE;

typedef struct tagAbsoluteJmp
// jmp [target]
{
  BYTE Opcode;   // 0xff
  BYTE modRm;    // 0x25, 00101001, Mod = 00, Reg = 100, R/M = 101 : 32 bit displacement follows
  DWORD Target;  // &(absolute target) which is pointer to an address (32 bits)
} ABSOLUTE_JUMP;

const ABSOLUTE_JUMP CAbsoluteJump = {0xff, 0x25, 0x00000000};

typedef struct tagIndirectAbsoluteJump
// jmp [target]
// AbsoluteAddress PVOID ??
{
  BYTE Opcode;   // 0xff
  BYTE modRm;    // 0x25, 00101001, Mod = 00, Reg = 100, R/M = 101 : 32 bit displacement follows
  DWORD Target;  // &(AbsoluteAddress) which is pointer to an address (32 bits)
  LPVOID AbsoluteAddress;
} INDIRECT_ABSOLUTE_JUMP;

const INDIRECT_ABSOLUTE_JUMP CIndirectAbsoluteJump = {0xff, 0x25, 0x00000000, NULL};

#pragma pack()

#endif