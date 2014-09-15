#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

void reset_vcount(dbfile_t *proot){
	proot->cnt->cmt_vcnt = 0;
	proot->cnt->c_rd_vcnt = 0;
	proot->cnt->c_wr_vcnt = 0;
	proot->cnt->n_rd_vcnt = 0;
	proot->cnt->n_wr_vcnt = 0;
	proot->cnt->f_rd_vcnt = 0;
	proot->cnt->f_wr_vcnt = 0;
	proot->cnt->d_rd_vcnt = 0;
	proot->cnt->d_wr_vcnt = 0;
}

void verify_db(dbfile_t *proot){
	if(proot->head.magic != MAGIC){
		close_dbfile(proot->vfp);
		close_db(proot);
		exit(1);
	}
}

dbfile_t *create_db(FILE *fp){
	dbfile_t *proot = (dbfile_t*)malloc(sizeof(dbfile_t));
	if(!proot){
		printf("Out of memory\n");
		exit(1);
	}
	proot->cnt = (dbcount_t*)malloc(sizeof(dbcount_t));
	if(!proot->cnt){
		printf("Out of memory\n");
		exit(1);
	}
	proot->vfp = fp;
	proot->head.tstamp = (int)time(NULL);
	proot->head.release = VER_REL;
	proot->head.ver_maj = VER_MAJOR;
	proot->head.ver_min = VER_MINOR;
	proot->head.flag_opt = 0;
	proot->head.magic = MAGIC;
	proot->data.c_pk = EMPTY;
	proot->data.c_root = EMPTY;
	proot->data.n_root = EMPTY;
	proot->data.c_free = EMPTY;
	proot->data.n_free = EMPTY;
	proot->data.d_free = EMPTY;
	proot->n_cnt = 0;
	proot->c_cnt = 0;
	proot->k_cnt = 0;
	proot->seq_cnt = SEQ_START;

	reset_vcount(proot);

	return proot;
}

void commit_db(dbfile_t *proot){
	rewind(proot->vfp);

	if(!fwrite(&proot->head, sizeof(proot->head), 1, proot->vfp)){ printf("Write head failed\n"); }
	if(!fwrite(&proot->data, sizeof(proot->data), 1, proot->vfp)){ printf("Write data failed\n"); }
	if(!fwrite(&proot->n_cnt, sizeof(proot->n_cnt), 1, proot->vfp)){ printf("Write count failed\n"); }
	if(!fwrite(&proot->c_cnt, sizeof(proot->c_cnt), 1, proot->vfp)){ printf("Write count failed\n"); }
	if(!fwrite(&proot->k_cnt, sizeof(proot->k_cnt), 1, proot->vfp)){ printf("Write count key failed\n"); }
	if(!fwrite(&proot->seq_cnt, sizeof(proot->seq_cnt), 1, proot->vfp)){ printf("Write count failed\n"); }

	proot->cnt->cmt_vcnt++;
}

dbfile_t *open_db(FILE *fp){
	dbfile_t *proot = (dbfile_t*)malloc(sizeof(dbfile_t));
	if(!proot){
		printf("Out of memory\n");
		exit(1);
	}
	proot->cnt = (dbcount_t*)malloc(sizeof(dbcount_t));
	if(!proot->cnt){
		printf("Out of memory\n");
		exit(1);
	}

	rewind(fp);
	if(!fread(&proot->head, sizeof(proot->head), 1, fp)){ printf("Read head failed\n"); }
	if(!fread(&proot->data, sizeof(proot->data), 1, fp)){ printf("Read data failed\n"); }
	if(!fread(&proot->n_cnt, sizeof(proot->n_cnt), 1, fp)){ printf("Read count failed\n"); }
	if(!fread(&proot->c_cnt, sizeof(proot->c_cnt), 1, fp)){ printf("Read count failed\n"); }
	if(!fread(&proot->k_cnt, sizeof(proot->k_cnt), 1, fp)){ printf("Read count key failed\n"); }
	if(!fread(&proot->seq_cnt, sizeof(proot->seq_cnt), 1, fp)){ printf("Read count failed\n"); }

	verify_db(proot);

	proot->vfp = fp;
	reset_vcount(proot);

	return proot;
}

void close_db(dbfile_t *proot){
	if(proot){
		if(proot->cnt){
			free(proot->cnt);
		}
		free(proot);
	}
}

dbfile_t *open_dbfile(char *dbname){
	FILE *fp;
	dbfile_t *tree;
	struct stat dbstat;

	strcat(dbname, "_.vdb");
	if(stat(dbname, &dbstat) < 0){
		fp = fopen(dbname, "w+b");
		tree = create_db(fp);
		printf("Create database\n");
	}else if(dbstat.st_size){
		fp = fopen(dbname, "r+b");
		tree = open_db(fp);
		printf("Open database\n");
	}else{
		printf("Cannot create or open database\n");
	}

	return tree;
}

void close_dbfile(FILE *fp){
	if(fp){
		fclose(fp);
	}
}

int main(int argc, char *argv[]){
	char buffer[64];
	char *c;

#if DEBUG
	printf("[DEBUG MODE]\n");
	printf("Compile: %s %s\n", __DATE__, __TIME__);
#endif

	printf("Version: %d.%d.%d\n", VER_REL, VER_MAJOR, VER_MINOR);
	printf("Database> ");
	dbfile_t *mdb = open_dbfile(get_input(buffer, 32, TRUE));
	commit_db(mdb);

	for(;;){
		printf("valca> ");

		c = get_input(buffer, 64, TRUE);
		if(!strcmp("quit", c)||!strcmp("exit", c)||!strcmp("\\q", c)){
			break;
		}
		if(!strcmp("help", c)){
			printf("COMMANDS:\n");
			printf("COMMIT        Commit file to disk\n");
			printf("COUNT         Show row count\n");
			printf("TRUNCATE      Delete all keys\n");
			printf("UPDATE        Update key value\n");
			printf("INSERT        Insert key\n");
			printf("DELETE \\\n");
			printf("  COLUMN      Delete column\n");
			printf("  ROW         Delete row\n");
			printf("ALTER \\\n");
			printf("  NAME        Change column name\n");
			printf("  LEFT        Shift column left\n");
			printf("  RIGHT       Shift column right\n");
			printf("SELECT        Select value from\n");
			printf("ADD \\\n");
			printf("  COLUMN      Add column\n");
			printf("SHOW \\\n");
			printf("  COLUMNS     Show all columns\n");
			printf("  TREE        Show storage tree\n");
			printf("  STATUS      Show database info\n");
			printf("  TABLE       Show dataset as table\n");
			printf("FREE \\\n");
			printf("  COLUMNS     Columns freelist\n");
			printf("  NODE        Node freelist\n");
			printf("  DATAFIELDS  Payload freelist\n");
			printf("EXIT          Quit the shell\n");
		}
		if(!strcmp("commit", c)){
			commit_db(mdb);
		}
		if(!strcmp("count", c)){
			printf("Total rows: %d\n", mdb->k_cnt);
		}
		if(!strcmp("truncate", c)){
			truncate_root(mdb);
		}
		if(!strcmp("update", c)){
			char tmp[32];
			printf(">psid: ");
			c = get_input(tmp, 32, FALSE);
			int key = atoi(tmp);
			char tmpcolname[32];
			printf(">column: ");
			c = get_input(tmpcolname, 32, FALSE);
			int idx = get_column_idx(mdb, tmpcolname);
			printf(">value: ");
			c = get_input(tmp, 32, FALSE);
			change(mdb, key, (void*)tmp, idx);
		}
		if(!strcmp("insert", c)){
			if(!mdb->c_cnt){
				printf("Cannot insert without column\n");
			}else{
				char tmp[32];
				printf(">psid[%d]: ", mdb->seq_cnt);
				c = get_input(tmp, 32, FALSE);
				int key;
				if(!strlen(tmp)){
					key = mdb->seq_cnt++;
				}else{
					key = atoi(tmp);
				}
				column_t col;
				read_column(mdb, mdb->data.c_root, &col);
				printf(">%s<%s(%d)>: ", col.name, get_datatype_name(col.d_type), col.maxsize);
				c = get_input(tmp, 32, FALSE);
				if(get_datatype(col.d_type).size == -1) tmp[col.maxsize] = '\0';
				if(col.usign) printf("<> %d\n", atoi(tmp));
					//if(atoi())
				if(insert_key(mdb, key, (void*)tmp) == SUCCESS){
					result_t rs = search_key(mdb, key);
					if(rs.rstat == SUCCESS){
						char tmp[32];
						int i = 1;
						while(col.c_next != EMPTY){
							read_column(mdb, col.c_next, &col);
							printf(">%s<%s(%d)>: ", col.name, get_datatype_name(col.d_type), col.maxsize);
							c = get_input(tmp, 32, FALSE);
							add_field(mdb, rs.fpos, rs.idx, (void*)tmp, i);
							i++;
						}
					}
				}
			}
		}
		if(!strcmp("delete", c)){
			printf(">");
			c = get_input(buffer, 64, TRUE);
			if(!strcmp("column", c)){
				char tmpcolname[32];
				printf(">column: ");
				c = get_input(tmpcolname, 32, FALSE);
				int idx = get_column_idx(mdb, tmpcolname);
				delete_column(mdb, idx);
			}
			if(!strcmp("row", c)){
				char tmp[32];
				printf(">psid: ");
				c = get_input(tmp, 32, FALSE);
				int key = atoi(tmp);
				result_t rs = search_key(mdb, key);
				if(rs.rstat == SUCCESS){
					int i;
					for(i=mdb->c_cnt; i>1; i--){
						delete_field(mdb, rs.fpos, rs.idx, (i-1));
					}
					delete_key(mdb, key);
				}
			}
		}
		if(!strcmp("alter", c)){
			printf(">");
			c = get_input(buffer, 64, TRUE);
			if(!strcmp("name", c)){
				printf(">column: ");
				char tmpcolname[32];
				c = get_input(tmpcolname, 32, FALSE);
				int idx = get_column_idx(mdb, tmpcolname);
				printf(">new name: ");
				c = get_input(tmpcolname, 32, FALSE);
				rename_column(mdb, tmpcolname, idx);
			}
			if(!strcmp("left", c)){
				char tmpcolname[32];
				printf(">column: ");
				c = get_input(tmpcolname, 32, FALSE);
				int idx = get_column_idx(mdb, tmpcolname);
				shift_column_left(mdb, idx);
			}
			if(!strcmp("right", c)){
				char tmpcolname[32];
				printf(">column: ");
				c = get_input(tmpcolname, 32, FALSE);
				int idx = get_column_idx(mdb, tmpcolname);
				shift_column_right(mdb, idx);
			}
		}
		if(!strcmp("select", c)){
			printf(">");
			char tmp[16];
			printf(">psid: ");
			c = get_input(tmp, 32, FALSE);
			int key = atoi(tmp);
			result_t rs = search_key(mdb, key);
			if(rs.rstat == SUCCESS){
				found(mdb, rs.fpos, rs.idx);
			}
		}
		if(!strcmp("add", c)){
			printf(">");
			c = get_input(buffer, 64, TRUE);
			if(!strcmp("column", c)){
				char colname[32];
				DTYPE type;
				int size;
				bool usign = FALSE;
				int idx;
				printf(">name: ");
				c = get_input(colname, 32, FALSE);
				printf(">type: ");
				char tmptype[32];
				c = get_input(tmptype, 32, FALSE);
				type = get_datatype_idx(tmptype);
				if(type == EMPTY){
					printf("Unkown datatype\n");
					continue;
				}
				size = get_datatype(type).size;
				if(size == EMPTY){
					printf(">size: ");
					char tmpsize[16];
					c = get_input(tmpsize, 16, FALSE);
					size = atoi(tmpsize);
				}
				if(get_datatype(type).signness){
					printf(">unsigned: ");
					char tmpsign[4];
					c = get_input(tmpsign, 4, FALSE);
					if(!strcmp("y", tmpsign)){
						usign = TRUE;
					}
				}
				printf(">before: ");
				char tmpcolname[32];
				c = get_input(tmpcolname, 32, FALSE);
				idx = get_column_idx(mdb, tmpcolname);
				add_column(mdb, colname, size, type, usign, idx);
			}
		}
		if(!strcmp("show", c)){
			printf(">");
			c = get_input(buffer, 64, TRUE);
			if(!strcmp("columns", c)){
				printf("Idx   Offset   Psid         Columnname  Size      Type    Unsigned    Next\n");
				printf("--------------------------------------------------------------------------\n");
				print_column(mdb, mdb->data.c_root, 0);
			}
			if(!strcmp("tree", c)){
				print_tree(mdb, mdb->data.n_root, 0);
			}
			if(!strcmp("status", c)){
				print_status(mdb);
			}
			if(!strcmp("table", c)){
				print_table(mdb);
			}
		}
		if(!strcmp("free", c)){
			printf(">");
			c = get_input(buffer, 64, TRUE);
			if(!strcmp("columns", c)){
				printf("Idx   Offset   Psid         Columnname  Size      Type    Unsigned    Next\n");
				printf("--------------------------------------------------------------------------\n");
				print_column(mdb, mdb->data.c_free, 0);
			}
			if(!strcmp("node", c)){
				print_tree(mdb, mdb->data.n_free, 0);
			}
			if(!strcmp("datafields", c)){
				print_datafield(mdb, mdb->data.d_free);
			}
		}
	}

	commit_db(mdb);
	close_dbfile(mdb->vfp);
	close_db(mdb);

	return 0;
}
