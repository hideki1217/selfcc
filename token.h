#pragma once

#include "vector.h"
#include <string.h>

#define token_join(token_a,token_b) \
    if(token_a)(token_a)->next = token_b;\
    if(token_b)(token_b)->prev = token_a;
#define TOKEN_FOR(name,start_token) for(Token *name = start_token;name->kind != TK_EOF;name = name->next)
#define token_match(token,string,length) \
    (token)->len == length && memcmp((token)->str,string,length) == 0
#define token_next(token) (token)->next
#define token_prev(token) (token)->prev

typedef struct Token Token;
typedef CC_SortedStrList HideSet;

typedef enum {
    TK_RESERVED,
    TK_STRING,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
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

bool _nforward(Token **token,int n);
Token *_forward(Token **token);
bool _nbackward(Token **token,int n);
Token *_backward(Token **token);

Token *_consume(char *op,Token **tk);
Token *_check(char *op,Token **tk);

HideSet *token_geths(Token *token);