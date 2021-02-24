#include<stdio.h>
#include <stdlib.h>
#include<stdarg.h>


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
int print(char * fmt,...){
    va_list ap;
    va_start(ap, fmt);
    printf(fmt,ap);
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
void doexit(int x){
    exit(x);
}