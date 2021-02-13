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
    push 9
    pop rdi
    pop rax
    cmp rax, rdi
    setne al
    movzb rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lend0
    push 5
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.Lend0:
    pop rax
    push 3
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
    mov rsp, rbp
    pop rbp
    ret
