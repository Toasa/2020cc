CFLAGS=-std=c11 -g -static

run:
	clang -o main main.c token.c parse.c codegen.c type.c util.c

test: run
	./main test.c > tmp.s
	cc -no-pie tmp.s -o tmp
	./tmp

clean:
	rm tmp* main

.PHONY: test clean
