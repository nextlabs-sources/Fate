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
    push rbx
    mov rbx, 1111111111111111h
    mov rdi, 2222222222222222h
    mov rdx, [rsp+28h]
    mov rcx, 250  ; IN_USE_COUNT
    xor rax, rax
  @loop:
    lock cmpxchg [rdi], rdx
    jz @success
    add rbx, 16
    add rdi, 8
    xor rax, rax
    loop @loop
    jmp @quit
  @success:
    mov [rsp+28h], rbx
  @quit:
    pop rbx
    pop rax
    pop rcx
    pop rdx
    pop rdi
    db 0ffh
    db 025h
    dd 0
    dq 0

InUseStub ENDP

_TEXT ENDS

END