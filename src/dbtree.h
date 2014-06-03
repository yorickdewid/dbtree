/*
 * Dbtree header / Valca
 * 
 * Rename long -> datafilepointer oid
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <valgrind/valgrind.h>

#ifndef __DB_TREE__
#define __DB_TREE__

#define ND_DEGREE	4
#define ND_POINTER	(ND_DEGREE+1)
#define ND_HALF		(ND_DEGREE/2)

#define TRUE		1
#define FALSE		0
#define EMPTY		-1
#define LAST		-1

#define PROGNAME	"valca"
#define VER_REL		2
#define VER_MAJOR	1
#define VER_MINOR	2

#define OPT_RLOCK    (1 << 0)  // 00000001
#define OPT_WLOCK    (1 << 1)  // 00000010
#define OPT_CMP64    (1 << 2)  // 00000100
#define OPT_UKWN1    (1 << 3)  // 00001000
#define OPT_UKWN2    (1 << 4)  // 00010000
#define OPT_UKWN3    (1 << 5)  // 00100000
#define OPT_UKWN4    (1 << 6)  // 01000000
#define OPT_UKWN5    (1 << 7)  // 10000000

#define MAGIC		0x7F3D1A90
#define SEQ_START	1

#if VLAUNCH
# ifndef DEBUG
#  define DEBUG
# endif
#endif

#if __x86_64__
#define LONGMODE
#endif

/* Suffix:
 *	d_ = data
 *	f_ = field
 *	n_ = node
 *	c_ = column
 *	k_ = key
 */

typedef unsigned char bool;
typedef int OPT;
typedef unsigned int FOSET;

typedef enum {
	SUCCESS,
	DUPLICATEKEY,
	UNDERFLOW,
	NOTCOMPLETE,
	NOTFOUND,
	LOCKED
} STATUS;

typedef enum {
	_BOOL,
//	_SMALLINT,
	_INT,
//	_FLOAT,
//	_DOUBLE,
	_DECIMAL,
//	_BLOB,
	_CHAR
//	_VARCHAR
//	_TEXT
} DTYPE;

typedef struct {
	char name[8];
	DTYPE type;
	int size;
	bool signness;
} dtype_t;

typedef struct {
	DTYPE type;
	void *value;
	int size;
} cast_t;

typedef struct {
	STATUS rstat;				/* Return status */
	FOSET fpos;					/* Field position */
	int idx;					/* Field index */
} result_t;

typedef struct {
	char name[64];				/* Column name */
	int maxsize;				/* Maximum size */
	DTYPE d_type;				/* Data type */
	bool usign;					/* Unsigned value */
	FOSET c_next;				/* Next column offset */
} column_t;

typedef struct {
	int d_size;					/* Datasize */
	FOSET f_next;				/* Next field offset */
} field_t;

typedef struct {
	int cnt;					/* Internal key counter */
	int key[ND_DEGREE];			/* Key array */
	FOSET f_val[ND_DEGREE];		/* Field offset array */
	FOSET n_child[ND_POINTER];	/* Next node offset array */
} node_t;

typedef struct {
	int cmt_vcnt;				/* Virtual commit count */
	int c_rd_vcnt;				/* Virtual column read count */
	int c_wr_vcnt;				/* Virtual column write count */
	int n_rd_vcnt;				/* Virtual node read count */
	int n_wr_vcnt;				/* Virtual node write count */
	int f_rd_vcnt;				/* Virtual field read count */
	int f_wr_vcnt;				/* Virtual field write count */
	int d_rd_vcnt;				/* Virtual datafield read count */
	int d_wr_vcnt;				/* Virtual datafield write count */
} dbcount_t;

typedef struct {
	char version[7];			/* Unused */
	int tstamp;					/* Timestamp */
	int release;				/* Version release */
	char ver_maj;				/* Version major */
	char ver_min;				/* Version minor */
	char flag_opt;				/* Options flag */
	int magic;					/* Magic checksum */
} dbheader_t;

typedef struct {
	FOSET c_pk;					/* Primary key column */
	FOSET c_root;				/* Column root offset */
	FOSET n_root;				/* Node root offset */
	FOSET c_free;				/* Column freelist offset */
	FOSET n_free;				/* Node freelist offset */
	FOSET d_free;				/* Datafield freelist offset */
} dboffset_t;

typedef struct {
	FILE *vfp;					/* Virutal file pointer */
	dbheader_t head;			/* Database file header */
	dbcount_t *cnt;				/* Virtual counter */
	dboffset_t data;			/* Database offset data */
	int n_cnt;					/* Node count */
	int c_cnt;					/* Column count */
	int k_cnt;					/* Key count */
	int seq_cnt;				/* Auto sequence counter */
} dbfile_t;

void dbtree_printf(OPT mode, const char *file, int line, const char *format, ...);
void print_status(dbfile_t *proot);
void print_column(dbfile_t *proot, FOSET c_next, int cnt);
void print_table(dbfile_t *proot);
void print_tree(dbfile_t *proot, FOSET n_child, int cnt);
char *get_bool(bool test);
bool strtobool(char *str);

DTYPE get_datatype_idx(char *name);
dtype_t get_datatype(DTYPE type);
const char *get_datatype_name(DTYPE type);
int get_datatype_size(DTYPE type);
bool get_datatype_signness(DTYPE type);
void found(dbfile_t *proot, FOSET ppos, int idx);
void print_datafield(dbfile_t *proot, FOSET ppos);

FOSET alloc_column(dbfile_t *proot);
FOSET alloc_node(dbfile_t *proot);
//FOSET alloc_field(dbfile_t *proot, void *value);
FOSET alloc_field(dbfile_t *proot, void *value, DTYPE type);
FOSET alloc_datafield(dbfile_t *proot);
void free_node(dbfile_t *proot, FOSET ppos);
void free_field(dbfile_t *proot, FOSET ppos);

void read_node(dbfile_t *proot, FOSET ppos, node_t *pnode);
void write_node(dbfile_t *proot, FOSET ppos, node_t *pnode);

void read_field(dbfile_t *proot, FOSET ppos, field_t *pdesc);
void write_field(dbfile_t *proot, FOSET ppos, field_t *pdesc);

//void read_datafield(dbfile_t *proot, FOSET ppos, void *pdata, int size);
void read_datafield(dbfile_t *proot, FOSET ppos, cast_t *pdata);
//void write_datafield(dbfile_t *proot, FOSET ppos, void *pdata);
void write_datafield(dbfile_t *proot, FOSET ppos, cast_t pdata);

void read_column(dbfile_t *proot, FOSET ppos, column_t *pcolumn);
void write_column(dbfile_t *proot, FOSET ppos, column_t *pcolumn);

void add_primary_key(dbfile_t *proot, int idx);
void add_field_all(dbfile_t *proot, FOSET ppos, int idx);
void delete_field_all(dbfile_t *proot, FOSET ppos, int idx);
void add_column(dbfile_t *proot, char *value, int size, DTYPE type, bool usign, int idx);
void delete_column(dbfile_t *proot, int idx);
void rename_column(dbfile_t *proot, char *value, int idx);
int get_column_idx(dbfile_t *proot, char *value);
column_t get_column(dbfile_t *proot, int idx);
void shift_column_left(dbfile_t *proot, int idx);
void shift_column_right(dbfile_t *proot, int idx);
void shift_field_left(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx);
void shift_field_right(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx);
void shift_field_left_all(dbfile_t *proot, FOSET ppos, int idx);
void shift_field_right_all(dbfile_t *proot, FOSET ppos, int idx);

STATUS truncate_root(dbfile_t *proot);
result_t search_key(dbfile_t *proot, int key);
STATUS change(dbfile_t *proot, int key, void *value, int idx);
STATUS insert_key(dbfile_t *proot, int key, void *value);
void change_field(dbfile_t *proot, FOSET t, int i, void *value, int idx);
void add_field(dbfile_t *proot, FOSET ppos, int node_idx, void *value, int field_idx);
STATUS delete_key(dbfile_t *proot, int key);
void delete_field(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx);

dbfile_t *create_db(FILE *fp);
dbfile_t *open_db(FILE *fp);
dbfile_t *open_dbfile(char *dbname);
void close_dbfile(FILE *fp);
void commit_db(dbfile_t *proot);
void close_db(dbfile_t *proot);
void strtolower(char *str);
char *get_input(char*, int, bool);
void arch_info();

#endif
