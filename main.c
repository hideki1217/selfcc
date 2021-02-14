#include "selfcc.h"
#include<stdio.h>


int main(int argc,char **argv){
    if(argc != 2){
        error("引数の個数が不正です\n");
        return 1;
    }

    // Grobal変数に値をセット
    user_input=argv[1];
    locals=NULL;
    token=tokenize(user_input);
    nullNode=(Node*)new_NumNode(1);
    pointargReg[0]="rdi";//関数引数の規約
    pointargReg[1]="rsi";
    pointargReg[2]="rdx";
    pointargReg[3]="rcx";
    pointargReg[4]="r8d";
    pointargReg[5]="r9d";
    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保　
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for ( int i=0; code[i] ; i++){
        gen(code[i]);

        // 各式の評価結果がスタックに残っているので、pop
        printf("    pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
