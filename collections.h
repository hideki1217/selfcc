#include<stdbool.h>

typedef struct CC_QueueNode CC_QueueNode;
typedef struct CC_Queue CC_Queue;

typedef struct CC_LimitedQueue CC_LimitedQueue;

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

