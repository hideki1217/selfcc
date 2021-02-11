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

echo OK