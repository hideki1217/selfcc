#include "collections.h"

#include<stdlib.h>
#include<stdbool.h>

bool cc_queue_new(CC_Queue *table){
    table=calloc(1,sizeof(CC_Queue));
    table->top=NULL;
    table->size=0;
    return true;
}
bool cc_queue_delete(CC_Queue *table){
    if(table==NULL)false;
    while(cc_queue_pop(table));
    free(table);
    return true;
}
bool cc_queue_push(CC_Queue *table,void *item){
    CC_QueueNode *node=calloc(1,sizeof(CC_QueueNode));
    node->item=item;
    node->back=table->top;
    table->top=node;
    table->size++;
    return true;
}
bool cc_queue_top(CC_Queue *table, void *value){
    if(table->size==0)return false;
    value=table->top;
    return true;
}
bool cc_queue_pop(CC_Queue *table){
    if(table->size==0)return false;
    CC_QueueNode *nd=table->top;
    table->top=table->top->back;
    table->size--;
    free(nd);
    return true;
}

bool cc_limitedqueue_new(CC_LimitedQueue *table,int max_size){
    table=calloc(1,sizeof(CC_LimitedQueue));
    table->list=calloc(max_size,sizeof(void*));
    table->max_size=max_size;
    table->tail=-1;
    table->top=0;
    table->size=0;
    return true;
}
bool cc_limitedqueue_delete(CC_LimitedQueue *table){
    if(table==NULL)false;
    free(table->list);
    free(table);
    return true;
}
bool cc_limitedqueue_push(CC_LimitedQueue *table,void *item){
    *(table->list+table->top)=item;
    if(table->size==table->max_size)
        table->tail=(table->tail+1)&table->max_size;
    else
        table->size++;
    table->top=(table->top+1)&table->max_size;
    return true;
}
bool cc_limitedqueue_top(CC_LimitedQueue *table, void *value){
    if(table->size==0)return false;
    value=table->list[(table->top-1+table->max_size)%table->max_size];
    return true;
}
bool cc_limitedqueue_pop(CC_LimitedQueue *table){
    if(table->size==0)return false;
    table->size--;
    table->tail=(table->tail+1)%table->max_size;
    return true;
}

