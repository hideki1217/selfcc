#pragma once

#include "token.h"
#include "vector.h"

void Initialize_preprocesser();

typedef enum { MC_OBJECT, MC_FUNC } MacroKind;

typedef struct Macro Macro;
struct Macro {
    MacroKind kind;
    char *key;        // キー
    int keylen;       // キーの長さ
    TkSequence *ts;     // マクロの中身
    CC_Vector *params;  // 引数名リスト
};
Macro *new_Macro();
void set_Macro(Macro *macro, char *key, int keylen, Token *begin, Token *end,
               CC_Vector *params);
Macro *macro_Search(char *key, int len);
void macro_Delete(char *key, int len);

/**
 * @brief  TK_MACROENDのノードを返す。
 * @note
 * @param  *start: 探索の始点
 * @retval TK_MACROENDのノードへのpointer
 */
Token *skip2MacroEnd(Token *begin);
TkSequence *preproccess(TkSequence *ts);

TkSequence *expand_include(TkSequence *ts);
TkSequence *combine_strings(TkSequence *ts);
void free_Hideset(TkSequence *ts);

TkSequence *expand(TkSequence *ts);
TkSequence *subst(TkSequence *is, CC_Vector *fp, CC_Vector *ap, HideSet *hs);
Token *glue(Token *os_end, Token *rt);
Token *ts(Token *tk);
Token *fp(Token *tk);
#define select(i,vec_ts) ((vec_ts)->_)[i].ptr
Token *stringize(const TkSequence *ts);
