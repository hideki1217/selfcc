#include <stdlib.h>

#include "selfcc.h"
#include "utility.h"

////////////////定数
CVar *new_CStr(char* text,int len){
    CStr *var=calloc(1,sizeof(CStr));
    var->base.base.type =new_Pointer(find_type_from_name("char"));
    var->base.base.kind=GLOBAL;
    var->text=text;
    var->len=len;
    var->base.LC_id=LCcount++;
    return (CVar*)var;
}

////////////////ローカル変数
LVar *new_LVar(Token *token, Type *type) {
    LVar *var = calloc(1, sizeof(LVar));
    var->base.kind = LOCAL;
    var->base.name = token->str;
    var->base.len = token->len;
    var->base.type = type;

    var->offset = cc_map_for_var_empty(locals)
                      ? make_memorysize(type)
                      : locals->offset + make_memorysize(type);
    return var;
}
//宣言済み変数一覧に存在するか確認
LVar *find_lvar(Token *token) {
    return (LVar *)cc_map_for_var_search(locals, token->str, token->len);
}
//なければ作る。あれば二重に定義したことをエラー
LVar *add_lvar(Token *token, Type *type) {
    LVar *res = find_lvar(token);
    if (res == NULL) {
        res = new_LVar(token, type);
        cc_map_for_var_add(locals, res->base.name, res->base.len, res);
        locals->offset = max(res->offset, locals->offset);
        return res;
    } else
        error_at(token->str, "同名のローカル変数が既に定義されています");
}

///////////////////////グローバル変数
GVar *new_GVar(Token *token, Type *type) {
    GVar *var = calloc(1, sizeof(GVar));
    var->base.kind = GLOBAL;
    var->base.name = token->str;
    var->base.len = token->len;
    var->base.type = type;
    return var;
}
GVar *find_gvar(Token *token) {
    return (GVar *)cc_avltree_Search(globals, token->str, token->len);
}
GVar *add_gvar(Token *token, Type *type) {
    GVar *res = find_gvar(token);
    if (res == NULL) {
        res = new_GVar(token, type);
        cc_avltree_Add(globals, res->base.name, res->base.len, (void *)res);
        return res;
    } else
        error_at(token->str, "同名のグローバル変数が既に定義されています");
}

////////////////変数全般

Var *find_Var(Token *token) {
    Var *res = (Var *)find_lvar(token);
    if (res) return res;

    res = (Var *)find_gvar(token);
    if (res) return res;

    return NULL;
}
Var *get_Var(Token *token) {
    Var *var = find_Var(token);
    if (var) return var;

    error_at(token->str, "宣言されていない変数です。");
}