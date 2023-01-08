#include "../Include/arrayobject.h"

struct array_object {
	VarObject ob_base;
	size_t item_size;
	size_t capacity;
	int locked;
	char *items;
};

TypeObject Array_Type = {
		.tp_name="Array",
		.tp_basicsize=sizeof(ArrayObject),
		.tp_itemsize=0,
		.tp_dealloc=(dealloc_proc) Array_Dealloc,
//		.tp_flags=TPFLAGS_HAVE_GC,
};
#define item_at(array, index) ((array)->items + (array)->item_size * (index))

ArrayObject *Array_New(size_t size, size_t item_size) {
	assert(item_size > 0);
	ArrayObject *res = CAST(ArrayObject *, TypeGenericAlloc(&Array_Type, 0));
	SIZE(res) = (int) size;
	res->item_size = item_size;
	res->capacity = size + 1;
	res->locked = 0;
//	while (res->capacity < size) res->capacity <<= 1;
	res->items = calloc(res->capacity, item_size);
	assert(res->items);
	return res;
}

void Array_Dealloc(ArrayObject *self) {
	assert(!self->locked);
	free(self->items);
}

void Array_Clear(ArrayObject *self) {
	assert(!self->locked);
	memset(self->items, 0, self->item_size * SIZE(self));
	SIZE(self) = 0;
}

void Array_Lock(ArrayObject *self) {
	assert(!self->locked);
	self->locked = 1;
}

void Array_Unlock(ArrayObject *self) {
	assert(self->locked);
	self->locked = 0;
}

static void reserve(ArrayObject *self, size_t capacity) {
	assert(self->capacity >= 1);
	if (capacity > self->capacity) {
		self->items = reallocf(self->items, self->item_size * capacity);
		assert(self->items);
		memset(item_at(self, self->capacity), 0,
			   self->item_size * (capacity - self->capacity));
	}
}

void Array_Append(ArrayObject *self, void *item) {
	assert(!self->locked);
	while (SIZE(self) + 1 >= self->capacity) {
		reserve(self, self->capacity * 2);
		self->capacity *= 2;
	}
	memcpy(item_at(self, SIZE(self)), item, self->item_size);
	SIZE(self)++;
}

// return SIZE(self) if not found.
size_t Array_Find(ArrayObject *self, void *item, size_t start_index) {
	size_t i = start_index;
	while (i < SIZE(self)) {
		if (!memcmp(item_at(self, i), item, self->item_size))
			return i;
		++i;
	}
	return i;
}

size_t Array_Count(ArrayObject *self, void *item) {
	size_t ans = 0;
	for (size_t i = 0; i < SIZE(self); ++i) {
		if (!memcmp(item_at(self, i), item, self->item_size)) ans++;
	}
	return ans;
}

void Array_Remove(ArrayObject *self, size_t index) {
	assert(0 <= index && index < SIZE(self));
	assert(!self->locked);
	ArrayObject *check = Array_Copy(self);
	memmove(item_at(self, index), item_at(self, index + 1), self->item_size * (SIZE(self) - index - 1));
	memset(item_at(self, SIZE(self) - 1), 0, self->item_size);
	SIZE(self)--;

	assert(&self->items[0] == self->items);
	assert(!memcmp(&self->items[0], &check->items[0], index * self->item_size));
	assert(!memcmp(&self->items[index * self->item_size], &check->items[(index + 1) * self->item_size],
				   (SIZE(self) - index) * self->item_size));
	CLEAR(check);

}

void Array_SetItem(ArrayObject *self, size_t index, void *item) {
	assert(0 <= index && index < SIZE(self));
	memcpy(item_at(self, index), item, self->item_size);
}

void *Array_GetItem(ArrayObject *self, size_t index) {
	assert(0 <= index && index < SIZE(self));
	return item_at(self, index);
}

void *Array_GetRawPtr(ArrayObject *self) {
	return self->items;
}

ArrayObject *Array_Copy(ArrayObject *self) {
	ArrayObject *res = Array_New(SIZE(self), self->item_size);
	memcpy(res->items, self->items, self->item_size * SIZE(self));
	assert(SIZE(res) + 1 == res->capacity);
	memset(item_at(res, SIZE(res)), 0, res->item_size);
	return res;
}

#define SWAP(x, y) do{ typeof(x) tmp = (x); (x) = (y); (y) = tmp; }while(0)

void Array_Swap(ArrayObject *self, ArrayObject *other) {
	assert(self->item_size == other->item_size);
	assert(!self->locked);
	if (self == other) return;
	SWAP(SIZE(self), SIZE(other));
	SWAP(self->capacity, other->capacity);
	SWAP(self->items, other->items);
}

#undef SWAP
#undef item_at