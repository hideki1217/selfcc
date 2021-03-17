#pragma once

#include<stdbool.h>

typedef struct CC_IntQueue CC_IntQueue;
typedef struct CC_IntQueueNode CC_IntQueueNode;

typedef struct CC_QueueNode CC_QueueNode;
typedef struct CC_Queue CC_Queue;

typedef struct CC_LimitedQueue CC_LimitedQueue;

typedef struct CC_HeapNode CC_HeapNode;
typedef struct CC_Heap CC_Heap;

typedef struct CC_AVLTreeNode CC_AVLTreeNode;
typedef struct CC_AVLTree CC_AVLTree;

typedef struct CC_Vector CC_Vector;
typedef struct CC_VecNode CC_VecNode;

struct CC_Vector{
    CC_VecNode *first;
};
CC_Vector *cc_vector_new();
void cc_vector_add(CC_Vector* vec,void *item);
bool cc_vector_empty(CC_Vector* vec);
struct CC_VecNode{
    CC_VecNode* next;
    void *item;
};

struct CC_AVLTreeNode{
    char* key;
    int len;
    void *item;
    CC_AVLTreeNode *left;
    CC_AVLTreeNode *right;
};
void cc_avltree_clear(CC_AVLTreeNode *root);
CC_AVLTreeNode *cc_avltree_add(CC_AVLTreeNode *root,char *key,int key_len,void* item);
CC_AVLTreeNode *cc_avltree_search(CC_AVLTreeNode *root,char *key,int key_len);
CC_AVLTreeNode *cc_avltree_deleteNode(CC_AVLTreeNode *root,char *key,int key_len);
struct CC_AVLTree{
    CC_AVLTreeNode *root;
};
CC_AVLTree *cc_avltree_new();
void cc_avltree_Delete(CC_AVLTree* tree);
void cc_avltree_Clear(CC_AVLTree *tree);
void cc_avltree_Add(CC_AVLTree *tree,char *key,int key_len,void* item);
/**
 * @brief  keyとlenをもとに要素を検索(見つからなければNULL)
 * @note   
 * @param  *tree: 対象のAVL木
 * @param  *key: 文字列
 * @param  key_len: 文字列長
 * @retval void*型のpointer
 */
void *cc_avltree_Search(CC_AVLTree *tree,char *key,int key_len);
void cc_avltree_DeleteNode(CC_AVLTree *tree,char *key,int key_len);

struct CC_IntQueue{
    CC_IntQueueNode *top;
    int size;
};
struct CC_IntQueueNode{
    CC_IntQueueNode *back;
    int item;
};
CC_IntQueue *cc_intqueue_new();
bool cc_intqueue_delete(CC_IntQueue *table);
bool cc_intqueue_push(CC_IntQueue *table,int item);
bool cc_intqueue_top(CC_IntQueue *table, int *value);
bool cc_intqueue_pop(CC_IntQueue *table);
void cc_intqueue_clear(CC_IntQueue *table);

struct CC_QueueNode{
    CC_QueueNode *back;
    void *item;
};
struct CC_Queue{
    CC_QueueNode *top;
    int size;
};
CC_Queue *cc_queue_new();
bool cc_queue_delete(CC_Queue *table);
bool cc_queue_push(CC_Queue *table,void *item);
bool cc_queue_top(CC_Queue *table, void **value);
bool cc_queue_pop(CC_Queue *table);
void cc_queue_clear(CC_Queue *table);

struct CC_LimitedQueue{
    void* *list;
    int tail;
    int top;
    int max_size;
    int size;
};
bool cc_limitedqueue_new(CC_LimitedQueue *table,int max_size);
bool cc_limitedqueue_delete(CC_LimitedQueue *table);
bool cc_limitedqueue_push(CC_LimitedQueue *table,void *item);
bool cc_limitedqueue_top(CC_LimitedQueue *table, void *value);
bool cc_limitedqueue_pop(CC_LimitedQueue *table);
bool cc_limitedqueue_get(CC_LimitedQueue *table,int i);


struct CC_HeapNode {
    char* key;
    int key_len;
    void *item;
    int s;
    CC_HeapNode *l, *r;
};
CC_HeapNode* new_cc_heapnode(char* key,int key_len,void* item,CC_HeapNode* lt,CC_HeapNode* rt,int s);
void cc_heapnode_delete(CC_HeapNode* node);
CC_HeapNode* cc_heapnode_meld(CC_HeapNode *a, CC_HeapNode *b);
void swapChildren(CC_HeapNode *h);

struct CC_Heap {
    CC_HeapNode *root;
};
CC_Heap* new_cc_heap();
void cc_heap_delete(CC_Heap* heap);
void cc_heap_clear(CC_Heap* heap);
void cc_heap_push(CC_Heap *heap,char*key,int key_len,void* item);
void cc_heap_meld(CC_Heap* r,CC_Heap *l);
void* cc_heap_top(CC_Heap* heap); 
void cc_heap_pop(CC_Heap* heap);
bool cc_heap_empty(CC_Heap* heap);

int compare_string(const char* a,int a_len,const char* b,int b_len);


