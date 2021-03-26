#pragma once

#include <stdbool.h>

#include "token.h"
#include "vector.h"

#define POINTER_SIZE 8

typedef struct Type Type;
typedef struct BaseType BaseType;
typedef struct bLeafType bLeafType;
typedef struct bPointer bPointer;
typedef struct bFunction bFunction;
typedef struct bArray bArray;
typedef struct bStruct bStruct;
typedef struct bUnion bUnion;

typedef CC_Vector Params;
typedef struct Param Param;

typedef struct TypeModel TypeModel;

typedef enum {
    TY_VOID,
    TY_INT,
    TY_CHAR,
    TY_LONG,
    TY_FLOAT,
    TY_DOUBLE,
    TY_PTR,
    TY_ARRAY,
    TY_STRUCT,
    TY_UNION,
    TY_ENUM,
    TY_FUNCTION,
    TY_INCOMPLETE
} TypeKind;

struct Type {
    BaseType *btype;
    bool isConst;
    bool isVolatile;
};
Type *new_Type(BaseType *btype, bool isConst, bool isVolatile);
char *type2str(Type *tp);
bool equal(Type *l, Type *r);
bool isArrayorPtr(Type *type);
bool isNum(Type *type);
bool isInteger(Type *type);
bool isFloat(Type *type);
bool isAssignable(Type *l, Type *r);
bool isLeftsidevalue(Type *tp);
bool isAddSubable(Type *l, Type *r);
bool isMulDivable(Type *l, Type *r);
typedef enum { CANNOT, AsFUNCTION, AsPTR2FUNC } Callability;
Callability isCallable(Type *tp);

Type *commonType(Type *l, Type *r);
#define TYPE_PARAMETERS(type_ptr) \
    (type_ptr)->btype, (type_ptr)->isConst, (type_ptr)->isVolatile
#define LOCAL_TYPE_COPY(type_ptr) \
    { TYPE_PARAMETERS(type_ptr) }
#define LOCAL_POINTER(name, base_type)         \
    bPointer b##name = {TY_PTR, 8, base_type}; \
    Type name = {&b##name, false, false};
#define LOCAL_INCOMPLETE(name)                   \
    bStruct b##name = {TY_INCOMPLETE, -1, NULL}; \
    Type name = {&b##name, false, false};

struct BaseType {
    TypeKind kind;
    int size;
};
struct bLeafType {
    BaseType base;
    char *name;
    int namelen;
};
struct bPointer {
    BaseType base;
    Type *ptr_to;
};
struct bFunction {
    BaseType base;
    Type *ptr_to;
    Params /*<Param>*/ *params;
};
struct bArray {
    BaseType base;
    Type *ptr_to;
    int arraylen;
};
struct bStruct {
    bLeafType base;
    Params /*<Param>*/ *members;
};
struct bUnion {
    bLeafType base;
    Params /*<Param>*/ *members;
};
#define btype_kind(btype_) ((BaseType *)(btype_))->kind
#define btype_size(btype_) ((BaseType *)(btype_))->size
// 以下キャストが必要なのでTypeKindによる検査必須
#define btype_name(btype_) ((bLeafType *)(btype_))->name
#define btype_namelen(btype_) ((bLeafType *)(btype_))->namelen
#define btype_ptr_to(btype_) ((bPointer *)(btype_))->ptr_to
#define btype_params(btype_) ((bFunction *)(btype_))->params
#define btype_members(btype_) ((bStruct *)(btype_))->members
#define btype_arraylen(btype_) ((bArray *)(btype_))->arraylen

#define type_const(type_) (type_)->isConst
#define type_volatile(type_) (type_)->isVolatile
#define type_kind(type_) btype_kind((type_)->btype)
#define type_size(type_) btype_size((type_)->btype)
// 以下キャストが必要なのでTypeKindによる検査必須
#define type_name(type_) btype_name((type_)->btype)
#define type_namelen(type_) btype_namelen((type_)->btype)
#define type_ptr_to(type_) btype_ptr_to((type_)->btype)
#define type_params(type_) btype_params((type_)->btype)
#define type_members(type_) btype_members((type_)->btype)
#define type_arraylen(type_) btype_arraylen((type_)->btype)

#define type_isstruct(type_) (type_kind(type_) == TY_STRUCT)
#define type_isunion(type_) (type_kind(type_) == TY_UNION)
#define type_isptr(type_) (type_kind(type_) == TY_PTR)
#define type_isarray(type_) (type_kind(type_) == TY_ARRAY)
#define type_isfunc(type_) (type_kind(type_) == TY_FUNCTION)
#define type_isincomplete(type_) (type_kind(type_) == TY_INCOMPLETE)

#define type_hasptr_to(type_) \
    (type_isptr(type_) || type_isfunc(type_) || type_isarray(type_))
#define type_hasname(type_) !(type_hasptr_to(type_))

typedef enum { PA_ARG, PA_VAARG } ParamKind;
struct Param {
    ParamKind kind;
    Type *type;
    Token *token;
};
/**
 * @brief  関数呼び出しの引数チェック
 * @note
 * @param  *base: 元の引数
 * @param  *act: 実際の引数
 * @retval 0::正常終了, 1::型が不正, 2::引数が少ない, 3::引数が多い
 */
int params_compare(const Params *base, const Params *act);

typedef enum { BK_OTHER = 0, BK_STRUCT = 1, BK_UNION = 2, BK_SIZE } BaseKind;
struct TypeModel {
    Type *type;
};
TypeModel *tpmodel_tnew(Type *base);
#define tpmodel_new(bname, blen, kind) \
    tpmodel_tnew(typemgr_find(bname, blen, kind))
void tpmodel_addptr(TypeModel *model);
void tpmodel_addarray(TypeModel *model, int arraylen);
void tpmodel_addfunc(TypeModel *model);
void tpmodel_addprm(TypeModel *model, Type *prm_tp, Token *ident);
void tpmodel_addvaarg(TypeModel *model);
void tpmodel_addmem(TypeModel *model, Type *prm_tp, Token *ident);
void tpmodel_setbase(TypeModel *model, Type *base);
void tpmodel_setconst(TypeModel *model);
void tpmodel_setvltle(TypeModel *model);

/**
 * @brief  typeを管理するためのインスタンスを初期化
 * @note
 * @retval None
 */
void initialize_typemgr();
/**
 * @brief  名前から型を見つける(なければNULL)
 * @note
 * @param  *name: 型名
 * @param  len: 型名の長さ
 * @retval 結果、なければNULL
 */
Type *typemgr_find(char *name, int len, BaseKind kind);
/**
 * @brief  型を宣言する関数(最初は不完全な型になっている)
 * @note
 * @param  *name: 型の名前
 * @param  len: 型の名前の長さ
 * @retval そのTypeModel or 既存であればNULL
 */
TypeModel *typemgr_excl(char *name, int len, BaseKind kind);
/**
 * @brief  型モデルを登録する関数
 * @note
 * @param  *name: 型の名前
 * @param  len: 名前の長さ
 * @param  *model: 型モデル
 * @retval None
 */
void typemgr_reg(char *name, int len, Type *type);

