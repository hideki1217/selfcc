#pragma once

#include <stdbool.h>

typedef struct CC_Iterable CC_Iterable;

typedef union CC_Item CC_Item;

typedef struct CC_Vector CC_Vector;
typedef struct CC_VecIterator CC_VecIterator;

typedef struct CC_BidListNode CC_BidListNode;
typedef struct CC_BidList CC_BidList;


struct CC_Iterable{
    CC_Iterable *(*next)(CC_Iterable *this);
    CC_Item (*item)(CC_Iterable *this);
};

union CC_Item{
    void* ptr; 
    int val;
    struct{
        char *str;
        int len;
    } string;
};

#define VEC_MAX_SIZE 10
#define VEC_FOR(name,vector) \
    for(CC_Iterable *name = cc_vector_begin(vector);name;name = name->next(name))
/**
 * @brief  ランダムアクセス可能な可変長配列
 */
struct CC_Vector{
    CC_Item *items;
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
CC_Iterable *cc_vector_begin(CC_Vector *vec);
CC_Item cc_vector_(CC_Vector *vec,int index);
struct CC_VecIterator{
    CC_Iterable base;
    CC_Vector *vec;
    int index;
};
void cc_veciterator_init(CC_VecIterator *iter,CC_Vector *vec);
CC_Iterable *cc_veciterator_next(CC_Iterable *this);
CC_Item cc_veciterator_item(CC_Iterable *this);


#define LIST_FOR(name,list) \
    for(CC_BidListNode *name = (list)->front;name != NULL; name = (name)->next)
/**
 * @brief  双方向リスト
 */
struct CC_BidListNode{
    CC_BidListNode *next;
    CC_BidListNode *prev;
    CC_Item item;
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
 * @brief  ...*begin...X*end... => ...*item*end...
 * @note   
 * @param  *vec: 元のvector
 * @param  *begin: 挿入の左端
 * @param  *end: 挿入の右端の右
 * @param  *item: 挿入したいvector
 * @retval None
 */
void cc_bidlist_insert(CC_BidList *vec,CC_BidListNode *begin,CC_BidListNode *end,CC_BidList *item);
bool cc_bidlist_isEmpty(const CC_BidList *vec);
int cc_bidlist_size(const CC_BidList *vec);
