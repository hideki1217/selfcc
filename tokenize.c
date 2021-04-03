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
Token *check(char *op) { return _check(op, tkstream); }

Token *check_ahead(char *s) {
    return token_match(tkstream->next, s, strlen(s)) ? tkstream->next : NULL;
}

//変数ならばそれを返して一つ進める

Token *expect_ident() { return _expect_ident(&tkstream); }
Token *_expect_ident(Token **tk) {
    if ((*tk)->kind != TK_IDENT)
        error_at((*tk)->str, "変数もしくは関数ではありません");
    return _forward(tk);
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
        error_at((*token)->str, "\"%s\"ではありません", op);
}
void expect(char *op) { _expect(op, &tkstream); }


bool check_Type() {
    BaseKind bkind = BK_OTHER;
    if (token_match(tkstream,"struct",6))bkind = BK_STRUCT;
    else if (token_match(tkstream,"union",5))bkind = BK_UNION;

    Type *type = typemgr_find(tkstream->str,tkstream->len,bkind);
    if (type == NULL) return false;
    return true;
}
// checkした後に実行すべき
Token *consume_Type(Type **tp) {
    BaseKind bkind;
    if(tkstream->kind == TK_IDENT)bkind = BK_OTHER;
    else if (token_match(tkstream,"struct",6))bkind = BK_STRUCT;
    else if (token_match(tkstream,"union",5))bkind = BK_UNION;
    else return NULL;
    
    Type *type = typemgr_find(tkstream->str,tkstream->len,bkind); 
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
Token *consume_ident() { return _consume_ident(&tkstream); }
Token *consume_integer() { return _consume_integer(&tkstream); }
Token *consume_float() { return _consume_float(&tkstream); }
Token *consume_char() { return _consume_char(&tkstream); }
Token *consume_enum() { return _consume_enum(&tkstream); }
Token *consume_string() { return _consume_string(&tkstream); }

Token *check_ident() { return _check_ident(tkstream); }
Token *check_integer() { return _check_integer(tkstream); }
Token *check_float() { return _check_float(tkstream); }
Token *check_char() { return _check_char(tkstream); }
Token *check_enum() { return _check_enum(tkstream); }
Token *check_string() { return _check_string(tkstream); }

bool at_eof() { return tkstream->kind == TK_END; }

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
#define macrokey(p, s) \
    n = strlen(s);\
    if (match(p, s)) {     \
        cur = new_Token(TK_MACRO, cur, p, n); \
        p += n;                               \
        continue;                             \
    }
TkSequence *tokenize(char *p) {
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

        if( *p == '\'') {
            char *q = p+1; // 文字部
            p+= 2;
            if( *p != '\'')error_at(p,"\'がありません");
            cur = new_Token(TK_CHAR, cur,q,1);
            p++; // "'"を消費
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
        if (macroMode) {
            macroword(p, "defined");
            macrokey(p, "##");
            macrokey(p, "#");
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

    cur = new_Token(TK_END, cur, p, 0);
    return create_TkSq(head.next, cur);
}

//////ヘルパ関数

#undef keyward