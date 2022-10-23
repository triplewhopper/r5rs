#ifndef _UTIL_H_
#define _UTIL_H_

#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <stdio.h>
#include "Include/typedefs.h"

typedef struct {
	i32 *_obj;
	u32 _capacity;
	u32 _size;
	// _size<=_capacity
} util_i32_vector;
#define vector util_i32_vector


u32 util_i32_vector_capacity(const vector *const self) {
	return self->_capacity;
}

u32 util_i32_vector_size(const vector *const self) {
	return self->_size;
}


i32 util_i32_vector_at(const vector *self, const u32 index) {
	assert(0 <= index && index < self->_size);
	return self->_obj[index];
}

void util_i32_vector_init(vector *const self) {
	self->_obj = NULL;
	self->_capacity = 0u;
	self->_size = 0u;
}

vector *util_i32_vector_new() {
	vector *v = (vector *) malloc(sizeof(vector));
	if (v == NULL) {
		perror("util_i32_vector_new failed");
		// to be discussed: how to free all allocated memory?
		exit(EXIT_FAILURE);
	}
	assert(v);
	util_i32_vector_init(v);
	return v;
}


void util_i32_vector_delete(vector *const self) {
	free(self->_obj);
	self->_obj = NULL;
	self->_capacity = 0;
	self->_size = 0;
	free(self);
}

void util_i32_vector_copy(vector *const self, const vector *const from) {
	if (self == from) return;
	free(self->_obj);
	self->_obj = (i32 *) malloc(sizeof(i32) * from->_capacity);
	memcpy(self->_obj, from->_obj, sizeof(i32) * from->_capacity);
	self->_capacity = from->_capacity;
	self->_size = from->_size;

	assert(self->_size <= self->_capacity);
}

void util_i32_vector_move(util_i32_vector *const self, const util_i32_vector *const from) {
	perror("Not implemented yet");
	exit(EXIT_FAILURE);
}


i32 util_i32_vector_empty(vector *self) {
	return self->_size == 0;
}

i32 util_i32_vector_front(vector *const self) {
	assert(self->_size > 0);
	return self->_obj[0];
}

i32 util_i32_vector_back(vector *const self) {
	assert(self->_size > 0);
	return self->_obj[self->_size - 1];
}

void util_i32_vector_push_back(vector *const self, i32 item) {
	if (self->_size == self->_capacity) {
		if (self->_capacity == 0) {
			self->_capacity = 8;
		}
		i32 *const tmp = (i32 *) realloc(self->_obj, sizeof(i32) * self->_capacity * 2);
		if (tmp == NULL) {
			free(self->_obj);
			perror("unable to realloc in push_back");
			exit(EXIT_FAILURE);
		} else {
			self->_obj = tmp;
			self->_capacity *= 2;
		}
	}
	self->_obj[self->_size] = item;
	++self->_size;
}

void util_i32_vector_reserve(vector *const self, u32 new_cap) {
	if (new_cap > self->_capacity) {

	}
}

void util_i32_vector_swap(vector *const self, vector *const other) {
	i32 *const obj = self->_obj;
	self->_obj = other->_obj;
	other->_obj = obj;

	const u32 capacity = self->_capacity;
	self->_capacity = other->_capacity;
	other->_capacity = capacity;

	const u32 size = self->_size;
	self->_size = other->_size;
	other->_size = size;
}

void util_i32_vector_pop_back(util_i32_vector *const self) {
//	assert(self->_size);
//	se
}

void util_i32_vector_set(vector *const self, u32 index, i32 item) {
	assert(0 <= index && index < self->_size);
	self->_obj[index] = item;
}


void util_i32_vector_print(util_i32_vector *const self) {
	assert(self);
	if (util_i32_vector_empty(self)) {
		printf("[]");
	} else {
		printf("[");
		printf("%d", self->_obj[0]);
		for (u32 k = 1; k < self->_size; ++k) {
			printf(", %d", self->_obj[k]);
		}
		printf("]");
	}
}

#undef vector
#endif