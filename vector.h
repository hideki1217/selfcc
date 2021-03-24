#pragma once

#include <stdbool.h>
#include "utility.h"

typedef struct CC_Iterable CC_Iterable;

typedef union Object Object;

typedef struct CC_Vector CC_Vector;
typedef struct CC_VecIterator CC_VecIterator;

typedef struct CC_BidListNode CC_BidListNode;
typedef struct CC_BidList CC_BidList;

typedef struct CC_BidList CC_SortedStrList; 


#define ITER_NEXT(iter) (iter)->next(iter)
#define ITER_ITEM(iter) (iter)->item(iter)
struct CC_Iterable{
    CC_Iterable *(*next)(CC_Iterable *this); // 次に進める
    Object (*item)(CC_Iterable *this); // 要素を返す
};


#define VEC_MAX_SIZE 10
#define VEC_FOR(name,vector) \
    for(CC_Iterable *name = cc_vector_begin(vector);name;name = ITER_NEXT(name))
#define VEC(vector,index) (vector)->_[index]
/**
 * @brief  ランダムアクセス可能な可変長配列
 */
struct CC_Vector{
    Object *_;
    int size;
    int max_size;
};
CC_Vector *cc_vector_new();
void cc_vector_delete(CC_Vector *vec);
void cc_vector_clear(CC_Vector *vec);
void cc_vector_init(CC_Vector *vec);
void cc_vector_pbPtr(CC_Vector *vec, void *ptr);
void cc_vector_pbInt(CC_Vector *vec, int val);
void cc_vector_pbStr(CC_Vector *vec, char *str,int len);
/**あればindexなければ-1*/
int cc_vector_findStr(CC_Vector *vec, char *str,int len);
CC_Iterable *cc_vector_begin(CC_Vector *vec);
Object cc_vector_(CC_Vector *vec,int index);
struct CC_VecIterator{
    CC_Iterable base;
    CC_Vector *vec;
    int index;
};
void cc_veciterator_init(CC_VecIterator *iter,CC_Vector *vec);
CC_Iterable *cc_veciterator_next(CC_Iterable *this);
Object cc_veciterator_item(CC_Iterable *this);


#define LIST_FOR(name,list) \
    for(CC_BidListNode *name = (list)->front;name != NULL; name = (name)->next)
#define LIST_rFOR(name,list) \
    for(CC_BidListNode *name = (list)->back;name != NULL; name = (name)->prev)
/**
 * @brief  双方向リスト
 */
struct CC_BidListNode{
    CC_BidListNode *next;
    CC_BidListNode *prev;
    Object obj;
};
void cc_bidlistnode_delete(CC_BidListNode *front);
/**
 * @brief  ...cur => ...cur*(new)
 * @note   
 * @param  *cur: 挿入する右端
 * @retval 挿入したノード
 */
CC_BidListNode *cc_bidlistnode_new(CC_BidListNode *cur);
struct CC_BidList{
    CC_BidListNode *front;
    CC_BidListNode *back;
    int size;
};
CC_BidList *cc_bidlist_new();
void cc_bidlist_delete(CC_BidList *vec);
/**
 * @brief  要素を消す
 * @note   
 * @param  *vec: 対象のvector
 * @retval None
 */
void cc_bidlist_clear(CC_BidList *vec);
void cc_bidlist_pbPtr(CC_BidList *vec, void *ptr);
void cc_bidlist_pbInt(CC_BidList *vec, int val);
void cc_bidlist_pbStr(CC_BidList *vec, char *str,int len);
/**
 * @brief  lf...lb,rf...rb => lf...lb*rf...rb
 * @note   
 * @param  *l: 結合の右側
 * @param  *r: 結合の左側
 * @retval None
 */
void cc_bidlist_concat(CC_BidList *l,CC_BidList *r);
/**
 * @brief  ...*begin...X*end... => ...*obj*end...
 * @note   
 * @param  *vec: 元のvector
 * @param  *begin: 挿入の左端
 * @param  *end: 挿入の右端の右
 * @param  *obj: 挿入したいvector
 * @retval None
 */
void cc_bidlist_insert(CC_BidList *vec,CC_BidListNode *begin,CC_BidListNode *end,CC_BidList *item);
bool cc_bidlist_isEmpty(const CC_BidList *vec);
int cc_bidlist_size(const CC_BidList *vec);


#define cc_sortedstrlist_new() cc_bidlist_new()
#define cc_sortedstrlist_delete(vec) cc_bidlist_delete(vec)
CC_SortedStrList *cc_sortedstrlist_clone(CC_SortedStrList *list);
void cc_sortedstrlist_add(CC_SortedStrList *list,char *str,int len);
bool cc_sortedstrlist_find(CC_SortedStrList *list,char *str,int len);
CC_SortedStrList *cc_sortedstrlist_cross(CC_SortedStrList *l,CC_SortedStrList *r);
CC_SortedStrList *cc_sortedstrlist_sum(CC_SortedStrList *l,CC_SortedStrList *r);