CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

selfcc: $(OBJS)
			$(CC) -o selfcc $(OBJS) $(LDFLAGS)

$(OBJS): selfcc.h

test: selfcc
		./test.sh

testoftest: test.c
		gcc -o test test.c collections.c avltree.c -g

clean: 
		rm -f selfcc *.o *~ tmp*

.PHONY: test clean