#include "vector.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define NULL ((void *)0)

void cc_vector_init(CC_Vector *vec) {
    vec->_ = calloc(VEC_MAX_SIZE, sizeof(Object));
    vec->max_size = VEC_MAX_SIZE;
    vec->size = 0;
}
CC_Vector *cc_vector_new() {
    CC_Vector *vec = calloc(1, sizeof(CC_Vector));
    cc_vector_init(vec);
    return vec;
}
void cc_vector_delete(CC_Vector *vec) {
    free(vec->_);
    free(vec);
}
void cc_vector_clear(CC_Vector *vec) {
    free(vec->_);
    cc_vector_init(vec);
}
static void *cc_vector_grow(CC_Vector *vec) {
    Object *tmp = vec->_;
    vec->_ = calloc(vec->max_size + VEC_MAX_SIZE, sizeof(Object));
    memcpy(vec->_, tmp, vec->max_size * sizeof(Object));
    free(tmp);
}
void cc_vector_pbPtr(CC_Vector *vec, void *ptr) {
    if (vec->size == vec->max_size) cc_vector_grow(vec);
    vec->_[vec->size++].ptr = ptr;
}
void cc_vector_pbInt(CC_Vector *vec, int val) {
    if (vec->size == vec->max_size) cc_vector_grow(vec);
    vec->_[vec->size++].ival = val;
}
void cc_vector_pbStr(CC_Vector *vec, char *str, int len) {
    if (vec->size == vec->max_size) cc_vector_grow(vec);
    vec->_[vec->size].string.str = str;
    vec->_[vec->size++].string.len = len;
}
void cc_vector_pfPtr(CC_Vector *vec,void *ptr){
    if (vec->size == vec->max_size) cc_vector_grow(vec);
    for(int i = vec->size-1;i>=0 ;i--){
        vec->_[i+1] = vec->_[i];
    }
    vec->_[0].ptr = ptr;
    vec->size++;
}
int cc_vector_findStr(CC_Vector *vec, char *str, int len) {
    for (int i = 0; i < vec->size; i++) {
        Object item = (vec->_)[i];
        if (string_cmp(item.string.str, item.string.len, str, len) == 0)
            return i;
    }
    return -1;
}
CC_Iterable *cc_vector_begin(CC_Vector *vec) {
    CC_VecIterator *iter = calloc(1, sizeof(CC_VecIterator));
    cc_veciterator_init(iter, vec);
    return (CC_Iterable *)iter;
}
Object cc_vector_(CC_Vector *vec, int index) { return vec->_[index]; }

void cc_veciterator_init(CC_VecIterator *iter, CC_Vector *vec) {
    iter->base.next = cc_veciterator_next;
    iter->base.item = cc_veciterator_item;
    iter->index = 0;
    iter->vec = vec;
}
CC_Iterable *cc_veciterator_next(CC_Iterable *this) {
    CC_VecIterator *iter = (CC_VecIterator *)this;
    iter->index++;
    if (iter->index == iter->vec->size) return NULL;
    return this;
}
Object cc_veciterator_item(CC_Iterable *this) {
    CC_VecIterator *iter = (CC_VecIterator *)this;
    return iter->vec->_[iter->index];
}

#define CONCAT(vnd1, vnd2)         \
    if (vnd1) (vnd1)->next = vnd2; \
    if (vnd2) (vnd2)->prev = vnd1;

int _cc_bidlistnode_delete(CC_BidListNode *begin, CC_BidListNode *end) {
    int count = 0;
    while (begin != end) {
        count++;
        CC_BidListNode *tmp = begin;
        begin = begin->next;
        free(tmp);
    }
    return count;
}
void cc_bidlistnode_delete(CC_BidListNode *front) {
    _cc_bidlistnode_delete(front, NULL);
}
CC_BidListNode *cc_bidlistnode_new(CC_BidListNode *cur) {
    CC_BidListNode *nd = calloc(1, sizeof(CC_BidListNode));
    CONCAT(cur, nd);
    nd->next = NULL;
}

CC_BidList *cc_bidlist_new() {
    CC_BidList *vec = calloc(1, sizeof(CC_BidList));
    vec->front = NULL;
    vec->back = NULL;
    vec->size = 0;
    return vec;
}
void cc_bidlist_delete(CC_BidList *vec) {
    cc_bidlistnode_delete(vec->front);
    free(vec);
}
void cc_bidlist_clear(CC_BidList *vec) { cc_bidlistnode_delete(vec->front); }
void cc_bidlist_pbPtr(CC_BidList *vec, void *ptr) {
    CC_BidListNode *nd = cc_bidlistnode_new(vec->back);
    if (vec->back == NULL) vec->front = nd;
    nd->obj.ptr = ptr;
    vec->back = nd;
    vec->size++;
}
void cc_bidlist_pbInt(CC_BidList *vec, int val) {
    CC_BidListNode *nd = cc_bidlistnode_new(vec->back);
    if (vec->back == NULL) vec->front = nd;
    nd->obj.ival = val;
    vec->back = nd;
    vec->size++;
}
void cc_bidlist_pbStr(CC_BidList *vec, char *str, int len) {
    CC_BidListNode *nd = cc_bidlistnode_new(vec->back);
    if (vec->back == NULL) vec->front = nd;
    nd->obj.string.str = str;
    nd->obj.string.len = len;
    vec->back = nd;
    vec->size++;
}
void cc_bidlist_concat(CC_BidList *l, CC_BidList *r) {
    CONCAT(l->back, r->front);
    l->back = r->back;
    l->size += r->size;
}
void cc_bidlist_insert(CC_BidList *vec, CC_BidListNode *begin,
                       CC_BidListNode *end, CC_BidList *item) {
    CC_BidListNode *left = begin->prev;
    CONCAT(left, item->front);
    CONCAT(item->back, end);
    int size = _cc_bidlistnode_delete(begin, end);
    vec->size += item->size - size;
}
bool cc_bidlist_isEmpty(const CC_BidList *vec) { return vec->size == 0; }
int cc_bidlist_size(const CC_BidList *vec) { return vec->size; }

CC_SortedStrList *cc_sortedstrlist_clone(CC_SortedStrList *list) {
    CC_SortedStrList *res = cc_sortedstrlist_new();
    LIST_rFOR(iter, list) {
        cc_sortedstrlist_add(res, iter->obj.string.str, iter->obj.string.len);
    }
    return res;
}
void cc_sortedstrlist_add(CC_SortedStrList *list, char *str, int len) {
    if (!list) return;
    if (list->size == 0) {
        CC_BidListNode *nd = cc_bidlistnode_new(NULL);
        nd->obj.string.str = str;
        nd->obj.string.len = len;
        list->front = nd;
        list->back = nd;
        list->size++;
        return;
    }
    LIST_FOR(iter, list) {
        String string = iter->obj.string;
        int res = string_cmp(string.str, string.len, str, len);
        if (res == 0) return;
        if (res < 0) {  // str < iter
            CC_BidListNode *nd = cc_bidlistnode_new(iter->prev);
            if (iter == list->front) {
                list->front = nd;
            }
            nd->obj.string.str = str;
            nd->obj.string.len = len;
            list->size++;
            CONCAT(nd, iter);
            return;
        }
        if (res > 0)  // str > iter
            continue;
    }
    CC_BidListNode *nd = cc_bidlistnode_new(list->back);
    nd->obj.string.str = str;
    nd->obj.string.len = len;
    list->size++;
    list->back = nd;
    return;
}
bool cc_sortedstrlist_find(CC_SortedStrList *list, char *str, int len) {
    LIST_FOR(iter, list) {
        int res =
            string_cmp(iter->obj.string.str, iter->obj.string.len, str, len);
        if (res == 0) return true;
    }
    return false;
}
CC_SortedStrList *cc_sortedstrlist_cross(CC_SortedStrList *l,
                                         CC_SortedStrList *r) {
    CC_SortedStrList *res = cc_sortedstrlist_new();
    LIST_FOR(iter, l) {
        if (cc_sortedstrlist_find(r, iter->obj.string.str,
                                  iter->obj.string.len)) {
            cc_sortedstrlist_add(res, iter->obj.string.str,
                                 iter->obj.string.len);
        }
    }
    return res;
}
CC_SortedStrList *cc_sortedstrlist_sum(CC_SortedStrList *l,
                                       CC_SortedStrList *r) {
    CC_SortedStrList *res = cc_sortedstrlist_new();
    LIST_rFOR(iter, l) {
        cc_sortedstrlist_add(res, iter->obj.string.str, iter->obj.string.len);
    }
    LIST_rFOR(iter, r) {
        cc_sortedstrlist_add(res, iter->obj.string.str, iter->obj.string.len);
    }
    return res;
}

#undef CONCAT
#undef NULL