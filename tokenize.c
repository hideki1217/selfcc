#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"

Token *tkstream;
char *user_input;
CC_Map_for_LVar *locals;
CC_AVLTree* globals;

Token *new_Token(TokenKind kind, Token *cur, char *str, int len) {
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    token->len = len;
    cur->next = token;
    return token;
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool token_ismutch(Token *token, char *str, int len) {
    return token->len == len && memcmp(token->str, str, len) == 0;
}

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op) {
    if (tkstream->kind != TK_RESERVED ||
        !token_ismutch(tkstream, op, strlen(op))) {
        return false;
    }
    tkstream = tkstream->next;
    return true;
}
//　強制的に一つトークンを進める(不用意に使うべきではない)
Token *consume_hard(){
    Token *tk=tkstream;
    tkstream=tkstream->next;
    return tk;
}

//次の文字がopかどうかを判定、文字は進めない
bool check(char *op) { return token_ismutch(tkstream, op, strlen(op)); }

//変数ならばそれを返して一つ進める
Token *consume_ident() {
    if (tkstream->kind != TK_IDENT) return NULL;
    Token *ident = tkstream;
    tkstream = tkstream->next;
    return ident;
}

Token *expect_ident() {
    if (tkstream->kind != TK_IDENT)
        error_at(tkstream->str, "変数もしくは関数ではありません");
    Token *ident = tkstream;
    tkstream = tkstream->next;
    return ident;
}

Token *expect_var() {
    if (tkstream->kind != TK_IDENT || *(tkstream->next->str) == '(') {
        error_at(tkstream->str, "変数ではありません");
    }
    Token *ident = tkstream;
    tkstream = tkstream->next;
    return ident;
}
Token *expect_var_not_proceed() {
    if (tkstream->kind != TK_IDENT || *(tkstream->next->str) == '(') {
        error_at(tkstream->str, "変数ではありません");
    }
    Token *ident = tkstream;
    return ident;
}

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op) {
    if (tkstream->kind != TK_RESERVED || tkstream->str[0] != op) {
        error_at(tkstream->str, "\"%c\"ではありません", op);
    }
    tkstream = tkstream->next;
}

Type *expect_type() {
    if (tkstream->kind != TK_IDENT) {
        error_at(tkstream->str, "型名ではありません");
    }
    Type *type = find_type(tkstream);
    if (type == NULL) error_at(tkstream->str, "宣言されていない型です。");
    tkstream = tkstream->next;
    while (consume("*")) {
        type = new_Pointer(type);
    }
    return type;
}
bool check_Type() {
    Type *type = find_type(tkstream);
    if (type == NULL) return false;
    return true;
}
// checkした後に実行すべき
Type *consume_Type() {
    if (tkstream->kind != TK_IDENT) {
        error_at(tkstream->str, "型名ではありません");
    }
    Type *type = find_type(tkstream);
    tkstream = tkstream->next;
    while (consume("*")) {
        type = new_Pointer(type);
    }
    return type;
}

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_number() {
    if (tkstream->kind != TK_NUM) {
        error_at(tkstream->str, "数ではありません");
    }
    int val = tkstream->val;
    tkstream = tkstream->next;
    return val;
}

bool at_eof() { return tkstream->kind == TK_EOF; }

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

int is_alp(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        //制御構文
        if (memcmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_Token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        if (memcmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_Token(TK_RESERVED, cur, p, 5);
            p += 5;
            continue;
        }

        if (memcmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_Token(TK_RESERVED, cur, p, 4);
            p += 4;
            continue;
        }

        if (memcmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_Token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (memcmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_Token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        //演算子
        if (memcmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            cur = new_Token(TK_RESERVED, cur, p, 6);
            p += 6;
            continue;
        }

        //確実に非変数名なもの
        if (memcmp(p, "!=", 2) == 0 || memcmp(p, "==", 2) == 0 ||
            memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0) {
            cur = new_Token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == '>' || *p == '<' || *p == '=' || *p == ';' ||
            *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&' ||
            *p == '[' || *p == ']') {
            cur = new_Token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        //数字
        if (isdigit(*p)) {
            char *q = p;
            cur = new_Token(TK_NUM, cur, p, 1);  //数字の長さを1で適当に初期化
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        //変数名
        if (is_alp(*p)) {
            char *q = p;
            while (1) {
                p++;
                if (!is_alnum(*p)) break;
            }
            cur = new_Token(TK_IDENT, cur, q, p - q);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_Token(TK_EOF, cur, p, 0);
    return head.next;
}