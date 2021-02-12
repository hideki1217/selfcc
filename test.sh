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

assert 0 0
assert 42 42

assert 2 "4+3-5" 
assert 3 "9-10+4"

assert 2 " 4 + 3   - 5"
assert 3 "9 -  10 +4"

assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'

assert 2 "-8+10"
assert 2 "+(-8)-(-10)"

assert 1 "4+1>3"
assert 1 "4*1>=4"
assert 0 "4*1<+3"
assert 1 "4*1 <= -(-4)"
assert 1 "(4*3)-1==11"
assert 0 "(4*3)-1==(3+2)*2"
assert 1 "(4*3)-1!=5*2"


echo OK