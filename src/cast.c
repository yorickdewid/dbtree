#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbtree.h"

/* Reference pointer to datatype */
int ref_to_int(const void *ptr){
	return *((int *)ptr);
}

char ref_to_char(const void *ptr){
	return *((char *)ptr);
}

char *ref_to_string(const void *ptr){
	return(char *)ptr;
}

bool ref_to_bool(const void *ptr){
	return *((bool *)ptr);
}

float ref_to_float(const void *ptr){
	return *((float *)ptr);
}

double ref_to_double(const void *ptr){
	return *((double *)ptr);
}

/* String to datatype */
int str_to_int(const char *str){
	return atoi(str);
}

double str_to_double(const char *str){
	return atof(str);
}

float str_to_float(const char *str){
	return (float)str_to_double(str);
}

bool str_to_bool(const char *str){
	if(!strcmp(str, "Y")||!strcmp(str, "1")||!strcmp(str, "y")||!strcmp(str, "T")||!strcmp(str, "t")||!strcmp(str, "TRUE")||!strcmp(str, "true")){
		return TRUE;
	}
	return FALSE;
}

char str_to_char(const char *str){
	return (char)str[0];
}
