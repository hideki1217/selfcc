#!/bin/bash

assert() { 
    expect="$1"
    input="$2"

    ./selfcc "$input" > tmp.s
    cc -o tmp tmp.s
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
assert 5 "x=9; if(x==3) if(x==8)return 3; else return 5; else return 6;"


echo OK