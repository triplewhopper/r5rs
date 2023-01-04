#include <ctype.h>
#include "../Include/typeobject.h"
#include "../Include/charobject.h"
#include "../Include/stringobject.h"

CompareMethods char_compare = {
		.cmp_eqv =(int_binaryfunc) Char_Eqv,
};

TypeObject Char_Type = {
		.tp_name="char",
		.tp_basicsize=sizeof(CharObject),
		.tp_itemsize=0,
		.tp_print=(print_proc) Char_Print,
		.tp_repr=(print_proc) Char_Print,
		.tp_cmp = &char_compare,
//		.tp_eval = Eval_Default
};
typedef struct {
	const char key;
	const char *value;
} escape_character_table_t;
static const escape_character_table_t table[] = {
		{0x00, "nul"},
		{0x07, "alarm"},
		{0x08, "backspace"},
		{0x09, "tab"},
		{0x0a, "newline"},
		{0x0b, "vtab"},
		{0x0c, "page"},
		{0x0d, "return"},
		{0x1b, "esc"},
		{0x20, "space"},
		{0x7f, "delete"},
		{0x00, NULL}
};

static void CStyleChar_Repr_Impl(int c, FILE *out);

CharObject *Char_New(int c) {
	CharObject *res = (CharObject *) Char_Type.tp_alloc(&Char_Type, 0);
	return res;
}

CharObject *Char_FromLiteral(StringObject *s) {
	assert(String_GetSize(s) >= 3);
	assert(strncmp(s->ob_sval, "#\\", 2) == 0);
	for (const escape_character_table_t *p = table; p->value; ++p) {
		if (strcmp(s->ob_sval + 2, p->value) == 0) {
			return Char_New(p->key);
		}
	}
	assert(0);
}

int Char_Ord(CharObject *self) {
	return self->ob_val;
}

void Char_Print(CharObject *self, FILE *out) {
	CStyleChar_Repr_Impl(self->ob_val, out);
}

static void CStyleChar_Repr_Impl(int c, FILE *out) {
	if (isprint(c) && c != ' ') {
		fprintf(out, "#\\%c", c);
	} else {
		for (const escape_character_table_t *p = table; p->value; ++p) {
			if (c == p->key) {
				fprintf(out, "#\\%s", p->value);
				break;
			}
		}
		char error_message[100] = "errmsg is now N/A";
		snprintf(error_message, 99, "Unknown character (ascii code=%d)", (int) c);
		perror(error_message);
		exit(EXIT_FAILURE);
	}
}

int Char_Eqv(CharObject *obj1, Object *obj2) {
	if (IS_TYPE(obj2, Char_Type))
		return obj1->ob_val == CAST(typeof(obj1), obj2)->ob_val;
	return 0;
}