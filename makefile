# You are welcome to modiy the Makefile as long as it still produces the
# executables test_malloc and test_malloc_opt when make is run with no
# arguments

all : test_malloc test_malloc_opt

test_malloc: test_malloc.o mymemory.o
	gcc -Wall -g -o test_malloc test_malloc.c mymemory.c -lpthread

test_malloc_opt: test_malloc.o mymemory_opt.o
	gcc -Wall -g -o test_malloc_opt test_malloc.c mymemory_opt.c -lpthread

