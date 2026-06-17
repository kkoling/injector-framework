.intel_syntax noprefix

.section .data
.global SSN
.global JMP_ADDR

SSN:
    .long 0

JMP_ADDR:
    .quad 0

.section .text

.global InitSyscall
InitSyscall:
    mov dword ptr [rip + SSN], ecx
    mov qword ptr [rip + JMP_ADDR], rdx
    ret

.global DirectSyscall
DirectSyscall:
    mov r10, rcx
    mov eax, dword ptr [rip + SSN]
    syscall
    ret

.global IndirectSyscall
IndirectSyscall:
    mov r10, rcx
    mov eax, dword ptr [rip + SSN]
    jmp qword ptr [rip + JMP_ADDR]

.global DirectSyscallSSN
DirectSyscallSSN:
    mov r10, rdx
    mov eax, ecx
    mov rcx, rdx
    mov rdx, r8
    mov r8,  r9
    mov r9,  [rsp + 0x28]
    add rsp, 0x8
    syscall
    sub rsp, 0x8
    ret

.global ProxySyscall
ProxySyscall:
    mov rbx, rdx
    mov rax, [rbx]
    mov r11, [rbx + 0x60]
    mov rcx, [rbx + 0x08]
    cmp r11, 1
    jl .CALL
    mov rdx, [rbx + 0x10]
    cmp r11, 2
    jl .CALL
    mov r8,  [rbx + 0x18]
    cmp r11, 3
    jl .CALL
    mov r9,  [rbx + 0x20]
    cmp r11, 4
    jl .CALL
    mov r10, [rbx + 0x28]
    mov [rsp + 0x28], r10
    cmp r11, 5
    jl .CALL
    mov r10, [rbx + 0x30]
    mov [rsp + 0x30], r10
    cmp r11, 6
    jl .CALL
    mov r10, [rbx + 0x38]
    mov [rsp + 0x38], r10
    cmp r11, 7
    jl .CALL
    mov r10, [rbx + 0x40]
    mov [rsp + 0x40], r10
    cmp r11, 8
    jl .CALL
    mov r10, [rbx + 0x48]
    mov [rsp + 0x48], r10
    cmp r11, 9
    jl .CALL
    mov r10, [rbx + 0x50]
    mov [rsp + 0x50], r10
    cmp r11, 10
    jl .CALL
    mov r10, [rbx + 0x58]
    mov [rsp + 0x58], r10
.CALL:
    jmp rax