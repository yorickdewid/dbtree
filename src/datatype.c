#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

static const dtype_t mtx_datatype[] = {
	{"BOOL",	_BOOL,		1, 	FALSE},
	{"INT",		_INT,		4,	TRUE},
	{"DECIMAL",	_DECIMAL,	4,	TRUE},
	{"CHAR",	_CHAR,		-1,	FALSE}};

DTYPE get_datatype_idx(char *name){
	int i;
	for(i=0; i<(sizeof(mtx_datatype)/sizeof(dtype_t)); i++){
		if(!strcmp(mtx_datatype[i].name, name)){
			return i;
		}
	}

	return -1;
}

dtype_t get_datatype(DTYPE type){
	int i;
	for(i=0; i<(sizeof(mtx_datatype)/sizeof(dtype_t)); i++){
		if(type == mtx_datatype[i].type){
			return mtx_datatype[i];
		}
	}

	return mtx_datatype[0];
}

const char *get_datatype_name(DTYPE type){
	int i;
	for(i=0; i<(sizeof(mtx_datatype)/sizeof(dtype_t)); i++){
		if(type == mtx_datatype[i].type){
			return mtx_datatype[i].name;
		}
	}

	return "UNKNOWN";
}

