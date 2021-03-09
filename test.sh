#!/bin/bash

assert() { 
    expect="$1"
    input="$2"

    ./selfcc --row "$input" > tmp.s
    cc -o tmp tmp.s test/fortest.o -g
    ./tmp
    actual="$?"

    if [ "$actual" = "$expect" ]; then 
        echo "$input => $actual"
    else
        echo "$input => $expect expected, but got $actual"
        exit 1
    fi
}


assert 5 "
int sum(int x,int y){
    return x+y;
}
int main(){
    int (*x)(int,int);
    x=sum;
    return (*x)(2,3);
}

"

echo OK
