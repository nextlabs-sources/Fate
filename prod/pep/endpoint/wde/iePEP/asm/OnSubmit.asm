;OnSubmit.asm 
;ml64 /c /D"x64" /D"RELEASE" /Fo".\OnSubmit.obj" OnSubmit.asm
;for move 32bit assemble to 64bit
public Fire_OnSubmit
.code
Fire_OnSubmit proc
push RAX			; keep rax
push RCX			; keep the first paramter, in 64bit ,the first four paramters will passed by RCX,RDX,R8,R9, and then use stack
mov  RCX,[RBP+10h]	;keep this pointer address into RAX 
pop  RAX			;set the first paramter into RAX
call RAX			; invoke the function
mov  RCX,RAX		; recover RCX
pop  RAX
ret	 8
Fire_OnSubmit endp
end