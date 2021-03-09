CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c) test/fortest.c
OBJS=$(SRCS:.c=.o) 

selfcc: $(OBJS)
			gcc -o test/fortest.o -c test/fortest.c 
			$(CC) -o selfcc $(OBJS) $(LDFLAGS)

$(OBJS): selfcc.h

test: selfcc 
		#./test.sh
		./ctest.sh


clean: 
		rm -f selfcc *.o *~ tmp* a.out

.PHONY: test clean 