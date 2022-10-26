; ***************************************************************
;  InUseStub.asm             version: 1.0.0  ·  date: 2010-01-10
;  -------------------------------------------------------------
;  asm stub for "in use" checking
;  -------------------------------------------------------------
;  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
; ***************************************************************

_TEXT SEGMENT

PUBLIC InUseStub
InUseStub PROC

    push rdi
    push rdx
    push rcx
    push rax
    lea rdi, [@inUseArray]
    mov rdx, [rsp+20h]
    mov rcx, 250  ; IN_USE_COUNT
    xor rax, rax
  @loop:
    lock cmpxchg [rdi+10h], rdx
    jz @success
    add rdi, 24
    xor rax, rax
    loop @loop
    jmp @quit
  @success:
    mov [rsp+20h], rdi
  @quit:
    pop rax
    pop rcx
    pop rdx
    pop rdi
    db 0ffh
    db 025h
    dd 0
    dq 0
  @inUseArray:

InUseStub ENDP

_TEXT ENDS

END