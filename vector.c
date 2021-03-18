#include "vector.h"

#include <stdbool.h>
#include <stdlib.h>

#define NULL ((void *)0)
#define CONCAT(vnd1, vnd2) \
    if(vnd1)(vnd1)->next = vnd2;     \
    if(vnd2)(vnd2)->prev = vnd1;

int _cc_vecnode_delete(CC_VecNode *begin,CC_VecNode *end){
    int count =0;
    while(begin != end){
        count ++;
        CC_VecNode *tmp = begin;
        begin =begin->next;
        free(tmp);
    }
    return count;
} 
void cc_vecnode_delete(CC_VecNode *front) {
    _cc_vecnode_delete(front,NULL);
}
CC_VecNode *cc_vecnode_new(CC_VecNode *cur){
    CC_VecNode *nd=calloc(1,sizeof(CC_VecNode));
    CONCAT(cur,nd);
    nd->next = NULL;
}

CC_Vector *cc_vector_new() {
    CC_Vector *vec = calloc(1, sizeof(CC_Vector));
    vec->front = NULL;
    vec->back = NULL;
    vec->size = 0;
    return vec;
}
void cc_vector_delete(CC_Vector *vec) {
    cc_vecnode_delete(vec->front);
    free(vec);
}
void cc_vector_clear(CC_Vector *vec) { cc_vecnode_delete(vec->front); }
void cc_vector_pbPtr(CC_Vector *vec, void *ptr) {
    CC_VecNode *nd = cc_vecnode_new(vec->back);
    if(vec->back == NULL)vec->front = nd;
    nd->item.ptr = ptr;
    vec->back = nd;
    vec->size++;
}
void cc_vector_pbInt(CC_Vector *vec, int val) {
    CC_VecNode *nd = cc_vecnode_new(vec->back);
    if(vec->back == NULL)vec->front = nd;
    nd->item.val = val;
    vec->back = nd;
    vec->size++;
}
void cc_vector_pbStr(CC_Vector *vec, char *str, int len) {
    CC_VecNode *nd = cc_vecnode_new(vec->back);
    if(vec->back == NULL)vec->front = nd;
    nd->item.string.str = str;
    nd->item.string.len = len;
    vec->back = nd;
    vec->size++;
}
void cc_vector_concat(CC_Vector *l, CC_Vector *r) {
    CONCAT(l->back,r->front);
    l->back = r->back;
    l->size += r->size;
}
void cc_vector_insert(CC_Vector* vec,CC_VecNode *begin, CC_VecNode *end, CC_Vector *item) {
    CC_VecNode *left = begin->prev;
    CONCAT(left,item->front);
    CONCAT(item->back,end);
    int size = _cc_vecnode_delete(begin,end);
    vec->size += item->size - size;
}
bool cc_vecotr_isEmpty(const CC_Vector *vec) { return vec->size == 0; }
int cc_vector_size(const CC_Vector *vec) { return vec->size; }



#undef CONCAT
#undef NULL