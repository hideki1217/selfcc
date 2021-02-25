#!/bin/bash

./selfcc "./test/test.c" > ./test/tmp.s
cc -o ./test/tmp ./test/tmp.s ./test/fortest.o -g
./test/tmp