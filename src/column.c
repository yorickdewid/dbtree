#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

void read_column(dbfile_t *proot, FOSET ppos, column_t *pcolumn){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
	}
	if(!fread(pcolumn, sizeof(column_t), 1, proot->vfp)){
		printf("Read failed\n");
	}
	proot->cnt->c_rd_vcnt++;
}

void write_column(dbfile_t *proot, FOSET ppos, column_t *pcolumn){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
	}
	if(!fwrite(pcolumn, sizeof(column_t), 1, proot->vfp)){
		printf("Write failed\n");
	}
	proot->cnt->c_wr_vcnt++;
}

/*
void add_primary_key(dbfile_t *proot, int idx){
	proot->c_pk = idx;
	commit_db(proot);
}
*/

// rename colname, add maxsize, add dtype
void add_column(dbfile_t *proot, char *value, int size, DTYPE type, bool usign, int idx){
	column_t new_column, prev_column;
	FOSET prev, next = proot->data.c_root;
	FOSET ncol = alloc_column(proot);
	int i = 0;

	if(get_column_idx(proot, value) != EMPTY){
		return;
	}
	
	strcpy(new_column.name, value);
	new_column.maxsize = size;
	new_column.d_type = type;
	new_column.usign = usign;
	
	if(next == EMPTY){
		new_column.c_next = next;
		write_column(proot, ncol, &new_column);
		proot->data.c_root = ncol;
	}else{
		read_column(proot, next, &prev_column);
		while(prev_column.c_next != EMPTY){
			if(i == idx){
				break;
			}
			prev = next;
			next = prev_column.c_next;
			read_column(proot, next, &prev_column);
			i++;
		}
		if(idx == EMPTY){
			prev_column.c_next = ncol;
			new_column.c_next = EMPTY;
			write_column(proot, next, &prev_column);
			write_column(proot, ncol, &new_column);
		}else if(idx == 0){
			new_column.c_next = proot->data.c_root;
			proot->data.c_root = ncol;
			write_column(proot, ncol, &new_column);
		}else{
			read_column(proot, prev, &prev_column);
			prev_column.c_next = ncol;
			new_column.c_next = next;
			write_column(proot, prev, &prev_column);
			write_column(proot, ncol, &new_column);
		}
	}
	add_field_all(proot, proot->data.n_root, idx);
	
	proot->c_cnt++;
	commit_db(proot);
}

void delete_column(dbfile_t *proot, int idx){
	FOSET prev, next = -1;
	column_t next_column, prev_column;
	int i = 0;

	if(proot->data.c_root != EMPTY){
		prev = EMPTY;
		next = proot->data.c_root;
		read_column(proot, next, &next_column);
		while(next != EMPTY && i != idx){
			prev = next;
			next = next_column.c_next;
			if(next == EMPTY){
				break;
			}
			read_column(proot, next, &next_column);
			i++;
		}
		if(next == EMPTY){
			read_column(proot, prev, &prev_column);
			prev_column.c_next = EMPTY;
			write_column(proot, prev, &prev_column);
		}else{
			if(prev != EMPTY){
				read_column(proot, prev, &prev_column);
				read_column(proot, next, &next_column);
				prev_column.c_next = next_column.c_next;
				write_column(proot, prev, &prev_column);
			}else{
				read_column(proot, next, &next_column);
				proot->data.c_root = next_column.c_next;
				commit_db(proot);
			}
		}

		if(next == EMPTY){
			printf("Column not found\n");
		}else{
			next_column.c_next = proot->data.c_free;
			proot->data.c_free = next;
			write_column(proot, next, &next_column);
			if(proot->c_cnt == 1 && idx == 0){
				truncate_root(proot);
			}else{
				delete_field_all(proot, proot->data.n_root, idx);
			}
			
			proot->c_cnt--;
			commit_db(proot);
		}
	}
}

void rename_column(dbfile_t *proot, char *value, int idx){
	FOSET next;
	column_t next_column;
	int i = 0;

	if(idx < 0 || idx > (proot->c_cnt-1)){
		return;
	}

	if(get_column_idx(proot, value) != EMPTY){
		return;
	}

	if(proot->data.c_root != EMPTY){
		next = proot->data.c_root;
		read_column(proot, next, &next_column);
		while(next != EMPTY && i != idx){
			next = next_column.c_next;
			if(next == EMPTY){
				break;
			}
			read_column(proot, next, &next_column);
			i++;
		}

		strcpy(next_column.name, value);
		write_column(proot, next, &next_column);
	}
}

void shift_column_left(dbfile_t *proot, int idx){
	FOSET pntr, prev, next;
	column_t pntr_column, next_column, prev_column;
	int i = 0;

	if(idx < 1 || idx > (proot->c_cnt-1)){
		return;
	}

	if(proot->data.c_root != EMPTY && proot->c_cnt > 1){
		pntr = EMPTY;
		prev = proot->data.c_root;
		read_column(proot, prev, &prev_column);
		next = prev_column.c_next;
		read_column(proot, next, &next_column);

		while(next_column.c_next != EMPTY && i != (idx-1)){
			pntr = prev;
			prev = next;
			next = next_column.c_next;
			read_column(proot, next, &next_column);
			i++;
		}
		if(pntr == EMPTY){
			read_column(proot, prev, &prev_column);
			read_column(proot, next, &next_column);

			prev_column.c_next = next_column.c_next;
			next_column.c_next = prev;
			proot->data.c_root = next;

			write_column(proot, prev, &prev_column);
			write_column(proot, next, &next_column);
			commit_db(proot);
		}else{
			read_column(proot, pntr, &pntr_column);
			read_column(proot, prev, &prev_column);
			read_column(proot, next, &next_column);

			prev_column.c_next = next_column.c_next;
			next_column.c_next = prev;
			pntr_column.c_next = next;

			write_column(proot, pntr, &pntr_column);
			write_column(proot, prev, &prev_column);
			write_column(proot, next, &next_column);
		}
		shift_field_left_all(proot, proot->data.n_root, idx);
	}
}

void shift_column_right(dbfile_t *proot, int idx){
	FOSET pntr, prev, next;
	column_t pntr_column, next_column, prev_column;
	int i = 0;

	if(idx < 0 || idx > (proot->c_cnt-2)){
		return;
	}

	if(proot->data.c_root != EMPTY && proot->c_cnt > 1){
		pntr = EMPTY;
		prev = proot->data.c_root;
		read_column(proot, prev, &prev_column);
		next = prev_column.c_next;
		read_column(proot, next, &next_column);

		while(next_column.c_next != EMPTY && i != idx){
			pntr = prev;
			prev = next;
			next = next_column.c_next;
			read_column(proot, next, &next_column);
			i++;
		}
		if(pntr == EMPTY){
			read_column(proot, prev, &prev_column);
			read_column(proot, next, &next_column);

			prev_column.c_next = next_column.c_next;
			next_column.c_next = prev;
			proot->data.c_root = next;

			write_column(proot, prev, &prev_column);
			write_column(proot, next, &next_column);
			commit_db(proot);
		}else{
			read_column(proot, pntr, &pntr_column);
			read_column(proot, prev, &prev_column);
			read_column(proot, next, &next_column);

			prev_column.c_next = next_column.c_next;
			next_column.c_next = prev;
			pntr_column.c_next = next;

			write_column(proot, pntr, &pntr_column);
			write_column(proot, prev, &prev_column);
			write_column(proot, next, &next_column);
		}
		shift_field_right_all(proot, proot->data.n_root, idx);
	}
}

int get_column_idx(dbfile_t *proot, char *value){
	FOSET next = EMPTY;
	column_t next_column;
	int i = 0;

	if(proot->data.c_root != EMPTY){
		next = proot->data.c_root;
		read_column(proot, next, &next_column);
		while(next != EMPTY && strcmp(next_column.name, value)){
			next = next_column.c_next;
			if(next == EMPTY){
				break;
			}
			read_column(proot, next, &next_column);
			i++;
		}
	}
	if(next == EMPTY){
		i = -1;
	}

	return i;
}

column_t get_column(dbfile_t *proot, int idx){
	FOSET next = EMPTY;
	column_t next_column;
	int i = 0;

	if(proot->data.c_root != EMPTY){
		next = proot->data.c_root;
		read_column(proot, next, &next_column);
		while(next != EMPTY){
			if(i == idx){
				return next_column;
			}
			next = next_column.c_next;
			if(next == EMPTY){
				break;
			}
			read_column(proot, next, &next_column);
			i++;
		}
	}

	return next_column;
}
