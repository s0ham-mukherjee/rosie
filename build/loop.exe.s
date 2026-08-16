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
    mov rax, 0
    push rax
    pop rax
    mov [rbp-8], rax
    mov rax, 1
    push rax
    pop rax
    mov [rbp-16], rax
.L_while_0:
    mov rax, [rbp-16]
    push rax
    mov rax, 10
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    setle al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz .L_wend_1
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-8], rax
    push rax
    pop rax
    mov rax, [rbp-16]
    push rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-16], rax
    push rax
    pop rax
    jmp .L_while_0
.L_wend_1:
    mov rax, [rbp-8]
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
