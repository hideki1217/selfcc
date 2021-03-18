#pragma once

#include <stdbool.h>

typedef struct CC_Iterable CC_Iterable;

typedef struct CC_VecNode CC_VecNode;
typedef struct CC_Vector CC_Vector;

typedef struct CC_PtrNode CC_PtrNode;
typedef struct CC_PtrVector CC_PtrVector;

typedef struct CC_IntNode CC_IntNode;
typedef struct CC_IntVector CC_IntVector;

#define foreach(name,vector) \
    for(CC_VecNode *name = vector->front;name != NULL; name = (name)->next)

struct CC_Iterable
{
    CC_Iterable *next;
};

struct CC_VecNode{
    CC_VecNode *next;
    CC_VecNode *prev;
    union{
        void* ptr; 
        int val;
        struct{
            char *str;
            int len;
        } string;
    }item;
};
void cc_vecnode_delete(CC_VecNode *front);
/**
 * @brief  ...cur => ...cur*(new)
 * @note   
 * @param  *cur: 挿入する右端
 * @retval 挿入したノード
 */
CC_VecNode *cc_vecnode_new(CC_VecNode *cur);
struct CC_Vector{
    CC_VecNode *front;
    CC_VecNode *back;
    int size;
};
CC_Vector *cc_vector_new();
void cc_vector_delete(CC_Vector *vec);
/**
 * @brief  要素を消す
 * @note   
 * @param  *vec: 対象のvector
 * @retval None
 */
void cc_vector_clear(CC_Vector *vec);
void cc_vector_pbPtr(CC_Vector *vec, void *ptr);
void cc_vector_pbInt(CC_Vector *vec, int val);
void cc_vector_pbStr(CC_Vector *vec, char *str,int len);
/**
 * @brief  lf...lb,rf...rb => lf...lb*rf...rb
 * @note   
 * @param  *l: 結合の右側
 * @param  *r: 結合の左側
 * @retval None
 */
void cc_vector_concat(CC_Vector *l,CC_Vector *r);
/**
 * @brief  ...*begin...X*end... => ...*item*end...
 * @note   
 * @param  *vec: 元のvector
 * @param  *begin: 挿入の左端
 * @param  *end: 挿入の右端の右
 * @param  *item: 挿入したいvector
 * @retval None
 */
void cc_vector_insert(CC_Vector *vec,CC_VecNode *begin,CC_VecNode *end,CC_Vector *item);
bool cc_vecotr_isEmpty(const CC_Vector *vec);
int cc_vector_size(const CC_Vector *vec);

/*
struct CC_PtrNode{
    CC_Iterable itr;
    void *item;
};
struct CC_PtrVector{
    CC_PtrNode *front;
    CC_PtrNode *top;
    int size;
};
CC_PtrVector *cc_ptrvector_New();
void cc_ptrvector_Delete(CC_PtrVector *vec);
void cc_ptrvector_Add(CC_PtrVector *vec, void *item);
void cc_ptrvector_Concat(CC_PtrVector *l,CC_PtrVector *r);
bool cc_ptrvecotr_Empty(CC_PtrVector *vec);
int cc_ptrvector_Size(CC_PtrVector *vec);

struct CC_IntNode{
    CC_Iterable itr;
    int val;
};
struct CC_IntVector{
    CC_IntNode *front;
    CC_IntNode *top;
    int size;
};
CC_IntVector *cc_intvector_New();
void cc_intvector_Delete(CC_IntVector *vec);
void cc_intvector_Add(CC_IntVector *vec, int val);
void cc_intvector_Concat(CC_IntVector *l,CC_IntVector *r);
bool cc_intvecotr_Empty(CC_IntVector *vec);
int cc_intvector_Size(CC_IntVector *vec);

struct CC_StrNode{
    CC_Iterable itr;
    char *str;
    int len;
};
struct CC_StrVector{
    CC_StrNode *front;
    CC_StrNode *top;
    int size;
};
CC_IntVector *cc_intvector_New();
void cc_intvector_Delete(CC_IntVector *vec);
void cc_intvector_Add(CC_IntVector *vec, int val);
void cc_intvector_Concat(CC_IntVector *l,CC_IntVector *r);
bool cc_intvecotr_Empty(CC_IntVector *vec);
int cc_intvector_Size(CC_IntVector *vec);

*/
