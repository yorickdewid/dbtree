// memory.c
// show how data is stored in memory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE long int

char *readln(FILE*);

int main(int argc, char *argv[])
{
	union {
		TYPE num;
		unsigned char c[sizeof(TYPE)];
	} count;

	double n;
	int i;
	char buffer[BUFSIZ];

	fprintf(stdout, "Buffer size %d\n", BUFSIZ);
	fprintf(stdout, "Enter a number: ");
	strcpy(buffer, readln(stdin));

	n = atof(buffer);
	count.num = (n < 0) ? (n - 0.5) : (n + 0.5);

	fprintf(stdout,	"\nThe original number is: 0x%016X\n", count.num);

	fprintf(stdout,	"\nIn memory, the data is: 0x");

	for(i = 0; i < (int)sizeof(TYPE); i++) {
		fprintf(stdout,	"%02X",
		count.c[i]);
	}

	fprintf(stdout, "\n\n");
	return 0;
}


char *readln(FILE *fp)
{
	char buf[BUFSIZ];
	char *p1;
	int len;

	buf[0] = 0;
	fgets(buf, BUFSIZ, fp);
	len = strlen(buf);

	while(len >= 0) {
		if (buf[len] == 32 ||
			buf[len] == 13 ||
			buf[len] == 10 ||
			buf[len] ==  9 ||
			buf[len] ==  0) { buf[len] = 0; len--; }
		else { break; }
	}

	len++;
	buf[len] = 0;
	p1 = buf;
	return p1;
}
