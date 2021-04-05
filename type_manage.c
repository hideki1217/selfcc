#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"
#include "selfcc.h"
#include "utility.h"

/////////////////////データ構造

LVar_Manager *lvar_manager_new() {
    LVar_Manager *manager = calloc(1, sizeof(LVar_Manager));
    manager->queue = cc_queue_new();
    manager->top = NULL;
    manager->max_offset = 0;
}
void lvar_manager_PushScope(LVar_Manager *manager) {
    Map_for_LVar *map = map_for_var_new();
    map->offset = manager->top ? manager->top->offset : 0;
    cc_queue_push(manager->queue, map);
    manager->top = map;
}
void lvar_manager_PopScope(LVar_Manager *manager) {
    int offset = manager->top->offset;
    if (manager->queue->size) cc_queue_pop(manager->queue);
    cc_queue_top(manager->queue, (void **)&(manager->top));
    manager->max_offset = max(manager->max_offset, offset);
}
int lvar_manager_Add(LVar_Manager *manager, char *key, int len, LVar *var) {
    Map_for_LVar *map = manager->top;
    map_for_var_add(map, key, len, var);
}
int lvar_manager_GetOffset(LVar_Manager *manager) {
    if (manager->top) return manager->top->offset;
    return 0;
}
void lvar_manager_SetOffset(LVar_Manager *manager, int offset) {
    if (manager->top) manager->top->offset = offset;
}
void lvar_manager_Clear(LVar_Manager *manager) {
    if (manager == NULL) return;
    cc_queue_clear(manager->queue);
    manager->top = NULL;
    manager->max_offset = 0;
}
int lvar_manager_GetTotalOffset(LVar_Manager *manager) {
    return manager->max_offset;
}
LVar *lvar_manager_Find(LVar_Manager *manager, char *key, int len,
                        bool nowScope) {
    void *res;
    for (CC_QueueNode *nd = manager->queue->top; nd; nd = nd->back) {
        Map_for_LVar *map = nd->item;
        if (res = map_for_var_search(map, key, len)) return (LVar *)res;
        if (nowScope) break;
    }
    return NULL;
}

Map_for_LVar *map_for_var_new() {
    Map_for_LVar *heap = calloc(1, sizeof(Map_for_LVar));
    heap->base.root = NULL;
    heap->offset = 0;
    return heap;
}
void map_for_var_delete(Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
}
void map_for_var_clear(Map_for_LVar *map) {
    cc_avltree_Clear((CC_AVLTree *)map);
    map->offset = 0;
}
void map_for_var_add(Map_for_LVar *map, char *key, int key_len, LVar *item) {
    cc_avltree_Add((CC_AVLTree *)map, key, key_len, (void *)item);
}
void *map_for_var_search(Map_for_LVar *map, char *key, int key_len) {
    return cc_avltree_Search((CC_AVLTree *)map, key, key_len);
}
bool map_for_var_empty(Map_for_LVar *map) { return map->base.root == NULL; }
