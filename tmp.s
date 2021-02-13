.Intel_syntax noprefix
.globl main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 208
    mov rax, rbp
    sub rax, 8
    push rax
    push 9
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 3
    pop rdi
    pop rax
    cmp rax, rdi
    sete al
    movzb rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lelse0
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 8
    pop rdi
    pop rax
    cmp rax, rdi
    sete al
    movzb rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lelse0
    push 3
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    jmp .Lend0
.Lelse0:
    push 5
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.Lend0:
    jmp .Lend1
.Lelse1:
    push 6
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.Lend1:
    pop rax
    mov rsp, rbp
    pop rbp
    ret
