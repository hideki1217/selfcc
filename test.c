#include<stdio.h>

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