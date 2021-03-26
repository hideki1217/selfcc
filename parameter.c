#include "selfcc.h"
#include <stdlib.h>


/**
 * @brief  関数呼び出しの引数チェック
 * @note   
 * @param  *base: 元の引数
 * @param  *act: 実際の引数
 * @retval 0::正常終了, 1::型が不正, 2::引数が少ない, 3::引数が多い
 */
int params_compare(const Params *base,const Params *act) {
    Param *base_arg = base->root;
    Param *act_arg = act->root;
    while (1) {
        if (base_arg == NULL) break;
        if (base_arg->kind == PA_VAARG) return 0;  // 可変長引数
        if (act_arg == NULL) return 2;             // 引数が少ない
        if (isAssignable(base_arg->type, act_arg->type)) {
            base_arg = base_arg->next;
            act_arg = act_arg->next;
        } else {
            return 1;  // 型の互換性がない
        }
    }
    if (act_arg == NULL) return 0;  // 正常

    return 3;  // 引数が多い
}

void set_Params(Params *p) {
    p->root = NULL;
    p->front = NULL;
}
Params *new_Params() {
    Params *p = calloc(1, sizeof(Params));
    set_Params(p);
    return p;
}
void set_Param(Param *p,Type *tp){
    p->kind = PA_ARG;
    p->type = tp;
    p->token = NULL;
}
void set_VaArg(Param *p){
    p->kind = PA_VAARG;
}
void params_setIdent(Params *params,Token *tk){
    params->front->token=tk;
}

void params_addParam(Params *p, Type *tp) {
    Param *par = calloc(1, sizeof(Param));
    par->kind = PA_ARG;
    par->type = tp;
    if(p->front)p->front->next = par;
    p->front = par;
    if(p->root==NULL)p->root=par;
}

void params_addVaArg(Params *p) {
    Param *par = calloc(1, sizeof(Param));
    par->kind = PA_VAARG;
    if(p->front)p->front->next = par;
    p->front = par;
    if(p->root==NULL)p->root=par;
}

