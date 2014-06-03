#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datatype.h"

// current known datatypes
dset_t type_mtx[] = {
	{DT_INT, 4, "%d"},
	{DT_CHAR, 1, "%c"},
	{DT_STRING, 0, "%s"},
	{DT_BOOL, 1, "%d"},
	{DT_FLOAT, 4, "%f"},
	{DT_DOUBLE, 8, "%f"},
};

// retrieve type settings
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
		return type_mtx[2];
	}
}

static void dwrite(const void *ptr, unsigned int size, TYPE type, FILE *stream){
	fwrite(&type, sizeof(type), 1, stream);
	fwrite(&size, sizeof(size), 1, stream);
	fwrite(ptr, size, 1, stream);
}

static datatype_t dread(FILE *stream){
	datatype_t rtn;

	fread(&rtn.type, sizeof(rtn.type), 1, stream);
	fread(&rtn.size, sizeof(rtn.size), 1, stream);

	rtn.pval = malloc(rtn.size);
	fread(rtn.pval, rtn.size, 1, stream);

	return rtn;
}

void write_data(const char *fname, const char *dtype, void *ptr){
	dset_t type_settings = get_dopt_type(dtype);
	FILE *fpout = fopen(fname, "w+b");
	dwrite(ptr, type_settings.size, type_settings.type, fpout);
	fclose(fpout);
}

void read_data(const char *fname){
	FILE *fpin = fopen(fname, "r+b");
	datatype_t o = dread(fpin);
	printf("Data %p\n", o.pval);

	if(o.type == DT_INT){ printf("Data cast<int> %d\n", ref_to_int(o.pval));
	}else if(o.type == DT_CHAR){ printf("Data cast<char> %c\n", ref_to_char(o.pval));
	}else if(o.type == DT_STRING){ printf("Data cast<string> %s\n", ref_to_string(o.pval));
	}else if(o.type == DT_BOOL){ printf("Data cast<bool> %d\n", ref_to_bool(o.pval));
	}else if(o.type == DT_FLOAT){ printf("Data cast<float> %f\n", ref_to_float(o.pval));
	}else if(o.type == DT_DOUBLE){ printf("Data cast<double> %f\n", ref_to_double(o.pval));
	}else{ printf("Data cast<unkown> %s\n", ref_to_string(o.pval));
	}

	free(o.pval);
	fclose(fpin);
}

int main(int argc, char *argv[]){
	char fname[BUFSIZ];
	char data[BUFSIZ];
	char dtype[DT_SIZ];

	printf("Datatype: ");
	fgets(dtype, DT_SIZ, stdin);
	dtype[strlen(dtype)-1] = '\0';

	printf("Data: ");
	fgets(data, BUFSIZ, stdin);
	data[strlen(data)-1] = '\0';
	float q = 12.345678;
	printf("FLOAT: %f\n", q);

	printf("Filename: ");
	fgets(fname, BUFSIZ, stdin);
	fname[strlen(fname)-1] = '\0';

	float i = 1234.321;
	write_data(fname, dtype, &i);
	read_data(fname);

	return 0;
}

