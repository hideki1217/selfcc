#!/bin/bash

assert() { 
    expect="$1"
    input="$2"

    ./selfcc "$input" > tmp.s
    cc -o tmp tmp.s test.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expect" ]; then 
        echo "$input => $actual"
    else
        echo "$input => $expect expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"

assert 2 "4+3-5;" 
assert 3 "9-10+4;"

assert 2 " 4 + 3   - 5;"
assert 3 "9 -  10 +4;"

assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 2 "-8+10;"
assert 2 "+(-8)-(-10);"

assert 1 "4+1>3;"
assert 1 "4*1>=4;"
assert 0 "4*1<+3;"
assert 1 "4*1 <= -(-4);"
assert 1 "(4*3)-1==11;"
assert 0 "(4*3)-1==(3+2)*2;"
assert 1 "(4*3)-1!=5*2;"

assert 15 "x=3; x*4+3;"
assert 48 "a=4; d=3*a; +(a+d)*3;"
assert 120 "a=1;b=a*2;c=b*3;d=c*4;e=d*5;"

assert 4 "abc=4;"
assert 1 "abc=5;abcd=abc/5;"

assert 18 "aho=3; aho_aho=aho*2; return aho_aho*aho;"
assert 50 "x_1=100;x_2=2;return x_1/x_2; return 0;"

assert 5 "x=9; if(x==9)return 5;return 3;"
assert 3 "x=9; if(x!=9)return 5;return 3;"

assert 3 "x=9; if(x!=9)return 5;else return 3;return 1;"
assert 36 "x=9; if(x==5)x=x*3;else x=x*4;return x;"
assert 6 "x=9; if(x==3) if(x==8)return 3; else return 5; else return 6;"

assert 5 "x=0; while(x<5)x=x+1; return x;"
assert 8 "x=0; while(x<5) if(x<4)x=x+1;else x=x+4; return x;"

assert 10 "i=0;for(x=0;x<5;x=x+1)i=i+x;return i;"
assert 3 "i=0;for(x=0;x<5;x=x+1)if(x==3)i=i+x;return i;"

assert 3 "{3;}"
assert 5 "{x=3;x*4;}return 5;"
assert 3 "y_1=0;
y_2=1;
for(x=0;x<3;x=x+1){
    z=y_1;
    y_1=y_2;
    y_2=z+y_2;
}
return y_2;"

assert 3 "for(i=0;i<5;i=i+1)foo(); x=3;"

assert 8 "x=3;y=5;z=hoge(x,y);"
assert 13 "
x=1;
y=1;
for(i=0;i<5;i=i+1){
    z=hoge(x,y);
    x=y;
    y=z;
}
return y;
"
assert 6 "hogege(1,1,1,1,1,1);"
assert 7 "hogehoge(1,1,1,1,1,1,1);"

echo OK