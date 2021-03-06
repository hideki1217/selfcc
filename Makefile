CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c) test/fortest.c
OBJS=$(SRCS:.c=.o) 

selfcc: $(OBJS) test/test.c
			gcc -o test/fortest.o -c test/fortest.c 
			$(CC) -o selfcc $(OBJS) $(LDFLAGS)

$(OBJS): selfcc.h $(SRCS)

test: selfcc 
		./ctest.sh

tmptest: selfcc
		./tmptest.sh

pretest: selfcc	
		./test.sh

clean: 
		rm -f selfcc *.o *~ tmp* a.out

.PHONY: test clean 