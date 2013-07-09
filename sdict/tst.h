#ifndef TSTDB
#define TSTDB "tstdb"
#include <stdio.h>
#include <Python.h>

#define MAX_KEY_SIZE 2560
typedef unsigned int uint32;
typedef unsigned long long uint64;

#pragma pack(1)
typedef struct{
	unsigned char c;
	uint32 left,mid,right,parent;
	PyObject * value;	
}tst_node;
#pragma pack()


typedef struct{
	uint32 root;
	uint32 size;
	uint32 cap;
	uint32 f_head; //head of reuseable node list
	int ent_count;
	tst_node* data;//data[0] is not used
}tst_db;

enum SortingOrder 
{
	ASC=1,
	DESC=0
};

tst_db *create_tst_db(void);
void tst_put(tst_db * db, PyObject *key, PyObject * value);
PyObject * tst_get(tst_db * db, PyObject *key);
void tst_prefix(tst_db *db, PyObject* prefix,
             PyObject* result, int limit,enum SortingOrder);
void tst_less(tst_db *db, PyObject* key,
             PyObject* result, int limit);
void tst_greater(tst_db *db, PyObject* key,
             PyObject* result, int limit);

void tst_delete(tst_db *db, PyObject * key);
void free_tst_db(tst_db * db);
int tst_length(tst_db *db);
void tst_all(tst_db *db,PyObject * result);
void tst_from_list(tst_db *db, PyObject* py_list);
 
#endif
