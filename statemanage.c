#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>

#include "selfcc.h"
#include"collections.h"

int max(int x,int y){
    return x>y? x:y;
}

///////////////////////型
Type *new_Type(char *name, int len, int size) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = PRIM;
    type->name = name;
    type->len = len;
    type->size = size;
    type->next = types;
    return type;
}
Type *types;  //宣言済みの型
Type *new_Pointer(Type *base) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = PTR;
    type->ptr_to = base;
    type->size = 8;
    return type;
}
Type *new_Array(Type *base, int length) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = ARRAY;
    type->ptr_to = base;
    type->array_len = length;
    type->size = base->size * length;
    return type;
}
Type *find_type_from_name(char *name) {
    for (Type *type = types; type; type = type->next) {
        if (type->len == strlen(name) &&
            memcmp(type->name, name, type->len) == 0) {
            return type;
        }
    }
    return NULL;
}
Type *find_type(Token *token) {
    for (Type *type = types; type; type = type->next) {
        if (type->len == token->len &&
            memcmp(type->name, token->str, type->len) == 0) {
            return type;
        }
    }
    return NULL;
}
int make_memorysize(Type *type) {
    switch (type->ty) {
        case PRIM:
            return type->size;
        case PTR:
            return 8;
        case ARRAY:
            return type->size + 8;
    }
}
bool equal(Type *l, Type *r) {
    while (isArrayorPtr(l) && isArrayorPtr(r)) {
        l = l->ptr_to;
        r = r->ptr_to;
    }
    return l == r;
}
bool isArrayorPtr(Type *type) { return type->ty == PTR || type->ty == ARRAY; }

//////////////////////変数
LVar *new_LVar(Token *token, Type *type) {
    LVar *var = calloc(1, sizeof(LVar));
    var->name = token->str;
    var->len = token->len;
    var->type = type;
    var->offset = cc_map_for_var_empty(locals) ? make_memorysize(type) : locals->offset + make_memorysize(type);
    return var;
}
//宣言済み変数一覧に存在するか確認
LVar *find_lvar(Token *token) {
    return (LVar*)cc_map_for_var_search(locals,token->str,token->len);
}
//なければ作る。あれば二重に定義したことをエラー
LVar *add_lvar(Token *token, Type *type) {
    LVar *res = find_lvar(token);
    if (res == NULL) {
        res = new_LVar(token, type);
        cc_map_for_var_add(locals,res->name,res->len,res);
        return res;
    } else
        error_at(token->str, "同名の変数が既に定義されています");
}
//宣言済み変数一覧になければエラー
LVar *get_lvar(Token *token) {
    LVar *res = find_lvar(token);
    if (res == NULL)
        error_at(token->str, "宣言されていない変数です。");
    else
        return res;
}

/////////////////////データ構造

CC_Map_for_LVar* cc_map_for_var_new(){
    CC_Map_for_LVar *heap=calloc(1,sizeof(CC_Map_for_LVar));
    heap->base.root=NULL;
    heap->offset=0;
    return heap;
}
void cc_map_for_var_delete(CC_Map_for_LVar* map){
    cc_avltree_Clear((CC_AVLTree*)map);
}
void cc_map_for_var_clear(CC_Map_for_LVar* map){
    cc_avltree_Clear((CC_AVLTree*)map);
}
void cc_map_for_var_add(CC_Map_for_LVar *map,char*key,int key_len,LVar* item){
    map->offset=max(item->offset,map->offset);
    cc_avltree_Add((CC_AVLTree*)map,key,key_len,(void*)item); 
}
void* cc_map_for_var_search(CC_Map_for_LVar *map,char* key,int key_len){
    return cc_avltree_Search((CC_AVLTree*)map,key,key_len);
}
bool cc_map_for_var_empty(CC_Map_for_LVar* map){
    return map->base.root==NULL;
}
