#include<stdio.h>
#include<stdlib.h>

int main(int argc,char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が不正です\n");
        return 1;
    }

    char* p=argv[1];

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %ld\n",strtol(p,&p,10));
    while(*p){
        if ( p[0] == '+'){
            p++;
            printf("    add rax, %ld\n",strtol(p,&p,10));
            continue;
        }
        
        if( p[0] == '-' ){
            p++;
            printf("    sub rax, %ld\n",strtol(p,&p,10));
            continue;
        }

        fprintf(stderr,"予期しない文字が入力されました。");
        return 1;
    }

    printf("    ret\n");
    return 0;
}