#include "selfcc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "utility.h"

static Type *_void;
static Type *_int;
static Type *_char;
static Type *_float;
static Type *_long;
static Type *_double;

static CC_AVLTree *type_tree;
static CC_AVLTree *struct_tree;

Type *new_BaseType(TypeKind kind,int size);
void Initialize_type_tree() {
    _void = new_BaseType(TY_VOID, 8);
    _int = new_BaseType(TY_INT, 4);
    _char = new_BaseType(TY_CHAR, 1);
    _float = new_BaseType(TY_FLOAT,  1);
    _long = new_BaseType(TY_LONG, 1);
    _double = new_BaseType(TY_DOUBLE, 1);

    type_tree = cc_avltree_new();
    struct_tree = cc_avltree_new(); 

    cc_avltree_Add(type_tree, "void", 4, _void);
    cc_avltree_Add(type_tree,  "int", 3, _int);
    cc_avltree_Add(type_tree, "char", 4, _char);
    cc_avltree_Add(type_tree, "float", 5, _float);
    cc_avltree_Add(type_tree, "long", 4, _double);
    cc_avltree_Add(type_tree, "double", 6,  _long);
}

void regist_type(Type *tp) { cc_avltree_Add(type_tree, tp->name, tp->len, tp); }

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
            string_ncopy(s, tp->name, tp->len);
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
void InitType(Type *tp) {
    tp->isConst = false;
    tp->isVolatile = false;
}
Type *new_BaseType(TypeKind kind, int size) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = kind;
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
Type *new_Function(Type *base, Params *arg) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = TY_FUNCTION;
    type->ptr_to = base;
    type->params = arg;
    type->size = 8;
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
Type *new_Struct(Type *base) {
    return NULL;  // TODO: struct を作ったらここに加筆
}
Type *new_Alias(Type *base, char *name, int len) {
    Type *type = calloc(1, sizeof(Type));
    InitType(type);
    type->kind = TY_ALIAS;
    type->ptr_to = base;
    type->name = name;
    type->len = len;
    type->size = base->size;
    return type;
}

Type *clone_Type(Type *tp) {
    Type *type = calloc(1, sizeof(Type));
    memcpy(type, tp, sizeof(Type));
    return type;
}
Type *find_type_from_name(char *name) {
    Type *type = (Type *)cc_avltree_Search(type_tree, name, strlen(name));
    if (type) return clone_Type(type);  // そのものではなくてコピーを渡す
    return NULL;
}
Type *find_type(Token *token) {
    Type *type = (Type *)cc_avltree_Search(type_tree, token->str, token->len);
    if (type) return clone_Type(type);  // そのものではなくてコピーを渡す
    return NULL;
}
int make_memorysize(Type *type) { return type->size; }
bool equal(Type *l, Type *r) {
    if (l == NULL && r != NULL || l != NULL && r == NULL) return false;
    if (l == r) return true;
    if (l->kind != r->kind) return false;
    switch (r->kind) {
        case TY_PTR:
            return equal(l->ptr_to, r->ptr_to);
        case TY_ARRAY:
            return l->array_len == r->array_len ? equal(l->ptr_to, r->ptr_to)
                                                : false;
        case TY_FUNCTION: {
            Params *lpar = l->params;
            Params *rpar = r->params;
            Param *lp, *rp;
            for (lp = lpar->root, rp = rpar->root; lp != NULL && rp != NULL;
                 lp = lp->next, rp = rp->next) {
                if (!equal(lp->type, rp->type)) return false;
            }
            return lp == rp;
        }
        case TY_STRUCT:
        case TY_UNION:
        case TY_ENUM:
            return l->len == r->len && memcmp(l->name, r->name, l->len) == 0;
        default:
            return l->kind == r->kind;
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
bool isFloat(Type *type) { return isNum(type) && !isInteger(type); }
bool isAssignable(Type *l, Type *r) {
    if (equal(l, r)) return true;
    if (l->kind == TY_PTR && l->ptr_to->kind == TY_VOID && r->kind == TY_PTR)
        return true;
    if (l->kind == TY_PTR && isArrayorPtr(r)) return true;
    switch (l->kind) {
        case TY_DOUBLE:
            return isNum(r);
        case TY_FLOAT:
            return isNum(r) && r->kind != TY_DOUBLE;
        case TY_LONG:
            return isInteger(r);
        case TY_INT:
        case TY_CHAR:
            return isInteger(r) && r->kind != TY_LONG;
    }
    return false;
}
bool isLeftsidevalue(Type *tp) {
    return !(tp->kind == TY_ARRAY || tp->kind == TY_FUNCTION);
}
bool isAddSubable(Type *l, Type *r) {
    return isAssignable(l, r);  // TODO: ちゃんと考えるべし
}
bool isMulDivable(Type *l, Type *r) { return isNum(l) && isNum(r); }

Callability isCallable(Type *tp) {
    if (tp->kind == TY_FUNCTION) return AsFUNCTION;
    if (tp->kind == TY_PTR && tp->ptr_to->kind == TY_FUNCTION)
        return AsPTR2FUNC;
    return CANNOT;
}

Type *commonType(Type *l, Type *r) {
    switch (l->kind) {
        case TY_DOUBLE:
            return (isNum(r)) ? l : NULL;
        case TY_FLOAT:
            if (!isNum(r)) return NULL;
            return (r->kind == TY_DOUBLE) ? r : l;
        case TY_LONG:
            if (!isNum(r)) return NULL;
            return isInteger(r) ? l : r;
        case TY_INT:
            if (!isNum(r)) return NULL;
            if (!isInteger(r)) return r;
            return (r->kind == TY_LONG) ? r : l;
        case TY_CHAR:
            return ( ! isNum(r) )? NULL: r;
        default:
            error("その型の共通部分は未対応だよう");  
    }
}

/////////////////////データ構造

LVar_Manager *lvar_manager_new() {
    LVar_Manager *manager = calloc(1, sizeof(LVar_Manager));
    manager->queue = cc_queue_new();
    manager->top = NULL;
    manager->max_offset = 0;
}
void lvar_manager_PushScope(LVar_Manager *manager) {
    Map_for_LVar *map = map_for_var_new();
    map->offset = manager->top ? manager->top->offset : 0;
    cc_queue_push(manager->queue, map);
    manager->top = map;
}
void lvar_manager_PopScope(LVar_Manager *manager) {
    int offset = manager->top->offset;
    if (manager->queue->size) cc_queue_pop(manager->queue);
    cc_queue_top(manager->queue, (void **)&(manager->top));
    manager->max_offset = max(manager->max_offset, offset);
}
int lvar_manager_Add(LVar_Manager *manager, char *key, int len, LVar *var) {
    Map_for_LVar *map = manager->top;
    map_for_var_add(map, key, len, var);
}
int lvar_manager_GetOffset(LVar_Manager *manager) {
    if (manager->top) return manager->top->offset;
    return 0;
}
void lvar_manager_SetOffset(LVar_Manager *manager, int offset) {
    if (manager->top) manager->top->offset = offset;
}
void lvar_manager_Clear(LVar_Manager *manager) {
    if (manager == NULL) return;
    cc_queue_clear(manager->queue);
    manager->top = NULL;
    manager->max_offset = 0;
}
int lvar_manager_GetTotalOffset(LVar_Manager *manager) {
    return manager->max_offset;
}
LVar *lvar_manager_Find(LVar_Manager *manager, char *key, int len,
                        bool nowScope) {
    void *res;
    for (CC_QueueNode *nd = manager->queue->top; nd; nd = nd->back) {
        Map_for_LVar *map = nd->item;
        if (res = map_for_var_search(map, key, len)) return (LVar *)res;
        if (nowScope) break;
    }
    return NULL;
}

Map_for_LVar *map_for_var_new() {
    Map_for_LVar *heap = calloc(1, sizeof(Map_for_LVar));
    heap->base.root = NULL;
    heap->offset = 0;
    return heap;
}
void map_for_var_delete(Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
}
void map_for_var_clear(Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
    map->offset = 0;
}
void map_for_var_add(Map_for_LVar *map, char *key, int key_len, LVar *item) {
    cc_avltree_Add((CC_AVLTree *)map, key, key_len, (void *)item);
}
void *map_for_var_search(Map_for_LVar *map, char *key, int key_len) {
    return cc_avltree_Search((CC_AVLTree *)map, key, key_len);
}
bool map_for_var_empty(Map_for_LVar *map) { return map->base.root == NULL; }
