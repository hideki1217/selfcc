#!/bin/bash

assert() { 
    expect="$1"
    input="$2"

    ./selfcc "$input" > tmp.s
    cc -o tmp tmp.s test.o -g
    ./tmp
    actual="$?"

    if [ "$actual" = "$expect" ]; then 
        echo "$input => $actual"
    else
        echo "$input => $expect expected, but got $actual"
        exit 1
    fi
}

assert 0 "int main(){0;}"
assert 42 "int main(){42;}"

assert 2 "int main(){4+3-5;}" 
assert 3 "int main(){9-10+4;}"

assert 2 "int main(){ 4 + 3   - 5;}"
assert 3 "int main(){9 -  10 +4;}"

assert 47 'int main(){5+6*7;}'
assert 15 'int main(){5*(9-6);}'
assert 4 'int main(){(3+5)/2;}'

assert 2 "int main(){-8+10;}"
assert 2 "int main(){+(-8)-(-10);}"

assert 1 "int main(){4+1>3;}"
assert 1 "int main(){4*1>=4;}"
assert 0 "int main(){4*1<+3;}"
assert 1 "int main(){4*1 <= -(-4);}"
assert 1 "int main(){(4*3)-1==11;}"
assert 0 "int main(){(4*3)-1==(3+2)*2;}"
assert 1 "int main(){(4*3)-1!=5*2;}"

assert 15 "int main(){int x;x=3; x*4+3;}"
assert 48 "int main(){int a;a=4;int d; d=3*a; +(a+d)*3;}"

assert 4 "int main(){int abc;abc=4;}"
assert 1 "int main(){int abc;int abcd;abc=5;abcd=abc/5;}"

assert 18 "int main(){int aho;int aho_aho;aho=3; aho_aho=aho*2; return aho_aho*aho;}"
assert 50 "int main(){int x_1;int x_2;x_1=100;x_2=2;return x_1/x_2; return 0;}"

assert 5 "int main(){int x;x=9; if(x==9)return 5;return 3;}"
assert 3 "int main(){int x;x=9; if(x!=9)return 5;return 3;}"

assert 3 "int main(){int x;x=9; if(x!=9)return 5;else return 3;return 1;}"
assert 36 "int main(){int x;x=9; if(x==5)x=x*3;else x=x*4;return x;}"
assert 6 "int main(){int x;x=9; if(x==3) if(x==8)return 3; else return 5; else return 6;}"

assert 5 "int main(){int x;x=0; while(x<5)x=x+1; return x;}"
assert 8 "int main(){int x;x=0; while(x<5) if(x<4)x=x+1;else x=x+4; return x;}"

assert 10 "int main(){int i;int x;i=0;for(x=0;x<5;x=x+1)i=i+x;return i;}"
assert 3 "int main(){int i;int x;i=0;for(x=0;x<5;x=x+1)if(x==3)i=i+x;return i;}"

assert 3 "int main(){3;}"
assert 5 "int main(){int x;{x=3;x*4;}return 5;}"
assert 3 "int main(){
int y_1;
int y_2;
int z;
y_1=0;
y_2=1;
int x;
for(x=0;x<3;x=x+1){
    z=y_1;
    y_1=y_2;
    y_2=z+y_2;
}
return y_2;}"

assert 3 "int main(){int i;int x;for(i=0;i<5;i=i+1)foo(); x=3;}"

assert 8 "int main(){int x;int y;int z;x=3;y=5;z=hoge(x,y);}"
assert 13 "int main(){
int x;
int y;
int z;
x=1;
y=1;
int i;
for(i=0;i<5;i=i+1){
    z=hoge(x,y);
    x=y;
    y=z;
}
return y;}
"
assert 6 "int main(){hogege(1,1,1,1,1,1);}"
assert 7 "int main(){hogehoge(1,1,1,1,1,1,1);}"

assert 3 "int main(){{3;}}"

assert 144 "
int Fibonacci(int x,int y){
    int z;
    z=x+y;
    return z;
}
int main(){
    int x;
    int y;
    x=1;
    y=1;
    int i;
    int z;
    for(i=0;i<10;i=i+1){
        z=Fibonacci(x,y);
        x=y;
        y=z;
        print(y);
    }
    return y;
}
"

assert 6 "
int max(int x,int y){
    if(x>y)return x;
    else return y;
}
int main(){
    int x;
    int y;
    x=6;
    y=5;
    return max(x,y);
}
"

assert 5 "
int huga(int x){
    x=3;
}
int main(){
    int x;
    x=5;
    huga(x);
    return x;
}
"
assert 24 "
int main(){
    return fact(4);
}
"

assert 15 "
int rec(int x){
    if(x==0)return 0;
    return x+rec(x-1);
}
int main(){
    return rec(5);
}
"

assert 6 "
int Euclid(int x,int y){
    int z;
    if(x>y){
        z=x;
        x=y;
        y=z;
    }
    if(x==0)return y;
    return Euclid(x,y-x);
}
int main(){
    return Euclid(6,222);
}
"

assert 6 "
int sum(int a,int b,int c,int d,int e,int f){
    return a+b+c+d+e+f;
}
int main(){
    return sum(1,1,1,1,1,1);
}
"

#assert 7 "                     引数が7この場合は封印
#sum(a,b,c,d,e,f,g){
#    return a+b+c+d+e+f+g;
#}
#int main(){
#   return sum(1,1,1,1,1,1,1);
#}
#"

assert 5 "
int main(){
    int x;
    int y;
    int z;
    x=5;
    y=9;
    z=&y + 8;
    return *z;
}
"

assert 3 "
int main(){
    int x;
    int *y;
    y=&x;
    *y=3;
    return x;
}
"

assert 3 "
int main(){
    int x;
    int y;
    *(&y+8)=3;
    return x;
}
"

assert 3 "
int main(){
    int x;
    x=3;
    int *y;
    y=&x;
    return *y;
}
"

echo OK