#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "selfcc.h"
#include "utility.h"


static CC_AVLTree *defined_macros;

void Initialize_preprocesser() { defined_macros = cc_avltree_new(); }

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
void token_insert(Token *item, Token *head, Token *tail) {
    if (item == NULL) {
        head->next = tail;
        return;
    }
    head->next = item;
    for (Token *tmp = item; tmp; item = tmp, tmp = tmp->next)
        ;
    item->next = tail;
}
/**
 * @brief  マクロに登録されたTSをheadとtailの間に挿入
 * @note
 * @param  *macro: 対象のマクロ
 * @param  *head: TSの挿入したいところの先頭
 * @param  *tail: TSの挿入したいところの末尾
 * @retval None
 */
void macro_SetCloneToken(Macro *macro, Token *head, Token *tail) {
    Token *cur = macro->begin;
    Token *res = NULL,anker;
    if (macro->begin->kind != TK_MACROEND) {
        res = &anker;
        while (cur != macro->end) {
            res = token_clone(cur, res);
            cur = cur->next;
        }
    }
    res->next = NULL;

    token_insert(anker.next,head,tail);
}
Macro *macro_Search(char *key, int len) {
    return cc_avltree_Search(defined_macros, key, len);
}
void macro_Delete(char *key, int len) {
    cc_avltree_DeleteNode(defined_macros, key, len);
}

#define match(token, string)          \
    (token)->len == strlen(string) && \
        memcmp(string, (token)->str, (token)->len) == 0

Token *expand(Token *tk) {}

static void regist_macro(char *key, int keylen, Token *begin, Token *end) {
    Macro *macro = calloc(1, sizeof(Macro));
    macro->key = key;
    macro->keylen = keylen;
    macro->begin = begin;
    macro->end = end;
    cc_avltree_Add(defined_macros, macro->key, macro->keylen, macro);
}


/**
 * @brief  TK_MACROENDのノードを返す。
 * @note
 * @param  *start: 探索の始点
 * @retval TK_MACROENDのノードへのpointer
 */
Token *skip2MacroEnd(Token *begin) {
    Token *top = begin;
    for (; top->kind != TK_MACROEND; top = top->next)
        ;
    return top;
}
#define INCREMENT_ROOT_WITH_SAVE() last_nonemacro_token = root, root = root->next
#define INCREMENT_ROOT() root = root->next
#define CONNECT_PRE_2_ROOT() \
    INCREMENT_ROOT();        \
    last_nonemacro_token->next = root;  \
    root = last_nonemacro_token;
Token *preproccess(Token *root) {
    Token anker, *last_nonemacro_token = &anker;
    anker.next = root;
    while (root->kind != TK_EOF) {
        switch (root->kind) {
            case TK_MACROSTART: {
                INCREMENT_ROOT();
                if (root->kind == TK_MACROEND) {
                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("define", &root)) {
                    Token *ident = _expect_ident(&root);
                    if (_consume("(", &root)) {
                        // TODO:　関数型マクロ
                        _expect(')', &root);
                    }
                    Token *begin = root;
                    root = skip2MacroEnd(begin);

                    regist_macro(ident->str, ident->len, begin, root);

                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("undef", &root)) {
                    Token *ident = _expect_ident(&root);
                    macro_Delete(ident->str, ident->len);
                    root = skip2MacroEnd(ident);

                    CONNECT_PRE_2_ROOT();
                    continue;
                }
                if (_consume("include", &root)) {
                }
                if (_consume("if", &root)) {
                }
                if (_consume("elif", &root)) {
                }
                if (_consume("else", &root)) {
                }
                if (_consume("endif", &root)) {
                }
                if (_consume("ifdef", &root)) {
                }
                if (_consume("ifndef", &root)) {
                }
            }
            case TK_IDENT: {
                Macro *macro =
                    cc_avltree_Search(defined_macros, root->str, root->len);
                if (macro == NULL) {
                    break;
                }
                macro_SetCloneToken(macro, last_nonemacro_token, root->next);
                root = last_nonemacro_token;
                INCREMENT_ROOT_WITH_SAVE();
                continue;
            }
        }
        INCREMENT_ROOT_WITH_SAVE();
    }
    return anker.next;
}
#undef INCREMENT_ROOT
#undef INCREMENT_ROOT_WITH_SAVE

#undef match