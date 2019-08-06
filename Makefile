CFLAGS=-std=c11 -g -static

jcc: jcc.c

test: jcc
	./test.sh

clean:
	rm -f jcc *.o *~ tmp*

.PHONY: test clean
