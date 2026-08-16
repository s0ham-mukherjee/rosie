.intel_syntax noprefix
.text
.globl add
add:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    mov [rbp-8], rcx
    mov [rbp-16], rdx
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    jmp .L_add_end
.L_add_end:
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
    mov rax, 10
    push rax
    pop rax
    mov [rbp-8], rax
    mov rax, 20
    push rax
    pop rax
    mov [rbp-16], rax
    mov rax, [rbp-16]
    push rax
    pop rdx
    mov rax, [rbp-8]
    push rax
    pop rcx
    sub rsp, 32
    call add
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
