#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collections.h"


/////////////////////////////////AVLNODE
#define CC_AVLTREE_MAX_HEIGHT 100
#define CC_AVLTREE_TREE_LEFT 1
#define CC_AVLTREE_TREE_RIGHT 2

void cc_avltree_delete(CC_AVLTreeNode* node){
    if(node){
        cc_avltree_delete(node->left);
        cc_avltree_delete(node->right);
        free(node);
    }
}
/* cc_avltree_getHeight:二分探索木のノード全てを削除する
   引数１ node : 根ノードのアドレス
   返却値 : nodeを根とした木の高さ */
int cc_avltree_getHeight(CC_AVLTreeNode *node) {
    int left_height;
    int right_height;
    int tree_height;

    if (node == NULL) {
        /* nodeが無いなら高さは0 */
        return 0;
    }

    /* 左右の子を根とした木の高さを取得 */
    left_height = cc_avltree_getHeight(node->left);
    right_height = cc_avltree_getHeight(node->right);

    /* 大きい方に+1したものを木の高さとして返却 */
    if (left_height > right_height) {
        tree_height = left_height;
    } else {
        tree_height = right_height;
    }

    return tree_height + 1;
}

/* cc_avltree_leftRotate:nodeを根とする部分木を回転（左）
   引数１ root : 根のノードを指すアドレス
   引数２ node : 回転する部分木の根ノードを指すアドレス
   引数３ parent : nodeの親ノードを指すアドレス
   引数４ direction : parentから見たnodeのある方向
   返却値 : 根のノードを指すアドレス */
CC_AVLTreeNode *cc_avltree_leftRotate(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent, int direction) {
    /* nodeを根として左回転を行う */

    CC_AVLTreeNode *pivot;
    CC_AVLTreeNode *new_root;

    /* 新しい根とするノードをpivotとして設定 */
    pivot = node->right;

    /* 左回転 */
    if (pivot != NULL) {
        node->right = pivot->left;
        pivot->left = node;
    }

    /* parentもしくはrootに新しい根ノードを参照させる */
    if (parent == NULL) {
        new_root = pivot;
        return new_root;
    }

    /* どちらの子に設定するかはdirectionから判断 */
    if (direction == CC_AVLTREE_TREE_LEFT) {
        parent->left = pivot;
    } else {
        parent->right = pivot;
    }
    return root;
}

/* cc_avltree_rightRotate:nodeを根とする部分木を回転（右）
   引数１ root : 根のノードを指すアドレス
   引数２ node : 回転する部分木の根ノードを指すアドレス
   引数３ parent : nodeの親ノードを指すアドレス
   引数４ direction : parentから見たnodeのある方向
   返却値 : 根のノードを指すアドレス */
CC_AVLTreeNode *cc_avltree_rightRotate(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent, int direction) {
    CC_AVLTreeNode *pivot;
    CC_AVLTreeNode *new_root;

    /* 新しい根とするノードをpivotとして設定 */
    pivot = node->left;

    /* 右回転 */
    if (pivot != NULL) {
        node->left = pivot->right;
        pivot->right = node;
    }

    /* parentもしくはrootに新しい根ノードを参照させる */
    if (parent == NULL) {
        new_root = pivot;
        return new_root;
    }

    /* どちらの子に設定するかはdirectionから判断 */
    if (direction == CC_AVLTREE_TREE_LEFT) {
        parent->left = pivot;
    } else {
        parent->right = pivot;
    }

    return root;
}

/*cc_avltree_rightLeftRotate:nodeを根とする部分木を二重回転（右->左）
   引数１ root : 根のノードを指すアドレス
   引数２ node : 回転する部分木の根ノードを指すアドレス
   引数３ parent : nodeの親ノードを指すアドレス
   引数４ direction : parentから見たnodeのある方向
   返却値 : 根のノードを指すアドレス */
CC_AVLTreeNode *cc_avltree_rightLeftRotate(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent,
                        int direction) {
    /* ２重回転（Right Left Case）を行う */

    CC_AVLTreeNode *new_root;

    /* nodeの右の子ノードを根として右回転 */
    new_root = cc_avltree_rightRotate(root, node->right, node, CC_AVLTREE_TREE_RIGHT);

    /* nodeを根として左回転 */
    return cc_avltree_leftRotate(new_root, node, parent, direction);
}

/* cc_avltree_leftRightRotate:nodeを根する部分木を二重回転（左->右）
   引数１ root : 根のノードを指すアドレス
   引数２ node : 回転する部分木の根ノードを指すアドレス
   引数３ parent : nodeの親ノードを指すアドレス
   引数４ direction : parentから見たnodeのある方向
   返却値 : 根のノードを指すアドレス */
CC_AVLTreeNode *cc_avltree_leftRightRotate(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent,
                        int direction) {
    /* ２重回転（Left Right Case）を行う */

    CC_AVLTreeNode *new_root;

    /* nodeの左の子ノードを根として左回転 */
    new_root = cc_avltree_leftRotate(root, node->left, node, CC_AVLTREE_TREE_LEFT);

    /* nodeを根として右回転 */
    return cc_avltree_rightRotate(new_root, node, parent, direction);
}

/* cc_avltree_balancing:nodeからbranchで辿ったノードを平衡にする
   引数１ root : 根のノードを指すアドレス
   引数２ node : 平衡にするノードを指すアドレス
   引数３ parent : nodeの親ノードを指すアドレス
   引数４ direction : parentから見たnodeのある方向
   引数５ branch : 平衡化を行うノードへの経路
   引数６ num_branch : branchに格納された経路の数
   返却値 : 根のノードを指すアドレス */
CC_AVLTreeNode *cc_avltree_balancing(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent, int direction,
                  int *branch, int num_branch) {
    CC_AVLTreeNode *next;
    CC_AVLTreeNode *new_root;

    int left_height, right_height;
    int balance;

    if (node == NULL || root == NULL) {
        return root;
    }

    if (num_branch > 0) {
        /* 辿れる場合はまず目的のノードまで辿る */

        /* 辿る子ノードを設定 */
        if (branch[0] == CC_AVLTREE_TREE_LEFT) {
            next = node->left;
        } else {
            next = node->right;
        }

        /* 子ノードを辿る */
        new_root =
            cc_avltree_balancing(root, next, node, branch[0], &branch[1], num_branch - 1);
    }

    /* 平衡係数を計算 */
    left_height = cc_avltree_getHeight(node->left);
    right_height = cc_avltree_getHeight(node->right);
    balance = right_height - left_height;

    if (balance > 1) {
        /* 右の部分木が高くて並行状態でない場合 */

        /* ２重回転が必要かどうかを判断 */
        if (cc_avltree_getHeight(node->right->left) >
            cc_avltree_getHeight(node->right->right)) {
            /* ２重回転（Right Left Case）*/
            return cc_avltree_rightLeftRotate(new_root, node, parent, direction);

        } else {
            /*１重回転（左回転）*/
            return cc_avltree_leftRotate(new_root, node, parent, direction);
        }

    } else if (balance < -1) {
        /* 左の部分木が高くて並行状態でない場合 */

        /* ２重回転が必要かどうかを判断 */
        if (cc_avltree_getHeight(node->left->right) >
            cc_avltree_getHeight(node->left->left)) {
            /* ２重回転（Left Right Case）*/
            return cc_avltree_leftRightRotate(new_root, node, parent, direction);
        } else {
            /* １重回転（右回転）*/
            return cc_avltree_rightRotate(new_root, node, parent, direction);
        }
    }

    return root;
}

/* cc_avltree_clear:二分探索木のノード全てを削除する
   引数１ root : 根ノードのアドレス
   返却値 : なし */
void cc_avltree_clear(CC_AVLTreeNode *root) {
    if (root == NULL) {
        return;
    }

    if (root->left != NULL) {
        cc_avltree_clear(root->left);
    }
    if (root->right != NULL) {
        cc_avltree_clear(root->right);
    }
    free(root);
}

/* cc_avltree_new_node:ノードの構造体のメモリを確保し、データを設定
   引数１ key : 追加する文字列
   引数２ key_len : 追加する文字列の長さ
   引数3 item : keyに関連づけられたポインタ
   返却値 : 追加したノードのアドレス */
CC_AVLTreeNode *cc_avltree_new_node(char *key,int key_len,void* item) {
    CC_AVLTreeNode *add;

    add = (CC_AVLTreeNode *)malloc(sizeof(CC_AVLTreeNode));
    if (add == NULL) {
        return NULL;
    }

    add->left = NULL;
    add->right = NULL;
    add->key = key;
    add->len=key_len;
    add->item=item;

    return add;
}

/* cc_avltree_add:指定されたnumberとname持つノードを追加する
   引数１ root : 根ノードのアドレス
   引数２ key : 追加するkeyの名前
   引数３ key_len : 追加するkeyの長さ
   引数4 item : keyと関連付けられたポインタ
   返却値 : 根ルートのアドレス */
CC_AVLTreeNode *cc_avltree_add(CC_AVLTreeNode *root,char* key,int key_len,void* item) {
    CC_AVLTreeNode *node;
    int branch[CC_AVLTREE_MAX_HEIGHT] = {0};
    int num_branch = 0;

    /* まだノードが一つもない場合 */
    if (root == NULL) {
        /* 根ノードとしてノードを追加 */
        root = cc_avltree_new_node(key,key_len,item);
        if (root == NULL) {
            printf("malloc error\n");
            return NULL;
        }
        return root;
    }

    /* 根ノードから順に追加する場所を探索 */
    node = root;
    while (1) {
        int compare_res=compare_string(key,key_len,node->key,node->len);
        if (compare_res>0) {
            /* 追加する値がノードの値よりも小さい場合 */

            if (node->left == NULL) {
                /* そのノードの左の子が無い場合（もう辿るべきノードが無い場合）*/

                /* その左の子の位置にノードを追加 */
                node->left = cc_avltree_new_node(key,key_len,item);

                /* 追加完了したので処理終了 */
                break;
            }

            /* 左ノードを辿ったことを覚えておく */
            branch[num_branch] = CC_AVLTREE_TREE_LEFT;
            num_branch++;

            /* 左の子がある場合は左の子を新たな注目ノードに設定 */
            node = node->left;

        } else if (compare_res<0) {
            /* 追加する値がノードの値よりも大きい場合 */

            if (node->right == NULL) {
                /* そのノードの右の子が無い場合（もう辿るべきノードが無い場合）*/

                /* その右の子の位置にノードを追加 */
                node->right = cc_avltree_new_node(key,key_len,item);

                /* 追加完了したので処理終了 */
                break;
            }

            /* 右ノードを辿ったことを覚えておく */
            branch[num_branch] = CC_AVLTREE_TREE_RIGHT;
            num_branch++;

            /* 右の子がある場合は右の子を新たな注目ノードに設定 */
            node = node->right;
        } else {
            /* 追加する値とノードの値が同じ場合 */
            break;
        }
    }

    return cc_avltree_balancing(root, root, NULL, 0, branch, num_branch);
}

/* cc_avltree_search:指定されたnumberを持つノードを探索する
   引数１ root : 探索を開始するノードのアドレス
   引数２ key : 探索するkeyの名前
   引数３ key_len : 探索するkeyの長さ
   返却値 : number を持つノードのアドレス（存在しない場合は NULL）*/
CC_AVLTreeNode *cc_avltree_search(CC_AVLTreeNode *root,char* key,int key_len) {
    CC_AVLTreeNode *node;

    node = root;

    /* 探索を行うループ（注目ノードがNULLになったら終了 */
    while (node) {
        int compare_res=compare_string(key,key_len,node->key,node->len);
        if (compare_res>0) {
            /* 探索値がノードの値よりも小さい場合 */

            /* 注目ノードを左の子ノードに設定 */
            node = node->left;
        } else if (compare_res<0) {
            /* 探索値がノードの値よりも大きい場合 */

            /* 注目ノードを右の子ノードに設定 */
            node = node->right;
        } else {
            /* 探索値 = ノードの値の場合 */
            return node;
        }
    }

    /* 探索値を持つノードが見つからなかった場合 */
    return NULL;
}

/* cc_avltree_deleteNoChildeNode:指定された子の無いノードを削除する
   引数１ root : 木の根ノードのアドレス
   引数２ node : 削除するノードのアドレス
   引数３ parent：削除するノードの親ノードのアドレス
   返却値 : 根ノードのアドレス */
CC_AVLTreeNode *cc_avltree_deleteNoChildNode(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *parent) {
    if (parent != NULL) {
        /* 親がいる場合（根ノード以外の場合）は
        削除対象ノードを指すポインタをNULLに設定 */
        if (parent->left == node) {
            /* 削除対象ノードが親ノードから見て左の子の場合 */
            parent->left = NULL;
        } else {
            /* 削除対象ノードが親ノードから見て右の子の場合 */
            parent->right = NULL;
        }
        free(node);
    } else {
        /* 削除対象ノードが根ノードの場合 */
        free(node);

        /* 根ノードを指すポインタをNULLに設定 */
        root = NULL;
    }

    return root;
}

/* cc_avltree_deleteOneChildeNode:指定された子が一つのノードを削除する
   引数１ root : 木の根ノードのアドレス
   引数２ node : 削除するノードのアドレス
   引数３ child : 削除するノードの子ノードのアドレス
   返却値 : 根ノードのアドレス */
CC_AVLTreeNode *cc_avltree_deleteOneChildNode(CC_AVLTreeNode *root, CC_AVLTreeNode *node, CC_AVLTreeNode *child) {
    /* 削除対象ノードにその子ノードのデータとポインタをコピー */
    node->key = child->key;
    node->len=child->len;
    node->item=child->item;
    node->left = child->left;
    node->right = child->right;

    /* コピー元のノードを削除 */
    free(child);

    return root;
}

/* cc_avltree_deleteTwoChildeNode:指定された子が二つのノードを削除する
   引数１ root : 木の根ノードのアドレス
   引数２ node : 削除するノードのアドレス
   返却値 : 根ノードのアドレス */
CC_AVLTreeNode *cc_avltree_deleteTwoChildNode(CC_AVLTreeNode *root, CC_AVLTreeNode *node, int *branch,
                           int *num_branch) {
    CC_AVLTreeNode *max;
    CC_AVLTreeNode *maxParent;

    /* 左の子から一番大きい値を持つノードを探索 */
    max = node->left;
    maxParent = node;

    /* 左の子ノードを辿ったことを覚えておく */
    branch[*num_branch] = CC_AVLTREE_TREE_LEFT;
    (*num_branch)++;

    while (max->right != NULL) {
        maxParent = max;
        max = max->right;

        /* 右の子ノードを辿ったことを覚えておく */
        branch[*num_branch] = CC_AVLTREE_TREE_RIGHT;
        (*num_branch)++;
    }

    /* 最大ノードのデータのみ削除対象ノードにコピー */
    node->key = max->key;
    node->len=max->len;
    node->item=max->item;

    /* 最大ノードを削除 */

    /* maxは最大ノードなので必ずmax->rightはNULLになる */
    if (max->left == NULL) {
        /* 最大ノードに子がいない場合 */
        root = cc_avltree_deleteNoChildNode(root, max, maxParent);
    } else {
        /* 最大ノードに子供が一ついる場合 */
        root = cc_avltree_deleteOneChildNode(root, max, max->left);
    }

    return root;
}

/* cc_avltree_deleteNode:指定されたnumberを持つノードを削除する
   引数１ root : 木の根ノードのアドレス
   引数２ number : 削除する会員番号
   返却値 : 根ノードのアドレス */
CC_AVLTreeNode *cc_avltree_deleteNode(CC_AVLTreeNode *root, char* key,int key_len) {
    CC_AVLTreeNode *node;
    CC_AVLTreeNode *parent;
    int branch[CC_AVLTREE_MAX_HEIGHT] = {0};
    int num_branch = 0;

    if (root == NULL) {
        return NULL;
    }

    /* 削除対象ノードを指すノードを探索 */
    node = root;
    parent = NULL;

    while (node != NULL) {
        int compare_res=compare_string(key,key_len,node->key,node->len);
        if (compare_res>0) {
            parent = node;
            node = node->left;

            /* 左の子ノードを辿ったことを覚えておく */
            branch[num_branch] = CC_AVLTREE_TREE_LEFT;
            num_branch++;
        } else if (compare_res<0) {
            parent = node;
            node = node->right;

            /* 右の子ノードを辿ったことを覚えておく */
            branch[num_branch] = CC_AVLTREE_TREE_RIGHT;
            num_branch++;
        } else {
            break;
        }
    }

    /* 指定されたnumberを値として持つノードが存在しない場合は何もせず終了 */
    if (node == NULL) {
        return root;
    }

    if (node->left == NULL && node->right == NULL) {
        /* 子がいないノードの削除 */
        root = cc_avltree_deleteNoChildNode(root, node, parent);
    } else if ((node->left != NULL && node->right == NULL) ||
               (node->right != NULL && node->left == NULL)) {
        /* 子が一つしかない場合 */

        if (node->left != NULL) {
            root = cc_avltree_deleteOneChildNode(root, node, node->left);
        } else {
            root = cc_avltree_deleteOneChildNode(root, node, node->right);
        }
    } else {
        /* 左の子と右の子両方がいるノードの削除 */
        root = cc_avltree_deleteTwoChildNode(root, node, branch, &num_branch);
    }

    return cc_avltree_balancing(root, root, NULL, 0, branch, num_branch);
}

//////////////////////////AVLTree 

CC_AVLTree *cc_avltree_new(){
    CC_AVLTree *tree=calloc(1,sizeof(CC_AVLTree));
    tree->root=NULL;
    return tree;
}
void cc_avltree_Delete(CC_AVLTree* tree){
    cc_avltree_delete(tree->root);
    free(tree);
}
void cc_avltree_Clear(CC_AVLTree *tree){
    cc_avltree_clear(tree->root);
    tree->root=NULL;
}
void cc_avltree_Add(CC_AVLTree *tree,char *key,int key_len,void* item){
    tree->root=cc_avltree_add(tree->root,key,key_len,item);
}
void *cc_avltree_Search(CC_AVLTree *tree,char *key,int key_len){
    CC_AVLTreeNode *node=cc_avltree_search(tree->root,key,key_len);
    if(node)return node->item;
    return NULL;
}
void cc_avltree_DeleteNode(CC_AVLTree *tree,char *key,int key_len){
    tree->root=cc_avltree_deleteNode(tree->root,key,key_len);
}
