#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

void print_column(dbfile_t *proot, FOSET c_next, int cnt){
	FOSET ppos;
	column_t col;
	char *signness;

	if(c_next != EMPTY){
		read_column(proot, c_next, &col);
		if(get_datatype(col.d_type).signness){
			signness = get_bool(col.usign);
		}else{
			signness = "-";
		}
		printf("%3d%9d%6s%20s%6d%10s%12s%8d\n", cnt, c_next, "-", col.name, col.maxsize, get_datatype_name(col.d_type), signness, col.c_next);
		ppos = col.c_next;
		if(ppos != EMPTY){
			print_column(proot, ppos, ++cnt);
		}
	}
}

static void print_table_column(dbfile_t *proot, FOSET c_next){
	FOSET ppos;
	column_t col;

	if(c_next != EMPTY){
		read_column(proot, c_next, &col);
		printf("%10s", col.name);
		ppos = col.c_next;
		if(ppos != EMPTY){
			print_table_column(proot, ppos);
		}
	}
}

static void print_table_datafield(dbfile_t *proot, FOSET ppos, int idx){
	FOSET next;
	unsigned int size;
	field_t pfield;
	cast_t pdata;

	if(ppos != EMPTY){
		column_t col_type = get_column(proot, idx);
		read_field(proot, ppos, &pfield);
		next = pfield.f_next;
		size = pfield.d_size;

		pdata.type = col_type.d_type;
		pdata.size = size;

		if(size){
			FOSET pdpos = ppos + sizeof(field_t);
			//char *pmalloc = (char *)malloc(size+1);
			//pmalloc[size] = '\0';

			read_datafield(proot, pdpos, &pdata);
			printf("%10s", (char *)pdata.value);
			free(pdata.value);
			//printf("%10s", pmalloc);
			//sprintf("%p", pdata.value);
			//free(pmalloc);
		}else{
			printf("    (null)");
		}
		idx++;

		if(next != EMPTY){
			print_table_datafield(proot, next, idx);
		}
	}
}

static void print_table_row(dbfile_t *proot, FOSET n_child, int *cnt){
	int i;
	node_t node;

	if(n_child != EMPTY){
		read_node(proot, n_child, &node);
		for(i=0; i<node.cnt; i++){
			printf("%2d   ", (*cnt)++);
			printf("%4d", node.key[i]);
			print_table_datafield(proot, node.f_val[i], 0);
			printf("\n");
		}
		for(i=0; i<=node.cnt; i++){
			print_table_row(proot, node.n_child[i], cnt);
		}
	}
}

void print_table(dbfile_t *proot){
	int i;
	int cnt = 0;
	
	printf(" #  !psid ");
	print_table_column(proot, proot->data.c_root);

	printf("\n");
	printf("----------");
	for(i=0; i<proot->c_cnt; i++){
		printf("----------");
	}
	printf("\n");
	
	print_table_row(proot, proot->data.n_root, &cnt);
}

void print_tree(dbfile_t *proot, FOSET n_child, int cnt){
	int i;
	node_t node;

	if(n_child != EMPTY){
		read_node(proot, n_child, &node);
		int pcnt = node.cnt;

		printf("Node offset: %3d\n", n_child);
		printf("Node  count: %3d\n", cnt);
		printf("Idx   Key        Value    Next\n");
		printf("------------------------------\n");
		for(i=0; i<pcnt; i++){
			printf("%3d%6d%13d%8d\n", i, node.key[i], node.f_val[i], node.n_child[i]);
		}
		if(node.n_child[pcnt] != EMPTY){
			printf("%3d     -            -%8d\n", i, node.n_child[pcnt]);
		}
		for(i=0; i<=pcnt; i++){
			print_tree(proot, node.n_child[i], ++cnt);
		}
	}
}

void print_datafield(dbfile_t *proot, FOSET ppos){
	FOSET n;
	int size;
	field_t pfield;

	if(ppos != EMPTY){
		read_field(proot, ppos, &pfield);
		n = pfield.f_next;
		size = pfield.d_size;
		printf("\t[%d] %d\n", ppos, size);
		if(n != EMPTY){
			print_datafield(proot, n);
		}
	}
}

// refactor function and names
void found(dbfile_t *proot, FOSET ppos, int idx){
	node_t node;
	field_t field;
	cast_t pdata;
	int i = 0;

	read_node(proot, ppos, &node);

	printf("Idx   Offset            Datafied   Fieldsize      Type   Maxsize    Next\n");
	printf("------------------------------------------------------------------------\n");
	FOSET q = node.f_val[idx];
	while(q != EMPTY){
		column_t col_type = get_column(proot, i);
		if(fseek(proot->vfp, q, SEEK_SET)){
			printf("fseek in get_descriptor");
		}
		if(fread(&field, sizeof(field_t), 1, proot->vfp) == 0){
			printf("fseek in wrstart");
		}

		if(field.d_size){
			FOSET p = q + sizeof(field_t);
			//char *otest = malloc(field.d_size+1);
			//otest[field.d_size] = '\0';
			pdata.type = col_type.d_type;
			//pdata.value = otest;
			pdata.size = field.d_size;
			read_datafield(proot, p, &pdata);

			printf("%3d%9d%20s%12d%10s%10d%8d\n", i, q, (char *)pdata.value, field.d_size, get_datatype_name(i), col_type.maxsize, field.f_next);
			free(pdata.value);
		}else{
			printf("%3d%9d              (NULL)%6d%8d\n", i, q, field.d_size, field.f_next);
		}
		q = field.f_next;
		i++;
	}
}

void print_status(dbfile_t *proot){
	time_t t = (time_t)proot->head.tstamp;
	printf("Option                        Value\n");
	printf("-----------------------------------\n");
	printf("Version                %8d.%d.%d\n", proot->head.release, proot->head.ver_maj, proot->head.ver_min);
	printf("Created    %s", asctime(localtime(&t)));
	printf("Primary key            %12d\n", proot->data.c_pk);
	printf("Column root            %12d\n", proot->data.c_root);
	printf("Node root              %12d\n", proot->data.n_root);
	printf("Column freelist        %12d\n", proot->data.c_free);
	printf("Node freelist          %12d\n", proot->data.n_free);
	printf("Datafield freelist     %12d\n", proot->data.d_free);
	printf("Nodes                         Count\n");
	printf("-----------------------------------\n");
	printf("Node count             %12d\n", proot->n_cnt);
	printf("Column count           %12d\n", proot->c_cnt);
	printf("Key count              %12d\n", proot->k_cnt);
	printf("Auto sequence serial   %12d\n", proot->seq_cnt);
	printf("I/O stats [volatile]          Count\n");
	printf("-----------------------------------\n");
	printf("Commit                 %12d\n", proot->cnt->cmt_vcnt);
	printf("Column read            %12d\n", proot->cnt->c_rd_vcnt);
	printf("Column write           %12d\n", proot->cnt->c_wr_vcnt);
	printf("Node read              %12d\n", proot->cnt->n_rd_vcnt);
	printf("Node write             %12d\n", proot->cnt->n_wr_vcnt);
	printf("Field read             %12d\n", proot->cnt->f_rd_vcnt);
	printf("Field write            %12d\n", proot->cnt->f_wr_vcnt);
	printf("Datafield read         %12d\n", proot->cnt->d_rd_vcnt);
	printf("Datafield write        %12d\n", proot->cnt->d_wr_vcnt);
}

