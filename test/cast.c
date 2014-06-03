#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datatype.h"

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
