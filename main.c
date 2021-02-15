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
    tkstream=tokenize(user_input);
    nullNode=(Node*)new_NumNode(1);
    pointargReg[0]="rdi";//関数引数の規約
    pointargReg[1]="rsi";
    pointargReg[2]="rdx";
    pointargReg[3]="rcx";
    pointargReg[4]="r8";
    pointargReg[5]="r9";
    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");//TODO:ここの意味を把握

    for ( Node *now=code; now ; now=now->next){
        gen(now);
    }
    return 0;
}
