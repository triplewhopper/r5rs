#include "../Include/chainmapobject.h"

TypeObject ChainMap_Type = {
		.tp_name="ChainMap",
		.tp_basicsize=sizeof(ChainMap),
		.tp_itemsize=sizeof(DictObject *),
		.tp_print=(print_proc) ChainMap_Print,
		.tp_repr=(print_proc) ChainMap_Print,
		.tp_dealloc=(dealloc_proc) ChainMap_Dealloc,
		.tp_flags = TPFLAGS_HAVE_GC
};

size_t ChainMap_GetSize(ChainMap *self) {
	return self->ob_base.ob_size;
}


// args must be a NULL-terminated array.
ChainMap *ChainMap_New(DictObject *args[]) {
	size_t n = 0;
	while (args[n] != NULL) n++;
	ChainMap *res = (ChainMap *) TypeGenericAlloc(&ChainMap_Type, n);
	memset(res->maps, 0, sizeof(DictObject *) * n);
	for (size_t i = 0; i < n; ++i) {
//		INCREF(args[i]);
		res->maps[i] = args[i];
	}
	return res;
}

ChainMap *ChainMap_NewChild(ChainMap *self, DictObject *m) {
	size_t n = SIZE(self);
	ChainMap *res = (ChainMap *) TypeGenericAlloc(&ChainMap_Type, n + 1);
	if (m == NULL)
		res->maps[0] = Dict_New();
	else
		res->maps[0] = NEW_REF(m);
	for (size_t i = 0; i < n; ++i) {
		res->maps[i + 1] = NEW_REF(self->maps[i]);
	}
	return res;
}

ChainMap *ChainMap_NewEmpty() {
	ChainMap *res = (ChainMap *) TypeGenericAlloc(&ChainMap_Type, 1);
	memset(res->maps, 0, sizeof(DictObject *) * 1);
	res->maps[0] = Dict_New();
	return res;

}

ChainMap *ChainMap_Parents(ChainMap *self) {
	size_t n = SIZE(self);
	if (n == 1) return ChainMap_NewEmpty();
	ChainMap *res = (ChainMap *) TypeGenericAlloc(&ChainMap_Type, n - 1);
	for (size_t i = 0; i + 1 < n; ++i) {
//		INCREF(self->maps[i + 1]);
		res->maps[i] = NEW_REF(self->maps[i + 1]);
	}
//	DECREF(self);
	return res;
}

void ChainMap_SetItem(ChainMap *self, SymbolObject *key, Object *value) {
	assert(SIZE(self));
	Dict_SetItem(self->maps[0], key, value);
}

void ChainMap_DeepSetItem(ChainMap *self, SymbolObject *key, Object *value) {
	assert(key);
	assert(value);
	size_t n = SIZE(self);
	for (size_t i = 0; i < n; ++i) {
		if (Dict_Contains(self->maps[i], key)) {
			Dict_SetItem(self->maps[i], key, value);
			return;
		}
	}
	Dict_SetItem(self->maps[0], key, value);

}

Object *ChainMap_GetItem(ChainMap *self, SymbolObject *key) {
	size_t n = SIZE(self);
	for (size_t i = 0; i < n; ++i) {
		Object *res = Dict_GetItem(self->maps[i], key);
		if (res != NULL) return res;
	}
	return NULL;
}

int ChainMap_Contains(ChainMap *self, SymbolObject *key) {
	assert(key);
	size_t n = SIZE(self);
	for (size_t i = 0; i < n; ++i) {
		if (Dict_Contains(self->maps[i], key)) return 1;
	}
	return 0;
}

void ChainMap_DelItem(ChainMap *self, SymbolObject *key) {
	assert(key);
	if (SIZE(self) > 0 && Dict_Contains(self->maps[0], key)) {
		Dict_DelItem(self->maps[0], key);
		return;
	}
	fprintf(stderr, "in function %s: key not found", __FUNCTION__);
	exit(EXIT_FAILURE);
}

void ChainMap_DeepDelItem(ChainMap *self, SymbolObject *key) {
	assert(key);
	size_t n = SIZE(self);
	for (size_t i = 0; i < n; ++i) {
		if (Dict_Contains(self->maps[i], key)) {
			Dict_DelItem(self->maps[i], key);
			return;
		}
	}
	fprintf(stderr, "in function %s: key not found", __FUNCTION__);
	exit(EXIT_FAILURE);

}

void ChainMap_Print(ChainMap *self, FILE *out) {
	fprintf(out, "ChainMap(");
	size_t len = SIZE(self);
	if (len > 0) {
		fputc('[', out);
		PRINT(self->maps[0], out);
		for (size_t i = 1; i < len; ++i) {
			fprintf(out, ", ");
			PRINT(self->maps[i], out);
		}
		fputc(']', out);
	}
	fputc(')', out);
}

void ChainMap_Dealloc(ChainMap *self) {
	gc_untrack(AS_OBJECT(self));
	size_t len = SIZE(self);
	for (size_t i = 0; i < len; ++i) {
		DECREF(self->maps[i]);
	}
}
