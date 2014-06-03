#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

void read_field(dbfile_t *proot, FOSET ppos, field_t *pdesc){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
		return;
	}
	if(fread(pdesc, sizeof(field_t), 1, proot->vfp) == 0){
		printf("Read failed\n");
		return;
	}
	proot->cnt->f_rd_vcnt++;
}

void write_field(dbfile_t *proot, FOSET ppos, field_t *pdesc){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
		return;
	}
	if(fwrite(pdesc, sizeof(field_t), 1, proot->vfp) == 0){
		printf("Write failed\n");
		return;
	}
	proot->cnt->f_wr_vcnt++;
}

void read_datafield(dbfile_t *proot, FOSET ppos, cast_t *pdata){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
		return;
	}
	if(pdata->type == _BOOL){
		bool cast;
		if(fread(&cast, sizeof(bool), 1, proot->vfp) == 0){
			printf("Read failed\n");
			return;
		}
		pdata->value = strdup(get_bool(cast));
		printf("<read BOOL>\n");
	}else if(pdata->type == _INT){
		int cast;
		if(fread(&cast, sizeof(int), 1, proot->vfp) == 0){
			printf("<Read failed>\n");
			return;
		}
		char *tmp = malloc(10);
		sprintf(tmp, "%d", cast);
		pdata->value = tmp;
		printf("<read INT>\n");
	}else if(pdata->type == _CHAR){
		char *tmp = malloc(pdata->size+1);
		//char *tmp = malloc(5);
		if(fread(tmp, pdata->size, 1, proot->vfp) == 0){
		//if(fread(tmp, 4, 1, proot->vfp) == 0){
			printf("Read failed\n");
			return;
		}
		tmp[pdata->size] = '\0';
		pdata->value = tmp;
		printf("<read CHAR>\n");
	}
	proot->cnt->d_rd_vcnt++;
}

void write_datafield(dbfile_t *proot, FOSET ppos, cast_t pdata){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
		return;
	}
	if(pdata.type == _BOOL){
		bool cast = strtobool(pdata.value);
		if(fwrite(&cast, sizeof(bool), 1, proot->vfp) == 0){
			printf("Write failed\n");
			return;
		}
		printf("<write BOOL>\n");
	}else if(pdata.type == _INT){
		int cast = atoi(pdata.value);
		if(fwrite(&cast, sizeof(int), 1, proot->vfp) == 0){
			printf("Write failed\n");
			return;
		}
		printf("<write INT>\n");
	}else if(pdata.type == _CHAR){
		if(fwrite(pdata.value, pdata.size, 1, proot->vfp) == 0){
		//if(fwrite("KAAS", 4, 1, proot->vfp) == 0){
			printf("Write failed\n");
			return;
		}
		printf("<write CHAR>\n");
	}
	proot->cnt->d_wr_vcnt++;
}

void change_field(dbfile_t *proot, FOSET t, int i, void *value, int idx){
	node_t node;
	field_t fld_next, fld_prev;
	int size = strlen(value);
	int j = 0;
	column_t col_type = get_column(proot, idx);

	if(idx < 0 || idx > (proot->c_cnt-1)){
		return;
	}

	read_node(proot, t, &node);
	FOSET prev, next = node.f_val[i];
	while(next != EMPTY){
		read_field(proot, next, &fld_next);
		if(j == idx){
			break;
		}
		prev = next;
		next = fld_next.f_next;
		j++;
	}

	if(fld_next.d_size>=size){
		fld_next.d_size = size;
		write_field(proot, next, &fld_next);
		if(size){
			FOSET p = (next + sizeof(field_t));
			cast_t cast;
			cast.type = col_type.d_type;
			cast.size = strlen(value);
			cast.value = value;
			write_datafield(proot, p, cast);
		}
	}else{
		FOSET v = alloc_field(proot, value, col_type.d_type);
		read_field(proot, next, &fld_next);
		FOSET tmpptr = fld_next.f_next;
		free_field(proot, next);

		if(idx){
			read_field(proot, prev, &fld_prev);
			fld_prev.f_next = v;
			write_field(proot, prev, &fld_prev);
		}else{
			node.f_val[i] = v;
			write_node(proot, t, &node);
		}

		read_field(proot, v, &fld_next);
		fld_next.f_next = tmpptr;
		write_field(proot, v, &fld_next);
	}
}

void add_field(dbfile_t *proot, FOSET ppos, int node_idx, void *value, int field_idx){
	node_t node;
	field_t fld_next, fld_prev;
	FOSET prev, next, new;
	int i = 0;

	read_node(proot, ppos, &node);
	prev = EMPTY;
	next = node.f_val[node_idx];
	while(next != EMPTY){
		read_field(proot, next, &fld_next);
		if(i == field_idx){
			break;
		}
		prev = next;
		next = fld_next.f_next;
		i++;
	}

	new = alloc_field(proot, value, get_column(proot, field_idx).d_type);
	if(field_idx == 0){
		node.f_val[node_idx] = new;
		read_field(proot, new, &fld_prev);
		fld_prev.f_next = next;
		write_field(proot, new, &fld_prev);
		write_node(proot, ppos, &node);
	}else{
		if(next == EMPTY){
			read_field(proot, prev, &fld_prev);
			fld_prev.f_next = new;
			write_field(proot, prev, &fld_prev);
		}else{
			read_field(proot, prev, &fld_prev);
			read_field(proot, new, &fld_next);
			fld_prev.f_next = new;
			fld_next.f_next = next;
			write_field(proot, prev, &fld_prev);
			write_field(proot, new, &fld_next);
		}
	}
}

void delete_field(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx){
	node_t node;
	field_t fld_next, fld_prev;
	int j = 0;

	read_node(proot, ppos, &node);
	FOSET prev, next = node.f_val[node_idx];
	if(field_idx == 0){
		read_field(proot, next, &fld_next);
		if(fld_next.f_next == EMPTY){
			return;
		}
		free_field(proot, node.f_val[node_idx]);
		node.f_val[node_idx] = fld_next.f_next;
		write_node(proot, ppos, &node);
	}else{
		while(next != EMPTY){
			read_field(proot, next, &fld_next);
			if(j == field_idx){
				break;
			}
			prev = next;
			next = fld_next.f_next;
			j++;
		}
		if(next != EMPTY){
			read_field(proot, prev, &fld_prev);
			read_field(proot, next, &fld_next);
			fld_prev.f_next = fld_next.f_next;
			write_field(proot, prev, &fld_prev);
			free_field(proot, next);
		}
	}
}

void delete_field_all(dbfile_t *proot, FOSET ppos, int idx){
	int i, pcnt;
	node_t node;

	if(ppos != EMPTY){
		printf("delete field\n");
		read_node(proot, ppos, &node);
		pcnt = node.cnt;

		for(i=0; i<pcnt; i++){
			printf("delete field %d\n", i);
			delete_field(proot, ppos, i, idx);
		}

		for(i=0; i<=pcnt; i++){
			delete_field_all(proot, node.n_child[i], idx);
		}
	}
}

void add_field_all(dbfile_t *proot, FOSET ppos, int idx){
	int i, pcnt;
	node_t node;

	if(ppos != EMPTY){
		read_node(proot, ppos, &node);
		pcnt = node.cnt;

		for(i=0; i<pcnt; i++){
			add_field(proot, ppos, i, "", idx);
		}

		for(i=0; i<=pcnt; i++){
			add_field_all(proot, node.n_child[i], idx);
		}
	}
}

void shift_field_left_all(dbfile_t *proot, FOSET ppos, int idx){
	int i, pcnt;
	node_t node;

	if(ppos != EMPTY){
		read_node(proot, ppos, &node);
		pcnt = node.cnt;

		for(i=0; i<pcnt; i++){
			shift_field_left(proot, ppos, i, idx);
		}

		for(i=0; i<=pcnt; i++){
			shift_field_left_all(proot, node.n_child[i], idx);
		}
	}
}

void shift_field_right_all(dbfile_t *proot, FOSET ppos, int idx){
	int i, pcnt;
	node_t node;

	if(ppos != EMPTY){
		read_node(proot, ppos, &node);
		pcnt = node.cnt;

		for(i=0; i<pcnt; i++){
			shift_field_right(proot, ppos, i, idx);
		}

		for(i=0; i<=pcnt; i++){
			shift_field_right_all(proot, node.n_child[i], idx);
		}
	}
}

void shift_field_left(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx){
	FOSET pntr, prev, next;
	node_t node;
	field_t pntr_field, next_field, prev_field;
	int i = 0;

	if(field_idx < 1 || field_idx > (proot->c_cnt-1)){
		return;
	}

	if(proot->data.n_root != EMPTY && proot->c_cnt > 1){
		read_node(proot, ppos, &node);
		pntr = EMPTY;
		prev = node.f_val[node_idx];
		read_field(proot, prev, &prev_field);
		next = prev_field.f_next;
		read_field(proot, next, &next_field);

		while(next_field.f_next != EMPTY && i != (field_idx-1)){
			pntr = prev;
			prev = next;
			next = next_field.f_next;
			read_field(proot, next, &next_field);
			i++;
		}
		if(pntr == EMPTY){
			read_field(proot, prev, &prev_field);
			read_field(proot, next, &next_field);

			prev_field.f_next = next_field.f_next;
			next_field.f_next = prev;
			node.f_val[node_idx] = next;

			write_field(proot, prev, &prev_field);
			write_field(proot, next, &next_field);
			write_node(proot, ppos, &node);
		}else{
			read_field(proot, pntr, &pntr_field);
			read_field(proot, prev, &prev_field);
			read_field(proot, next, &next_field);

			prev_field.f_next = next_field.f_next;
			next_field.f_next = prev;
			pntr_field.f_next = next;

			write_field(proot, pntr, &pntr_field);
			write_field(proot, prev, &prev_field);
			write_field(proot, next, &next_field);
		}
	}
}

void shift_field_right(dbfile_t *proot, FOSET ppos, int node_idx, int field_idx){
	FOSET pntr, prev, next;
	node_t node;
	field_t pntr_field, next_field, prev_field;
	int i = 0;

	if(field_idx < 0 || field_idx > (proot->c_cnt-2)){
		return;
	}

	if(proot->data.n_root != EMPTY && proot->c_cnt > 1){
		read_node(proot, ppos, &node);
		pntr = EMPTY;
		prev = node.f_val[node_idx];
		read_field(proot, prev, &prev_field);
		next = prev_field.f_next;
		read_field(proot, next, &next_field);

		while(next_field.f_next != EMPTY && i != field_idx){
			pntr = prev;
			prev = next;
			next = next_field.f_next;
			read_field(proot, next, &next_field);
			i++;
		}
		if(pntr == EMPTY){
			read_field(proot, prev, &prev_field);
			read_field(proot, next, &next_field);

			prev_field.f_next = next_field.f_next;
			next_field.f_next = prev;
			node.f_val[node_idx] = next;

			write_field(proot, prev, &prev_field);
			write_field(proot, next, &next_field);
			write_node(proot, ppos, &node);
		}else{
			read_field(proot, pntr, &pntr_field);
			read_field(proot, prev, &prev_field);
			read_field(proot, next, &next_field);

			prev_field.f_next = next_field.f_next;
			next_field.f_next = prev;
			pntr_field.f_next = next;

			write_field(proot, pntr, &pntr_field);
			write_field(proot, prev, &prev_field);
			write_field(proot, next, &next_field);
		}
	}
}
