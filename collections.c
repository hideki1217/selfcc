#include "collections.h"

#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"

CC_Vector *cc_vector_new() {
    CC_Vector *vec = calloc(1, sizeof(CC_Vector));
    vec->first = NULL;
    return vec;
}
void cc_vector_add(CC_Vector *vec, void *item) {
    CC_VecNode *nd = calloc(1, sizeof(CC_VecNode));
    nd->next = vec->first;
    nd->item = item;

    vec->first = nd;
}
bool cc_vector_empty(CC_Vector *vec) { return vec->first == NULL; }

CC_HeapNode *new_cc_heapnode(char *key, int key_len, void *item,
                             CC_HeapNode *lt, CC_HeapNode *rt, int s) {
    CC_HeapNode *node = calloc(1, sizeof(CC_HeapNode));
    node->key = key;
    node->key_len = key_len;
    node->item = item;
    node->l = lt;
    node->r = rt;
    node->s = s;
    return node;
}
void cc_heapnode_delete(CC_HeapNode *node) {
    if (node == NULL) return;
    cc_heapnode_delete(node->l);
    cc_heapnode_delete(node->r);
    free(node);
}
CC_HeapNode *cc_heapnode_meld(CC_HeapNode *a, CC_HeapNode *b) {
    if (a == NULL) return b;
    if (b == NULL) return a;
    if (compare_string(a->key, a->key_len, b->key, b->key_len) > 0)
        swap((void **)&a, (void **)&b);
    if (a->l == NULL)
        a->l = b;
    else {
        a->r = cc_heapnode_meld(a->r, b);
        if (a->l->s < a->r->s) {
            swapChildren(a);
        }
        a->s = a->r->s + 1;
    }
    return a;
}
void swapChildren(CC_HeapNode *h) {
    CC_HeapNode *tmp = h->l;
    h->l = h->r;
    h->r = tmp;
}

CC_Heap *new_cc_heap() {
    CC_Heap *heap = calloc(1, sizeof(CC_Heap));
    heap->root = NULL;
}
void cc_heap_delete(CC_Heap *heap) {
    if (heap->root != NULL) {
        cc_heapnode_delete(heap->root->l);
        cc_heapnode_delete(heap->root->r);
        free(heap->root);
    }
    free(heap);
}
void cc_heap_clear(CC_Heap *heap) {
    if (heap->root != NULL) {
        cc_heapnode_delete(heap->root->l);
        cc_heapnode_delete(heap->root->r);
        free(heap->root);
    }
    heap->root = NULL;
}
void cc_heap_push(CC_Heap *heap, char *key, int key_len, void *item) {
    heap->root = cc_heapnode_meld(
        new_cc_heapnode(key, key_len, item, NULL, NULL, 0), heap->root);
}
void cc_heap_meld(CC_Heap *l, CC_Heap *r) {
    if (r == l) return;
    l->root = cc_heapnode_meld(l->root, r->root);
    r->root = NULL;
}
void *cc_heap_top(CC_Heap *heap) { return heap->root->item; }
void cc_heap_pop(CC_Heap *heap) {
    CC_HeapNode *oldroot = heap->root;
    heap->root = cc_heapnode_meld(heap->root->l, heap->root->r);
    free(oldroot);
}
bool cc_heap_empty(CC_Heap *heap) { return heap->root == NULL; }

CC_IntQueue *cc_intqueue_new(){
    CC_IntQueue *queue = calloc(1,sizeof(CC_IntQueue));
    return queue;
}
bool cc_intqueue_delete(CC_IntQueue *table){
    while(cc_intqueue_pop(table));
    free(table);
    return true;
}
bool cc_intqueue_push(CC_IntQueue *table,int item){
    CC_IntQueueNode *node = calloc(1, sizeof(CC_IntQueueNode));
    node->item = item;
    node->back = table->top;
    table->top = node;
    table->size++;
    return true;
}
bool cc_intqueue_top(CC_IntQueue *table, int *value){
    if (table->size == 0){
        *value = -1;
        return false;
    }
    *value = table->top->item;
    return true;
}
bool cc_intqueue_pop(CC_IntQueue *table){
    if (table->size == 0) return false;
    CC_IntQueueNode *nd = table->top;
    table->top = table->top->back;
    table->size--;
    free(nd);
    return true;
}
void cc_intqueue_clear(CC_IntQueue *table){
    if(table){
        while (cc_intqueue_pop(table))
        ;
    }
}

CC_Queue *cc_queue_new() {
    CC_Queue *table = calloc(1, sizeof(CC_Queue));
    table->top = NULL;
    table->size = 0;
    return table;
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
bool cc_queue_top(CC_Queue *table, void **value) {
    if (table->size == 0){
        *value = NULL;
        return false;
    }
    *value = table->top->item;
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
void cc_queue_clear(CC_Queue *table){
    if(table){
        while (cc_queue_pop(table))
        ;
    }
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
