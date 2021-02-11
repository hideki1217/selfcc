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
Token *new_Token(TokenKind kind,Token* cur,char *str){
    Token *token=calloc(1,sizeof(Token));
    token->kind=kind;
    token->str=str;
    cur->next =token;
    return token;
}

Token *token;
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc,char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);

    int pos=loc-user_input;
    fprintf(stderr,"%s\n",user_input);
    fprintf(stderr,"%*s",pos," ");
    fprintf(stderr,"^ ");
    fprintf(stderr,fmt,ap);
    fprintf(stderr,"\n");
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
        error_at(token->str,"\"%c\"ではありません",op);
    }
    token=token->next;
}

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number(){
    if (token ->kind != TK_NUM){
        error_at(token->str,"数ではありません");
    }
    int val=token->val;
    token = token->next;
    return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
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

        if( *p == '+' || *p == '-' 
         || *p == '*' || *p == '/'
         || *p == '(' || *p == ')'){
            cur=new_Token(TK_RESERVED,cur,p++);
            continue;
        }

        if(isdigit(*p)){
            cur=new_Token(TK_NUM,cur,p);
            cur->val =strtol(p,&p,10);
            continue;
        }

        error_at(p,"トークナイズできません");
    }

    new_Token(TK_EOF,cur,p);
    return head.next;
}

// region NFT
typedef enum{
    ND_ADD,//"+"
    ND_SUB,//"-"
    ND_MUL,//"*"
    ND_DIV,//"/"
    ND_NUM
}NodeKind;

typedef struct Node Node;

struct Node{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};
Node *new_Node(NodeKind kind,Node *lhs,Node* rhs){
    Node *node=calloc(1,sizeof(Node));
    node->kind=kind;
    node->lhs=lhs;
    node->rhs=rhs;
    return node;
}
Node *new_Node_num(int val){
    Node *node=calloc(1,sizeof(Node));
    node->kind=ND_NUM;
    node->val=val;
    return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *expr(){
    Node *node=mul();

    while(1){
        if (consume('+'))
            node=new_Node(ND_ADD,node,mul());
        else if (consume('-'))
            node=new_Node(ND_SUB,node,mul());
        else
            return node;
    }
}
Node *mul(){
    Node *node=primary();

    while(1){
        if (consume('*'))
            node=new_Node(ND_MUL,node,primary());
        else if (consume('/'))
            node=new_Node(ND_DIV,node,primary());
        else
            return node;
    }
}
Node *primary(){
    if (consume('(')){
        Node *node=expr();
        expect(')');
        return node;
    }
    return new_Node_num(expect_number());
}

// endregion

void gen(Node *node){
    if (node->kind == ND_NUM){
        printf("    push %d\n",node->val);
        return;
    }
    
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    default:
        break;
    }

    printf("    push rax\n");
}


int main(int argc,char **argv){
    if(argc != 2){
        error("引数の個数が不正です\n");
        return 1;
    }

    user_input=argv[1];
    token=tokenize(user_input);
    Node *node=expr();

    printf(".Intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}