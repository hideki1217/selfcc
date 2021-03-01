#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "selfcc.h"
#include "utility.h"

static Type *_void;
static Type *_int;
static Type *_char;
static Type *_float;
static Type *_long;
static Type *_double;

static CC_AVLTree *type_tree;
void Initialize_type_tree() {
    _void = new_PrimType(TY_VOID, "void", 4, 8);
    _int = new_PrimType(TY_INT, "int", 3, 4);
    _char = new_PrimType(TY_CHAR, "char", 4, 1);
    _float = new_PrimType(TY_FLOAT, "float", 4, 1);
    _long = new_PrimType(TY_LONG, "long", 4, 1);
    _double = new_PrimType(TY_DOUBLE, "double", 4, 1);

    type_tree = cc_avltree_new();

    cc_avltree_Add(type_tree, _void->name, _void->len, _void);
    cc_avltree_Add(type_tree, _int->name, _int->len, _int);
    cc_avltree_Add(type_tree, _char->name, _char->len, _char);
    cc_avltree_Add(type_tree, _float->name, _float->len, _float);
    cc_avltree_Add(type_tree, _double->name, _double->len, _double);
    cc_avltree_Add(type_tree, _long->name, _long->len, _long);
}

///////////////////////型
char *type2str(Type *tp) {
    if (tp == NULL) return "";
    char *res = type2str(tp->ptr_to);
    char *buffer = malloc(strlen(res) + 30);
    switch (tp->kind) {
        case TY_ARRAY:
            sprintf(buffer, "[%d] %s", tp->array_len, res);
            break;
        case TY_PTR:
            sprintf(buffer, "* %s", res);
            break;
        case TY_STRUCT: {
            char s[tp->len + 1];
            string_limitedcopy(s, tp->name, tp->len);
            sprintf(buffer, "%s %s", s, res);
            break;
        }
        default:
            sprintf(buffer, "%s %s", tp->name, res);
            break;
    }
    free(res);
    return buffer;
}
void InitType(Type *tp){
    tp->isConst =false;
    tp->isVolatile =false;
}
Type *new_PrimType(TypeKind kind, char *name, int len, int size) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = kind;
    type->name = name;
    type->len = len;
    type->size = size;
    return type;
}
Type *new_Pointer(Type *base) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = TY_PTR;
    type->ptr_to = base;
    type->size = 8;
    return type;
}
Type *new_Function(Type *base,Type *arg){
    Type *type=calloc(1,sizeof(Type));
    InitType(type);
    type->kind = TY_FUNCTION;
    type->ptr_to = base;
    type->next=arg;
    type->size=8;
    return type;
}
Type *new_Array(Type *base, int length) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = TY_ARRAY;
    type->ptr_to = base;
    type->array_len = length;
    type->size = base->size * length;
    return type;
}
Type *new_Struct(Type *bases) {
    return NULL;  // TODO: struct を作ったらここに加筆
}
Type *clone_Type(Type *tp){
    Type *type=calloc(1,sizeof(Type));
    memcpy(type,tp,sizeof(Type));
    return type;
}
Type *find_type_from_name(char *name) {
    Type *type = (Type *)cc_avltree_Search(type_tree, name, strlen(name));
    if (type) return clone_Type(type); // そのものではなくてコピーを渡す
    return NULL;
}
Type *find_type(Token *token) {
    Type *type = (Type *)cc_avltree_Search(type_tree, token->str, token->len);
    if (type) return clone_Type(type); // そのものではなくてコピーを渡す
    return NULL;
}
int make_memorysize(Type *type) { return type->size; }
bool equal(Type *l, Type *r) {
    if(l==NULL && r!=NULL || l!=NULL && r==NULL )return false;
    if(l==r)return true;
    while (l->kind == r->kind) {
        switch (r->kind)
        {
        case TY_PTR:
            return equal(l->ptr_to,r->ptr_to);
        case TY_ARRAY:
            return l->array_len == r->array_len ? equal(l->ptr_to,r->ptr_to): false;
        case TY_FUNCTION:{
            Type *larg,*rarg;
            for(larg=l->next,rarg=r->next;
                larg != NULL && rarg != NULL;
                larg = larg->next, rarg = rarg->next)
            {
                if(!equal(larg,rarg))return false;
            }
            return larg == rarg;
        }
        case TY_STRUCT:
        case TY_UNION:
        case TY_ENUM:
            return l->len == r->len && memcmp(l->name,r->name,l->len)==0;
        default:
            return l->kind == r->kind;
        }
    }
}


bool isArrayorPtr(Type *type) {
    return type->kind == TY_PTR || type->kind == TY_ARRAY;
}
bool isNum(Type *type) {
    return type->kind != TY_PTR && type->kind != TY_ARRAY &&
           type->kind != TY_STRUCT;
}
bool isInteger(Type *type) {
    return type->kind == TY_CHAR || type->kind == TY_INT ||
           type->kind == TY_LONG;
}
bool isAssignable(Type *l, Type *r) {
    if (equal(l, r)) return true;
    if (l->kind == TY_PTR && l->ptr_to->kind == TY_VOID && r->kind == TY_PTR)
        return true;
    switch (l->kind) {
        case TY_DOUBLE:
        case TY_FLOAT:
        case TY_LONG:
        case TY_INT:
        case TY_CHAR:
            return isNum(r);
    }
    return false;
}
bool isLeftsidevalue(Type *tp) {
    return !(tp->kind == TY_ARRAY || tp->kind == TY_STRUCT);
}
bool isAddSubable(Type *l, Type *r) {
    return isAssignable(l, r);  // TODO: ちゃんと考えるべし
}
bool isMulDivable(Type *l, Type *r) { return isNum(l) && isNum(r); }

/////////////////////データ構造

CC_Map_for_LVar *cc_map_for_var_new() {
    CC_Map_for_LVar *heap = calloc(1, sizeof(CC_Map_for_LVar));
    heap->base.root = NULL;
    heap->offset = 0;
    return heap;
}
void cc_map_for_var_delete(CC_Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
}
void cc_map_for_var_clear(CC_Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
    map->offset = 0;
}
void cc_map_for_var_add(CC_Map_for_LVar *map, char *key, int key_len,
                        LVar *item) {
    cc_avltree_Add((CC_AVLTree *)map, key, key_len, (void *)item);
}
void *cc_map_for_var_search(CC_Map_for_LVar *map, char *key, int key_len) {
    return cc_avltree_Search((CC_AVLTree *)map, key, key_len);
}
bool cc_map_for_var_empty(CC_Map_for_LVar *map) {
    return map->base.root == NULL;
}
