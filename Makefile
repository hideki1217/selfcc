CFLAGS=-std=c11 -g -static

selfcc: selfcc.c

test: selfcc
		./test.sh

clean: 
		rm -f selfcc *.o *~ tmp*

.PHONY: test clean