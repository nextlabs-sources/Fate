// ***************************************************************
//  Opcodes.h                 version: 1.0.1  ·  date: 2010-04-21
//  -------------------------------------------------------------
//  disassembler resources
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2010-04-21 1.0.1 added support for 3 byte nop instruction (0f 1f)
// 2010-01-10 1.0.0 initial version

#ifndef _OPCODES_H
#define _OPCODES_H

const WORD fInvalid = 0xffff;  // invalid opcode

const WORD fReg     = 0x0007;  // bit mask
const WORD fNoReg   = 0x0000;  // no register information available for this opcode
const WORD fRegAl   = 0x0001;  // no modrm byte: al register
const WORD fRegEax  = 0x0002;  // no modrm byte: (e)ax register
const WORD fRegO8   = 0x0004;  // no modrm byte: byte register depending on opcode
const WORD fRegO32  = 0x0005;  // no modrm byte: (d)word register depending on opcode
const WORD fRegEaxO = 0x0006;  // no modrm byte: fRegEax + fRegO32
const WORD fRegDxA  = 0x0007;  // no modrm byte: dx register + (e)ax/al register
const WORD fReg8    = 0x0001;  // byte register specified by modrm byte
const WORD fReg16   = 0x0002;  // word register specified by modrm byte
const WORD fRegxx   = 0x0003;  // segment/cr/dr register specified by modrm byte
const WORD fReg32   = 0x0004;  // (d)word register specified by modrm byte
const WORD fReg64   = 0x0005;  // qword register specified by modrm byte
const WORD fRegSt   = 0x0006;  // st floating point register specified by modrm byte
const WORD fReg128  = 0x0007;  // oword register specified by modrm byte

const WORD fMod     = 0x0038;  // bit mask
const WORD fModOpc  = 0x0008;  // real flags are stored in COpcodeFlagsEx
const WORD fMod8    = 0x0010;  // byte register/memory
const WORD fMod16   = 0x0018;  // word register/memory
const WORD fMod32   = 0x0020;  // (d)word register/memory
const WORD fMod64   = 0x0028;  // qword register/memory
const WORD fMod80   = 0x0030;  // st floating point register/memory
const WORD fMod128  = 0x0038;  // oword register/memory

const WORD f66      = 0x00C0;  // bit mask
const WORD f66R     = 0x0040;  // 66 prefix changes size of register  -> 16 (sse: 128)
const WORD f66M     = 0x0080;  // 66 prefix changes size of modrm     -> 16 (sse: 128)
const WORD f66RM    = 0x00C0;  // 66 prefix changes size of reg+modrm -> 16 (sse: 128)

const WORD fPtr     = 0x0100;  // disassembler shows "xword/byte ptr" or "[0xxxx]"

const WORD fOrder   = 0x0200;  // swapped order -> modrm or immediate data comes first

const WORD fI       = 0x0C00;  // bit mask
const WORD fI8      = 0x0400;  // immediate byte available
const WORD fI16     = 0x0800;  // immediate word available
const WORD fI32     = 0x0C00;  // immediate (d)word available

const WORD fJmpRel  = 0x1000;  // this opcode is a relative jump/call

const WORD fClr     = 0xe000;  // bit mask
const WORD fClrR    = 0x2000;  // clear modrm register/memory specified
const WORD fClrM    = 0x4000;  // clear register specified by modrm byte
const WORD fClrO    = 0x6000;  // clear register depending on opcode
const WORD fClrA    = 0x8000;  // clear eax
const WORD fClrRM   = 0xa000;  // fClrR + fClrM
const WORD fClrMA   = 0xc000;  // fClrM + fClrA
const WORD fClrOA   = 0xe000;  // fClrO + fClrA

// flags for one byte opcodes
const WORD COpCodeFlags[256] =
{
  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 00 /r       add     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 01 /r       add     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 02 /r       add     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 03 /r       add     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 04 ib       add     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 05 i?       add     (e)ax, i?         eax
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // 06          push    es
    (fNoReg                                                           ),  // 07          pop     es
  #endif
  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 08 /r       or      r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 09 /r       or      r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 0a /r       or      rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0b /r       or      r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 0c ib       or      al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 0d i?       or      (e)ax, i?         eax
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // 0e          push    cs
  #endif
  (fNoReg                                                           ),  // 0f          < extra table below >

  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 10 /r       adc     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 11 /r       adc     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 12 /r       adc     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 13 /r       adc     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 14 ib       adc     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 15 i?       adc     (e)ax, i?         eax
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // 16          push    ss
    (fNoReg                                                           ),  // 17          pop     ss
  #endif
  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 18 /r       sbb     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 19 /r       sbb     r/m?, i?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 1a /r       sbb     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 1b /r       sbb     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 1c ib       sbb     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 1d i?       sbb     (e)ax, i?         eax
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // 1e          push    ds
    (fNoReg                                                           ),  // 1f          pop     ds
  #endif

  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 20 /r       and     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 21 /r       and     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 22 /r       and     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 23 /r       and     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 24 ib       and     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 25 i?       and     (e)ax, i?         eax
  (fNoReg                                                           ),  // 26          PREFIX: es segment override
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  +                                                  fClrA ),  // 27          daa                       eax
  #endif
  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 28 /r       sub     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 29 /r       sub     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 2a /r       sub     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 2b /r       sub     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 2c ib       sub     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 2d i?       sub     (e)ax, i?         eax
  (fNoReg                                                           ),  // 2e          PREFIX: cs segment override / branch not taken hint
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  +                                                  fClrA ),  // 2f          das                       eax
  #endif

  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 30 /r       xor     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 31 /r       xor     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 32 /r       xor     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 33 /r       xor     r?, r/m?          r
  (fRegAl  +                                   fI8  +         fClrA ),  // 34 ib       xor     al, ib            eax
  (fRegEax +           f66R  +                 fI32 +         fClrA ),  // 35 i?       xor     (e)ax, i?         eax
  (fNoReg                                                           ),  // 36          PREFIX: SS segment override
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  +                                                  fClrA ),  // 37          aaa                       eax
  #endif
  (fReg8   + fMod8   +                fOrder                        ),  // 38 /r       cmp     r/mb, rb
  (fReg32  + fMod32  + f66RM +        fOrder                        ),  // 39 /r       cmp     r/m?, r?
  (fReg8   + fMod8                                                  ),  // 3a /r       cmp     rb, r/mb
  (fReg32  + fMod32  + f66RM                                        ),  // 3b /r       cmp     r?, r/m?
  (fRegAl  +                                   fI8                  ),  // 3c ib       cmp     al, ib
  (fRegEax +           f66R  +                 fI32                 ),  // 3d i?       cmp     (e)ax, i?
  (fNoReg                                                           ),  // 3e          PREFIX: DS segment override / branch taken hint
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  +                                                  fClrA ),  // 3f          aas                       eax
  #endif

  #ifdef _WIN64
    (fNoReg                                                           ),  // 40          PREFIX: REX
    (fNoReg                                                           ),  // 41          
    (fNoReg                                                           ),  // 42          
    (fNoReg                                                           ),  // 43          
    (fNoReg                                                           ),  // 44          
    (fNoReg                                                           ),  // 45          
    (fNoReg                                                           ),  // 46          
    (fNoReg                                                           ),  // 47          
    (fNoReg                                                           ),  // 48          
    (fNoReg                                                           ),  // 49          
    (fNoReg                                                           ),  // 4a          
    (fNoReg                                                           ),  // 4b          
    (fNoReg                                                           ),  // 4c          
    (fNoReg                                                           ),  // 4d          
    (fNoReg                                                           ),  // 4e          
    (fNoReg                                                           ),  // 4f
  #else
    (fRegO32 +           f66R  +                                fClrO ),  // 40          inc     (e)ax             r
    (fRegO32 +           f66R  +                                fClrO ),  // 41          inc     (e)cx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 42          inc     (e)dx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 43          inc     (e)bx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 44          inc     (e)sp             r
    (fRegO32 +           f66R  +                                fClrO ),  // 45          inc     (e)bp             r
    (fRegO32 +           f66R  +                                fClrO ),  // 46          inc     (e)si             r
    (fRegO32 +           f66R  +                                fClrO ),  // 47          inc     (e)di             r
    (fRegO32 +           f66R  +                                fClrO ),  // 48          dec     (e)ax             r
    (fRegO32 +           f66R  +                                fClrO ),  // 49          dec     (e)cx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4a          dec     (e)dx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4b          dec     (e)bx             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4c          dec     (e)sp             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4d          dec     (e)bp             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4e          dec     (e)si             r
    (fRegO32 +           f66R  +                                fClrO ),  // 4f          dec     (e)di             r
  #endif

  (fRegO32 +           f66R                                         ),  // 50          push    (e)ax
  (fRegO32 +           f66R                                         ),  // 51          push    (e)cx
  (fRegO32 +           f66R                                         ),  // 52          push    (e)dx
  (fRegO32 +           f66R                                         ),  // 53          push    (e)bx
  (fRegO32 +           f66R                                         ),  // 54          push    (e)sp
  (fRegO32 +           f66R                                         ),  // 55          push    (e)bp
  (fRegO32 +           f66R                                         ),  // 56          push    (e)si
  (fRegO32 +           f66R                                         ),  // 57          push    (e)di
  (fRegO32 +           f66R  +                                fClrO ),  // 58          pop     (e)ax             r
  (fRegO32 +           f66R  +                                fClrO ),  // 59          pop     (e)cx             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5a          pop     (e)dx             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5b          pop     (e)bx             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5c          pop     (e)sp             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5d          pop     (e)bp             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5e          pop     (e)si             r
  (fRegO32 +           f66R  +                                fClrO ),  // 5f          pop     (e)di             r

  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
    (fReg32  + fMod32  + f66R  + fPtr +                         fClrR ),  // 63 /r       movsxd  rq, r/md          r
  #else
    (fNoReg                                                           ),  // 60          pusha(d)
    (fNoReg  +                                                  fClrA ),  // 61          popa(d)                   edi esi ebp ebx edx ecx eax
    (fReg32  + fMod32  + f66RM                                        ),  // 62 /r       bound   r?, m?&?
    (fReg16  + fMod16  +                fOrder +                fClrM ),  // 63 /r       arpl    r/mw, rw          (r)
  #endif
  (fNoReg                                                           ),  // 64          PREFIX: fs segment override
  (fNoReg                                                           ),  // 65          PREFIX: gs segment override
  (fNoReg                                                           ),  // 66          PREFIX: operand size override
  (fNoReg                                                           ),  // 67          PREFIX: address size override
  (fNoReg  +                                   fI32                 ),  // 68 i?       push    i?
  (fReg32  + fMod32  + f66RM +                 fI32 +         fClrR ),  // 69 /r i?    imul    r?, [r/m?,] i?    r
  (fNoReg  +                                   fI8                  ),  // 6a ib       push    ib
  (fReg32  + fMod32  + f66RM +                 fI8  +         fClrR ),  // 6b /r ib    imul    r?, [r/m?,] ib    r
  (fNoReg                                                           ),  // 6c          insb                      edi
  (fNoReg                                                           ),  // 6d          insw/insd                 edi
  (fNoReg                                                           ),  // 6e          outsb                     esi
  (fNoReg                                                           ),  // 6f          outsw/d                   esi

  (fNoReg  +                                   fI8  + fJmpRel       ),  // 70 cb       jo      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 71 cb       jno     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 72 cb       jb      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 73 cb       jnb     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 74 cb       jz      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 75 cb       jnz     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 76 cb       jbe     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 77 cb       ja      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 78 cb       js      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 79 cb       jns     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7a cb       jp      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7b cb       jnp     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7c cb       jl      relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7d cb       jge     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7e cb       jle     relb
  (fNoReg  +                                   fI8  + fJmpRel       ),  // 7f cb       jg      relb

  (fNoReg  + fMod8   +         fPtr +          fI8                  ),  // 80 /x ib    xxx     r/mb, ib          (r)  -  add/or/adc/sbb/and/sub/xor/cmp
  (fNoReg  + fMod32  + f66M  + fPtr +          fI32                 ),  // 81 /x i?    xxx     r/m?, i?          (r)
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  + fMod8   +         fPtr +          fI8                  ),  // 82 /x ib    xxx     r/mb, ib          (r)
  #endif
  (fNoReg  + fMod32  + f66M  + fPtr +          fI8                  ),  // 83 /x ib    xxx     r/m?, ib          (r)
  (fReg8   + fMod8   +                fOrder                        ),  // 84 /r       test    r/mb, rb
  (fReg32  + fMod32  + f66RM +        fOrder                        ),  // 85 /r       test    r/m?, r?
  (fReg8   + fMod8   +                                        fClrRM),  // 86 /r       xchg    rb, r/mb          (r) r
  (fReg32  + fMod32  + f66RM +                                fClrRM),  // 87 /r       xchg    r?, r/m?          (r) r
  (fReg8   + fMod8   +                fOrder +                fClrM ),  // 88 /r       mov     r/mb, rb          (r)
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 89 /r       mov     r/m?, r?          (r)
  (fReg8   + fMod8   +                                        fClrR ),  // 8a /r       mov     rb, r/mb          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 8b /r       mov     r?, r/m?          r
  (fRegxx  + fMod32  + f66RM +        fOrder +                fClrM ),  // 8c /r       mov     r/m?, sreg        (r)
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 8d /r       lea     r?, m             r
  (fRegxx  + fMod16  + f66RM + fPtr                                 ),  // 8e /r       mov     sreg, r/m?
  (fNoReg  + fMod32  + f66M  + fPtr                                 ),  // 8f /0       pop     m?

  (fNoReg                                                           ),  // 90          nop
  (fRegEaxO+           f66R  +                                fClrOA),  // 91          xchg    (e)ax, (e)cx      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 92          xchg    (e)ax, (e)dx      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 93          xchg    (e)ax, (e)bx      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 94          xchg    (e)ax, (e)sp      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 95          xchg    (e)ax, (e)bp      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 96          xchg    (e)ax, (e)si      r eax
  (fRegEaxO+           f66R  +                                fClrOA),  // 97          xchg    (e)ax, (e)di      r eax
  (fNoReg  +                                                  fClrA ),  // 98          cbw/cwde                  eax
  (fNoReg                                                           ),  // 99          cwd/cdq                   edx
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // 9a c? cw    call    cw:c?
  #endif
  (fNoReg                                                           ),  // 9b          wait
  (fNoReg                                                           ),  // 9c          pushf(d)
  (fNoReg                                                           ),  // 9d          popf(d)
  (fNoReg                                                           ),  // 9e          sahf
  (fNoReg  +                                                  fClrA ),  // 9f          lahf                      eax

  (fRegAl  +                   fPtr +                         fClrA ),  // a0          mov     al, moffsb        eax
  (fRegEax +           f66R  + fPtr +                         fClrA ),  // a1          mov     (e)ax, moffs?     eax
  (fRegAl  +                   fPtr + fOrder                        ),  // a2          mov     moffsb, al
  (fRegEax +           f66R  + fPtr + fOrder                        ),  // a3          mov     moffs?, (e)ax
  (fNoReg                                                           ),  // a4          movsb                     esi edi
  (fNoReg                                                           ),  // a5          movsw/d                   esi edi
  (fNoReg                                                           ),  // a6          cmpsb                     esi edi
  (fNoReg                                                           ),  // a7          cmpsd/w                   esi edi
  (fRegAl  +                                   fI8                  ),  // a8 ib       test    al, ib
  (fRegEax +           f66R  +                 fI32                 ),  // a9 i?       test    (e)ax, i?
  (fNoReg                                                           ),  // aa          stosb                     edi
  (fNoReg                                                           ),  // ab          stosw/d                   edi
  (fNoReg  +                                                  fClrA ),  // ac          lodsb                     eax esi
  (fNoReg  +                                                  fClrA ),  // ad          lodsw/d                   eax esi
  (fNoReg                                                           ),  // ae          scasb                     edi
  (fNoReg                                                           ),  // af          scasw/d                   edi

  (fRegO8  +                                   fI8  +         fClrO ),  // b0 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b1 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b2 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b3 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b4 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b5 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b6 ib       mov     rb, ib            r
  (fRegO8  +                                   fI8  +         fClrO ),  // b7 ib       mov     rb, ib            r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // b8 i?       mov     (e)ax, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // b9 i?       mov     (e)cx, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // ba i?       mov     (e)dx, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // bb i?       mov     (e)bx, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // bc i?       mov     (e)sp, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // bd i?       mov     (e)bp, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // be i?       mov     (e)si, i?         r
  (fRegO32 +           f66R  +                 fI32 +         fClrO ),  // bf i?       mov     (e)di, i?         r

  (fNoReg  + fMod8   +         fPtr +          fI8  +         fClrM ),  // c0 /x ib    xxx     r/mb, ib          (r)  -  rol/ror/rcl/rcr/shl/shr/sar
  (fNoReg  + fMod32  + f66M  + fPtr +          fI8  +         fClrM ),  // c1 /x ib    xxx     r/m?, ib          (r)
  (fNoReg  +                                   fI16                 ),  // c2 iw       ret     iw
  (fNoReg                                                           ),  // c3          ret
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
  #else
    (fReg32  + fMod32  + f66RM +                                fClrR ),  // c4 /r       les     r?, m16:?         r
    (fReg32  + fMod32  + f66RM +                                fClrR ),  // c5 /r       lds     r?, m16:?         r
  #endif
  (fNoReg  + fMod8   +         fPtr +          fI8  +         fClrM ),  // c6 /0 ib    mov     r/mb, ib          (r)
  (fNoReg  + fMod32  + f66M  + fPtr +          fI32 +         fClrM ),  // c7 /0 i?    mov     r/m?, i?          (r)
  (fNoReg                                                           ),  // c8 iw ib    enter   iw, ib            ebp
  (fNoReg                                                           ),  // c9          leave                     ebp
  (fNoReg  +                                   fI16                 ),  // ca iw       ret     iw
  (fNoReg                                                           ),  // cb          ret
  (fNoReg                                                           ),  // cc          int 3
  (fNoReg  +                                   fI8                  ),  // cd ib       int     ib
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // ce          into
  #endif
  (fNoReg                                                           ),  // cf          iret(d)

  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // d0 /x       xxx     r/mb, 1           (r)  -  rol/ror/rcl/rcr/shl/shr/sar
  (fNoReg  + fMod32  + f66M  + fPtr +                         fClrM ),  // d1 /x       xxx     r/m?, 1           (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // d2 /x       xxx     r/mb, cl          (r)
  (fNoReg  + fMod32  + f66M  + fPtr +                         fClrM ),  // d3 /x       xxx     r/m?, cl          (r)
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg  +                                   fI8  +         fClrA ),  // d4 ib       aam                       eax
    (fNoReg  +                                   fI8  +         fClrA ),  // d5 ib       aad                       eax
    (fNoReg                                                           ),  // d6          salc
  #endif
  (fNoReg  +                                                  fClrA ),  // d7          xlatb                     eax
  (fModOpc                                                          ),  // d8 /r       xxx     mdreal/st, st(1)
  (fModOpc                                                          ),  // d9 /x/r     xxx
  (fModOpc                                                          ),  // da /x/r     xxx
  (fModOpc                                                          ),  // db /x/r     xxx
  (fModOpc                                                          ),  // dc /r       xxx     mdreal/st(1), st
  (fModOpc                                                          ),  // dd /x/r     xxx
  (fModOpc                                                          ),  // de /x/r     xxx
  (fModOpc                                                          ),  // df /x/r     xxx

  (fNoReg  +                                   fI8  + fJmpRel       ),  // e0 cb       loopne  relb              ecx
  (fNoReg  +                                   fI8  + fJmpRel       ),  // e1 cb       loope   relb              ecx
  (fNoReg  +                                   fI8  + fJmpRel       ),  // e2 cb       loop    relb              ecx
  (fNoReg  +                                   fI8  + fJmpRel       ),  // e3 cb       jcxz    relb
  (fRegAl  +                                   fI8  +         fClrA ),  // e4 ib       in      al, ib            eax
  (fRegEax +           f66R  +                 fI8  +         fClrA ),  // e5 ib       in      (e)ax, ib         eax
  (fRegAl  +                          fOrder + fI8                  ),  // e6 ib       out     ib, al
  (fRegEax +           f66R  +        fOrder + fI8                  ),  // e7 ib       out     ib, (e)ax
  (fNoReg  +                                   fI32 + fJmpRel       ),  // e8 c?       call    rel?
  (fNoReg  +                                   fI32 + fJmpRel       ),  // e9 c?       jmp     rel?
  #ifdef _WIN64
    (fInvalid                                                         ),  // -----
  #else
    (fNoReg                                                           ),  // ea c? cw    jmp     ptr16:?
  #endif
  (fNoReg  +                                   fI8  + fJmpRel       ),  // eb cb       jmp     relb
  (fRegDxA +                                                  fClrA ),  // ec          in      al, dx            eax
  (fRegDxA +           f66R  +                                fClrA ),  // ed          in      (e)ax, dx         eax
  (fRegDxA +                          fOrder                        ),  // ee          out     dx, al
  (fRegDxA +           f66R  +        fOrder                        ),  // ef          out     dx, (e)ax

  (fNoReg                                                           ),  // f0          PREFIX: lock
  (fNoReg                                                           ),  // f1          int01
  (fNoReg                                                           ),  // f2          PREFIX: repne             +ecx
  (fNoReg                                                           ),  // f3          PREFIX: rep(e)            +ecx
  (fNoReg                                                           ),  // f4          hlt
  (fNoReg                                                           ),  // f5          cmc
  (fModOpc                                                          ),  // f6 /x (ib)  xxx     r/mb (,ib)        (r) (eax)  -  test/not/neg/mul/imul/div/idiv
  (fModOpc                                                          ),  // f7 /x (i?)  xxx     r/m? (,i?)        (r) (eax)
  (fNoReg                                                           ),  // f8          clc
  (fNoReg                                                           ),  // f9          stc
  (fNoReg                                                           ),  // fa          cli
  (fNoReg                                                           ),  // fb          sti
  (fNoReg                                                           ),  // fc          cld
  (fNoReg                                                           ),  // fd          std
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // fe /x       xxx     r/mb              (r)  -  inc/dec
  (fNoReg  + fMod32  + f66M  + fPtr                                 )   // ff /x       xxx     r/m?              (r)  -  inc/dec/call/call/jmp/jmp/push
};

// flags for two byte opcodes (0x0f 0xxx)
const WORD COpCodeFlags0f[256] =
{
  (fNoReg  + fMod16  +         fPtr                                 ),  // 0f 00 /x    xxx     r/mw              (r)  -  sldt/str/lldt/ltr/verr/verw
  (fNoReg  + fMod16                                                 ),  // 0f 01 /x    xxx     r/m?              (r)  -  sgdt/sidt/lgdt/lidt/smsw/-/lmsw/invlpg
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 02 /r    lar     r?, r/m?          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 03 /r    lsl     r?, r/m?          r
  (fInvalid                                                         ),  // -----
  (fNoReg                                                           ),  // 0f 05       syscall (AMD)
  (fNoReg                                                           ),  // 0f 06       clts
  (fNoReg                                                           ),  // 0f 07       sysret (AMD)
  (fNoReg                                                           ),  // 0f 08       invd
  (fNoReg                                                           ),  // 0f 09       wbinvd
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fNoReg  + fMod8   +         fPtr                                 ),  // 0f 0d /x    prefetch(w) r/mb
  (fNoReg                                                           ),  // 0f 0e       femms
  (fReg64  + fMod64  +         fPtr +          fI8                  ),  // 0f 0f xx    xxx     pq, qq

  (fReg128 + fMod128                                                ),  // 0f 10 /r    movups  xmm, xmm/m
  (fReg128 + fMod128 +                fOrder                        ),  // 0f 11 /r    movups  xmm/m, xmm
  (fReg128 + fMod128                                                ),  // 0f 12 /r    movlps  xmm, m
  (fReg128 + fMod128 +                fOrder                        ),  // 0f 13 /r    movlps  m, xmm
  (fReg128 + fMod128                                                ),  // 0f 14 /r    unpcklps xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 15 /r    unpckhps xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 16 /r    movhps  xmm, m
  (fReg128 + fMod128 +                fOrder                        ),  // 0f 17 /r    movhps  m, xmm
  (fNoReg  + fMod8   +         fPtr                                 ),  // 0f 18 /x    prefetchxxx
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fNoReg  + fMod32  +         fPtr                                 ),  // 0f 1f /0    nop     md

  (fRegxx  + fMod32  +                fOrder +                fClrM ),  // 0f 20 /r    mov     rd, cr0-4         r
  (fRegxx  + fMod32  +                fOrder +                fClrM ),  // 0f 21 /r    mov     rd, dr0-7         r
  (fRegxx  + fMod32                                                 ),  // 0f 22 /r    mov     cr0-4, rd
  (fRegxx  + fMod32                                                 ),  // 0f 23 /r    mov     dr0-7, rd
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fReg128 + fMod128                                                ),  // 0f 28 /r    movaps  xmm, xmm/m
  (fReg128 + fMod128 +                fOrder                        ),  // 0f 29 /r    movaps  xmm/m, xmm
  (fReg128 + fMod64                                                 ),  // 0f 2a /r    cvtpi2ps xmm, mm/r/m
  (fReg128 + fMod128 +                fOrder                        ),  // 0f 2b /r    movntps m, xmm
  (fReg64  + fMod128                                                ),  // 0f 2c /r    cvttps2pi m/r, xmm/m      (r)
  (fReg64  + fMod128                                                ),  // 0f 2d /r    cvtps2pi  m/r, xmm/m      (r)
  (fReg128 + fMod128                                                ),  // 0f 2e /r    ucomiss xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 2f /r    comiss  xmm, xmm/m

  (fNoReg                                                           ),  // 0f 30       wrmsr
  (fNoReg  +                                                  fClrA ),  // 0f 31       rdtsc                     edx eax
  (fNoReg  +                                                  fClrA ),  // 0f 32       rdmsr                     edx eax
  (fNoReg  +                                                  fClrA ),  // 0f 33       rdpmc                     edx eax
  (fNoReg                                                           ),  // 0f 34       sysenter
  (fNoReg                                                           ),  // 0f 35       sysexit
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----

  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 40 /r    cmovo   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 41 /r    cmovno  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 42 /r    cmovb   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 43 /r    cmovnb  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 44 /r    cmovz   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 45 /r    cmovnz  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 46 /r    cmovbe  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 47 /r    cmova   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 48 /r    cmovs   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 49 /r    cmovns  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4a /r    cmovp   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4b /r    cmovnp  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4c /r    cmovl   r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4d /r    cmovge  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4e /r    cmovle  r?,r/m?           r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f 4f /r    cmovg   r?,r/m?           r

  (fReg32  + fMod128 +                                        fClrM ),  // 0f 50 /r    movmskps r, xmm           r
  (fReg128 + fMod128                                                ),  // 0f 51 /r    sqrtps  xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 52 /r    rsqrtps xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 53 /r    rcpps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 54 /r    andps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 55 /r    andnps  xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 56 /r    orps    xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 57 /r    xorps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 58 /r    addps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 59 /r    mulps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5a /r    cvtps2pd xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5b /r    cvtdq2ps xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5c /r    subps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5d /r    minps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5e /r    divps   xmm, xmm/m
  (fReg128 + fMod128                                                ),  // 0f 5f /r    maxps   xmm, xmm/m

  (fReg64  + fMod64  + f66RM                                        ),  // 0f 60 /r    punpcklbw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 61 /r    punpcklwd mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 62 /r    punpckldq mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 63 /r    packsswb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 64 /r    pcmpgtb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 65 /r    pcmpgtw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 66 /r    pcmpgtd mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 67 /r    packuswb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 68 /r    punpckhbw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 69 /r    punpckhwd mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 6a /r    punpckhdq mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 6b /r    packssdw mm1, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 6c /r    punpcklqdq xmm, xmm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 6d /r    punpckhqdq xmm, xmm/m
  (fReg64  + fMod32  + f66R                                         ),  // 0f 6e /r    movd    mm, r/md
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 6f /r    movq    mm, mm/m

  (fReg64  + fMod64  + f66RM +                 fI8                  ),  // 0f 70 /r ib pshufw  mm, mm/m, ib
  (fNoReg  + fMod64  + f66M  +                 fI8                  ),  // 0f 71 /x ib xxx     (x)mm, ib
  (fNoReg  + fMod64  + f66M  +                 fI8                  ),  // 0f 72 /x ib xxx     (x)mm, ib
  (fNoReg  + fMod64  + f66M  +                 fI8                  ),  // 0f 73 /x ib xxx     (x)mm, ib
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 74 /r    pcmpeqb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 75 /r    pcmpeqw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f 76 /r    pcmpeqd mm, mm/m
  (fNoReg                                                           ),  // 0f 77       emms
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fReg128 + fMod128 + f66RM                                        ),  // 0f 7c /r    haddpd  xmm, xmm/m
  (fReg128 + fMod128 + f66RM                                        ),  // 0f 7d /r    hsubpd  xmm, xmm/m
  (fReg64  + fMod32  + f66R  +        fOrder                        ),  // 0f 7e /r    movd    r/md, mm          (r)
  (fReg64  + fMod64  + f66RM +        fOrder                        ),  // 0f 7f /r    movq    mm/m, mm

  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 80 c?    jo      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 81 c?    jno     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 82 c?    jb      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 83 c?    jnb     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 84 c?    jz      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 85 c?    jnz     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 86 c?    jbe     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 87 c?    ja      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 88 c?    js      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 89 c?    jns     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8a c?    jp      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8b c?    jnp     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8c c?    jl      relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8d c?    jge     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8e c?    jle     relb
  (fNoReg  +                                   fI32 + fJmpRel       ),  // 0f 8f c?    jg      relb

  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 90       seto    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 91       setno   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 92       setb    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 93       setae   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 94       sete    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 95       setne   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 96       setbe   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 97       seta    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 98       sets    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 99       setns   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9a       setp    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9b       setnp   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9c       setl    r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9d       setge   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9e       setle   r/mb              (r)
  (fNoReg  + fMod8   +         fPtr +                         fClrM ),  // 0f 9f       setg    r/mb              (r)

  (fNoReg                                                           ),  // 0f a0       push    fs
  (fNoReg                                                           ),  // 0f a1       pop     fs
  (fNoReg  +                                                  fClrA ),  // 0f a2       cpuid                     eax ebx ecx edx
  (fReg32  + fMod32  + f66RM +        fOrder                        ),  // 0f a3       bt      r/m?, r?
  (fReg32  + fMod32  + f66RM + fPtr + fOrder + fI8  +         fClrM ),  // 0f a4 ib    shld    r/m?, r?, ib      (r)
  (fReg32  + fMod32  + f66RM + fPtr + fOrder +                fClrM ),  // 0f a5       shld    r/m?, r?, cl      (r)
  (fReg8   + fMod8   +                fOrder +                fClrMA),  // 0f a6 /r    cmpxchg r/mb, rb          (r) eax
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrMA),  // 0f a7 /r    cmpxchg r/m?, r?          (r) eax
  (fNoReg                                                           ),  // 0f a8       push    gs
  (fNoReg                                                           ),  // 0f a9       pop     gs
  (fNoReg                                                           ),  // 0f aa       rsm
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 0f ab       bts     r/m?, r?          (r)
  (fReg32  + fMod32  + f66RM + fPtr + fOrder + fI8  +         fClrM ),  // 0f ac ib    shrd    r/m?, r?, ib      (r)
  (fReg32  + fMod32  + f66RM + fPtr + fOrder +                fClrM ),  // 0f ad       shrd    r/m?, r?, cl      (r)
  (fNoReg  + fMod32                                                 ),  // 0f ae /x    xxx     (m)
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f af /r    imul    r?, r/m?          r

  (fReg8   + fMod8   +                fOrder +                fClrMA),  // 0f b0 /r    cmpxchg r/mb, rb          (r) eax
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrMA),  // 0f b1 /r    cmpxchg r/m?, r?          (r) eax
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f b2 /r    lss     r?, m16:?         r
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 0f b3       btr     r/m?, r?          (r)
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f b4 /r    lfs     r?, m16:?         r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f b5 /r    lgs     r?, m16:?         r
  (fReg32  + fMod8   + f66R  + fPtr +                         fClrR ),  // 0f b6 /r    movzx   r?, r/mb          r
  (fReg32  + fMod16  + f66R  + fPtr +                         fClrR ),  // 0f b7 /r    movzx   rd, r/mw          r
  (fInvalid                                                         ),  // -----
  (fInvalid                                                         ),  // -----
  (fNoReg  + fMod32  + f66M  + fPtr +          fI8                  ),  // 0f ba /x ib btx     r/m?, ib          (r)  -  bt/bts/btr/btc
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrM ),  // 0f bb       btc     r/m?, r?          (r)
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f bc       bsf     r?, r/m?          r
  (fReg32  + fMod32  + f66RM +                                fClrR ),  // 0f bd       bsr     r?, r/m?          r
  (fReg32  + fMod8   + f66R  + fPtr +                         fClrR ),  // 0f be /r    movsx   r?, r/mb          r
  (fReg32  + fMod16  + f66R  + fPtr +                         fClrR ),  // 0f bf /r    movsx   rd, r/mw          r

  (fReg8   + fMod8   +                fOrder +                fClrRM),  // 0f c0 /r    xadd    r/mb, rb          (r) r
  (fReg32  + fMod32  + f66RM +        fOrder +                fClrRM),  // 0f c1 /r    xadd    r/m?, r?          (r) r
  (fReg128 + fMod128 +                         fI8                  ),  // 0f c2 /r ib cmpps   xmm, xmm/m, ib
  (fReg32  + fMod32  +                fOrder                        ),  // 0f c3 /r    movnti  md, rd
  (fReg64  + fMod32  + f66R  +                 fI8                  ),  // 0f c4 /r ib pinsrw  mm, rd/mw, ib
  (fReg32  + fMod64  + f66M  +                 fI8  +         fClrR ),  // 0f c5 /r ib pextrw  rd, mm, ib        r
  (fReg128 + fMod128 +                         fI8                  ),  // 0f c6 /r ib shufps  xmm, xmm/m, ib
  (fNoReg  + fMod64  +         fPtr +                         fClrA ),  // 0f c7 /1 mq cmpxchg8b mq              edx eax
  (fRegO32 +                                                  fClrO ),  // 0f c8       bswap   (e)ax             r
  (fRegO32 +                                                  fClrO ),  // 0f c9       bswap   (e)cx             r
  (fRegO32 +                                                  fClrO ),  // 0f ca       bswap   (e)dx             r
  (fRegO32 +                                                  fClrO ),  // 0f cb       bswap   (e)bx             r
  (fRegO32 +                                                  fClrO ),  // 0f cc       bswap   (e)sp             r
  (fRegO32 +                                                  fClrO ),  // 0f cd       bswap   (e)bp             r
  (fRegO32 +                                                  fClrO ),  // 0f ce       bswap   (e)si             r
  (fRegO32 +                                                  fClrO ),  // 0f cf       bswap   (e)di             r

  (fReg128 + fMod128 + f66RM                                        ),  // 0f d0 /r    addsubpd xmm, xmm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d1 /r    psrlw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d2 /r    psrld   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d3 /r    psrlq   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d4 /r    paddq   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d5 /r    pmullw  mm, mm/m
  (fReg64  + fMod64  + f66RM +        fOrder                        ),  // 0f d6 /r    movq    xmm/m, xmm
  (fReg32  + fMod64  + f66M  +                                fClrR ),  // 0f d7 /r    pmovmskb rd, mm           r
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d8 /r    psubusb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f d9 /r    psubusw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f da /r    pminub  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f db /r    pand    mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f dc /r    paddusb mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f dd /r    paddusw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f de /r    pmaxub  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f df /r    pandn   mm, mm/m

  (fReg64  + fMod64  + f66RM                                        ),  // 0f e0 /r    pavgb   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e1 /r    psraw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e2 /r    psrad   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e3 /r    pavgw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e4 /r    pmulhuw mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e5 /r    pmulhw  mm, mm/m
  (fReg128 + fMod128                                                ),  // 0f e6 /r    cvttpd2dq xmm, xmm/m
  (fReg64  + fMod64  + f66RM +        fOrder                        ),  // 0f e7 /r    movntq  m, mm
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e8 /r    psubsb  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f e9 /r    psubsw  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f ea /r    pminsw  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f eb /r    por     mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f ec /r    paddsb  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f ed /r    paddsw  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f ee /r    pmaxsw  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f ef /r    pxor    mm, mm/m

  (fReg128 + fMod128                                                ),  // 0f f0 /r    lddqu   xmm, m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f1 /r    psllw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f2 /r    pslld   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f3 /r    psllq   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f4 /r    pmuludq mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f5 /r    pmaddwd mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f6 /r    psadbw  mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f7 /r    maskmovq mm, mm
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f8 /r    psubb   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f f9 /r    psubw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f fa /r    psubd   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f fb /r    psubq   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f fc /r    paddb   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f fd /r    paddw   mm, mm/m
  (fReg64  + fMod64  + f66RM                                        ),  // 0f fe /r    paddd   mm, mm/m
  (fNoReg                                                           )   // -----
};

typedef struct tagOpcodeFlagsRecord
{
  BYTE OpCode;
  WORD Flags[16];
} OPCODE_FLAGS_RECORD;

// flags for some opcodes which differ a lot depending on the Mod R/M byte
const int OpCodeFlagsExCount = 10;
const OPCODE_FLAGS_RECORD COpCodeFlagsEx[10] =
{
  { 0xf6, {( fNoReg + fMod8 + fPtr + fI8              ),    // f6 /0 ib    test    r/mb, ib
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod8 + fPtr + fClrM            ),    // f6 /2       not     r/mb              (r)
           ( fNoReg + fMod8 + fPtr + fClrM            ),    // f6 /3       neg     r/mb              (r)
           ( fNoReg + fMod8 + fPtr + fClrA            ),    // f6 /4       mul     r/mb              eax
           ( fNoReg + fMod8 + fPtr + fClrA            ),    // f6 /5       imul    r/mb              eax
           ( fNoReg + fMod8 + fPtr + fClrA            ),    // f6 /6       div     r/mb              eax
           ( fNoReg + fMod8 + fPtr + fClrA            ),    // f6 /7       idiv    r/mb              eax
           ( fNoReg + fMod8 + fPtr + fI8              ),
           ( fInvalid                                 ),
           ( fNoReg + fMod8 + fPtr + fClrM            ),
           ( fNoReg + fMod8 + fPtr + fClrM            ),
           ( fNoReg + fMod8 + fPtr + fClrA            ),
           ( fNoReg + fMod8 + fPtr + fClrA            ),
           ( fNoReg + fMod8 + fPtr + fClrA            ),
           ( fNoReg + fMod8 + fPtr + fClrA            )} },
  { 0xf7, {( fNoReg + fMod32 + f66M + fPtr + fI32     ),    // f7 /0 i?    test    r/m?, i?
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod32 + f66M + fPtr + fClrM    ),    // f7 /2       not     r/m?              (r)
           ( fNoReg + fMod32 + f66M + fPtr + fClrM    ),    // f7 /3       neg     r/m?              (r)
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),    // f7 /4       mul     r/m?              eax edx
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),    // f7 /5       imul    r/m?              eax edx
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),    // f7 /6       div     r/m?              eax edx
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),    // f7 /7       idiv    r/m?              eax edx
           ( fNoReg + fMod32 + f66M + fPtr + fI32     ),
           ( fInvalid                                 ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrM    ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrM    ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    ),
           ( fNoReg + fMod32 + f66M + fPtr + fClrA    )} },
  { 0xd8, {( fNoReg + fMod32 + fPtr                   ),    // d8 /0       fadd    mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /1       fmul    mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /2       fcom    mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /3       fcomp   mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /4       fsub    mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /5       fsubr   mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /6       fdiv    mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d8 /7       fdivr   mdreal
           ( fRegSt + fMod80                          ),    // d8 c0+i     fadd    st(0), st(i)
           ( fRegSt + fMod80                          ),    // d8 c8+i     fmul    st(0), st(i)
           ( fNoReg + fMod80                          ),    // d8 d0+i     fcom    st(i)
           ( fNoReg + fMod80                          ),    //   d8 d8+i      fcomp   st(i)
           ( fRegSt + fMod80                          ),    // d8 e0+i     fsub    st(0), st(i)
           ( fRegSt + fMod80                          ),    // d8 e8+i     fsubr   st(0), st(i)
           ( fRegSt + fMod80                          ),    // d8 f0+i     fdiv    st(0), st(i)
           ( fRegSt + fMod80                          )} }, // d8 f8+i     fdivr   st(0), st(i)
  { 0xd9, {( fNoReg + fMod32 + fPtr                   ),    // d9 /0       fld     mdreal
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod32 + fPtr                   ),    // d9 /2       fst     mdreal
           ( fNoReg + fMod32 + fPtr                   ),    // d9 /3       fstp    mdreal
           ( fNoReg + fMod8  + fPtr                   ),    // d9 /4       fldenv  m14/28byte
           ( fNoReg + fMod16 + fPtr                   ),    // d9 /5       fldcw   m2byte
           ( fNoReg + fMod8  + fPtr                   ),    // d9 /6       fnstenv m14/28byte
           ( fNoReg + fMod16 + fPtr                   ),    // d9 /7       fnstcw  m2byte
           ( fNoReg + fMod80                          ),    // d9 c0+i     fld     st(i)
           ( fNoReg + fMod80                          ),    // d9 c8+i     fxch    st(i)
           ( fNoReg                                   ),    // d9 d0       fnop
           ( fNoReg + fMod80                          ),    // d9 d8+i     fstp1   st(i)
           ( fNoReg                                   ),    // d9 e0       fxxx
           ( fNoReg                                   ),    // d9 e8       fxxx
           ( fNoReg                                   ),    // d9 f0       fxxx
           ( fNoReg                                   )} }, // d9 f8       fxxx
  { 0xda, {( fNoReg + fMod32 + fPtr                   ),    // da /0       fiadd   mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /1       fimul   mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /2       ficom   mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /3       ficomp  mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /4       fisub   mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /5       fisubr  mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /6       fidiv   mdint
           ( fNoReg + fMod32 + fPtr                   ),    // da /7       fidivr  mdint
           ( fRegSt + fMod80                          ),    // da c0+i     fcmovb  st(0), st(i)
           ( fRegSt + fMod80                          ),    // da c8+i     fcmove  st(0), st(i)
           ( fRegSt + fMod80                          ),    // da d0+i     fcmovbe st(0), st(i)
           ( fRegSt + fMod80                          ),    // da d8+i     fcmovu  st(0), st(i)
           ( fInvalid                                 ),    // -----
           ( fNoReg                                   ),    // da e9       fucompp
           ( fInvalid                                 ),    // -----
           ( fInvalid                                 )} }, // -----
  { 0xdb, {( fNoReg + fMod32 + fPtr                   ),    // db /0       fild    mdint
           ( fNoReg + fMod32 + fPtr                   ),    // db /1       fisttp  mdint
           ( fNoReg + fMod32 + fPtr                   ),    // db /2       fist    mdint
           ( fNoReg + fMod32 + fPtr                   ),    // db /3       fistp   mdint
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod80 + fPtr                   ),    // db /5       fld     m80real
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod80 + fPtr                   ),    // db /7       fstp    m80real
           ( fRegSt + fMod80                          ),    // db c0+i     fcmovnb st(0), st(i)
           ( fRegSt + fMod80                          ),    // db c8+i     fcmovne st(0), st(i)
           ( fRegSt + fMod80                          ),    // db d0+i     fcmovnbe st(0), st(i)
           ( fRegSt + fMod80                          ),    // db d8+i     fcmovnu st(0), st(i)
           ( fNoReg                                   ),    // db e0       fxxx
           ( fRegSt + fMod80                          ),    // db e8+i     fucomi  st(0), st(i)
           ( fRegSt + fMod80                          ),    // db f0+i     fcomi   st(0), st(i)
           ( fInvalid                                 )} }, // -----
  { 0xdc, {( fNoReg + fMod64 + fPtr                   ),    // dc /0       fadd    mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /1       fmul    mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /2       fcom    mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /3       fcomp   mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /4       fsub    mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /5       fsubr   mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /6       fdiv    mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dc /7       fdivr   mqreal
           ( fRegSt + fMod80 + fOrder                 ),    // dc c0+i     fadd    st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // dc c8+i     fmul    st(i), st(0)
           ( fNoReg + fMod80                          ),    // dc d0+i     fcom    st(i)
           ( fNoReg + fMod80                          ),    // dc d8+i     fcomp   st(i)
           ( fRegSt + fMod80 + fOrder                 ),    // dc e0+i     fsubr   st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // dc e8+i     fsub    st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // dc f0+i     fdivr   st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 )} }, // dc f8+i     fdiv    st(i), st(0)
  { 0xdd, {( fNoReg + fMod64 + fPtr                   ),    // dd /0       fld     mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dd /1       fisttp  mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dd /2       fst     mqreal
           ( fNoReg + fMod64 + fPtr                   ),    // dd /3       fstp    mqreal
           ( fNoReg + fMod8  + fPtr                   ),    // dd /4       frstor  m94/108byte
           ( fInvalid                                 ),    // -----
           ( fNoReg + fMod8  + fPtr                   ),    // dd /6       fnsave  m94/108byte
           ( fNoReg + fMod16 + fPtr                   ),    // dd /7       fnstsw  m2byte
           ( fNoReg + fMod80                          ),    // dd c0+i     ffree   st(i)
           ( fNoReg + fMod80                          ),    // dd c8+i     xch4    st(i)
           ( fNoReg + fMod80                          ),    // dd d0+i     fst     st(i)
           ( fNoReg + fMod80                          ),    // dd d8+i     fstp    st(i)
           ( fNoReg + fMod80                          ),    // dd e0+i     fucom   st(i)
           ( fNoReg + fMod80                          ),    // dd e8+i     fucomp  st(i)
           ( fInvalid                                 ),    // -----
           ( fInvalid                                 )} }, // -----
  { 0xde, {( fNoReg + fMod16 + fPtr                   ),    // de /0       fiadd   mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /1       fimul   mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /2       ficom   mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /3       ficomp  mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /4       fisub   mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /5       fisubr  mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /6       fidiv   mwint
           ( fNoReg + fMod16 + fPtr                   ),    // de /7       fidivr  mwint
           ( fRegSt + fMod80 + fOrder                 ),    // de c0+i     faddp   st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // de c8+i     fmulp   st(i), st(0)
           ( fNoReg + fMod80                          ),    // de c0+i     fcomp5  st(i)
           ( fRegSt + fMod80 + fOrder                 ),    // de d8+i     fcompp  st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // de e0+i     fsubrp  st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // de e8+i     fsubp   st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 ),    // de f0+i     fdivrp  st(i), st(0)
           ( fRegSt + fMod80 + fOrder                 )} }, // de f8+i     fdivp   st(i), st(0)
  { 0xdf, {( fNoReg + fMod16 + fPtr                   ),    // df /0       fild    mwint
           ( fNoReg + fMod16 + fPtr                   ),    // df /1       fisttp  mwint
           ( fNoReg + fMod16 + fPtr                   ),    // df /2       fist    mwint
           ( fNoReg + fMod16 + fPtr                   ),    // df /3       fistp   mwint
           ( fNoReg + fMod80 + fPtr                   ),    // df /4       fbld    m80dec
           ( fNoReg + fMod64 + fPtr                   ),    // df /5       fild    mqint
           ( fNoReg + fMod80 + fPtr                   ),    // df /6       fbstp   m80bcd
           ( fNoReg + fMod64 + fPtr                   ),    // df /7       fistp   mqint
           ( fNoReg + fMod80                          ),    // df c0+i     ffreep  st(i)
           ( fNoReg + fMod80                          ),    // df c8+i     fxch7   st(i)
           ( fNoReg + fMod80                          ),    // df d0+i     fstp8   st(i)
           ( fNoReg + fMod80                          ),    // df d8+i     fstp9   st(i)
           ( fNoReg                                   ),    // df e0       fnstsw  ax                eax
           ( fRegSt + fMod80                          ),    // df e8+i     fucomip st(0), st(i)
           ( fRegSt + fMod80                          ),    // df f0+i     fcomip  st(0), st(i)
           ( fInvalid                                 )} } 
};

#ifdef _WIN64
  // Note index is 0-6, 0-15... not 1-6, 0-15
  // (byte/word/segment/dword/qword/byte2)
  const LPCWSTR CRegisterLabels[7][16] =
  {
    { L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD", L"BAD", L"BAD", L"BAD", L"BAD", L"BAD"  },
    { L"al", L"cl", L"dl", L"bl", L"ah", L"ch", L"dh", L"bh", L"r8b",L"r9b",L"r10b",L"r11b",L"r12b",L"r13b",L"r14b",L"r15b" },
    { L"ax", L"cx", L"dx", L"bx", L"sp", L"bp", L"si", L"di", L"r8w",L"r9w",L"r10w",L"r11w",L"r12w",L"r13w",L"r14w",L"r15w" },
    { L"es", L"cs", L"ss", L"ds", L"fs", L"gs", NULL,  NULL,  NULL,  NULL,  NULL,   NULL,   NULL,   NULL,   NULL,   NULL    },
    { L"eax",L"ecx",L"edx",L"ebx",L"esp",L"ebp",L"esi",L"edi",L"r8d",L"r9d",L"r10d",L"r11d",L"r12d",L"r13d",L"r14d",L"r15d" },
    { L"rax",L"rcx",L"rdx",L"rbx",L"rsp",L"rbp",L"rsi",L"rdi",L"r8", L"r9", L"r10", L"r11", L"r12", L"r13", L"r14", L"r15"  },
    { L"al", L"cl", L"dl", L"bl", L"spl",L"bpl",L"sil",L"dil",NULL,  NULL,  NULL,   NULL,   NULL,   NULL,   NULL,   NULL    }
  };
#else
  // Note index is 0-4, 0-7... not 1-4, 0-7
  // (byte/word/segment/dword)
  const LPCWSTR CRegisterLabels[5][8] =
  {
    { L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD",L"BAD" },
    { L"al", L"cl", L"dl", L"bl", L"ah", L"ch", L"dh", L"bh"  },
    { L"ax", L"cx", L"dx", L"bx", L"sp", L"bp", L"si", L"di"  },
    { L"es", L"cs", L"ss", L"ds", L"fs", L"gs", NULL,  NULL   },
    { L"eax",L"ecx",L"edx",L"ebx",L"esp",L"ebp",L"esi",L"edi" }
  };
#endif

// index is 0-15, this replaces %cc in label
const LPCWSTR CConditionalLabels[] = 
{
  L"o", L"no", L"b", L"nb", L"z", L"nz", L"be", L"a",
  L"s", L"ns", L"p", L"np", L"l", L"ge", L"le", L"g"
};

const LPCWSTR COpCodeLabels[] =
{
  L"DUMMY", // eat zero index since Delphi starts at 1
  L"aaa",       L"aad",      L"aam",      L"aas",      L"adc",      L"add",       L"addPS",    L"and",
  #ifdef _WIN64
    L"andnPS",    L"movsxd",   L"bound",    L"bsf",      L"bsr",      L"bswap",     L"bt",       L"btc",
  #else
    L"andnPS",    L"arpl",     L"bound",    L"bsf",      L"bsr",      L"bswap",     L"bt",       L"btc",
  #endif
  L"btr",       L"bts",      L"call",     L"clc",      L"cld",      L"cli",       L"clts",     L"cmc",
  L"cmov%cc",   L"cmp",      L"cmpPS",    L"cmpsb",    L"cmpxchg",  L"cmpxchg8b", L"comisS",   L"cpuid",
  L"cvtPi2PS",  L"cvtPS2Pi", L"cvttPS2Pi",L"daa",      L"das",      L"dec",       L"divPS",    L"emms",
  L"enter",     L"femms",    L"hlt",      L"imul",     L"in",       L"inc",       L"insb",     L"int",
  L"int     3", L"int01",    L"into",     L"invd",     L"iret",     L"j%cc",      L"jecxz",    L"jmp",
  L"lahf",      L"lar",      L"lds",      L"lea",      L"leave",    L"les",       L"lfs",      L"lgs",
  L"sysret",    L"lodsb",    L"loop",     L"loope",    L"loopne",   L"lsl",       L"lss",      L"maxPS",
  L"minPS",     L"mov",      L"movaPS",   L"movd",     L"movhPS",   L"movlPS",    L"movmskPS", L"movnti",
  L"movntPS",   L"movsb",    L"movsx",    L"movzx",    L"mulPS",    L"nop",       L"or",       L"orPS",
  L"out",       L"outsb",    L"packssdw", L"packsswb", L"packuswb", L"paddb",     L"paddd",    L"paddq",
  L"paddsb",    L"paddsw",   L"paddusb",  L"paddusw",  L"paddw",    L"pand",      L"pandn",    L"pavgb",
  L"pavgw",     L"pcmpeqb",  L"pcmpeqd",  L"pcmpeqw",  L"pcmpgtb",  L"pcmpgtd",   L"pcmpgtw",  L"pextrw",
  L"pinsrw",    L"pmaddwd",  L"pmaxsw",   L"pmaxub",   L"pminsw",   L"pminub",    L"pmovmskb", L"pmulhuw",
  L"pmulhw",    L"pmullw",   L"pmuludq",  L"pop",      L"por",      L"psadbw",    L"pslld",    L"psllq",
  L"psllw",     L"psrad",    L"psraw",    L"psrld",    L"psrlq",    L"psrlw",     L"psubb",    L"psubd",
  L"psubq",     L"psubsb",   L"psubsw",   L"psubusb",  L"psubusw",  L"psubw",     L"punpckhbw",L"punpckhdq",
  L"punpckhqdq",L"punpckhwd",L"punpcklbw",L"punpckldq",L"punpcklwd",L"punpcklqdq",L"push",     L"pxor",
  L"rcpPS",     L"rdmsr",    L"rdpmc",    L"rdtsc",    L"ret",      L"rsm",       L"rsqrtPS",  L"sahf",
  L"salc",      L"sbb",      L"scasb",    L"set%cc",   L"shld",     L"shrd",      L"shufPS",   L"sqrtPS",
  L"stc",       L"std",      L"sti",      L"stosb",    L"sub",      L"subPS",     L"syscall",  L"sysenter",
  L"sysexit",   L"test",     L"ucomisS",  L"unpckhPS", L"unpcklPS", L"wait",      L"wbinvd",   L"wrmsr",
  L"xadd",      L"xchg",     L"xlat",     L"xor",      L"xorPS",
  L"cwde/cbw",     L"cdq/cwd",         L"pop     %seg",           L"popad/popa",
  #ifdef _WIN64
    L"popfq/popf",   L"push    %seg",    L"pushad/pusha",           L"pushfq/pushf",
  #else
    L"popfd/popf",   L"push    %seg",    L"pushad/pusha",           L"pushfd/pushf",
  #endif
  L"outsd/outsw",  L"insd/insw",       L"movsd/movsw",            L"cmpsd/cmpsw",
  L"lodsd/lodsw",  L"stosd/stosw",     L"scasd/scasw",            L"maskmovq/maskmovdqu",
  L"movd///movq",  L"movntq/movntdq",  L"movq//movdq2q/movq2dq",  L"movq/movdqa//movdqu",
  L"movuPS//movPS/movPS",                    L"prefetch|prefetchw",
  L"pshufw/pshufd/pshuflw/pshufhw",          L"sgdt|sidt|lgdt|lidt|smsw||lmsw|invlpg",
  L"sldt|str|lldt|ltr|verr|verw",            L"||||bt|bts|btr|btc",
  L"||psrld||psrad||pslld",                  L"||psrlq|psrldq|||psllq|pslldq",
  L"||psrlw||psraw||psllw",                  L"cvtdq2ps/cvtps2dq//cvttps2dq",
  L"cvtPs2Pd/cvtPd2Ps/cvtPd2Ps",             L"cvttpd2dq//cvtpd2dq/cvtdq2pd",
  L"add|or|adc|sbb|and|sub|xor|cmp",         L"inc|dec|call|call|jmp|jmp|push",
  L"rol|ror|rcl|rcr|shl|shr|sal|sar",        L"test||not|neg|mul|imul|div|idiv",
  L"add|mul|com|comp|sub|subr|div|divr|add|mul|com|comp|subr|sub|divr|div",
  L"movhlPS:movlPS/movlPS/movddup/movsldup",
  L"movlhPS:movhPS/movhPS/movlhPS:movhPS/movshdup",
  L"prefetchnta|prefetcht0|prefetcht1|prefetcht2|prefetcht3",
  L"fxsave|fxrstor|ldmxcsr|stmxcsr||lfence|mfence|sfence:clflush",
  L"ld||st|stp|ldenv|ldcw|nstenv|nstcw|ld|xch|nop|stp1|chs-abs---tst-xam|ld1-ldl2t-ldl2e-ldpi-ldlg2-ldln2-ldz|2xm1-yl2x-ptan-patan-xtract-prem1-decstp-incstp|prem-yl2xp1-sqrt-sincos-rndint-scale-sin-cos",
  L"iadd|imul|icom|icomp|isub|isubr|idiv|idivr|cmovb|cmove|cmovbe|cmovu||ucompp",
  L"ild|isttp|ist|istp||ld||stp|cmovnb|cmovne|cmovnbe|cmovnu|neni-ndisi-nclex-ninit-nsetpm|ucomi|comi",
  L"ld|isttp|st|stp|rstor||nsave|nstsw|free|xch4|st|stp|ucom|ucomp",
  L"iadd|imul|icom|icomp|isub|isubr|idiv|idivr|addp|mulp|comp5|compp|subrp|subp|divrp|divp",
  L"ild|isttp|ist|istp|bld|ild|bstp|istp|freep|xch7|stp8|stp9|nstsw  ax|ucomip|comip",
  L"andPS",
  L"addsubpd/addsubpd/addsubps",
  L"haddpd/haddpd/haddps",
  L"hsubpd/hsubpd/hsubps",
  L"lddqu//lddqu"
};

typedef struct tagNowStruct
{
  BYTE Code;
  LPCWSTR Label;
} NOW_STRUCT;

const int NowLabelArrayLength = 24;
const NOW_STRUCT CNowLabels[] = 
{
  { 0x0c, L"pi2fw" },
  { 0x0d, L"pi2fd" },
  { 0x1c, L"pf2iw" },
  { 0x1d, L"pf2id" },
  { 0x8a, L"pfnacc" },
  { 0x8e, L"pfpnacc" },
  { 0x90, L"pfcmpge" },
  { 0x94, L"pfmin" },
  { 0x96, L"pfrcp" },
  { 0x97, L"pfrsqrt" },
  { 0x9a, L"pfsub" },
  { 0x9e, L"pfadd" },
  { 0xa0, L"pfcmpgt" },
  { 0xa4, L"pfmax" },
  { 0xa6, L"pfrcpit1" },
  { 0xa7, L"pfrsqit1" },
  { 0xaa, L"pfsubr" },
  { 0xae, L"pfacc" },
  { 0xb0, L"pfcmpeq" },
  { 0xb4, L"pfmul" },
  { 0xb6, L"pfrcpit2" },
  { 0xb7, L"pmulhrw" },
  { 0xbb, L"pswapd" },
  { 0xbf, L"pavgusb" }
};

// one byte opcode index into opcode label array
const BYTE COpCodeLabelIndex[256] =
{
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xc3, 0xc0, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0xc3, 0x00,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0xc3, 0xc0, 0xa2, 0xa2, 0xa2, 0xa2, 0xa2, 0xa2, 0xc3, 0xc0,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x24, 0xad, 0xad, 0xad, 0xad, 0xad, 0xad, 0x00, 0x25,
  0xbc, 0xbc, 0xbc, 0xbc, 0xbc, 0xbc, 0x00, 0x01, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x00, 0x04,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26,
  0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c,
  0xc4, 0xc1, 0x0b, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x97, 0x2c, 0x97, 0x2c, 0x2f, 0xc7, 0x5a, 0xc6,
  0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
  0xde, 0xde, 0xde, 0xde, 0xb2, 0xb2, 0xba, 0xba, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x3c, 0x4a, 0x7c,
  0x56, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xbe, 0xbf, 0x13, 0xb6, 0xc5, 0xc2, 0xa0, 0x39,
  0x4a, 0x4a, 0x4a, 0x4a, 0x52, 0xc8, 0x1c, 0xc9, 0xb2, 0xb2, 0xac, 0xcb, 0x42, 0xca, 0xa3, 0xcc,
  0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a,
  0xe0, 0xe0, 0x9d, 0x9d, 0x3e, 0x3b, 0x4a, 0x4a, 0x29, 0x3d, 0x9d, 0x9d, 0x31, 0x30, 0x33, 0x35,
  0xe0, 0xe0, 0xe0, 0xe0, 0x03, 0x02, 0xa1, 0xbb, 0xe2, 0xe7, 0xe8, 0xe9, 0xe2, 0xea, 0xeb, 0xec,
  0x45, 0x44, 0x43, 0x37, 0x2d, 0x2d, 0x59, 0x59, 0x13, 0x38, 0x38, 0x38, 0x2d, 0x2d, 0x59, 0x59,
  0x00, 0x32, 0x00, 0x00, 0x2b, 0x18, 0xe1, 0xe1, 0x14, 0xa9, 0x16, 0xab, 0x15, 0xaa, 0xdf, 0xdf
};

// two byte (0x0f 0xxx) opcode index into opcode label array
const BYTE COpCodeLabelIndex0f[256] =
{
  0xd6, 0xd5, 0x3a, 0x46, 0x00, 0xaf, 0x17, 0x41, 0x34, 0xb7, 0x00, 0x00, 0x00, 0xd3, 0x2a, 0x00,
  0xd2, 0xd2, 0xe3, 0x4e, 0xb5, 0xb4, 0xe4, 0x4d, 0xe5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56,
  0x4a, 0x4a, 0x4a, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x4b, 0x21, 0x51, 0x23, 0x22, 0xb3, 0x1f,
  0xb8, 0x9c, 0x9a, 0x9b, 0xb0, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19, 0x19,
  0x4f, 0xa8, 0x9f, 0x99, 0xed, 0x09, 0x58, 0xbd, 0x07, 0x55, 0xdc, 0xdb, 0xae, 0x49, 0x27, 0x48,
  0x93, 0x95, 0x94, 0x5c, 0x6d, 0x6f, 0x6e, 0x5d, 0x8f, 0x92, 0x90, 0x5b, 0x96, 0x91, 0x4c, 0xd1,
  0xd4, 0xda, 0xd8, 0xd9, 0x6a, 0x6c, 0x6b, 0x28, 0x00, 0x00, 0x00, 0x00, 0xef, 0xf0, 0xce, 0xd1,
  0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
  0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xa4,
  0xc3, 0xc0, 0x20, 0x0f, 0xa5, 0xa5, 0x1d, 0x1d, 0xc3, 0xc0, 0x9e, 0x12, 0xa6, 0xa6, 0xe6, 0x2c,
  0x1d, 0x1d, 0x47, 0x11, 0x3f, 0x40, 0x54, 0x54, 0x00, 0x00, 0xd7, 0x10, 0x0c, 0x0d, 0x53, 0x53,
  0xb9, 0xb9, 0x1b, 0x50, 0x71, 0x70, 0xa7, 0x1e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
  0xee, 0x86, 0x84, 0x85, 0x60, 0x7a, 0xd0, 0x77, 0x8c, 0x8d, 0x76, 0x66, 0x63, 0x64, 0x74, 0x67,
  0x68, 0x83, 0x82, 0x69, 0x78, 0x79, 0xdd, 0xcf, 0x8a, 0x8b, 0x75, 0x7d, 0x61, 0x62, 0x73, 0x98,
  0xf1, 0x81, 0x7f, 0x80, 0x7b, 0x72, 0x7e, 0xcd, 0x87, 0x8e, 0x88, 0x89, 0x5e, 0x65, 0x5f, 0x00
};

#endif