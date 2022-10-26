; ***************************************************************
;  IncEx.asm                 version: 1.0.0  ·  date: 2010-01-10
;  -------------------------------------------------------------
;  InterlockedIncrementEx but with exact return value
;  -------------------------------------------------------------
;  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
; ***************************************************************

_TEXT SEGMENT

PUBLIC _InterlockedIncrementEx
_InterlockedIncrementEx PROC

    ; parameter "int *value" is passed in rcx

    mov rdx, rcx
    mov eax, dword ptr [rdx]
  Again:
    mov ecx, eax
    inc ecx
    lock cmpxchg dword ptr [rdx], ecx
    jnz Again

    ret
_InterlockedIncrementEx ENDP

_TEXT ENDS

END