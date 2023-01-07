#include <ctype.h>
#include "../Include/typeobject.h"
#include "token.h"
#include "../library/builtins.h"
#include "../Include/charobject.h"
#include "../Include/numberobject.h"
#include "../Include/booleanobject.h"

TypeObject Token_Type = {
		"Token",
		.tp_basicsize=sizeof(TokenObject),
		.tp_itemsize=sizeof(char),
		.tp_print=(print_proc) Token_Print,
		.tp_repr=(print_proc) Token_Print,
		.tp_dealloc = (dealloc_proc) Token_Dealloc
};

void TokenKind_Print(TokenKind kind, FILE *out) {
#define MAKE_CASE(k) case k: fprintf(out, #k); break
	switch (kind) {
		MAKE_CASE(NOT_AVAILABLE);
		MAKE_CASE(IDENTIFIER);
		MAKE_CASE(BOOLEAN);
		MAKE_CASE(NUMBER);
		MAKE_CASE(CHARACTER);
		MAKE_CASE(STRING);
		MAKE_CASE(L_PAREN);
		MAKE_CASE(R_PAREN);
		MAKE_CASE(SHARP_L_PAREN);
		MAKE_CASE(SINGLE_QUOTE);
		MAKE_CASE(BACK_QUOTE);
		MAKE_CASE(COMMA);
		MAKE_CASE(COMMA_AT);
		MAKE_CASE(DOT);
		MAKE_CASE(COMMENT);
		default:
			fprintf(stderr, "unknown TokenKind value: %d\n", kind);
			exit(EXIT_FAILURE);
	}
#undef MAKE_CASE
}

PosPtr consume(PosPtr current, size_t n) {
	while (n--) {
		const char c = current.lookahead[0];
		if (c) {
			if (c != '\n') {
				current.pos.column++;
			} else {
				current.pos.column = 1;
				current.pos.line++;
			}
		} else {
			break;
		}
		++current.lookahead;
	}
	return current;
}

PosPtr consume_until_delimiters(PosPtr current, size_t n) {
	while (current.lookahead[n]) {
		if (isspace(current.lookahead[n])) break;
		if (current.lookahead[n] == '(') break;
		if (current.lookahead[n] == ')') break;
		if (current.lookahead[n] == '"') break;
		if (current.lookahead[n] == ';') break;
		n++;
	}
	assert(n > 0);
	return consume(current, n - 1);
}

TokenObject *Token_New(PosPtr start, PosPtr end, TokenKind kind) {
	assert(start.lookahead <= end.lookahead);
	TokenObject *res = (TokenObject *) TypeGenericAlloc(&Token_Type, 0);
	res->kind = kind;
	res->start = start.pos;
	res->end = end.pos;
	res->s = String_FromCStrN(start.lookahead, end.lookahead - start.lookahead + 1);
	res->ob_val = NULL;
#ifdef FLAG_TRACK_ALL_OBJS
	res->ob_val_obj_index = -1;
#endif
	if (kind == BOOLEAN) {
		assert(SIZE(res->s) == 2);
		if (res->s->ob_sval[1] == 't') {
			res->ob_val = NewRef(OBJ_TRUE);
		} else {
			res->ob_val = NewRef(OBJ_FALSE);
		}
	} else if (kind == CHARACTER) {
		assert(SIZE(res->s) >= 3);
		res->ob_val = AS_OBJECT(Char_FromLiteral(res->s));
	} else if (kind == STRING) {
		res->ob_val = AS_OBJECT(String_FromLiteral(res->s));
	}
	return res;
}

// start->pos will be the position right after the end of the new token.
void Token_Dealloc(TokenObject *self) {
	DECREF(self->s);
	XDECREF(self->ob_val);
}

void Token_SetKind(TokenObject *self, TokenKind kind) {
	assert(self->kind == NOT_AVAILABLE);
	assert(kind != NOT_AVAILABLE);
	self->kind = kind;
}

void Token_SetVal(TokenObject *self, Object *val) {
	assert(self->ob_val == NULL);
	assert(self->kind == NUMBER || self->kind == CHARACTER);
	if (self->kind == NUMBER) {
		assert(IS_NUMBER(val));
	} else {
		assert(IS_TYPE(val, Char_Type));
	}
	self->ob_val = NewRef(val);
#ifdef FLAG_TRACK_ALL_OBJS
	self->ob_val_obj_index = val->obj_index;
#endif
}

void Token_Print(TokenObject *self, FILE *out) {
	fprintf(out, "<token ");
	fprintf(out, "start=\"%zu:%zu\", ", self->start.line, self->start.column);
	fprintf(out, "end=\"%zu:%zu\", ", self->end.line, self->end.column);
	fprintf(out, "len=\"%zu\", ", String_GetSize(self->s));
	fprintf(out, "kind=\"");
	TokenKind_Print(self->kind, out);
	fprintf(out, "\">");
	if (self->ob_val) {
		fprintf(out, "<str>");
		String_Repr(self->s, out);
		fprintf(out, "</str>");
		fprintf(out, "<data type=\"%s\">", TYPE(self->ob_val)->tp_name);
		PRINT(self->ob_val, out);
		fprintf(out, "</data>");
	} else {
		String_Repr(self->s, out);
	}
	fprintf(out, "</token>");
}