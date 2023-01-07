#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "../Include/typeobject.h"
#include "../Include/stringobject.h"

CompareMethods string_compare = {
		.cmp_eqv = (int_binaryfunc) String_Eqv,
};

TypeObject String_Type = {
		"String",
		.tp_basicsize=sizeof(StringObject),
		.tp_itemsize=sizeof(char),
		.tp_print=(print_proc) String_Print,
		.tp_repr=(print_proc) String_Repr,
//		.tp_dealloc=(dealloc_proc) String_Dealloc
		.tp_cmp=&string_compare,
};

typedef struct {
	const char key;
	const char *value;
	const size_t len;
} escape_character_table_t;
static const escape_character_table_t table[] = {
		{'\a', "\\a",  2},
		{'\b', "\\b",  2},
		{'\f', "\\f",  2},
		{'\n', "\\n",  2},
		{'\r', "\\r",  2},
		{'\t', "\\t",  2},
		{'\v', "\\v",  2},
		{'\\', "\\\\", 2},
		{'\'', "\\\'", 2},
		{'\"', "\\\"", 2},
		{0, NULL,      0}
};

// (...) -> str
static StringObject *String_Alloc(size_t size) {
	return (StringObject *) TypeGenericAlloc(&String_Type, size);
}

static shash_t calculate_hash(const char *s, size_t len) {
	u64 x = len;
	if (len) {
		while (len--) {
			x = x * 8 % 0x1fffffffffffffffull;
			x = x * 8 % 0x1fffffffffffffffull;
			x = x * 4 % 0x1fffffffffffffffull;
			x = (x + (u64) *s) % 0x1fffffffffffffffull;
			s++;
		}
	}
	return (shash_t) x;
}

// a new string s[0..length-1], the length is guaranteed to be length, but strlen may return smaller than length.
StringObject *String_FromCStrN(const char *s, size_t length) {
	StringObject *t = AS_STRING(TypeGenericAlloc(&String_Type, length + 1));
	memcpy(t->ob_sval, s, length);
	t->ob_sval[length] = '\0';
	SIZE(t) = (int) length;
	t->ob_shash = calculate_hash(s, length);
	return t;
}

// (...) -> str
StringObject *String_FromCStr(const char *s) {
	return String_FromCStrN(s, strlen(s));
}

StringObject *String_FromLiteral(StringObject *literal) {
	size_t i = 0, j = 1, len = SIZE(literal);
	assert(len >= 2);
	assert(literal->ob_sval[0] == '"');
	assert(literal->ob_sval[len - 1] == '"');
	char *s = calloc(sizeof(char), 1 + len);
	assert(s != NULL);
	while (j < len - 1) {
		if (literal->ob_sval[j] != '\\') {
			s[i++] = literal->ob_sval[j++];
		} else {
			int flag = 0;
			for (const escape_character_table_t *p = table; p->value; ++p) {
				if (strncmp(literal->ob_sval + j, p->value, p->len) == 0) {
					s[i++] = p->key;
					j += p->len;
					flag = 1;
					break;
				}
			}
			assert(flag);
		}
	}
	StringObject *res = String_FromCStrN(s, j - 1);
	free(s);
	return res;
}

StringObject *String_VFormat(const char *format, va_list ap) {
	char *s = NULL;
	if (vasprintf(&s, format, ap) == -1) {
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	assert(s);
	StringObject *res = String_FromCStr(s);
	free(s);
	return res;
}

StringObject *String_Format(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	StringObject *res = String_VFormat(format, ap);
	va_end(ap);
	return res;
}

// (self: &str) -> ...
size_t String_GetSize(StringObject *self) {
	return self->ob_base.ob_size;
}

// (lhs: &str, rhs: &str) -> ...
int String_Eqv(StringObject *lhs, Object *rhs) {
	if (IS_TYPE(rhs, String_Type)) {
		if (String_Hash(lhs) != String_Hash(AS_STRING(rhs))) {
			return 0;
		}
		if (SIZE(lhs) != SIZE(rhs)) {
			return 0;
		}
		for (size_t i = 0, len = SIZE(lhs); i < len; ++i) {
			if (lhs->ob_sval[i] != AS_STRING(rhs)->ob_sval[i]) return 0;
		}
		return 1;
	}
	return 0;
}

int String_EqualCStyleStr(StringObject *lhs, const char *rhs) {
	size_t len_rhs = strlen(rhs);
	if (String_GetSize(lhs) != len_rhs) return 0;
	for (size_t i = 0; i < len_rhs; ++i) {
		if (lhs->ob_sval[i] != rhs[i]) return 0;
	}
	return 1;
}

// (self: &str) -> ...
shash_t String_Hash(StringObject *self) {
	return self->ob_shash;
}

static void print_escaped(int c, FILE *out) {
	for (const escape_character_table_t *p = table; p->value; ++p) {
		if (c == p->key) {
			fprintf(out, "%s", p->value);
			return;
		}
	}
	fprintf(out, "\\x%02x", c);
}

void String_Print(StringObject *self, FILE *out) {
	for (size_t i = 0, size = String_GetSize(self); i < size; ++i) {
		fputc(self->ob_sval[i], out);
	}
}

void String_Repr(StringObject *self, FILE *out) {
	fputc('\"', out);
	for (size_t i = 0, size = SIZE(self); i < size; ++i) {
		int c = (int) self->ob_sval[i];
		if (isprint(c) && c != '\\' && c != '\'' && c != '\"') {
			fputc(self->ob_sval[i], out);
		} else {
			print_escaped(self->ob_sval[i], out);
		}
	}
	fputc('\"', out);

}

//// (self: &str) -> ...
//void String_Dealloc(StringObject *self) {
//	free(self);
//}

// (self: &str, other: &str) -> str
StringObject *String_Add(StringObject *self, StringObject *other) {
	size_t s1 = SIZE(self);
	size_t s2 = SIZE(other);
	if (s1 == 0) {
		return NEW_REF(other);
	} else if (s2 == 0) {
		return NEW_REF(self);
	}

	StringObject *res = AS_STRING(TYPE(self)->tp_alloc(TYPE(self), 1 + s1 + s2));
	strcpy(res->ob_sval, self->ob_sval);
	strcpy(res->ob_sval + s1, other->ob_sval);
	return res;
}

char String_CharAt(StringObject *self, size_t index) {
	if (0 <= index && index < SIZE(self)) {
		return self->ob_sval[index];
	}
	fprintf(stderr, "IndexError: index out of bound\n");
	exit(EXIT_FAILURE);
}

int String_StartswithCStr(StringObject *lhs, const char *rhs) {
	return strncmp(lhs->ob_sval, rhs, strlen(rhs));
}

int String_EndswithCStr(StringObject *lhs, const char *rhs) {
	size_t len_lhs = String_GetSize(lhs);
	size_t len_rhs = strlen(rhs);
	if (len_lhs < len_rhs) return 0;
	return strncmp(lhs->ob_sval + len_lhs - len_rhs, rhs, len_rhs);
}

int String_Startswith(StringObject *lhs, StringObject *rhs) {
	return memcmp(lhs->ob_sval, rhs->ob_sval, String_GetSize(rhs) * String_Type.tp_itemsize);
}

int String_Endswith(StringObject *lhs, StringObject *rhs) {
	size_t len_lhs = String_GetSize(lhs);
	size_t len_rhs = String_GetSize(rhs);
	if (len_lhs < len_rhs) return 0;
	return memcmp(lhs->ob_sval + len_lhs - len_rhs, rhs->ob_sval, len_rhs * String_Type.tp_itemsize);
};


