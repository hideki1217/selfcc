#include "type.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "utility.h"
#include "vector.h"

#define NULL ((void *)0)

static Type *_void;
static Type *_int;
static Type *_char;
static Type *_float;
static Type *_long;
static Type *_double;

static Type *_enum;

static CC_AVLTree /*<ModelType*>*/ *trees[BK_SIZE];
static CC_AVLTree /*<Integer*>*/ *enum_vals;

static BaseType *new_BaseType(TypeKind kind, int size);
static Type *type_clone(const Type *type);
static Type *new_PrimType(char *name, int namelen, TypeKind kind,
                          int memory_size);
static BaseType *new_BaseType(TypeKind kind, int size);
static Type *new_PrimType(char *name, int namelen, TypeKind kind,
                          int memory_size);
static Type *new_Pointer(Type *base);
static Type *new_Function(Type *base);
static Type *new_Array(Type *base, int length);
static Type *new_Struct(char *name, int namelen);
static Type *new_Union(char *name, int namelen);
static Type *new_Enum(char *enum_name,int namelen);
static Type *new_Incomplete();

#define regist_Type(bkind, type_name, namelen, type_ptr) \
    cc_avltree_Add(trees[bkind], type_name, namelen, tpmodel_tnew(type_ptr))

void initialize_typemgr() {
    _void = new_PrimType("void", 4, TY_VOID, 8);
    _int = new_PrimType("int", 3, TY_INT, 4);
    _char = new_PrimType("char", 4, TY_CHAR, 1);
    _float = new_PrimType("float", 5, TY_FLOAT, 4);
    _long = new_PrimType("long", 4, TY_LONG, 8);
    _double = new_PrimType("double", 6, TY_DOUBLE, 8);

    _enum = type_clone(_int);// すべてのenumのベース(中身はconst int)
    _enum->isConst = true;

    trees[BK_OTHER] = cc_avltree_new();
    trees[BK_STRUCT] = cc_avltree_new();
    trees[BK_UNION] = cc_avltree_new();
    trees[BK_ENUM] = cc_avltree_new();
    enum_vals = cc_avltree_new();

    regist_Type(BK_OTHER, "void", 4, _void);
    regist_Type(BK_OTHER, "int", 3, _int);
    regist_Type(BK_OTHER, "char", 4, _char);
    regist_Type(BK_OTHER, "_Bool",5,_char);
    regist_Type(BK_OTHER, "float", 5, _float);
    regist_Type(BK_OTHER, "long", 4, _double);

    regist_Type(BK_ENUM, "enum",3,_enum);
}

TypeModel *_typemgr_find(char *name, int len, BaseKind kind) {
    TypeModel *model;
    model = cc_avltree_Search(trees[kind], name, len);
    return model;
}

Type *typemgr_find(char *name, int len, BaseKind kind) {
    TypeModel *model = _typemgr_find(name, len, kind);
    return (ISNULL(model)) ? NULL : type_clone(model->type);
}

TypeModel *typemgr_excl(char *name, int len, BaseKind kind) {
    if (kind == BK_OTHER) error("structやunionへの利用を目的としています。");

    TypeModel *model = tpmodel_tnew(NULL);
    cc_avltree_Add(trees[kind], name, len, model);
    return model;
}

void typemgr_reg(char *name, int len, BaseKind kind, Type *type) {
    TypeModel *model;
    if (type_hasname(type)) {  // 名前の付いた型ならばエイリアス
        BaseKind bkind;
        if (type_isstruct(type))
            bkind = BK_STRUCT;
        else if (type_isunion(type))
            bkind = BK_UNION;
        else if(type_isenum(type))
            bkind = BK_ENUM;
        else
            bkind = BK_OTHER;

        model = cc_avltree_Search(trees[bkind], type_name(type),
                                  type_namelen(type));
    } else {  // そうでなければ新規modelを登録
        model = tpmodel_tnew(type);
    }
    cc_avltree_Add(trees[kind], name, len, model);
}
bool typemgr_regenum(char *name,int namelen,int val){
    if(ISNNULL(cc_avltree_Search(enum_vals,name,namelen)))return false;
    cc_avltree_Add(enum_vals,name,namelen,new_Integer(val));
    return true;
}

TypeModel *tpmodel_tnew(Type *base) {
    TypeModel *model = calloc(1, sizeof(TypeModel));
    if (base)
        model->type = base;
    else
        model->type = new_Incomplete();
    return model;
}
void tpmodel_addptr(TypeModel *model) {
    model->type = new_Pointer(model->type);
}
void tpmodel_addarray(TypeModel *model, int arraylen) {
    model->type = new_Array(model->type, arraylen);
}
void tpmodel_addfunc(TypeModel *model) {
    model->type = new_Function(model->type);
}
bool tpmodel_initstruct(TypeModel *model, char *struct_name, int namelen) {
    Type *reged_type = typemgr_find(struct_name, namelen, BK_STRUCT);
    if (ISNNULL(reged_type)) {
        model->type =
            reged_type;  //　登録済みがあって、それがINCOMPLETEならそれを返す。
        return type_isincomplete(reged_type);  // INCOMPMLETEなら編集可
    }
    model->type = new_Struct(struct_name, namelen);
    type_size(model->type) = INCOMPLETE_SIZE;  // 登録段階ではINCOMPLETE
    regist_Type(BK_STRUCT, struct_name, namelen, model->type);  // ひな形を登録

    return true;  // 編集可
}
bool tpmodel_initunion(TypeModel *model, char *union_name, int namelen) {
    Type *reged_type = typemgr_find(union_name, namelen, BK_UNION);
    if (ISNNULL(reged_type)) {
        model->type =
            reged_type;  //　登録済みがあって、それがINCOMPLETEならそれを返す。
        return type_isincomplete(reged_type);  // INCOMPMLETEなら編集可
    }
    model->type = new_Union(union_name, namelen);
    type_size(model->type) = INCOMPLETE_SIZE;  // 登録段階ではINCOMPLETE
    regist_Type(BK_UNION, union_name, namelen, model->type);  // ひな形を登録

    return true;  // 編集可
}
bool tpmodel_initenum(TypeModel *model, char *enum_name, int namelen){
    Type *reged_type = typemgr_find(enum_name, namelen, BK_ENUM);
    if (ISNNULL(reged_type)) {
        model->type =
            reged_type;  //　登録済みがあって、それがINCOMPLETEならそれを返す。
        return type_isincomplete(reged_type);  // INCOMPMLETEなら編集可
    }
    model->type = new_Enum(enum_name, namelen);
    type_size(model->type) = INCOMPLETE_SIZE;  // 登録段階ではINCOMPLETE
    regist_Type(BK_ENUM, enum_name, namelen, model->type);  // ひな形を登録

    return true;  // 編集可
}

void tpmodel_addprm(TypeModel *model, Type *prm_tp, Token *ident) {
    if (!type_isfunc(model->type))
        error("関数でない型に引数をつけようとしました。");
    Param *par = calloc(1, sizeof(Param));
    par->kind = PA_ARG;
    par->type = prm_tp;
    par->token = ident;
    cc_vector_pbPtr(type_params(model->type), par);
}
void tpmodel_addvaarg(TypeModel *model) {
    if (!type_isfunc(model->type))
        error("関数でない型に引数をつけようとしました。");
    Param *par = calloc(1, sizeof(Param));
    par->kind = PA_VAARG;
    cc_vector_pbPtr(type_params(model->type), par);
}
void tpmodel_addmem(TypeModel *model, Type *prm_tp, Token *ident) {
    if (!(type_isstruct(model->type) || type_isunion(model->type)))
        error("structやunion以外にmemberを足そうとしました。");
    Param *par = calloc(1, sizeof(Param));
    par->kind = PA_ARG;
    par->type = prm_tp;
    par->token = ident;

    // メンバリストに追加
    cc_vector_pbPtr(type_members(model->type), par);

    // 型のサイズの更新
    if (type_isincomplete(model->type)) type_size(model->type) = 0;
    if (type_isstruct(model->type)) {
        type_size(model->type) += type_size(prm_tp);
    }
    if (type_isunion(model->type)) {
        type_size(model->type) = max(type_size(model->type), type_size(prm_tp));
    }
}
void tpmodel_setbase(TypeModel *model, Type *base) {
    Type *top = model->type, *pre = top;
    if (ISNULL(top)) {
        model->type = base;
        return;
    }
    while (ISNNULL(top) && type_hasptr_to(top)) {
        pre = top;
        top = type_ptr_to(top);
    }
    type_ptr_to(pre) = base;
}
void tpmodel_setconst(TypeModel *model) { model->type->isConst = true; }
void tpmodel_setvltle(TypeModel *model) { model->type->isVolatile = true; }

char *_type2str(char *buffer, Type *tp) {
    char tmp[BUFFER_SIZE];
    switch (type_kind(tp)) {
        case TY_ARRAY: {
            _type2str(tmp, type_ptr_to(tp));
            sprintf(buffer, "%s [%d]", tmp, type_arraylen(tp));
            if (tp->isConst) strncat(buffer, " const", 7);
            if (tp->isVolatile) strncat(buffer, " volatile", 10);
            break;
        }
        case TY_PTR: {
            _type2str(tmp, type_ptr_to(tp));
            sprintf(buffer, "%s *", tmp);
            if (tp->isConst) strncat(buffer, " const", 7);
            if (tp->isVolatile) strncat(buffer, " volatile", 10);
            break;
        }
        case TY_FUNCTION: {
            _type2str(tmp, type_ptr_to(tp));
            sprintf(buffer, "%s func(", tmp);
            for (int i = 0; i < type_params(tp)->size; i++) {
                Param *par = type_params(tp)->_[i].ptr;
                _type2str(tmp, par->type);
                strncat(buffer, tmp, strlen(tmp));
                if (i != type_params(tp)->size - 1)
                    strncat(buffer, ", ", 3);
                else
                    strncat(buffer, ") ", 3);
            }
        }
        default: {
            strncpy(buffer, type_name(tp), type_namelen(tp));
            if (tp->isConst) strncat(buffer, " const", 7);
            if (tp->isVolatile) strncat(buffer, " volatile", 10);
            break;
        }
    }
    return buffer;
}
char *type2str(Type *tp) {
    char *buffer = calloc(BUFFER_SIZE, sizeof(char));
    return _type2str(buffer, tp);
}
bool equal(Type *l, Type *r) {
    if (ISNULL(l) && ISNNULL(r) || ISNNULL(l) && ISNULL(r)) return false;
    if (l == r) return true;
    if (type_kind(l) != type_kind(r)) return false;
    switch (type_kind(r)) {
        case TY_PTR:
            return equal(type_ptr_to(l), type_ptr_to(r));
        case TY_ARRAY:
            return type_arraylen(l) == type_arraylen(r)
                       ? equal(type_ptr_to(l), type_ptr_to(r))
                       : false;
        case TY_FUNCTION: {
            Params *lparams = type_params(l), *rparams = type_params(r);
            Param *lp, *rp;
            if (lparams->size != rparams->size) return false;
            for (int i = 0; i < rparams->size; i++) {
                lp = lparams->_[i].ptr;
                rp = rparams->_[i].ptr;
                if (lp->kind != rp->kind) return false;
                if (lp->kind == PA_VAARG) break;
                if (!equal(lp->type, rp->type)) return false;
            }
            return true;
        }
        case TY_STRUCT:
        case TY_UNION:
        case TY_ENUM:
            return type_namelen(l) == type_namelen(r) &&
                   memcmp(type_name(l), type_name(r), type_namelen(l)) == 0;
        default:
            return type_kind(l) == type_kind(r);
    }
}
bool isArrayorPtr(Type *type) { return type_isptr(type) || type_isarray(type); }
bool isNum(Type *type) {
    return !type_isptr(type) && !type_isarray(type) && !type_isstruct(type);
}
bool isInteger(Type *type) {
    return type_kind(type) == TY_CHAR || type_kind(type) == TY_INT ||
           type_kind(type) == TY_LONG;
}
bool isFloat(Type *type) { return isNum(type) && !isInteger(type); }
bool isAssignable(Type *l, Type *r) {
    if (equal(l, r)) return true;
    if (type_isptr(l) && type_kind(type_ptr_to(l)) == TY_VOID && type_isptr(r))
        return true;
    if (type_isptr(l) && isArrayorPtr(r)) return true;
    switch (type_kind(l)) {
        case TY_DOUBLE:
            return isNum(r);
        case TY_FLOAT:
            return isNum(r) && type_kind(r) != TY_DOUBLE;
        case TY_LONG:
            return isInteger(r);
        case TY_INT:
        case TY_CHAR:
            return isInteger(r) && type_kind(r) != TY_LONG;
    }
    return false;
}
bool isLeftsidevalue(Type *tp) {
    return !(type_isarray(tp) || type_isfunc(tp));
}
bool isAddSubable(Type *l, Type *r) {
    return isAssignable(l, r);  // TODO: ちゃんと考えるべし
}
bool isMulDivable(Type *l, Type *r) { return isNum(l) && isNum(r); }
Callability isCallable(Type *tp) {
    if (type_isfunc(tp)) return AsFUNCTION;
    if (type_isptr(tp) && type_isfunc(type_ptr_to(tp))) return AsPTR2FUNC;
    return CANNOT;
}
Type *commonType(Type *l, Type *r) {
    switch (type_kind(l)) {
        case TY_DOUBLE:
            return (isNum(r)) ? l : NULL;
        case TY_FLOAT:
            if (!isNum(r)) return NULL;
            return (type_kind(r) == TY_DOUBLE) ? r : l;
        case TY_LONG:
            if (!isNum(r)) return NULL;
            return isInteger(r) ? l : r;
        case TY_INT:
            if (!isNum(r)) return NULL;
            if (!isInteger(r)) return r;
            return (type_kind(r) == TY_LONG) ? r : l;
        case TY_CHAR:
            return (!isNum(r)) ? NULL : r;
        default:
            error("その型の共通部分は未対応だよう");
    }
}

/**
 * @brief  関数呼び出しの引数チェック
 * @note
 * @param  *base: 元の引数
 * @param  *act: 実際の引数
 * @retval 0::正常終了, 1::型が不正, 2::引数が少ない, 3::引数が多い
 */
int params_compare(const Params *base, const Params *act) {
    if (base->size == 0 && act->size == 0) return 0;
    int base_i = 0, act_i = 0;
    Param *base_arg, *act_arg;
    while (1) {
        if (base_i == base->size) break;
        base_arg = base->_[base_i].ptr;
        if (base_arg->kind == PA_VAARG) return 0;  // 可変長引数
        if (act_i == act->size) return 2;          // 引数が少ない
        act_arg = act->_[act_i].ptr;
        if (isAssignable(base_arg->type, act_arg->type)) {
            base_i++;
            act_i++;
        } else {
            return 1;  // 型の互換性がない
        }
    }
    if (act_i == act->size) return 0;  // 正常

    return 3;  // 引数が多い
}
int params_indexof(const Params *params, char *word, int wordlen) {
    int index = -1;
    for (int i = 0; i < params->size; i++) {
        Param *par = params->_[i].ptr;
        int cmp_res =
            string_cmp(param_name(par), param_namelen(par), word, wordlen);
        if (cmp_res == 0) {
            index = i;
            break;
        }
    }
    return index;
}

////////////ヘルパ関数
static Type *type_clone(const Type *tp) {
    Type *type = calloc(1, sizeof(Type));
    memcpy(type, tp, sizeof(Type));
    return type;
}

static BaseType *new_BaseType(TypeKind kind, int size) {
    BaseType *btype = calloc(1, sizeof(BaseType));
    btype_kind(btype) = kind;
    btype_size(btype) = size;
    return btype;
}
static Type *new_PrimType(char *name, int namelen, TypeKind kind,
                          int memory_size) {
    BaseType *btype = calloc(1, sizeof(bLeafType));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = kind;
    type_size(type) = memory_size;
    type_name(type) = name;
    type_namelen(type) = namelen;
    return type;
}
static Type *new_Pointer(Type *base) {
    BaseType *btype = calloc(1, sizeof(bPointer));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_PTR;
    type_ptr_to(type) = base;
    type_size(type) = POINTER_SIZE;
    return type;
}
static Type *new_Function(Type *base) {
    BaseType *btype = calloc(1, sizeof(bFunction));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_FUNCTION;
    type_ptr_to(type) = base;
    type_size(type) = POINTER_SIZE;
    type_params(type) = cc_vector_new();
    return type;
}
static Type *new_Array(Type *base, int length) {
    BaseType *btype = calloc(1, sizeof(bArray));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_ARRAY;
    type_ptr_to(type) = base;
    type_arraylen(type) = length;
    type_size(type) = type_size(base) * length;
    return type;
}
static Type *new_Struct(char *name, int namelen) {
    BaseType *btype = calloc(1, sizeof(bStruct));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_STRUCT;
    type_name(type) = name;
    type_namelen(type) = namelen;
    type_members(type) = cc_vector_new();
    return type;
}
static Type *new_Union(char *name, int namelen) {
    BaseType *btype = calloc(1, sizeof(bUnion));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_UNION;
    type_name(type) = name;
    type_namelen(type) = namelen;
    type_members(type) = cc_vector_new();
    return type;
}
static Type *new_Enum(char *enum_name,int namelen){
    BaseType *btype = calloc(1, sizeof(bEnum));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_ENUM;
    type_name(type) = enum_name;
    type_namelen(type) = namelen;
    return type;
}

static Type *new_Incomplete() {
    BaseType *btype = calloc(1, sizeof(bStruct));
    Type *type = new_Type(btype, false, false);
    type_kind(type) = TY_INCOMPLETE;
    return type;
}
Type *new_Type(BaseType *btype, bool isConst, bool isVolatile) {
    Type *type = calloc(1, sizeof(Type));
    type->btype = btype;
    type->isConst = isConst;
    type->isVolatile = isVolatile;
    return type;
}