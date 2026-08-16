.intel_syntax noprefix
.text
.globl constantTest
constantTest:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    mov rax, 10
    push rax
    mov rax, 20
    push rax
    mov rax, 3
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-8], rax
    mov rax, [rbp-8]
    push rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    imul rax, rbx
    push rax
    mov rax, 0
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-16], rax
    mov rax, [rbp-16]
    push rax
    pop rax
    jmp .L_constantTest_end
    mov rax, 999
    push rax
    pop rax
    mov [rbp-24], rax
.L_constantTest_end:
    add rsp, 128
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
.globl main
main:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    sub rsp, 32
    call constantTest
    add rsp, 32
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
