#include <stdio.h>
#include <stdlib.h>

#include "dbtree.h"

void read_node(dbfile_t *proot, FOSET ppos, node_t *pnode){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
	}
	if(fread(pnode, sizeof(node_t), 1, proot->vfp) == 0){
		printf("Read failed\n");
	}
	proot->cnt->n_rd_vcnt++;
}

void write_node(dbfile_t *proot, FOSET ppos, node_t *pnode){
	if(fseek(proot->vfp, ppos, SEEK_SET)){
		printf("Seek failed\n");
	}
	if(fwrite(pnode, sizeof(node_t), 1, proot->vfp) == 0){
		printf("Write failed\n");
	}
	proot->cnt->n_wr_vcnt++;
}

static int get_key_position(int key, int *pkey, int pcnt){
	int i, left, right;

	if(key <= pkey[0]){
		return 0;
	}
	if(key > pkey[pcnt-1]){
		return pcnt;
	}

	left = 0;
	right = pcnt-1;
	while(right - left > 1){
		i = (right + left)/2;
		if(key <= pkey[i]){
			right = i;
		}else{
			left = i;
		}
	}

	return right;
}

STATUS truncate_root(dbfile_t *proot){
	node_t node;

	while(proot->data.n_root != EMPTY){
		read_node(proot, proot->data.n_root, &node);
		delete_key(proot, node.key[0]);
	}

	if(proot->k_cnt > 0 || proot->n_cnt > 0){
		return NOTCOMPLETE;
	}

	return SUCCESS;
}

result_t search_key(dbfile_t *proot, int key){
	int i, *pkey, pcnt;
	node_t node;
	result_t rs;
	FOSET ppos = proot->data.n_root;

	while(ppos != EMPTY){
		read_node(proot, ppos, &node);
		pkey = node.key;
		pcnt = node.cnt;

		i = get_key_position(key, pkey, pcnt);
		if(i < pcnt && key == pkey[i]){
			rs.rstat = SUCCESS;
			rs.fpos = ppos;
			rs.idx = i;
			return rs;
		}
		ppos = node.n_child[i];
	}

	rs.rstat = NOTFOUND;
	return rs;
}

STATUS change(dbfile_t *proot, int key, void *value, int idx){
	int i, *pkey, pcnt;
	node_t node;
	FOSET ppos = proot->data.n_root;

	while(ppos != EMPTY){
		read_node(proot, ppos, &node);
		pkey = node.key;
		pcnt = node.cnt;

		i = get_key_position(key, pkey, pcnt);
		if(i < pcnt && key == pkey[i]){
			change_field(proot, ppos, i, value, idx);

			return SUCCESS;
		}
		ppos = node.n_child[i];
	}

	return NOTFOUND;
}

/* Recursively insert */
static STATUS recursive_insert(dbfile_t *proot, int key, void *value, FOSET ppos, int *rkey, FOSET *rpos, FOSET *rval){
	FOSET *node_child, *node_val, final_child, final_val, new_ppos, new_value;
	int *node_cnt, *node_key, final_key;
	int i, j, new_key;
	STATUS code;
	node_t node, new_node;
	column_t col;

	// Check if leaf
	if(ppos == EMPTY){
		read_column(proot, proot->data.c_root, &col);
		*rkey = key;
		*rpos = EMPTY;
		*rval = alloc_field(proot, value, col.d_type);

		return NOTCOMPLETE;
	}

	// Not leaf, read current node
	read_node(proot, ppos, &node);
	node_cnt = &node.cnt;
	node_key = node.key;
	node_child = node.n_child;
	node_val = node.f_val;

	// Get key position in current key
	i = get_key_position(key, node_key, *node_cnt);
	if(i < *node_cnt && key == node_key[i]){
		return DUPLICATEKEY;
	}

	// Try insert in child node, even if not exist
	code = recursive_insert(proot, key, value, node_child[i], &new_key, &new_ppos, &new_value);
	if(code != NOTCOMPLETE){
		return code;
	}

	// Try insert in this node if node is not full
	if(*node_cnt < ND_DEGREE){
		i = get_key_position(new_key, node_key, *node_cnt);
		for(j=*node_cnt; j>i; j--){
			node_key[j] = node_key[j-1];
			node_child[j+1] = node_child[j];
			node_val[j] = node_val[j-1];
		}
		node_key[i] = new_key;
		node_child[i+1] = new_ppos;
		node_val[i] = new_value;
		++*node_cnt;

		write_node(proot, ppos, &node);
		return SUCCESS;
	}

	// If node is full continue otherwise shift to half
	if(i == ND_DEGREE){
		final_key = new_key;
		final_child = new_ppos;
		final_val = new_value;
	}else{
		final_key = node_key[ND_DEGREE-1]; //preserve most right key
		final_child = node_child[ND_DEGREE]; //preserve most right pointer
		final_val = node_val[ND_DEGREE-1]; //preserve most right value
		for(j=ND_DEGREE-1; j>i; j--){
			node_key[j] = node_key[j-1];
			node_child[j+1] = node_child[j];
			node_val[j] = node_val[j-1];
		}
		node_key[i] = new_key;
		node_child[i+1] = new_ppos;
		node_val[i] = new_value;
	}

	// Setup new node
	*rkey = node_key[ND_HALF];
	*rpos = alloc_node(proot);
	*rval = node_val[ND_HALF];
	*node_cnt = ND_HALF;
	proot->n_cnt++;

	new_node.cnt = ND_HALF;
	for(j=0; j<ND_HALF-1; j++){
		new_node.key[j] = node_key[j+ND_HALF+1];
		new_node.n_child[j] = node_child[j+ND_HALF+1];
		new_node.f_val[j] = node_val[j+ND_HALF+1];
	}
	new_node.n_child[ND_HALF-1] = node_child[ND_DEGREE];
	new_node.key[ND_HALF-1] = final_key;
	new_node.n_child[ND_HALF] = final_child;
	new_node.f_val[ND_HALF-1] = final_val;

	write_node(proot, ppos, &node);
	write_node(proot, *rpos, &new_node);
	return NOTCOMPLETE;
}

/* Insert key */
STATUS insert_key(dbfile_t *proot, int key, void *value){
	FOSET rpos, rval, root_pos;
	int rkey;

	STATUS code = recursive_insert(proot, key, value, proot->data.n_root, &rkey, &rpos, &rval);
	node_t rootnode;

	if(code == DUPLICATEKEY){
		printf("Duplicate\n");
	}else{
		if(code == NOTCOMPLETE){
			root_pos = alloc_node(proot);
			rootnode.cnt = 1;
			rootnode.key[0] = rkey;
			rootnode.f_val[0] = rval;
			rootnode.n_child[0] = proot->data.n_root;
			rootnode.n_child[1] = rpos;
			proot->data.n_root = root_pos;
			proot->n_cnt++;
			write_node(proot, root_pos, &rootnode);
			commit_db(proot);

			code = SUCCESS;
		}
	}

	if(code == SUCCESS){
		proot->k_cnt++;
	}
	return code;
}

static STATUS recursive_delete(dbfile_t *ptree, int x, FOSET t){
	int i, j, z, *k, *n, *item, *nleft, *nright, *lkey, *rkey, borrowleft = 0, nq, *addr;
	STATUS code;
	FOSET *p, left, right, *lptr, *rptr, q, q1, *v, *lval, *rval, *ival, *addr2;
	node_t nod, nod1, nod2, nodL, nodR;

	if(t == -1){
		return NOTFOUND;
	}
	read_node(ptree, t, &nod);
	n = &nod.cnt;
	k = nod.key;
	p = nod.n_child;
	v = nod.f_val;
	i = get_key_position(x, k, *n);
	/* *t is a leaf */
	if(p[0] == -1){
		if(i == *n || x < k[i]){
			return NOTFOUND;
		}

		free_field(ptree, v[i]);
		/* x is now equal to k[i], located in a leaf:  */
		for(j=i+1; j < *n; j++){
			k[j-1] = k[j];
			p[j] = p[j+1];
			v[j-1] = v[j];
		}
		--*n;
		write_node(ptree, t, &nod);

		if(t == ptree->data.n_root){
			z = 1;
		}else{
			z = ND_HALF;
		}

		if(*n >= z){
			return SUCCESS;
		}else{
			return UNDERFLOW;
		}
	}
	/*  t is an interior node (not a leaf): */
	item = k+i;
	ival = v+i;
	left = p[i];
	read_node(ptree, left, &nod1);
	nleft = &nod1.cnt;
	/* x found in interior node.  Go to left child *p[i] and then follow a

	  path all the way to a leaf, using rightmost branches:  */
	if(i < *n && x == *item){
		FOSET vv = v[i];
		q = p[i];
		read_node(ptree, q, &nod1);
		nq = nod1.cnt;
		while(q1 = nod1.n_child[nq], q1!= -1){
			q = q1;
			read_node(ptree, q, &nod1);
			nq = nod1.cnt;
		}
		/*  Exchange k[i] with the rightmost item in that leaf:   */
		addr = nod1.key + nq -1;
		addr2 = nod1.f_val + nq -1;
		*item = *addr;
		*ival = *addr2;
		*addr = x;
		*addr2 = vv;
		write_node(ptree, t, &nod);
		write_node(ptree, q, &nod1);
	}

	/*  Delete x in subtree with root p[i]:  */
	code = recursive_delete(ptree, x, left);
	if(code != UNDERFLOW){
		return code;
	}

	/*  Underflow, borrow, and , if necessary, merge:  */
	if(i < *n){ //check if not last item
		read_node(ptree, p[i+1], &nodR);
	}
	if(i == *n || nodR.cnt == ND_HALF){ //check if last item or if right node == half
		if(i > 0){
			read_node(ptree, p[i-1], &nodL);
			if(i == *n || nodL.cnt > ND_HALF){
				borrowleft = 1;
			}
		}
	}
	/* borrow from left sibling */
	if(borrowleft){
		item = k+i-1;
		ival = v+i-1;
		left = p[i-1];
		right = p[i];
		nod1 = nodL;
		read_node(ptree, right, &nod2);
		nleft = &nod1.cnt;
	}else{
		right = p[i+1];        /*  borrow from right sibling   */
		read_node(ptree, left, &nod1);
		nod2 = nodR;
	}
	//nod1 = left
	//nod2 = right
	// save al node elements from left and right
	nright = &nod2.cnt;
	lkey = nod1.key;
	rkey = nod2.key;
	lptr = nod1.n_child;
	rptr = nod2.n_child;
	lval = nod1.f_val;
	rval = nod2.f_val;

	if(borrowleft){
		rptr[*nright + 1] = rptr[*nright];
		for(j=*nright; j>0; j--){
			rkey[j] = rkey[j-1];
			rptr[j] = rptr[j-1];
			rval[j] = rval[j-1];
		}
		++*nright;
		rkey[0] = *item;
		rval[0] = *ival;
		rptr[0] = lptr[*nleft];
		*item = lkey[*nleft - 1];
		*ival = lval[*nleft - 1];
		if(--*nleft >= ND_HALF){
			write_node(ptree, t, &nod);
			write_node(ptree, left, &nod1);
			write_node(ptree, right, &nod2);

			return SUCCESS;
		}
	}else{
		/* borrow from right sibling */
		if(*nright > ND_HALF){
			lkey[ND_HALF-1] = *item;
			lval[ND_HALF-1] = *ival;
			lptr[ND_HALF] = rptr[0];
			*item = rkey[0];
			*ival = rval[0];
			++*nleft;
			--*nright;
			for(j=0; j < *nright; j++){
				rptr[j] = rptr[j+1];
				rkey[j] = rkey[j+1];
				rval[j] = rval[j+1];
			}
			rptr[*nright] = rptr[*nright + 1];
			write_node(ptree, t, &nod);
			write_node(ptree, left, &nod1);
			write_node(ptree, right, &nod2);

			return SUCCESS;
		}
	}

	/*  Merge   */
	lkey[ND_HALF-1] = *item;
	lval[ND_HALF-1] = *ival;
	lptr[ND_HALF] = rptr[0];
	for(j=0; j<ND_HALF; j++){
		lkey[ND_HALF+j] = rkey[j];
		lptr[ND_HALF+j+1] = rptr[j+1];
		lval[ND_HALF+j] = rval[j];
	}
	*nleft = ND_DEGREE;
	free_node(ptree, right);
	ptree->n_cnt--;

	for(j=i+1; j < *n; j++){
		k[j-1] = k[j];
		p[j] = p[j+1];
		v[j-1] = v[j];
	}
	--*n;
	write_node(ptree, t, &nod);
	write_node(ptree, left, &nod1);

	if(t == ptree->data.n_root){
		z = 1;
	}else{
		z = ND_HALF;
	}

	if(*n >= z){
		return SUCCESS;
	}else{
		return UNDERFLOW;
	}
}

STATUS delete_key(dbfile_t *proot, int key){
	FOSET newroot;
	node_t rootnode;

	STATUS code = recursive_delete(proot, key, proot->data.n_root);
	if(code == UNDERFLOW){
		if(proot->data.n_root != -1){
			read_node(proot, proot->data.n_root, &rootnode);
			newroot = rootnode.n_child[0];
			free_node(proot, proot->data.n_root);
			proot->n_cnt--;
			proot->data.n_root = newroot;
			commit_db(proot);
		}
		code = SUCCESS;
	}

	if(code == SUCCESS){
		proot->k_cnt--;
	}

	return code;
}

