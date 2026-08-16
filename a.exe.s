.intel_syntax noprefix
.text
.globl compute
compute:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    mov [rbp-8], rcx
    mov rax, 0
    push rax
    pop rax
    mov [rbp-16], rax
    mov rax, 1
    push rax
    pop rax
    mov [rbp-24], rax
.L_for_0:
    mov rax, [rbp-24]
    push rax
    mov rax, [rbp-8]
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    setle al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz .L_fend_1
    mov rax, [rbp-16]
    push rax
    mov rax, [rbp-24]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-16], rax
    push rax
    pop rax
    mov rax, [rbp-24]
    push rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-24], rax
    push rax
    pop rax
    jmp .L_for_0
.L_fend_1:
    mov rax, [rbp-16]
    push rax
    pop rax
    jmp .L_compute_end
.L_compute_end:
    add rsp, 128
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
.globl categorize
categorize:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 128
    mov [rbp-8], rcx
    mov rax, 0
    push rax
    pop rax
    mov [rbp-16], rax
    mov rax, [rbp-8]
    push rax
    pop rax
    cmp rax, 10
    je .L_case_1
    cmp rax, 20
    je .L_case_2
    jmp .L_default_3
    jmp .L_swend_0
.L_case_1:
    mov rax, 100
    push rax
    pop rax
    mov [rbp-16], rax
    push rax
    pop rax
    jmp .L_swend_0
.L_case_2:
    mov rax, 200
    push rax
    pop rax
    mov [rbp-16], rax
    push rax
    pop rax
    jmp .L_swend_0
.L_default_3:
    mov rax, 999
    push rax
    pop rax
    mov [rbp-16], rax
    push rax
    pop rax
    jmp .L_swend_0
.L_swend_0:
    mov rax, [rbp-16]
    push rax
    pop rax
    jmp .L_categorize_end
.L_categorize_end:
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
    pop rcx
    sub rsp, 32
    call compute
    add rsp, 32
    push rax
    pop rax
    mov [rbp-8], rax
    mov rax, 20
    push rax
    pop rcx
    sub rsp, 32
    call categorize
    add rsp, 32
    push rax
    pop rax
    mov [rbp-16], rax
    mov rax, [rbp-8]
    push rax
    mov rax, 50
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    setg al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz .L_tern_else_0
    mov rax, 1
    push rax
    jmp .L_tern_end_1
.L_tern_else_0:
    mov rax, 0
    push rax
.L_tern_end_1:
    pop rax
    mov [rbp-24], rax
    mov rax, 0
    push rax
    pop rax
    mov [rbp-32], rax
.L_dowhile_2:
    mov rax, [rbp-32]
    push rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    mov [rbp-32], rax
    push rax
    pop rax
    mov rax, [rbp-32]
    push rax
    mov rax, 5
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    setl al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jnz .L_dowhile_2
.L_dend_3:
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    mov rax, [rbp-24]
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    mov rax, [rbp-32]
    push rax
    pop rbx
    pop rax
    add rax, rbx
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
