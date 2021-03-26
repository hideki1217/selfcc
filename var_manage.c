#include "selfcc.h"

#include <stdlib.h>

#include "utility.h"

////////////////定数
CStr *new_CStr(char *text, int len) {
    CStr *var = calloc(1, sizeof(CStr));
    TypeModel model = { typemgr_find("char",4,BK_OTHER) };
    tpmodel_addptr(&model);
    var->base.base.type = model.type;
    var->base.base.kind = GLOBAL;
    var->text = text;
    var->len = len;
    var->base.LC_id = LCcount++;
    return var;
}
CVar *add_CStr(char *text, int len) {
    CVar *var = (CVar *)new_CStr(text, len);
    cc_bidlist_pbPtr(constants, var);
    return var;
}

////////////////ローカル変数
LVar *new_LVar(Token *token, Type *type) {
    LVar *var = calloc(1, sizeof(LVar));
    var->base.kind = LOCAL;
    var->base.name = token->str;
    var->base.len = token->len;
    var->base.type = type;

    var->offset = lvar_manager_GetOffset(locals) + type_size(type);
    return var;
}
//宣言済み変数一覧に存在するか確認
LVar *find_lvar(Token *token,bool nowScope) {
    return lvar_manager_Find(locals, token->str, token->len,nowScope);
}
//なければ作る。あれば二重に定義したことをエラー
LVar *add_lvar(Token *token, Type *type) {
    LVar *res = find_lvar(token,true);
    if (res == NULL) {
        res = new_LVar(token, type);
        lvar_manager_Add(locals, res->base.name, res->base.len, res);
        lvar_manager_SetOffset(locals,max(res->offset, locals->top->offset));
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
GVar *add_gvar(Token *token, Type *type,bool isStatic) {
    GVar *res = find_gvar(token);
    if (res == NULL) {
        res = new_GVar(token, type);
        cc_avltree_Add(globals, res->base.name, res->base.len, (void*)res);
        if(! isStatic) cc_bidlist_pbPtr(global_list,(void*)res);
        return res;
    } else
        error_at(token->str,
                    "同名のグローバル変数or関数が既に定義されています");
}
////////////////外部宣言済み変数や関数
ExVar *find_exvar(Token *token) {
    return (ExVar *)cc_avltree_Search(externs, token->str, token->len);
}
ExVar *add_exvar(Token *token,Type *type){
    ExVar *res = find_exvar(token);
    if (res == NULL) {
        res = (ExVar*)new_GVar(token, type);
        cc_avltree_Add(externs, res->base.name, res->base.len, (void *)res);
    }
    return res;
}

////////////////変数全般

Var *find_Var(Token *token) {
    Var *var = (Var *)find_lvar(token,false);// local
    if (var) return var;
    var = (Var *)find_gvar(token);// global 
    if (var) return var;
    var = (Var *)find_exvar(token);// extern
    if(var) return var;

    return NULL;
}
Var *get_Var(Token *token) {
    Var *var =find_Var(token);
    if(var) return var;

    error_at(token->str, "宣言されていない変数または関数です。");
}

NodeKind pairOf(NodeKind kind){
    switch(kind){
        case ND_ADDASS:
            return ND_ADD;
        case ND_SUBASS:
            return ND_SUB;
        case ND_DIVASS:
            return ND_DIV;
        case ND_MULASS:
            return ND_MUL;
        case ND_ORASS:
            return ND_OR;
        case ND_ANDASS:
            return ND_AND;
        case ND_XORASS:
            return ND_XOR;
        case ND_MODASS:
            return ND_MOD;
        case ND_LSHASS:
            return ND_LSHFT;
        case ND_RSHASS:
            return ND_RSHFT;
        default:
            error("未実装だよう");
    }
}

bool IsVarNode(Node *nd) { return nd->kind == ND_LVAR || nd->kind == ND_GVAR; }

bool IsExtern(flag_n flag) { return flag & 1; }
bool IsStatic(flag_n flag) { return flag & 2; }
bool IsAuto(flag_n flag) { return flag & 4; }
bool IsRegister(flag_n flag) { return flag & 8; }
flag_n setExtern(flag_n flag, bool tof) {
    if (tof && !IsExtern(flag))
        flag += 1;
    else if (!tof && IsExtern(flag))
        flag -= 1;
    return flag;
}
flag_n setStatic(flag_n flag, bool tof) {
    if (tof && !IsExtern(flag))
        flag += 2;
    else if (!tof && IsExtern(flag))
        flag -= 2;
    return flag;
}
flag_n setAuto(flag_n flag, bool tof) {
    if (tof && !IsExtern(flag))
        flag += 4;
    else if (!tof && IsExtern(flag))
        flag -= 4;
    return flag;
}
flag_n setRegister(flag_n flag, bool tof) {
    if (tof && !IsExtern(flag))
        flag += 8;
    else if (!tof && IsExtern(flag))
        flag -= 8;
    return flag;
}
flag_n makeFlag(bool isTypedef, bool isExtern, bool isStatic, bool isAuto,
                bool isRegister) {
    int res = 0;
    if (isTypedef) res += 1;
    if (isExtern) res += 2;
    if (isStatic) res += 4;
    if (isAuto) res += 8;
    if (isRegister) res += 16;
    return (flag_n)res;
}