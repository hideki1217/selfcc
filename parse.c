#include "selfcc.h"

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>



Token *tkstream;
char *user_input;
LVar *locals;

Token *new_Token(TokenKind kind,Token* cur,char *str,int len){
    Token *token=calloc(1,sizeof(Token));
    token->kind=kind;
    token->str=str;
    token->len=len;
    cur->next =token;
    return token;
}
//宣言済み変数一覧に存在するか確認
LVar *find_lvar(Token *token){
    for(LVar *var=locals;var;var=var->next){
        if(var->len==token->len 
        && memcmp(token->str, var->name,var->len)==0){
            return var;
        }
    }
    return NULL;
}
//なければ作る。あれば二重に定義したことをエラー
LVar *add_lvar(Token *token,Mold *mold){
    LVar* res=find_lvar(token);
    if( res==NULL ){
        res=new_LVar(token,mold);
        locals=res;
        return res;
    }
    else
        error_at(token->str,"同名の変数が既に定義されています");
}
//宣言済み変数一覧になければエラー
LVar *get_lvar(Token *token){
    LVar* res=find_lvar(token);
    if( res==NULL )
        error_at(token->str,"宣言されていない変数です。");
    else
        return res;
}

Mold *new_Mold(char* name,int len,int size){
    Mold *mold=calloc(1,sizeof(Mold));
    mold->name=name;
    mold->len=len;
    mold->size=size;
    return mold;
}
Mold *find_mold(Token *token){
    for(Mold *mold=molds;
        mold;
        mold=mold->next)
    {
        if(mold->len==token->len 
        && memcmp(mold->name,token->str,mold->len)==0){
            return mold;
        }
    }
    return NULL;
}
Mold *molds;

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
bool consume(char *op) {
    if (tkstream->kind != TK_RESERVED ||
        strlen(op) != tkstream->len ||
        memcmp(tkstream->str,op,tkstream->len)!=0){
        return false;
    }
    tkstream=tkstream->next;
    return true;
}

//次の文字がopかどうかを判定、文字は進めない
bool check(char *op){
    return strlen(op) == tkstream->len &&
        memcmp(tkstream->str,op,tkstream->len)==0;
}

//変数ならばそれを返して一つ進める
Token *consume_ident(){
    if (tkstream->kind != TK_IDENT)
        return NULL;
    Token *ident=tkstream;
    tkstream=tkstream->next;
    return ident;
}

Token *expect_ident(){
    if (tkstream->kind != TK_IDENT)
        error_at(tkstream->str,"変数もしくは関数ではありません");
    Token *ident=tkstream;
    tkstream=tkstream->next;
    return ident;
}

Token *expect_var(){
    if(tkstream->kind!=TK_IDENT && *(tkstream->next->str)!='('){
        error_at(tkstream->str,"変数ではありません");
    }
    Token *ident=tkstream;
    tkstream=tkstream->next;
    return ident;
}

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op){
    if (tkstream->kind != TK_RESERVED || tkstream->str[0] != op){
        error_at(tkstream->str,"\"%c\"ではありません",op);
    }
    tkstream=tkstream->next;
}

Mold *expect_mold(){
    if (tkstream->kind != TK_IDENT ){
        error_at(tkstream->str,"型名ではありません");
    }
    Mold *mold=find_mold(tkstream);
    if(mold ==NULL)
        error_at(tkstream->str,"宣言されていない型です。");
    tkstream=tkstream->next;
    return mold;
}
bool *check_mold(){
    Mold *mold=find_mold(tkstream);
    if(mold ==NULL)
        return false;
    return true;
}
// checkした後に実行すべき
Mold *consume_mold(){
    if (tkstream->kind != TK_IDENT ){
        error_at(tkstream->str,"型名ではありません");
    }
    Mold *mold=find_mold(tkstream);
    tkstream=tkstream->next;
    return mold;
}

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number(){
    if (tkstream ->kind != TK_NUM){
        error_at(tkstream->str,"数ではありません");
    }
    int val=tkstream->val;
    tkstream= tkstream->next;
    return val;
}

bool at_eof() {
  return tkstream->kind == TK_EOF;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

int is_alp(char c){
    return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         (c == '_');
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
        //制御構文
        if( memcmp(p,"return",6)==0 && !is_alnum(p[6])) { 
            cur=new_Token(TK_RESERVED,cur,p,6);
            p+=6;
            continue;
        }

        if( memcmp(p,"while",5)==0 && !is_alnum(p[5])){
            cur=new_Token(TK_RESERVED,cur,p,5);
            p+=5;
            continue;
        }

        if( memcmp(p,"else",4)==0 && !is_alnum(p[4])){
            cur=new_Token(TK_RESERVED,cur,p,4);
            p+=4;
            continue;
        }

        if( memcmp(p,"for",3)==0 && !is_alnum(p[3])){
            cur=new_Token(TK_RESERVED,cur,p,3);
            p+=3;
            continue;
        }

        if( memcmp(p,"if",2)==0 && !is_alnum(p[2])){
            cur=new_Token(TK_RESERVED,cur,p,2);
            p+=2;
            continue;
        }

        //確実に非変数名なもの
        if( memcmp(p,"!=",2)==0 || memcmp(p,"==",2)==0 
         || memcmp(p,"<=",2)==0 || memcmp(p,">=",2)==0){
            cur=new_Token(TK_RESERVED,cur,p,2);
            p+=2;
            continue;
        }

        if( *p == '+' || *p == '-' 
         || *p == '*' || *p == '/'
         || *p == '(' || *p == ')'
         || *p == '>' || *p == '<'
         || *p == '=' || *p == ';'
         || *p == '{' || *p == '}'
         || *p == ',' || *p == '*'
         || *p == '&'){
            cur=new_Token(TK_RESERVED,cur,p++,1);
            continue;
        }

        //数字
        if(isdigit(*p)){
            char* q=p;
            cur=new_Token(TK_NUM,cur,p,1);//数字の長さを1で適当に初期化
            cur->val =strtol(p,&p,10);
            cur->len=p-q;
            continue;
        }

        //変数名
        if( is_alp(*p) ){
            char *q=p;
            while(1){
                p++;
                if( !is_alnum(*p) )
                    break;
            }
            cur=new_Token(TK_IDENT,cur,q,p-q);
            continue;
        }

        error_at(p,"トークナイズできません");
    }

    new_Token(TK_EOF,cur,p,0);
    return head.next;
}