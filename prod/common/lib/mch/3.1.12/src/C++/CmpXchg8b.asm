; ***************************************************************
;  CmpXchg8b.asm             version: 1.0.0  ·  date: 2010-01-10
;  -------------------------------------------------------------
;  call asm instruction cmpxchg8b
;  -------------------------------------------------------------
;  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
; ***************************************************************

_TEXT SEGMENT

PUBLIC _CmpXchg8b
_CmpXchg8b PROC
                               ; stack
                               ; 40 64 -- (unused)
                               ;       44 68 local1, HIGH DWORD of destination value
                               ;       40 64 local2, LOW DWORD of destination value
                               ; 38 56 length
                               ; 30 48 destination
                               ; 28 40 source
                               ; 20 32 return address
    push  rsi                  ; 18 24 rsi
    push  rdi                  ; 10 16 rdi
    push  rbx                  ; 08 8  rbx
    push  rbp                  ; 00 0  rbp

    mov   [rsp+28h], rcx       ; fill parameters in
    mov   [rsp+30h], rdx
    mov   [rsp+38h], r8

    mov   rbp, [rsp+30h]       ; move address of destination to rbp
    mov   rax, rbp             ; rax gets address of destination
    mov   edx, [rax+04h]       ; edx gets value at destination+4, HIGH DWORD
    mov   eax, [rax]           ; eax gets value at destination, LOW DWORD
                               ; EDX:EAX is 8 bytes that destination points to

  TRYAGAIN:
    mov   [rsp+40h], eax       ; local2 gets EAX, LOW DWORD
    mov   [rsp+44h], edx       ; local1 gets EDX, HIGH DWORD

    mov   rsi, [rsp+28h]       ; rsi gets source address parameter
    lea   rdi, [rsp+40h]       ; rdi gets address of first byte of local 2
    mov   rcx, [rsp+38h]       ; rcx gets length
    cld                        ; clear the direction flag
                               ; thus, string instructions increment esi/edi
    rep   movsb                ; repeat cx times a move of a byte from [esi] to [edi] (cx will be 1-8)
                               ; thus, the locals are populated with SOURCE

; we want to write to the destination by using "cmpxchg8b"; this asm instruction always writes 8 bytes
; but in reality we don't want to change 8 bytes, but somewhat less
; so we use a temp buffer of 8 bytes, which are prefilled with the current first 8 bytes of the destination
; if we would write the buffer in this form back to the destination with cmpxchg8b, nothing would change
; but we do want to change something, so we overwrite the first "length" bytes of our temp buffer with the source content
; after this our temp buffer contains exactly what we want to have in the destination later
; the first "length" bytes shall be changed, the remaining bytes (8 - length) shall remain unchanged

    mov ebx, [rsp+40h]         ; ebx = LOW DWORD of Source value
    mov ecx, [rsp+44h]         ; ecx = HIGH DWORD of Source value

    lock cmpxchg8b qword ptr [rbp]
    ; if (edx:eax == dest)    if DESTINATION 8 bytes in EDX:EAX == original DESTINATION 8 bytes from parameter
    ;   dest = ecx:ebx          then [destination] = SOURCE 8 bytes in ECX:EBX atomically
    ; else
    ;   edx:eax = dest        EDX:EAX is re-set to [destination] 8 bytes atomically

    jnz TRYAGAIN

; cmpxchg8b works this way: we must tell it what value we believe is in "*dest";
; if what we believe is right, cmpxchg8b executes our write request;
; if what we believe is wrong, cmpxchg8b does not execute our write request;
; this sometimes is a useful logic for multi thread purposes
; in our specific case we always want to write, no matter what, so we have to loop until we're done
; basically most of the time cmpxchg8b will succeed the first time it is called here
; however, if two threads try to write to the destination at the same time, the 2nd thread's call to cmpxchg8b
; will fail and must be repeated

; if there was a "lock mov [destination], something" which supported writing of 8 bytes
; simultanously, I wouldn't use cmpxchg8b here; but as things are, cmpxchg8b is the only assembler
; instruction which can write more than 4 bytes atomatically; so I'm misusing it

    pop rbp
    pop rbx
    pop rdi
    pop rsi

    ret
_CmpXchg8b ENDP

_TEXT ENDS

END