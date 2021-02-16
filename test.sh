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

assert 0 "main(){0;}"
assert 42 "main(){42;}"

assert 2 "main(){4+3-5;}" 
assert 3 "main(){9-10+4;}"

assert 2 "main(){ 4 + 3   - 5;}"
assert 3 "main(){9 -  10 +4;}"

assert 47 'main(){5+6*7;}'
assert 15 'main(){5*(9-6);}'
assert 4 'main(){(3+5)/2;}'

assert 2 "main(){-8+10;}"
assert 2 "main(){+(-8)-(-10);}"

assert 1 "main(){4+1>3;}"
assert 1 "main(){4*1>=4;}"
assert 0 "main(){4*1<+3;}"
assert 1 "main(){4*1 <= -(-4);}"
assert 1 "main(){(4*3)-1==11;}"
assert 0 "main(){(4*3)-1==(3+2)*2;}"
assert 1 "main(){(4*3)-1!=5*2;}"

assert 15 "main(){x=3; x*4+3;}"
assert 48 "main(){a=4; d=3*a; +(a+d)*3;}"
assert 120 "main(){a=1;b=a*2;c=b*3;d=c*4;e=d*5;}"

assert 4 "main(){abc=4;}"
assert 1 "main(){abc=5;abcd=abc/5;}"

assert 18 "main(){aho=3; aho_aho=aho*2; return aho_aho*aho;}"
assert 50 "main(){x_1=100;x_2=2;return x_1/x_2; return 0;}"

assert 5 "main(){x=9; if(x==9)return 5;return 3;}"
assert 3 "main(){x=9; if(x!=9)return 5;return 3;}"

assert 3 "main(){x=9; if(x!=9)return 5;else return 3;return 1;}"
assert 36 "main(){x=9; if(x==5)x=x*3;else x=x*4;return x;}"
assert 6 "main(){x=9; if(x==3) if(x==8)return 3; else return 5; else return 6;}"

assert 5 "main(){x=0; while(x<5)x=x+1; return x;}"
assert 8 "main(){x=0; while(x<5) if(x<4)x=x+1;else x=x+4; return x;}"

assert 10 "main(){i=0;for(x=0;x<5;x=x+1)i=i+x;return i;}"
assert 3 "main(){i=0;for(x=0;x<5;x=x+1)if(x==3)i=i+x;return i;}"

assert 3 "main(){3;}"
assert 5 "main(){{x=3;x*4;}return 5;}"
assert 3 "main(){y_1=0;
y_2=1;
for(x=0;x<3;x=x+1){
    z=y_1;
    y_1=y_2;
    y_2=z+y_2;
}
return y_2;}"

assert 3 "main(){for(i=0;i<5;i=i+1)foo(); x=3;}"

assert 8 "main(){x=3;y=5;z=hoge(x,y);}"
assert 13 "main(){
x=1;
y=1;
for(i=0;i<5;i=i+1){
    z=hoge(x,y);
    x=y;
    y=z;
}
return y;}
"
assert 6 "main(){hogege(1,1,1,1,1,1);}"
assert 7 "main(){hogehoge(1,1,1,1,1,1,1);}"

assert 3 "main(){{3;}}"

assert 144 "
Fibonacci(x,y){
    z=x+y;
    return z;
}
main(){
    x=1;
    y=1;
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
max(x,y){
    if(x>y)return x;
    else return y;
}
main(){
    x=6;
    y=5;
    return max(x,y);
}
"

assert 5 "
huga(x){
    x=3;
}
main(){
    x=5;
    huga(x);
    return x;
}
"
assert 24 "
main(){
    return fact(4);
}
"

assert 15 "
rec(x){
    if(x==0)return 0;
    return x+rec(x-1);
}
main(){
    return rec(5);
}
"

assert 6 "
Euclid(x,y){
    if(x>y){
        z=x;
        x=y;
        y=z;
    }
    if(x==0)return y;
    return Euclid(x,y-x);
}
main(){
    return Euclid(6,222);
}
"

assert 6 "
sum(a,b,c,d,e,f){
    return a+b+c+d+e+f;
}
main(){
    return sum(1,1,1,1,1,1);
}
"

#assert 7 "                     引数が7この場合は封印
#sum(a,b,c,d,e,f,g){
#    return a+b+c+d+e+f+g;
#}
#main(){
#   return sum(1,1,1,1,1,1,1);
#}
#"

assert 5 "
main(){
    x=5;
    y=9;
    z=&y + 8;
    return *z;
}
"

echo OK