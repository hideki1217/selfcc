#include "token.h"

#include <stdlib.h>
#include <stdio.h>
#include "utility.h"

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
void token_delete(Token *token){
    if(token->hs) cc_sortedstrlist_delete(token->hs);
    free(token);
}

Token *_consume(char *op, Token **tk) {
    if (!(token_match(*tk, op, strlen(op)))) {
        return NULL;
    }
    return _forward(tk);
}
Token *_check(char *op, Token *tk) {
    return token_match(tk, op, strlen(op)) ? tk : NULL;
}
#define _consume_(tk_kind) \
        if((*tk)->kind != tk_kind) return NULL;\
        return _forward(tk);
Token *_consume_ident(Token **tk){_consume_(TK_IDENT)}
Token *_consume_integer(Token **tk){_consume_(TK_NUM)}
Token *_consume_float(Token **tk){_consume_(TK_FLOAT)}
Token *_consume_char(Token **tk){_consume_(TK_CHAR)}
Token *_consume_enum(Token **tk){_consume_(TK_ENUM)}
Token *_consume_string(Token **tk){_consume_(TK_STRING)}
#undef _consume_
#define _check_(tk_kind) \
    return ((tk)->kind == tk_kind)? tk:NULL;
Token *_check_ident(Token *tk){_check_(TK_IDENT)}
Token *_check_integer(Token *tk){_check_(TK_NUM)}
Token *_check_float(Token *tk){_check_(TK_FLOAT)}
Token *_check_char(Token *tk){_check_(TK_CHAR)}
Token *_check_enum(Token *tk){_check_(TK_ENUM)}
Token *_check_string(Token *tk){_check_(TK_STRING)}
#undef _check_


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


TkSequence *new_TkSq(){
    TkSequence * ts = calloc(1,sizeof(TkSequence));
    ts->begin = ts->end = new_Token(TK_END,ts->begin,NULL,0);
    return ts;
}
TkSequence *create_TkSq(Token *begin,Token *end){
    TkSequence * ts = calloc(1,sizeof(TkSequence));
    construct_TkSq(ts,begin,end);
    return ts;
}
void construct_TkSq(TkSequence *ts,Token *begin,Token *end){  
    ts->begin = begin;
    ts->end = end;
    ts->end->kind = TK_END;
}
TkSequence *tksq_clone(const TkSequence *ts){
    if(!ts)return NULL;
    TkSequence * res = new_TkSq();
    for(const Token *token=ts->begin;token->kind != TK_END;token = token_next(token)){
        Token *clone = token_clone(token,NULL);
        tksq_pushback(res,clone);
    }
    return res;
}
void tksq_pushback(TkSequence *ts,Token *tk){
    if(ts->begin == ts->end)ts->begin = tk;
    token_join(ts->end->prev,tk);
    token_join(tk,ts->end);
}
void tksq_remove(TkSequence *ts,Token *tk){
    if(tk == ts->begin){
        ts->begin = tk->next;
    }
    else token_join(tk->prev,tk->next);
    token_delete(tk);
}
void tksq_print(TkSequence *ts){
    char s[200];
    TOKEN_FOR(token,ts->begin){
        string_ncopy(s,token->str,token->len);
        fprintf(stderr,"%s ",s);
    }
}
void tksq_insert(TkSequence *ts,Token *pos,TkSequence *insert){
    if(tksq_empty(ts))return;
    if(ts->begin == pos){
        token_join(insert->end->prev, ts->begin);
        ts->begin = insert->begin;
    }
    else {
        token_join(pos->prev,insert->begin);
        token_join(insert->end->prev, pos);
    }
    free(insert->end);
}