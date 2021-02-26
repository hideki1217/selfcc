#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"

Token *tkstream;
char *user_input;
char *filename;
CC_Map_for_LVar *locals;
CC_AVLTree *globals;
CC_Vector *constants;

Token *new_Token(TokenKind kind, Token *cur, char *str, int len) {
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    token->len = len;
    cur->next = token;
    return token;
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
Token *consume_hard() {
    Token *tk = tkstream;
    tkstream = tkstream->next;
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

Token *consume_string() {
    if (tkstream->kind != TK_STRING) return NULL;
    Token *str = tkstream;
    tkstream = tkstream->next;
    return str;
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

bool match(const char *p, const char *word) {
    return memcmp(p, word, strlen(word)) == 0;
}

#define keyword(p, s)                            \
    n = strlen(s);                               \
    if (match(p, s) && !is_alnum(p[n])) {        \
        cur = new_Token(TK_RESERVED, cur, p, n); \
        p += n;                                  \
        continue;                                \
    }

Token *tokenize(char *p) {
    int n;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (match(p, "/*")) {
            char *q = strstr(p + 2, "*/");
            if (!q) error_at(p, "コメントが閉じられていません");
            p = q + 2;
            continue;
        }
        if (match(p, "//")) {
            p += 2;
            while (*p != '\n') p++;
            continue;
        }

        if (*p == '"') {
            char *q = ++p;
            while (*p != '"') {
                if (*p == '\\') p++;
                p++;
            }
            cur = new_Token(TK_STRING, cur, q, p - q);
            p++;  // 最後の「"」を消費する
            continue;
        }

        //制御構文
        keyword(p, "return");
        keyword(p, "while");
        keyword(p, "else");
        keyword(p, "for");
        keyword(p, "if");
        //演算子
        keyword(p, "sizeof");
        //修飾子
        keyword(p, "extern");
        keyword(p, "register");
        keyword(p, "auto");
        keyword(p, "static");
        keyword(p, "typedef");
        //データ構造
        keyword(p,"struct");
        keyword(p,"union");
        keyword(p,"enum");

        //確実に非変数名なもの
        if (match(p, "...")) {
            cur = new_Token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (match(p, "!=") || match(p, "==") || match(p, "<=") ||
            match(p, ">=") || match(p, "++") || match(p, "--") ||
            match(p, "+=") || match(p, "-=") || match(p, "*=") ||
            match(p, "/=")) {
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