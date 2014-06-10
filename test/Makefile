all:
	cc -g -Wall cast.c datatype.c -o datatype.out
	valgrind ./datatype.out --leak-check=full
clean:
	rm -rf *.o *.tmp datatype.out
