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
 * @brief  マクロに登録されたTSをheadとtailの間に挿入
 * @note
 * @param  *macro: 対象のマクロ
 * @param  *head: TSの挿入したいところの先頭
 * @param  *tail: TSの挿入したいところの末尾
 * @retval None
 */
void macro_SetCloneToken(Macro *macro, Token *head, Token *tail) {
    Token anker;
    Token *cur = macro->begin;
    Token *res = NULL;
    if (macro->begin->kind != TK_MACROEND) {
        res = token_clone(cur, &anker);
        while (cur != macro->end) {
            res = token_clone(cur, res);
            cur = cur->next;
        }
    }
    head->next = res;
    if (res) res->next = tail;
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

#define INCREMENT_ROOT_WITH_SAVE() pre_token = root, root = root->next
#define INCREMENT_ROOT() root = root->next
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
Token *preproccess(Token *root) {
    Token anker, *pre_token = &anker;
    anker.next = root;
    while (root->kind != TK_EOF) {
        switch (root->kind) {
            case TK_MACROSTART: {
                INCREMENT_ROOT();
                if (_consume("define", &root)) {
                    Token *ident = _expect_ident(&root);
                    if (_consume("(", &root)) {
                        // TODO:　関数型マクロ
                        _expect(')', &root);
                    }
                    Token *begin = root;
                    root = skip2MacroEnd(begin);

                    regist_macro(ident->str, ident->len, begin, root);
                    
                    INCREMENT_ROOT();
                    pre_token->next = root;
                    continue;
                }
                if (_consume("undef", &root)) {
                    Token *ident = _expect_ident(&root);
                    macro_Delete(ident->str, ident->len);
                    pre_token->next = skip2MacroEnd(ident)->next;
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
                macro_SetCloneToken(macro, pre_token, root->next);
                root = pre_token;
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