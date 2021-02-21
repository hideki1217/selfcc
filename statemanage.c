#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>

#include "selfcc.h"
#include"collections.h"
#include "utility.h"

CC_AVLTree *type_tree;
void Initialize_type_tree(){
    Type *_void=new_PrimType("void",4,8);
    Type *_int=new_PrimType("int",3,4);
    Type *_char=new_PrimType("char",4,1);

    cc_avltree_Add(type_tree,_void->name,_void->len,_void);
    cc_avltree_Add(type_tree,_int->name,_int->len,_int);
    cc_avltree_Add(type_tree,_char->name,_char->len,_char);
}

///////////////////////型
Type *new_PrimType(char *name, int len, int size) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = PRIM;
    type->name = name;
    type->len = len;
    type->size = size;
    return type;
}
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
    Type* type=(Type*)cc_avltree_Search(type_tree,name,strlen(name));
    if(type)return type;
    return NULL;
}
Type *find_type(Token *token) {
    Type* type=(Type*)cc_avltree_Search(type_tree,token->str,token->len);
    if(type)return type;
    return NULL;
}
int make_memorysize(Type *type) {
    switch (type->ty) {
        case PRIM:
            return type->size;
        case PTR:
            return 8;
        case ARRAY:
            return type->size;
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
    cc_avltree_Add((CC_AVLTree*)map,key,key_len,(void*)item); 
}
void* cc_map_for_var_search(CC_Map_for_LVar *map,char* key,int key_len){
    return cc_avltree_Search((CC_AVLTree*)map,key,key_len);
}
bool cc_map_for_var_empty(CC_Map_for_LVar* map){
    return map->base.root==NULL;
}
