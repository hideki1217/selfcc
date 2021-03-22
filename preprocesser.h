#pragma once

#include "token.h"

void Initialize_preprocesser();

typedef struct Macro Macro;
struct Macro {
    char *key;     // キー
    int keylen;    // キーの長さ
    Token *begin;  // マクロの最初のToken
    Token *end;    // マクロの最後のTokenの直後
};
/**
 * @brief  終端がNULLのTokenSequance itemをheadとtailの間に挿入
 * @note
 * @param  *item: 挿入したいTS
 * @param  *head: 挿入したい場所の左端
 * @param  *tail: 挿入したい場所の右端
 * @retval None
 */
void token_insert(Token *item, Token *head, Token *tail);
/**
 * @brief  マクロに登録されたTSをheadとtailの間に挿入
 * @note
 * @param  *macro: 対象のマクロ
 * @param  *head: TSの挿入したいところの先頭
 * @param  *tail: TSの挿入したいところの末尾
 * @retval None
 */
void macro_SetCloneToken(Macro *macro, Token *head, Token *tail);
Macro *macro_Search(char *key, int len);
void macro_Delete(char *key, int len);


static void regist_macro(char *key, int keylen, Token *begin, Token *end);


/**
 * @brief  TK_MACROENDのノードを返す。
 * @note
 * @param  *start: 探索の始点
 * @retval TK_MACROENDのノードへのpointer
 */
Token *skip2MacroEnd(Token *begin);
Token *preproccess(Token *begin);

Token *expand_include(Token *root);
Token *combine_strings(Token *root);
void free_Hideset(Token *root);

Token *expand(Token *start);
Token *subst(Token *is,Token *fp, Token *ap, HideSet *hs, Token *os);
Token *glue(Token *ls,Token *rs);
Token *hsadd(HideSet *hs,Token *ts);
Token *ts(Token *tk);
Token *fp(Token *tk);
Token *select(int i,Token *ts);
Token *stringize(Token *ts);

