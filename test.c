#include<stdio.h>
#include <stdlib.h>

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
/*
int main(){
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 2;
    *q;  
    q = p + 3;
    return *q;
}*/