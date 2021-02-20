#include "collections.h"

#include <stdbool.h>
#include <stdlib.h>


CC_HeapNode* new_cc_heapnode(char* key,int key_len,void* item,CC_HeapNode* lt,CC_HeapNode* rt,int s){
    CC_HeapNode *node=calloc(1,sizeof(CC_HeapNode));
    node->key=key;
    node->key_len=key_len;
    node->item=item;
    node->l=lt;
    node->r=rt;
    node->s=s;
    return node;
}
void cc_heapnode_delete(CC_HeapNode* node){
    cc_heapnode_delete(node->l);
    cc_heapnode_delete(node->r);
    free(node);
}
CC_HeapNode* cc_heapnode_meld(CC_HeapNode *a, CC_HeapNode *b){
    if (a == NULL) return b;
    if (b == NULL) return a;
    if (!compare_string(a->key,a->key_len, b->key,b->key_len)) swap(a, b);
    if (a->l == NULL)
        a->l = b;
    else {
        a->r = meld(a->r, b);
        if (a->l->s < a->r->s) {
            swapChildren(a);
        }
        a->s = a->r->s + 1;
    }
    return a;
}
void* cc_heapnode_find(CC_HeapNode *node,char *key,int key_len){
    if(node==NULL)return NULL;
    int result=compare_string(node->key,node->key_len,key,key_len);
    if(result==0)return node->item;
    else if(result>0)return cc_heapnode_find(node->l,key,key_len);
    else return cc_heapnode_find(node->r,key,key_len);
}
void swapChildren(CC_HeapNode *h){
    CC_HeapNode *tmp = h->l;
    h->l = h->r;
    h->r = tmp;
}

CC_Heap* new_cc_heap(){
    CC_Heap *heap=calloc(1,sizeof(CC_Heap));
    heap->root=NULL;
}
void cc_heap_delete(CC_Heap* heap){
    cc_heapnode_delete(heap->l);
    cc_heapnode_delete(heap->r);
    free(heap->root);
    free(heap);
}
void cc_heap_push(CC_Heap *heap,char*key,int key_len,void* item){
    heap->root = meld(new_cc_heapnode(key,key_len,item,NULL,NULL,0), heap->root);
}
void cc_heap_meld(CC_Heap* l,CC_Heap *r){
    if (r == l) return;
    l->root = meld(l->root, r->root);
    r->root = NULL;
}
void* cc_heap_top(CC_Heap* heap){
    return heap->root->item;
}
void* cc_heap_find(CC_Heap *heap,char *key,int key_len){
    return cc_heapnode_find(heap->root,key,key_len);
}
void cc_heap_pop(CC_Heap* heap){
    CC_HeapNode *oldroot = heap->root;
    heap->root = meld(heap->root->l, heap->root->r);
    free(oldroot);
}
bool cc_heap_empty(CC_Heap* heap){
    return heap->root == NULL;
}

// a>b -> 負数, a<b -> 正数, a=b -> 0
int compare_string(const char* a,int a_len,const char* b,int b_len){
    int a_i=0,b_i=0;
    while(a[a_i]==b[b_i]){
        a_i++;
        b_i++;
        if(a_i==a_len&&b_i!=b_len)return 1;
        if(a_i!=a_len&&b_i==b_len)return -1;
        if(a_i==a_len&&b_i==b_len)return 0;   
    }
    return (*a-*b);
}



bool cc_queue_new(CC_Queue *table) {
    table = calloc(1, sizeof(CC_Queue));
    table->top = NULL;
    table->size = 0;
    return true;
}
bool cc_queue_delete(CC_Queue *table) {
    if (table == NULL) false;
    while (cc_queue_pop(table))
        ;
    free(table);
    return true;
}
bool cc_queue_push(CC_Queue *table, void *item) {
    CC_QueueNode *node = calloc(1, sizeof(CC_QueueNode));
    node->item = item;
    node->back = table->top;
    table->top = node;
    table->size++;
    return true;
}
bool cc_queue_top(CC_Queue *table, void *value) {
    if (table->size == 0) return false;
    value = table->top;
    return true;
}
bool cc_queue_pop(CC_Queue *table) {
    if (table->size == 0) return false;
    CC_QueueNode *nd = table->top;
    table->top = table->top->back;
    table->size--;
    free(nd);
    return true;
}

bool cc_limitedqueue_new(CC_LimitedQueue *table, int max_size) {
    table = calloc(1, sizeof(CC_LimitedQueue));
    table->list = calloc(max_size, sizeof(void *));
    table->max_size = max_size;
    table->tail = -1;
    table->top = 0;
    table->size = 0;
    return true;
}
bool cc_limitedqueue_delete(CC_LimitedQueue *table) {
    if (table == NULL) false;
    free(table->list);
    free(table);
    return true;
}
bool cc_limitedqueue_push(CC_LimitedQueue *table, void *item) {
    *(table->list + table->top) = item;
    if (table->size == table->max_size)
        table->tail = (table->tail + 1) & table->max_size;
    else
        table->size++;
    table->top = (table->top + 1) & table->max_size;
    return true;
}
bool cc_limitedqueue_top(CC_LimitedQueue *table, void *value) {
    if (table->size == 0) return false;
    value = table->list[(table->top - 1 + table->max_size) % table->max_size];
    return true;
}
bool cc_limitedqueue_pop(CC_LimitedQueue *table) {
    if (table->size == 0) return false;
    table->size--;
    table->tail = (table->tail + 1) % table->max_size;
    return true;
}
