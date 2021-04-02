#pragma once

#include <string.h>

#include "vector.h"

#define token_join(token_a, token_b)        \
    if (token_a) (token_a)->next = token_b; \
    if (token_b) (token_b)->prev = token_a;
#define TOKEN_FOR(name, start_token) \
    for (Token *name = start_token; name->kind != TK_END; name = name->next)
#define token_match(token, string, length) \
    (token)->len == length &&memcmp((token)->str, string, length) == 0
#define token_next(token) (token)->next
#define token_prev(token) (token)->prev

typedef struct Token Token;
typedef CC_SortedStrList HideSet;

typedef enum {
    TK_RESERVED,
    TK_STRING,   // "..."
    TK_INCLUDE,  // <...>
    TK_IDENT,
    TK_NUM,
    TK_END,
    TK_FLOAT,
    TK_CHAR,
    TK_ENUM,
    TK_MACRO,
    TK_MACROSTART,
    TK_MACROEND
} TokenKind;

struct Token {
    TokenKind kind;
    Token *next;
    Token *prev;
    int val;
    char *str;
    int len;
    HideSet *hs;
};
Token *new_Token(TokenKind kind, Token *cur, char *str, int len);
Token *token_clone(const Token *token, const Token *pre);
void token_delete(Token *token);

bool _nforward(Token **token, int n);
Token *_forward(Token **token);
bool _nbackward(Token **token, int n);
Token *_backward(Token **token);

Token *_consume(char *op, Token **tk);
Token *_check(char *op, Token *tk);

Token *_consume_ident(Token **tk);
Token *_consume_integer(Token **tk);
Token *_consume_float(Token **tk);
Token *_consume_char(Token **tk);
Token *_consume_enum(Token **tk);
Token *_consume_string(Token **tk);

Token *_check_ident(Token *tk);
Token *_check_integer(Token *tk);
Token *_check_float(Token *tk);
Token *_check_char(Token *tk);
Token *_check_enum(Token *tk);
Token *_check_string(Token *tk);

HideSet *token_geths(Token *token);
#define hs_clone(hs_ptr) cc_sortedstrlist_clone(hs_ptr)
#define hs_cross(lhs, rhs) cc_sortedstrlist_cross(lhs, rhs)
#define hs_sum(lhs, rhs) cc_sortedstrlist_sum(lhs, rhs)
#define hs_add(hs_ptr, string, len) cc_sortedstrlist_add(hs_ptr, string, len)
#define hs_find(hs_ptr, string, len) cc_sortedstrlist_find(hs_ptr, string, len)
#define hs_delete(hs) cc_sortedstrlist_delete(hs)

typedef struct TkSequence TkSequence;
struct TkSequence {
    Token *begin;
    Token *end;  // タグは基本的にTK_END
};
TkSequence *new_TkSq();
TkSequence *create_TkSq(Token *begin, Token *end);
void construct_TkSq(TkSequence *ts, Token *begin, Token *end);
TkSequence *tksq_clone(const TkSequence *ts);
void tksq_pushback(TkSequence *ts, Token *tk);
void tksq_remove(TkSequence *ts, Token *tk);
void tksq_print(TkSequence *ts);
/**posの前にinsert*/
void tksq_insert(TkSequence *ts, Token *pos, TkSequence *insert);
#define tksq_empty(ts) (ts)->begin == (ts)->end