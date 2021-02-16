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
    
    //プリミティブ型
    types=new_Type("int",3,4);
    types=new_Type("void",4,8);

    program();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");//TODO:ここの意味を把握
    
    for ( Node *now=code; now ; now=now->next){
        gen(now);
    }
    return 0;
}
