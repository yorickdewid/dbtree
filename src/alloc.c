#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

FOSET alloc_column(dbfile_t *proot){
	FOSET ppos;
	column_t pcolumn;

	if(proot->data.c_free == EMPTY){
		if(fseek(proot->vfp, 0, SEEK_END)){
			printf("Forward failed\n");
		}
		ppos = ftell(proot->vfp);
		write_column(proot, ppos, &pcolumn);
	}else{
		ppos = proot->data.c_free;
		read_column(proot, ppos, &pcolumn);
		proot->data.c_free = pcolumn.c_next;
		commit_db(proot);
	}
	return ppos;
}

FOSET alloc_node(dbfile_t *proot){
	FOSET ppos;
	node_t pnode;

	if(proot->data.n_free == EMPTY){
		if(fseek(proot->vfp, 0, SEEK_END)){
			printf("fseek in getnode");
		}
		ppos = ftell(proot->vfp);
		write_node(proot, ppos, &pnode);
	}else{
		ppos = proot->data.n_free;
		read_node(proot, ppos, &pnode);
		proot->data.n_free = pnode.n_child[0];
		commit_db(proot);
	}
	return ppos;
}

FOSET alloc_field(dbfile_t *proot, void *value, DTYPE type){
	FOSET ppos, pdata, next, prev;
	int size = strlen(value);
	field_t desc, next_desc, prev_desc;

	dtype_t col_type = get_datatype(type);

	next = EMPTY;
	if(proot->data.d_free != EMPTY){
		prev = EMPTY;
		next = proot->data.d_free;
		read_field(proot, next, &next_desc);

		while(next != EMPTY && next_desc.d_size<size){
			prev = next;
			next = next_desc.f_next;
			if(next == EMPTY){
				break;
			}
			read_field(proot, next, &next_desc);
		}
		if(next == EMPTY){
			read_field(proot, prev, &prev_desc);
			prev_desc.f_next = EMPTY;
			write_field(proot, prev, &prev_desc);
		}else{
			if(prev != EMPTY){
        		read_field(proot, prev, &prev_desc);
        		read_field(proot, next, &next_desc);
				prev_desc.f_next = next_desc.f_next;
				write_field(proot, prev, &prev_desc);
			}else{
				read_field(proot, next, &next_desc);
				proot->data.d_free = next_desc.f_next;
				commit_db(proot);
			}
		}
	}
	if(next == EMPTY){
		if(fseek(proot->vfp, 0, SEEK_END)){
			printf("fseek in get_descriptor");
		}
		ppos = ftell(proot->vfp);
	}else{
		ppos = next;
	}

	desc.d_size = size;
	desc.f_next = EMPTY;
	pdata = (ppos + sizeof(field_t));

	write_field(proot, ppos, &desc);
	if(size){
		cast_t cast;
		cast.type = col_type.type;
		cast.size = size;
		cast.value = value;
		write_datafield(proot, pdata, cast);
	}
	return ppos;
}

void free_node(dbfile_t *proot, FOSET ppos){
	node_t pnode;

	read_node(proot, ppos, &pnode);
	pnode.n_child[0] = proot->data.n_free;
	proot->data.n_free = ppos;
	write_node(proot, ppos, &pnode);
	commit_db(proot);
}

void free_field(dbfile_t *proot, FOSET ppos){
	field_t temp_field;
	field_t next_field;
	field_t prev_field;
	FOSET prev, next;
	unsigned int size;

	read_field(proot, ppos, &temp_field);
	size = temp_field.d_size;

	if(!size){
		return;
	}

	if(proot->data.d_free == EMPTY){
		temp_field.f_next = EMPTY;
		write_field(proot, ppos, &temp_field);
		proot->data.d_free = ppos;
	}else{
		prev = EMPTY;
		next = proot->data.d_free;
		read_field(proot, next, &next_field);
		while(next != EMPTY && next_field.d_size<=size){
			prev = next;
			next = next_field.f_next;
			if(next == EMPTY){
				break;
			}
			read_field(proot, next, &next_field);
		}
		if(next == EMPTY){
			read_field(proot, prev, &prev_field);
			prev_field.f_next = ppos;
			temp_field.f_next = EMPTY;
			write_field(proot, prev, &prev_field);
			write_field(proot, ppos, &temp_field);
		}else{
        	if(prev != EMPTY){
        		read_field(proot, prev, &prev_field);
				temp_field.f_next = prev_field.f_next;
				prev_field.f_next = ppos;
				write_field(proot, ppos, &temp_field);
				write_field(proot, prev, &prev_field);
        	}else{
				temp_field.f_next = proot->data.d_free;
				proot->data.d_free = ppos;
				write_field(proot, ppos, &temp_field);
        	}
		}
	}
	commit_db(proot);
}
