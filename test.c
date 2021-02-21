#include<stdio.h>
#include <stdlib.h>

#include "collections.h"

int foo(){
    printf("Hello World\n");
}

int hoge(int x,int y){
    int z=x+y;
    printf("%d\n",z);
    return z;
}

int hogege(int a,int b,int c,int d,int e,int f){
    return a+b+c+d+e+f;
}
int hogehoge(int a,int b,int c,int d,int e,int f,int g){
    return a+b+c+d+e+f+g;
}

int print(int x){
    printf("%d\n",x);
}

int fact(int x){
    if(x==1)return 1;
    return x*fact(x-1);
}

void alloc4(int **p ,int a,int b,int c,int d){
    *p=calloc(4,sizeof(int));
    **p=a;
    *(*p+1)=b;
    *(*p+2)=c;
    *(*p+3)=d;
}

struct aho{
    int a;
    int b;
};
struct aho* new_aho(int a,int b){
    struct aho* hoge=calloc(1,sizeof(struct aho));
    hoge->a=a;
    hoge->b=b;
    return hoge;  
}

/*
int main(){
    CC_AVLTree *tree=cc_avltree_new();
    char *s_1="x_1";
    void *item_1=new_aho(1,2);

    char*s_2="x_2";
    void*item_2=new_aho(3,4);

    char*s_3="x_3";
    void *item_3=new_aho(5,6);

    char*s_4="x_4";
    void *item_4=new_aho(7,8);

    print(compare_string(s_1,3,s_2,3));
    print(compare_string(s_2,3,s_3,3));
    print(compare_string(s_3,3,s_4,4));
    print(compare_string(s_2,3,s_2,3));

    cc_avltree_Add(tree,s_1,3,item_1);
    cc_avltree_Add(tree,s_2,3,item_2);
    cc_avltree_Add(tree,s_3,3,item_3);
    cc_avltree_Add(tree,s_4,3,item_4);

    print(((struct aho*)cc_avltree_Search(tree,s_1,3))->a);
    print(((struct aho*)cc_avltree_Search(tree,s_2,3))->a);
    print(((struct aho*)cc_avltree_Search(tree,s_3,3))->a);
    print(((struct aho*)cc_avltree_Search(tree,s_4,3))->a);
}*/