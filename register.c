#include <stdio.h>
#include <stdlib.h>

#include "selfcc.h"

char *registry_for_arg(Type *tp, int i) {
    switch (i) {
        case 0:
            return rdi(tp);
        case 1:
            return rsi(tp);
        case 2:
            return rdx(tp);
        case 3:
            return rcx(tp);
        case 4:
            return r8(tp);
        case 5:
            return r9(tp);
        default:
            printf("いきすぎだよーー");
            exit(1);
    }
}
// 符号拡張しない
char *movzx2rax(Type *tp){
    switch(tp->size){
        case 1:
        case 2:
            return "movzx eax";
        case 4:
            return "mov eax";
        default:
            return "mov rax";
    }
}
char *movsx2rax(Type *tp){
    switch(tp->size){
        case 1:
        case 2:
            return "movsx eax";
        case 4:
            return "mov eax";
        default:
            return "mov rax";
    }
}
// 符号拡張しない
char *movzx2rdi(Type *tp){
    switch(tp->size){
        case 1:
        case 2:
            return "movzx edi";
        case 4:
            return "mov edi";
        default:
            return "mov rdi";
    }
}
char *movsx2rdi(Type *tp){
    switch(tp->size){
        case 1:
        case 2:
            return "movsx edi";
        case 4:
            return "mov edi";
        default:
            return "mov rdi";
    }
}



char *movsx(Type *tp){
    switch(tp->size){
        case 2:
        case 1:
            return "movsx";
        default:
            return "mov";
    }
}

char *movzx(Type *tp){
    switch(tp->size){
        case 2:
        case 1:
            return "movzx";
        default:
            return "mov";
    }
}


char *rax(Type *tp) {
    switch (tp->size) {
        case 1:
            return "al";
        case 2:
            return "ax";
        case 4:
            return "eax";
        default:
            return "rax";
    }
}
char *rbx(Type *tp) {
    switch (tp->size) {
        case 1:
            return "bl";
        case 2:
            return "bx";
        case 4:
            return "ebx";
        default:
            return "rbx";
    }
}
char *rcx(Type *tp) {
    switch (tp->size) {
        case 1:
            return "cl";
        case 2:
            return "cx";
        case 4:
            return "ecx";
        default:
            return "rcx";
    }
}
char *rdx(Type *tp) {
    switch (tp->size) {
        case 1:
            return "dl";
        case 2:
            return "dx";
        case 4:
            return "edx";
        default:
            return "rdx";
    }
}
char *rsi(Type *tp) {
    switch (tp->size) {
        case 1:
            return "sil";
        case 2:
            return "si";
        case 4:
            return "esi";
        default:
            return "rsi";
    }
}
char *rdi(Type *tp) {
    switch (tp->size) {
        case 1:
            return "dil";
        case 2:
            return "di";
        case 4:
            return "edi";
        default:
            return "rdi";
    }
}
char *r8(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r8b";
        case 2:
            return "r8w";
        case 4:
            return "r8d";
        default:
            return "r8";
    }
}
char *r9(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r9b";
        case 2:
            return "r9w";
        case 4:
            return "r9d";
        default:
            return "r9";
    }
}
char *r10(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r10b";
        case 2:
            return "r10w";
        case 4:
            return "r10d";
        default:
            return "r10";
    }
}
char *r11(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r11b";
        case 2:
            return "r11w";
        case 4:
            return "r11d";
        default:
            return "r11";
    }
}
char *r12(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r12b";
        case 2:
            return "r12w";
        case 4:
            return "r12d";
        default:
            return "r12";
    }
}
char *r13(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r13b";
        case 2:
            return "r13w";
        case 4:
            return "r13d";
        default:
            return "r13";
    }
}
char *r14(Type *tp) {
    switch (tp->size) {
        case 1:
            return "r14b";
        case 2:
            return "r14w";
        case 4:
            return "r14d";
        default:
            return "r14";
    }
}

char *sizeoption(Type *tp) {
    switch (tp->size) {
        case 1:
            return "BYTE PTR";
        case 2:
            return "WORD PTR";
        case 4:
            return "DWORD PTR";
        default:
            return "QWORD PTR";
    }
}
