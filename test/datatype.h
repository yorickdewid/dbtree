#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __HEAD__
#define __HEAD__

#define DT_SIZ		0x10

#define DT_INT		0x2
#define DT_CHAR		0x4
#define DT_STRING	0x8
#define DT_BOOL		0x10
#define DT_FLOAT	0x20
#define DT_DOUBLE	0x40

#define TRUE		0x1
#define FALSE		0x0

typedef char bool;
typedef char TYPE;

// datatype settings
typedef struct {
	TYPE type;
	unsigned int size;
	char print[4];
} dset_t;

// datatype structure
typedef struct {
	TYPE type;
	unsigned int size;
	void *pval;
} datatype_t;

int ref_to_int(const void *ptr);
char ref_to_char(const void *ptr);
char *ref_to_string(const void *ptr);
bool ref_to_bool(const void *ptr);
float ref_to_float(const void *ptr);
double ref_to_double(const void *ptr);

int str_to_int(const char *str);
double str_to_double(const char *str);
float str_to_float(const char *str);

#endif
