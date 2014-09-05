CFLAGS = -Wall -g -DDEBUG -DVLAUNCH
VFLAGS = --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes
PROGRAM = dbtree
PROGRAM32 = dbtree32

all: compile
compile:
	gcc $(CFLAGS) -c result.c
	gcc $(CFLAGS) -c util.c
	gcc $(CFLAGS) -c alloc.c
	gcc $(CFLAGS) -c datatype.c
	gcc $(CFLAGS) -c cast.c
	gcc $(CFLAGS) -c column.c
	gcc $(CFLAGS) -c node.c
	gcc $(CFLAGS) -c field.c
	gcc $(CFLAGS) -c main.c
	gcc -o $(PROGRAM) result.o util.o alloc.o datatype.o column.o node.o field.o main.o

compile-32:
	gcc -m32 $(CFLAGS) -c result.c
	gcc -m32 $(CFLAGS) -c util.c
	gcc -m32 $(CFLAGS) -c alloc.c
	gcc -m32 $(CFLAGS) -c datatype.c
	gcc -m32 $(CFLAGS) -c cast.c
	gcc -m32 $(CFLAGS) -c column.c
	gcc -m32 $(CFLAGS) -c node.c
	gcc -m32 $(CFLAGS) -c field.c
	gcc -m32 $(CFLAGS) -c main.c
	gcc -m32 -o $(PROGRAM32) result.o util.o alloc.o datatype.o column.o node.o field.o main.o

run: compile
	valgrind $(VFLAGS) ./$(PROGRAM)

run-32: compile-32
	valgrind $(VFLAGS) ./$(PROGRAM32)

clean: clean-obj clean-db
	rm -rf *~ $(PROGRAM) $(PROGRAM32)

clean-obj:
	rm -rf *.o

clean-db:
	rm -rf *.vdb
