.intel_syntax noprefix
.text
.globl main
main:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    mov qword ptr [rbp-8], 0
    mov rax, 3
    push rax
    pop rax
    push rax
    pop rax
    mov rax, 4
    push rax
    pop rax
    push rax
    pop rax
    mov qword ptr [rbp-16], 0
    mov rax, 5
    push rax
    pop rax
    push rax
    pop rax
    mov rax, 6
    push rax
    pop rax
    push rax
    pop rax
    pop rax
    mov [rbp-24], rax
    mov rax, [rbp-24]
    push rax
    pop rax
    jmp .L_main_end
    xor eax, eax
.L_main_end:
    add rsp, 128
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
