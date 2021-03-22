#include "token.h"

#include <stdlib.h>

Token *new_Token(TokenKind kind, Token *cur, char *str, int len) {
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    token->len = len;
    token_join(cur, token);
    token->hs = NULL;
    token->next = NULL;
    return token;
}
Token *token_clone(const Token *token, const Token *pre) {
    Token *cur = new_Token(token->kind, (Token *)pre, token->str, token->len);
    cur->val = token->val;
    cur->next = NULL;
    return cur;
}

Token *_consume(char *op, Token **tk) {
    if (!(token_match(*tk, op, strlen(op)))) {
        return NULL;
    }
    return _forward(tk);
}
Token *_check(char *op, Token **tk) {
    return token_match(*tk, op, strlen(op)) ? *tk : NULL;
}
HideSet *token_geths(Token *token) {
    if (token->hs == NULL) token->hs = cc_sortedstrlist_new();
    return token->hs;
}

bool _nforward(Token **token, int n) {
    times(n) {
        if (*token == NULL) return false;
        *token = token_next(*token);
    }
    return true;
}
Token *_forward(Token **token) {
    Token *tk = *token;
    *token = token_next(*token);
    return tk;
}
bool _nbackward(Token **token, int n) {
    times(n) {
        if (*token == NULL) return false;
        *token = token_prev(*token);
    }
    return true;
}
Token *_backward(Token **token) {
    Token *tk = *token;
    *token = token_prev(*token);
    return tk;
}