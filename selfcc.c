#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

typedef enum{
    TK_RESERVED,
    TK_NUM,
    TK_EOF
}TokenKind;

typedef struct Token Token;

struct Token{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

Token *token;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op){
        return false;
    }
    token=token->next;
    return true;
}

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op){
    if (token->kind != TK_RESERVED || token->str[0] != op){
        error("\"%c\"ではありません",op);
    }
    token=token->next;
}

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number(){
    if (token ->kind != TK_NUM){
        error("数ではありません");
    }
    int val=token->val;
    token = token->next;
    return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_Token(TokenKind kind,Token* cur,char *str){
    Token *token=calloc(1,sizeof(Token));
    token->kind=kind;
    token->str=str;
    cur->next =token;
    return token;
}

Token *tokenize(char *p){
    Token head;
    head.next=NULL;
    Token *cur=&head;
    while(*p){
        if (isspace(*p)){
            p++;
            continue;
        }

        if( *p == '+' || *p == '-'){
            cur=new_Token(TK_RESERVED,cur,p++);
            continue;
        }

        if(isdigit(*p)){
            cur=new_Token(TK_NUM,cur,p);
            cur->val =strtol(p,&p,10);
            continue;
        }

        error("トークナイズできません");
    }

    new_Token(TK_EOF,cur,p);
    return head.next;
}


int main(int argc,char **argv){
    if(argc != 2){
        error("引数の個数が不正です\n");
        return 1;
    }

    token=tokenize(argv[1]);

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("    mov rax, %d\n",expect_number());

    while(!at_eof()){
        if(consume('+')){
            printf("    add rax, %d\n",expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n",expect_number());
    }

    printf("    ret\n");
    return 0;
}