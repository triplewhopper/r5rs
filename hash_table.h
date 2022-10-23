#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_
#include "Include/typeobject.h"
typedef long long LL;
typedef unsigned long long ULL;
typedef struct HashTableNode{
	ScmObject *key;
	ScmObject *value;
	struct HashTableNode *next;
}HashTableNode;

struct HashTable{
	size_t _size;
	size_t n_bucket;
	const double _load_factor;
	HashTableNode **_buckets;
}

HashTable *hash_table_new(){
	HashTable *res = my_malloc(sizeof(HashTable));
	*res = (HashTable){0, 0, 0.75, NULL};
	return res;
}

void hash_table_init(HashTable *self, size_t n_bucket){
	self->_size = 0;
	self->n_bucket = n_bucket;
	self->_nodes = my_malloc(size*sizeof(HashTableNode*));
	memset(self->_nodes, 0, size*sizeof(HashTableNode*));
}
void hash_table_reserve(HashTable *self, size_t capacity){
	
}
size_t my_hash_func(size_t hash_v, size_t capacity){
	return hash_v%capacity;
}
void hash_table_insert(HashTable *self, ScmObject *key, ScmObject *value){
	if(self->_capacity*self->_load_factor<=self->_size){
		hash_table_reserve(self, 2*self->_capacity);
	}

	const size_t hash_v = hash_func(key);
	const size_t index = my_hash_func(hash_v, self->_capacity);

	if(self->_nodes[index]){
		HashTableNode *node1 = my_malloc(sizeof(HashTableNode));
		node1->key = key;
		node1->value = value;
		node1->next = self->_nodes[index];
		self->_nodes[index] = node1;
	}else{
		HashTableNode *node1 = my_malloc(sizeof(HashTableNode));
		node1->key = key;
		node1->value = value;
		node1->next = NULL;
		self->_nodes[index] = node1;
	}

}
void *hash_table_find(HashTable *self, void *key, size_t (*hash_func)(void *)){

}
void hash_table_remove(HashTable *self, void *key, void *value, size_t (*hash_func)(void *), int (*eq)(void *, void *)){
	const size_t hash_v = hash_func(key);
	const size_t index = my_hash_func(hash_v, self->_capacity);

	if(self->_nodes[index]){
		HashTableNode *node1 = my_malloc(sizeof(HashTableNode));
		node1->key = key;
		node1->value = value;
		node1->next = self->_nodes[index];
		self->_nodes[index] = node1;
	}else{
		HashTableNode *node1 = my_malloc(sizeof(HashTableNode));
		node1->key = key;
		node1->value = value;
		node1->next = NULL;
		self->_nodes[index] = node1;
	}

}
#endif