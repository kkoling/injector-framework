.data
SSN DWORD 0h
JMP_ADDR QWORD 0h

.code

PUBLIC InitSyscall
InitSyscall PROC
	mov SSN, ecx
	mov JMP_ADDR, rdx
	ret
InitSyscall ENDP

PUBLIC DirectSyscall
DirectSyscall PROC
    mov r10, rcx
    mov eax, SSN
    syscall
    ret
DirectSyscall ENDP

PUBLIC IndirectSyscall
IndirectSyscall PROC
	mov r10, rcx
	mov eax, SSN				
	jmp qword ptr [JMP_ADDR]	
IndirectSyscall ENDP

PUBLIC DirectSyscallSSN
DirectSyscallSSN PROC
    mov r10, rdx
    mov eax, ecx
    mov rcx, rdx         
    mov rdx, r8                  
    mov r8, r9                
    mov r9, [rsp + 28h]
    add rsp, 8h 
    syscall
    sub rsp, 8h
    ret
DirectSyscallSSN ENDP

PUBLIC ProxySyscall
ProxySyscall PROC
    mov rbx, rdx
    mov rax, [rbx]
    mov r11, [rbx + 60h]
    mov rcx, [rbx + 8h]
    cmp r11, 1
    jl CALL_LABEL
    mov rdx, [rbx + 10h]
    cmp r11, 2
    jl CALL_LABEL
    mov r8,  [rbx + 18h]
    cmp r11, 3
    jl CALL_LABEL
    mov r9,  [rbx + 20h]
    cmp r11, 4
    jl CALL_LABEL
    mov r10, [rbx + 28h]
    mov [rsp + 28h], r10
    cmp r11, 5
    jl CALL_LABEL
    mov r10, [rbx + 30h]
    mov [rsp + 30h], r10
    cmp r11, 6
    jl CALL_LABEL
    mov r10, [rbx + 38h]
    mov [rsp + 38h], r10
    cmp r11, 7
    jl CALL_LABEL
    mov r10, [rbx + 40h]
    mov [rsp + 40h], r10
    cmp r11, 8
    jl CALL_LABEL
    mov r10, [rbx + 48h]
    mov [rsp + 48h], r10
    cmp r11, 9
    jl CALL_LABEL
    mov r10, [rbx + 50h]
    mov [rsp + 50h], r10
    cmp r11, 10
    jl CALL_LABEL
    mov r10, [rbx + 58h]
    mov [rsp + 58h], r10
CALL_LABEL:
    jmp rax
ProxySyscall ENDP

end