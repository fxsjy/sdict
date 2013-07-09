#include "tst.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static tst_db *
new_tst_db(uint32 cap)
{
	tst_db *db = (tst_db *) malloc(sizeof(tst_db));
	db->size = 0;
	db->root = 0;
	db->f_head = 0;
	db->cap = cap;
	db->ent_count = 0;
	db->data = (tst_node *) malloc(sizeof(tst_node) * cap);
	memset(&db->data[0], 0, sizeof(db->data));
	return db;
}

int tst_length(tst_db *db)
{
	if(db){
		return db->ent_count;
	}
	return 0;
}

tst_db *
create_tst_db(void)
{
	return new_tst_db(409600);
}

void
free_tst_db(tst_db * db)
{
	int i;
	if (db != NULL) {
		if (db->data != NULL) {
			for(i=0;i< db->cap;i++){
				if(db->data[i].value)
					Py_DECREF(db->data[i].value);
			} 		
			free(db->data);
		}
		free(db);
	}
}

static void
ensure_enough_space(tst_db * db)
{
	tst_node *new_p;
	if (db != NULL) {
		if (db->size >= db->cap - 1) {
			//printf(">> allocate new memory\n");
			new_p = (tst_node *) realloc(db->data, sizeof(tst_node) * (db->cap) * 2);
			if (new_p == NULL) {
				printf("no enough space");
				exit(1);
			}
			db->data = new_p;
			db->cap = (db->cap) * 2;
		}
	}
}

static void
free_node(tst_db * db, uint32 node)
{
	db->data[node].mid = db->f_head;
	db->data[node].value = 0;
	db->f_head = node;
}

static uint32
new_node(tst_db * db)
{
	uint32 tmp;
	if (db->f_head == 0) {
		ensure_enough_space(db);
		db->size++;
		memset(&db->data[db->size], 0, sizeof(tst_node));
		db->data[db->size].value = 0;
		return db->size;
	} else {
		tmp = db->f_head;
		db->f_head = db->data[db->f_head].mid;
		memset(&db->data[tmp], 0, sizeof(tst_node));
		return tmp;
	}
}

static uint32
insert(tst_db * db, uint32 node, const unsigned char *s, PyObject * value, int d, int len_of_s, uint32 parent)
{
	unsigned char c = s[d];
	uint32 x = node, no;
	if (x == 0) {
		x = new_node(db);
		db->data[x].c = c;
	}
	if (c < db->data[x].c) {
		no = insert(db, db->data[x].left, s, value, d, len_of_s, x);
		db->data[x].left = no;
	} else if (c > db->data[x].c) {
		no = insert(db, db->data[x].right, s, value, d, len_of_s, x);
		db->data[x].right = no;
	} else if (d < len_of_s - 1) {

		no = insert(db, db->data[x].mid, s, value, d + 1, len_of_s, x);
		db->data[x].mid = no;
	} else {
		if(db->data[x].value){
			Py_DECREF(db->data[x].value);
		}else{
			db->ent_count ++;
		}
		db->data[x].value = value;
	}
	db->data[x].parent = parent;

	return x;
}

static uint32
search(tst_db * db, const unsigned char *key, int len_of_s)
{
	int len_of_key = len_of_s;
	uint32 node = db->root;
	unsigned char c;
	int d = 0;

	if (len_of_key <= 0)
		return 0;

	while (node) {
		c = key[d];
		if (c < db->data[node].c) {
			node = db->data[node].left;
		} else if (c > db->data[node].c) {
			node = db->data[node].right;
		} else if (d < len_of_key - 1) {
			node = db->data[node].mid;
			d++;
		} else {
			break;
		}
	}

	return node;
}

static uint32
search_with_path(tst_db * db, const unsigned char *key, int len_of_s, unsigned char *path,int *key_len)
{
	int len_of_key = len_of_s;
	uint32 node = db->root;
	unsigned char c;
	int d = 0;

	if (len_of_key <= 0)
		return 0;
	while (node) {
		c = key[d];
		if (c < db->data[node].c) {
			node = db->data[node].left;
		} else if (c > db->data[node].c) {
			node = db->data[node].right;
		} else if (d < len_of_key - 1) {
			path[d] = db->data[node].c;
			node = db->data[node].mid;
			d++;
		} else {
			path[d] = db->data[node].c;
			break;
		}
	}
	*key_len = d+1;
	return node;
}

void
tst_put(tst_db * db, PyObject *key, PyObject * value)
{
	//printf("db: %x\n",db);
	int len_of_key = PyString_Size(key);
	if (len_of_key <= 0)
		return;
	Py_INCREF(value);
	db->root = insert(db, db->root, (unsigned char*)PyString_AsString(key), value, 0, len_of_key, 0);
}

static uint32
remove_node(tst_db * db, uint32 node)
{

	uint32 max_p;
	uint32 left = db->data[node].left;
	uint32 right = db->data[node].right;

	//printf("remove %c\n",db->data[node].c);

	free_node(db, node);

	if (left == 0 && right == 0) {
		return 0;
	} else if (left > 0 && right == 0) {
		return left;
	} else if (right > 0 && left == 0) {
		return right;
	} else {
		max_p = left;
		while (db->data[max_p].right) {
			max_p = db->data[max_p].right;
		}
		db->data[max_p].right = right;
		return left;
	}
}

void
tst_delete(tst_db * db, PyObject *key)
{
	int len_of_key = PyString_Size(key);
	uint32 node = db->root;
	unsigned char c;
	int d = 0;
	uint32 myparent = 0;
	uint32 new_p;
	unsigned char * pkey = (unsigned char *)PyString_AsString(key);
	if (len_of_key <= 0)
		return;

	while (node) {
		c = pkey[d];
		if (c < db->data[node].c) {
			node = db->data[node].left;
		} else if (c > db->data[node].c) {
			node = db->data[node].right;
		} else if (d < len_of_key - 1) {
			node = db->data[node].mid;
			d++;
		} else {
			break;
		}
	}

	if (node) {
		Py_DECREF(db->data[node].value);
		db->data[node].value = 0;
		db->ent_count --;
	}
	while (node) {		//found it
		myparent = db->data[node].parent;

		if (db->data[node].mid == 0 && db->data[node].value == 0) {	//can delete
			if (myparent > 0) {
				if (node == db->data[myparent].left) {	//is left child
					new_p = remove_node(db, node);
					db->data[myparent].left = new_p;
					if (new_p) {
						db->data[new_p].parent = myparent;
					}
				} else if (node == db->data[myparent].right) {	//is right child
					new_p = remove_node(db, node);
					db->data[myparent].right = new_p;
					if (new_p) {
						db->data[new_p].parent = myparent;
					}
				} else {	//is middle child
					new_p = remove_node(db, node);
					db->data[myparent].mid = new_p;
					if (new_p) {
						db->data[new_p].parent = myparent;
					}
				}
			} else {
				new_p = remove_node(db, node);
				db->root = new_p;
				db->data[new_p].parent = myparent;
			}
		}
		node = myparent;
	}
}


static
    void
append_result(const unsigned char *key, PyObject* result, int *result_size,int key_len)
{
	//printf("append: %s\n",key);
	//strcat(result, key);
	//strcat(result, "\r\n");
	PyList_Append(result,Py_BuildValue("s#",key,key_len));
	*result_size = (*result_size) + 1;
}

static
    void
append_result_with_value(const unsigned char *key, PyObject* result, int *result_size,int key_len, PyObject* value)
{
	//printf("append: %s\n",key);
	//strcat(result, key);
	//strcat(result, "\r\n");
	PyObject * tup = PyTuple_New(2);
	PyTuple_SetItem(tup,0,Py_BuildValue("s#",key,key_len));
	PyTuple_SetItem(tup,1,value);
	PyList_Append(result,tup);
	*result_size = (*result_size) + 1;
}

static
void
dfs(tst_db * db, uint32 node, PyObject * result, int* result_size, unsigned char key_buf[], int d, int limit, enum SortingOrder sorting_order)
{
	uint32 first, second;
	//printf("--dfs--\n");
	if (node == 0)
		return;

	if (*result_size >= limit)
		return;
	if (sorting_order == ASC) {
		first = db->data[node].left;
		second = db->data[node].right;
	} else {
		first = db->data[node].right;
		second = db->data[node].left;
	}
	dfs(db, first, result,result_size, key_buf, d, limit, sorting_order);
	key_buf[d] = db->data[node].c;

	if (*result_size >= limit)
		return;

	if (sorting_order == ASC) {
		if (db->data[node].value) {
			append_result(key_buf, result, result_size,d+1);
		}

		dfs(db, db->data[node].mid, result,result_size, key_buf, d + 1, limit, sorting_order);
		key_buf[d] = db->data[node].c;
	} else {
		dfs(db, db->data[node].mid, result,result_size, key_buf, d + 1, limit, sorting_order);
		if (*result_size >= limit)
			return;

		if (db->data[node].value) {
			append_result(key_buf, result, result_size,d+1);
		}
	}
	if (*result_size >= limit)
		return;

	dfs(db, second, result,result_size, key_buf, d, limit, sorting_order);
	key_buf[d] = db->data[node].c;


}

static
void
dfs_with_value(tst_db * db, uint32 node, PyObject * result, int* result_size, unsigned char key_buf[], int d, int limit, enum SortingOrder sorting_order)
{
	uint32 first, second;
	//printf("--dfs--\n");
	if (node == 0)
		return;

	if (*result_size >= limit)
		return;
	if (sorting_order == ASC) {
		first = db->data[node].left;
		second = db->data[node].right;
	} else {
		first = db->data[node].right;
		second = db->data[node].left;
	}
	dfs_with_value(db, first, result,result_size, key_buf, d, limit, sorting_order);
	key_buf[d] = db->data[node].c;

	if (*result_size >= limit)
		return;

	if (sorting_order == ASC) {
		if (db->data[node].value) {
			append_result_with_value(key_buf, result, result_size,d+1,db->data[node].value);
		}

		dfs_with_value(db, db->data[node].mid, result,result_size, key_buf, d + 1, limit, sorting_order);
		key_buf[d] = db->data[node].c;
	} else {
		dfs_with_value(db, db->data[node].mid, result,result_size, key_buf, d + 1, limit, sorting_order);
		if (*result_size >= limit)
			return;

		if (db->data[node].value) {
			append_result_with_value(key_buf, result, result_size,d+1,db->data[node].value);
		}
	}
	if (*result_size >= limit)
		return;

	dfs_with_value(db, second, result,result_size, key_buf, d, limit, sorting_order);
	key_buf[d] = db->data[node].c;


}

static
    void
dfs_less(tst_db * db, uint32 node, const unsigned char *key, PyObject* result , int* result_size, unsigned char key_buf[], int d, int limit)
{
	unsigned char c;
	if (node == 0)
		return;
	if (limit <= 0)
		return;
	c = db->data[node].c;
	key_buf[d] = c;
	//printf("debug:%s,limit:%d,result_size:%d,c:%c\n",key,limit,*result_size,c);   
	//printf("key:%s\n",key);
	if (key[0] < c) {
		dfs_less(db, db->data[node].left, key, result, result_size, key_buf, d, limit);
	} else if (key[0] == c) {
		dfs_less(db, db->data[node].mid, key + 1, result, result_size, key_buf, d + 1, limit);
		if (*result_size < limit) {
			if (db->data[node].value) {
				append_result(key_buf, result, result_size,d+1);
			}
		}
		if (*result_size < limit) {
			dfs(db, db->data[node].left, result, result_size, key_buf, d, limit, DESC);
		}
	} else {
		dfs_less(db, db->data[node].right, key, result, result_size, key_buf, d, limit);
		key_buf[d] = c;

		if (*result_size < limit) {
			dfs(db, db->data[node].mid, result, result_size, key_buf, d + 1, limit, DESC);
		}

		if (*result_size < limit) {
			if (db->data[node].value) {
				append_result(key_buf, result, result_size,d+1);
			}
		}
		if (*result_size < limit) {
			dfs(db, db->data[node].left, result, result_size, key_buf, d, limit, DESC);
		}

	}

}

static
    void
dfs_greater(tst_db * db, uint32 node, const unsigned char *key, PyObject * result , int* result_size, unsigned char key_buf[], int d, int limit)
{
	unsigned char c;
	
	if (node == 0)
		return;
	if (limit <= 0)
		return;
	c = db->data[node].c;
	key_buf[d] = c;

	if (key[0] > c) {
		dfs_greater(db, db->data[node].right, key, result, result_size, key_buf, d, limit);
	} else if (key[0] == c) {
		if(*result_size < limit){
			if(*(key+1)=='\0' && db->data[node].value){
				append_result(key_buf, result, result_size,d+1);
			}
		}
		if (*result_size < limit)
			dfs_greater(db, db->data[node].mid, key + 1, result, result_size, key_buf, d + 1, limit);
		if (*result_size < limit) {
			dfs(db, db->data[node].right, result, result_size, key_buf, d, limit, ASC);
		}
	} else {
		dfs_greater(db, db->data[node].left, key, result, result_size, key_buf, d, limit);
		key_buf[d] = c;

		if (*result_size < limit) {
			if (db->data[node].value) {
				append_result(key_buf, result, result_size,d+1);
			}
		}

		if (*result_size < limit) {
			dfs(db, db->data[node].mid, result, result_size, key_buf, d + 1, limit, ASC);
		}

		if (*result_size < limit) {
			dfs(db, db->data[node].right, result, result_size, key_buf, d, limit, ASC);
		}

	}

}

void
tst_less(tst_db * db, PyObject *key, PyObject * result, int limit)
{
	int len_of_key = PyString_Size(key);
	int n_result_size;
	int * result_size = &n_result_size;
	unsigned char base_key[MAX_KEY_SIZE] = { 0 };
	unsigned char* pkey = (unsigned char*)PyString_AsString(key);
	int root;
	if (len_of_key <= 0)
		return;
	root = db->root;
	if (root == 0)
		return;
	*result_size = 0;
	assert(PyList_Check(result));
	dfs_less(db, root, pkey, result,result_size, base_key, 0, limit);

}

void
tst_greater(tst_db * db, PyObject *key, PyObject * result , int limit)
{
	int len_of_key = PyString_Size(key);
	int n_result_size;
	int *result_size = &n_result_size;
	unsigned char base_key[MAX_KEY_SIZE] = { 0 };
	int root;
	unsigned char * pkey = (unsigned char*)PyString_AsString(key);
	if (len_of_key <= 0)
		return;
	root = db->root;
	if (root == 0)
		return;
	*result_size = 0;
	assert(PyList_Check(result));

	dfs_greater(db, root, pkey, result, result_size, base_key, 0, limit);

}

void
tst_prefix(tst_db * db, PyObject *prefix, PyObject *result, int limit, enum SortingOrder sorting_order)
{
	int len_of_prefix = PyString_Size(prefix);
	unsigned char* p_prefix = (unsigned char*)PyString_AsString(prefix);
	int key_len;
	unsigned char base_key[MAX_KEY_SIZE] = { 0 };
	int n_result_size = 0;
	int* result_size = & n_result_size;
	if (len_of_prefix <= 0)
		return;
	uint32 node = search_with_path(db, p_prefix, len_of_prefix, base_key,&key_len);
	if (node == 0)
		return;
	assert(PyList_Check(result));

	if (sorting_order == ASC) {
		if (db->data[node].value && *result_size < limit)
			append_result(base_key, result, result_size,key_len);
	}
	if (*result_size < limit)
		dfs(db, db->data[node].mid, result, result_size, base_key, key_len, limit, sorting_order);
	if (sorting_order == DESC) {
		if (db->data[node].value && *result_size < limit)
			append_result(base_key, result, result_size,key_len);
	}

}


PyObject *
tst_get(tst_db * db, PyObject *key)
{
	int len_of_key = PyString_Size(key);
	unsigned char * pkey = (unsigned char*)PyString_AsString(key);
	PyObject * value;
	if (len_of_key <= 0){
		Py_INCREF(Py_None);
		return Py_None;
	}
	
	uint32 node = search(db, pkey, len_of_key);
	if(db->data[node].value == 0){
		Py_INCREF(Py_None);
		return Py_None;
	}
	value = db->data[node].value;
	//printf("ref_count:%d\n",value->ob_refcnt);
	Py_INCREF(value);
	return value;
}

void
tst_all(tst_db *db, PyObject * result)
{
	int key_len=0;
	unsigned char base_key[MAX_KEY_SIZE] = { 0 };
	int n_result_size = 0;
	int* result_size = & n_result_size;
	int limit = db->ent_count;
	dfs_with_value(db, db->root, result, result_size, base_key, key_len, limit, ASC);

}

void
tst_from_list(tst_db *db,PyObject * py_list)
{
	int list_len =0 ;
	int i;
	PyObject *tuple,*key,*value;
	assert(PyList_Check(py_list));
	list_len = PyList_Size(py_list);
	for (i=0; i<list_len ;i++){
		tuple = PyList_GetItem(py_list,i);
		key = PyTuple_GetItem(tuple,0);
		value = PyTuple_GetItem(tuple,1);	
		tst_put(db,key,value);
	}
}

