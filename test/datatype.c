#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "datatype.h"

// current known datatypes
dset_t type_mtx[] = {
	{DT_INT,	4, FALSE,	"INT"},
	{DT_CHAR,	1, FALSE,	"CHAR"},
	{DT_STRING,	1, TRUE,	"STRING"},
	{DT_BOOL,	1, FALSE,	"BOOL"},
	{DT_FLOAT,	4, FALSE,	"FLOAT"},
	{DT_DOUBLE,	8, FALSE,	"DOUBLE"},
};

// retrieve type settings by name
dset_t get_dopt_type(const char *type){
	if(!strcmp(type, "INT")){
		return type_mtx[0];
	}else if(!strcmp(type, "CHAR")){
		return type_mtx[1];
	}else if(!strcmp(type, "STRING")){
		return type_mtx[2];
	}else if(!strcmp(type, "BOOL")){
		return type_mtx[3];
	}else if(!strcmp(type, "FLOAT")){
		return type_mtx[4];
	}else if(!strcmp(type, "DOUBLE")){
		return type_mtx[5];
	}else{
		printf("Unknown type cast as string\n");
		return type_mtx[2];
	}
}

// retrieve type settings by type
dset_t get_dopt(TYPE type){
	if(type_mtx[0].type == type){
		return type_mtx[0];
	}else if(type_mtx[1].type == type){
		return type_mtx[1];
	}else if(type_mtx[2].type == type){
		return type_mtx[2];
	}else if(type_mtx[3].type == type){
		return type_mtx[3];
	}else if(type_mtx[4].type == type){
		return type_mtx[4];
	}else if(type_mtx[5].type == type){
		return type_mtx[5];
	}else{
		printf("Unknown type cast as string\n");
		return type_mtx[2];
	}
}

static void dwrite(const void *ptr, unsigned int size, TYPE type, FILE *stream){
	fwrite(&type, sizeof(type), 1, stream);
	fwrite(&size, sizeof(size), 1, stream);
	fwrite(ptr, size, 1, stream);
}

static dset_t dread(FILE *stream, datatype_t *rtn){
	fread(&rtn->type, sizeof(rtn->type), 1, stream);
	fread(&rtn->size, sizeof(rtn->size), 1, stream);

	dset_t type_settings = get_dopt(rtn->type);

	if(type_settings.varlen){
		rtn->pval = calloc(1, rtn->size+1);
	}else{
		rtn->pval = malloc(rtn->size);
	}
	fread(rtn->pval, rtn->size, 1, stream);

	return type_settings;
}

void write_data(const char *fname, const char *dtype, void *ptr){
	unsigned int size;
	void *cptr;

	dset_t type_settings = get_dopt_type(dtype);
	FILE *fpout = fopen(fname, "w+b");

	if(type_settings.varlen){
		size = strlen(ptr);
		cptr = ptr;
	}else{
		if(type_settings.type == DT_INT){
			int tmp = str_to_int((char *)ptr);
			cptr = &tmp;
		}else if(type_settings.type == DT_CHAR){
			char tmp = str_to_char((char *)ptr);
			cptr = &tmp;
		}else if(type_settings.type == DT_BOOL){
			bool tmp = str_to_bool((char *)ptr);
			cptr = &tmp;
		}else if(type_settings.type == DT_FLOAT){
			float tmp = str_to_float((char *)ptr);
			cptr = &tmp;
		}else if(type_settings.type == DT_DOUBLE){
			double tmp = str_to_double((char *)ptr);
			cptr = &tmp;
		}
		size = type_settings.size;
	}
	printf("write size %d\n", size);

	dwrite(cptr, size, type_settings.type, fpout);
	fclose(fpout);
}

void read_data(const char *fname){
	FILE *fpin = fopen(fname, "r+b");
	datatype_t rtn;
	dset_t o = dread(fpin, &rtn);

	if(o.type == DT_INT){
		printf("Data cast<%s> %d\n", o.name, ref_to_int(rtn.pval));
	}else if(o.type == DT_CHAR){
		printf("Data cast<%s> %c\n", o.name, ref_to_char(rtn.pval));
	}else if(o.type == DT_STRING){
		printf("Data cast<%s> %s\n", o.name, ref_to_string(rtn.pval));
	}else if(o.type == DT_BOOL){
		printf("Data cast<%s> %d\n", o.name, ref_to_bool(rtn.pval));
	}else if(o.type == DT_FLOAT){
		printf("Data cast<%s> %f\n", o.name, ref_to_float(rtn.pval));
	}else if(o.type == DT_DOUBLE){
		printf("Data cast<%s> %f\n", o.name, ref_to_double(rtn.pval));
	}else{
		printf("Data cast<unkown> %s\n", ref_to_string(rtn.pval));
	}

	free(rtn.pval);
	fclose(fpin);
}

void check_datatype(){
	assert(sizeof(int) == type_mtx[0].size);
	assert(sizeof(char) == type_mtx[1].size);
	assert(sizeof(bool) == type_mtx[3].size);
	assert(sizeof(float) == type_mtx[4].size);
	assert(sizeof(double) == type_mtx[5].size);
}

int main(int argc, char *argv[]){
	char fname[BUFSIZ];
	char data[BUFSIZ];
	char dtype[DT_SIZ];

	check_datatype();

	// Store datatype
	printf("Datatype: ");
	fgets(dtype, DT_SIZ, stdin);
	dtype[strlen(dtype)-1] = '\0';

	// The actual data
	printf("Data: ");
	fgets(data, BUFSIZ, stdin);
	data[strlen(data)-1] = '\0';

	// database filename
	printf("Filename: ");
	fgets(fname, BUFSIZ, stdin);
	fname[strlen(fname)-1] = '\0';
	strcat(fname, ".tmp");

//	char i[] = "testvar3@#!";
//	strcpy(dtype, "STRING");

	write_data(fname, dtype, (void *)data);
	read_data(fname);

	return 0;
}
