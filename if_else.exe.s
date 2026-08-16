.intel_syntax noprefix
.text
.globl max
max:
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
    cmp rax, rbx
    setg al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz .L_else_0
    mov rax, [rbp-8]
    push rax
    pop rax
    jmp .L_max_end
    jmp .L_endif_1
.L_else_0:
    mov rax, [rbp-16]
    push rax
    pop rax
    jmp .L_max_end
.L_endif_1:
.L_max_end:
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
    mov rax, 17
    push rax
    pop rdx
    mov rax, 42
    push rax
    pop rcx
    sub rsp, 32
    call max
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
