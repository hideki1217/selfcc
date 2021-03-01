#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"

Token *tkstream;
Token *nowToken;
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
Token *forward() {
    Token *tk = tkstream;
    tkstream = tkstream->next;
    nowToken = tk;
    return tk;
}

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
bool consume(char *op) {
    if (tkstream->kind != TK_RESERVED ||
        !token_ismutch(tkstream, op, strlen(op))) {
        return false;
    }
    forward();
    return true;
}
//　強制的に一つトークンを進める(不用意に使うべきではない)
Token *consume_hard() { return forward(); }

//次の文字がopかどうかを判定、文字は進めない
bool check(char *op) { return token_ismutch(tkstream, op, strlen(op)); }

bool check_ahead(char *s) {
    return token_ismutch(tkstream->next, s, strlen(s));
}

//変数ならばそれを返して一つ進める
bool consume_ident(Token **tk) {
    if (tkstream->kind != TK_IDENT) return false;
    *tk = forward();
    return true;
}

Token *expect_ident() {
    if (tkstream->kind != TK_IDENT)
        error_at(tkstream->str, "変数もしくは関数ではありません");
    return forward();
}

bool consume_string(Token **tk) {
    if (tkstream->kind != TK_STRING) return false;
    *tk = forward();
    return true;
}

Token *expect_var() {
    if (tkstream->kind != TK_IDENT || *(tkstream->next->str) == '(') {
        error_at(tkstream->str, "変数ではありません");
    }
    return forward();
}
Token *expect_var_not_proceed() {
    if (tkstream->kind != TK_IDENT || *(tkstream->next->str) == '(') {
        error_at(tkstream->str, "変数ではありません");
    }
    return tkstream;
}

//文字が期待する文字列に当てはまらないならエラーを吐く
void expect(char op) {
    if (tkstream->kind != TK_RESERVED || tkstream->str[0] != op) {
        error_at(tkstream->str, "\"%c\"ではありません", op);
    }
    forward();
}
void expect_str(char *s) {
    if (tkstream->kind != TK_RESERVED || tkstream->len != strlen(s) ||
        memcmp(s, tkstream->str, strlen(s)) == 0) {
        error_at(tkstream->str, "\"%s\"ではありません", s);
    }
    forward();
}

Type *expect_type() {
    if (tkstream->kind != TK_IDENT) {
        error_at(tkstream->str, "型名ではありません");
    }
    Type *type = find_type(tkstream);
    if (type == NULL) error_at(tkstream->str, "宣言されていない型です。");
    forward();
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
bool consume_Type(Type **tp) {
    if (tkstream->kind != TK_IDENT) return false;
    Type *type = find_type(tkstream);
    if (!type) return false;
    *tp = type;
    forward();
    return true;
}

//トークンが数であればそれを出力し、トークンを一つ進める。
int expect_integer() {
    if (tkstream->kind != TK_NUM) {
        error_at(tkstream->str, "数ではありません");
    }
    int val = tkstream->val;
    forward();
    return val;
}
bool consume_integer(Token **tk) {
    if (tkstream->kind != TK_NUM) return false;
    *tk = forward();
    return true;
}
bool consume_float(Token **tk) {
    if (tkstream->kind != TK_FLOAT) return false;
    *tk = forward();
    return true;
}
bool consume_char(Token **tk) {
    if (tkstream->kind != TK_CHAR) return false;
    *tk = forward();
    return true;
}
bool consume_enum(Token **tk) {
    if (tkstream->kind != TK_ENUM) return false;
    *tk = forward();
    return true;
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
        keyword(p, "while");
        keyword(p, "else");
        keyword(p, "for");
        keyword(p, "if");
        keyword(p, "switch");
        keyword(p, "case");
        keyword(p, "default");
        keyword(p, "goto");
        //演算子
        keyword(p, "sizeof");
        //修飾子
        keyword(p, "extern");    // TODO
        keyword(p, "register");  // TODO
        keyword(p, "auto");      // TODO
        keyword(p, "static");    // TODO
        keyword(p, "typedef");   // TODO
        //データ構造
        keyword(p, "struct");  // TODO
        keyword(p, "union");   // TODO
        keyword(p, "enum");    // TODO
        //制御文字
        keyword(p, "return");
        keyword(p, "continue");  // TODO
        keyword(p, "break");     // TODO
        //変数修飾子
        keyword(p,"const");
        keyword(p,"volatile");

        //確実に非変数名なもの
        if (match(p, "...")) {
            cur = new_Token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (match(p, "!=") || match(p, "==") || match(p, "<=") ||
            match(p, ">=") || match(p, "++") || match(p, "--") ||
            match(p, "+=") || match(p, "-=") || match(p, "*=") ||
            match(p, "/=") || match(p, "||") || match(p, "&&") ||
            match(p, "->") || match(p, "<<") || match(p, ">>")) {
            cur = new_Token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == '>' || *p == '<' || *p == '=' || *p == ';' ||
            *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&' ||
            *p == '[' || *p == ']' || *p == '?' || *p == ':' || *p == '|' ||
            *p == '?' || *p == '%' || *p == '.' || *p == '~') {
            cur = new_Token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        //数字
        if (isdigit(*p)) {  // TODO: int->float->
                            // doubleと自動で読み込むレベルをあげるようにすべき
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