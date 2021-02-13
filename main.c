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
    nullNode=new_Node_num(1);
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
