#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

char *get_bool(bool test){
	if(test){
		return "TRUE";
	}else{
		return "FALSE";
	}
}

void strtolower(char *str){
	for(; *str; ++str){
		*str = tolower(*str);
	}
}

bool strtobool(char *str){
	strtolower(str);
	
	if(!strcmp("true", str)||!strcmp("t", str)||!strcmp("y", str)||!strcmp("1", str)){
		return TRUE;
	}else{
		return FALSE;
	}
}

#if 0
char *get_lock(OPT lock){
	switch(lock){
		case RLOCK:
			return "READ LOCK";
		case WLOCK:
			return "WRITE LOCK";
		default:
			return "NONE";
	}
}
#endif

void set_option(char *flag, int opt) {
	*flag |= opt;
}
 
void unset_option(char *flag, int opt) {
    *flag &= ~opt;
}
 
bool check_option(char *flag, int opt) {
    return *flag & opt;
}

char *get_input(char *buffer, int size, bool lower){
	fflush(stdin);
	buffer[0] = '\0';
	fgets(buffer, size, stdin);
	buffer[strlen(buffer)-1] = '\0';  //TODO fix carriage return
	if(lower){
		strtolower(buffer);
	}

	return buffer;
}

void arch_info(){
#ifdef LONGMODE
	printf("64bit\n");
	printf("Int %ld\n", sizeof(int));
	printf("Long %ld\n", sizeof(long));
	printf("Long long %ld\n", sizeof(long long));
	printf("Short %ld\n", sizeof(short));
	printf("Float %ld\n", sizeof(float));
	printf("Double %ld\n", sizeof(double));
	printf("Enum %ld\n", sizeof(DTYPE));
#else
	printf("32bit\n");
	printf("Int %d\n", sizeof(int));
	printf("Long %d\n", sizeof(long));
	printf("Long long %d\n", sizeof(long long));
	printf("Short %d\n", sizeof(short));
	printf("Float %d\n", sizeof(float));
	printf("Double %d\n", sizeof(double));
	printf("Enum %d\n", sizeof(DTYPE));
#endif
}

