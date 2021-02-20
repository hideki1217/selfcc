#include<stdbool.h>

typedef struct CC_QueueNode CC_QueueNode;
typedef struct CC_Queue CC_Queue;

typedef struct CC_LimitedQueue CC_LimitedQueue;

typedef struct CC_HeapNode CC_HeapNode;
typedef struct CC_Heap CC_Heap;

struct CC_QueueNode{
    CC_QueueNode *back;
    void *item;
};
struct CC_Queue{
    CC_QueueNode *top;
    int size;
};
bool cc_queue_new(CC_Queue *table);
bool cc_queue_delete(CC_Queue *table);
bool cc_queue_push(CC_Queue *table,void *item);
bool cc_queue_top(CC_Queue *table, void *value);
bool cc_queue_pop(CC_Queue *table);

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
    CC_HeapNode *l, *r;
};
CC_Heap* new_cc_heap();
void cc_heap_delete(CC_Heap* heap);
void cc_heap_push(CC_Heap *heap,char*key,int key_len,void* item);
void cc_heap_meld(CC_Heap* r,CC_Heap *l);
void* cc_heap_top(CC_Heap* heap); 
void cc_heap_pop(CC_Heap* heap);
bool cc_heap_empty(CC_Heap* heap)

int compare_string(const char* a,int a_len,const char* b,int b_len);


