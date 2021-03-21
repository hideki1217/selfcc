#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfcc.h"
#include "utility.h"
#include "vector.h"

Token *tkstream;
char *user_input;
char *filename;


bool nforward(int n) { return _nforward(&tkstream, n); }
Token *forward() { return _forward(&tkstream); }
bool nbackward(int n) { return _nbackward(&tkstream, n); }
Token *backward() { return _backward(&tkstream); }

void unconsume() { backward(); }

//文字が期待する文字列にに当てはまるなら、trueを返して一つ進める
Token *consume(char *op) { return _consume(op, &tkstream); }

//次の文字がopかどうかを判定、文字は進めない
Token *check(char *op) { return _check(op, &tkstream); }

Token *check_ahead(char *s) {
    return token_match(tkstream->next, s, strlen(s)) ? tkstream->next : NULL;
}

//変数ならばそれを返して一つ進める
Token *consume_ident() {
    if (tkstream->kind != TK_IDENT) return NULL;
    return forward();
}

Token *expect_ident() { return _expect_ident(&tkstream); }
Token *_expect_ident(Token **tk) {
    if ((*tk)->kind != TK_IDENT)
        error_at((*tk)->str, "変数もしくは関数ではありません");
    return _forward(tk);
}

Token *consume_string() {
    if (tkstream->kind != TK_STRING) return NULL;
    return forward();
}

Token *expect_var() {
    if (tkstream->kind != TK_IDENT || *(tkstream->next->str) == '(') {
        error_at(tkstream->str, "変数ではありません");
    }
    return forward();
}

//文字が期待する文字列に当てはまらないならエラーを吐く
void _expect(char *op, Token **token) {
    Token *res;
    if ((res = _consume(op, token)) == NULL)
        error_at(token_prev(tkstream)->str, "\"%s\"ではありません", op);
}
void expect(char *op) { _expect(op, &tkstream); }

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
Token *consume_Type(Type **tp) {
    if (tkstream->kind != TK_IDENT) return NULL;
    Type *type = find_type(tkstream);
    if (!type) return NULL;
    *tp = type;
    return forward();
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
Token *consume_integer() {
    if (tkstream->kind != TK_NUM) return NULL;
    return forward();
}
Token *consume_float() {
    if (tkstream->kind != TK_FLOAT) return NULL;
    return forward();
}
Token *consume_char() {
    if (tkstream->kind != TK_CHAR) return NULL;
    return forward();
}
Token *consume_enum() {
    if (tkstream->kind != TK_ENUM) return NULL;
    return forward();
}

bool at_eof() { return tkstream->kind == TK_EOF; }

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
#define macroword(p, s)                       \
    n = strlen(s);                            \
    if (match(p, s) && !is_alnum(p[n])) {     \
        cur = new_Token(TK_MACRO, cur, p, n); \
        p += n;                               \
        continue;                             \
    }
Token *tokenize(char *p) {
    int n;
    int macroMode = 0;

    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {
        /////////////////読み飛ばす
        if (!macroMode) {
            if (isspace(*p)) {
                p++;
                continue;
            }
        } else {
            if (*p == ' ' || *p == '\t') {
                p++;
                continue;
            }
            if (*p == '\\' && p[1] == '\n') {
                p += 2;
                continue;
            }
            if (*p == '\n') {
                macroMode = 0;
                cur = new_Token(TK_MACROEND, cur, p, 1);
                p++;
                continue;
            }
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
        //////////////////////////

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
        if (*p == '#') {
            cur = new_Token(TK_MACROSTART, cur, p, 1);
            p++;
            macroMode++;
            macroword(p, "define");
            macroword(p, "ifdef");
            macroword(p, "ifndef");
            macroword(p, "include");
            macroword(p, "if");
            macroword(p, "endif");
            macroword(p, "else");
            macroword(p, "elif");
            continue;
        }
        macroword(p, "defined");

        //制御構文
        keyword(p, "while");
        keyword(p, "else");
        keyword(p, "for");
        keyword(p, "if");
        keyword(p, "switch");
        keyword(p, "case");
        keyword(p, "default");
        keyword(p, "goto");
        keyword(p, "do");
        //演算子
        keyword(p, "sizeof");
        //修飾子
        keyword(p, "extern");
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
        keyword(p, "const");
        keyword(p, "volatile");

        //確実に非変数名なもの
        if (match(p, "...")) {
            cur = new_Token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (match(p, "<<=") || match(p, ">>=")) {
            cur = new_Token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (match(p, "!=") || match(p, "==") || match(p, "<=") ||
            match(p, ">=") || match(p, "++") || match(p, "--") ||
            match(p, "+=") || match(p, "-=") || match(p, "*=") ||
            match(p, "/=") || match(p, "||") || match(p, "&&") ||
            match(p, "->") || match(p, "<<") || match(p, ">>") ||
            match(p, "|=") || match(p, "&=") || match(p, "^=") ||
            match(p, "%=")) {
            cur = new_Token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == '>' || *p == '<' || *p == '=' || *p == ';' ||
            *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&' ||
            *p == '[' || *p == ']' || *p == '?' || *p == ':' || *p == '|' ||
            *p == '?' || *p == '%' || *p == '.' || *p == '~' || *p == '^') {
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

//////ヘルパ関数

#undef keyward