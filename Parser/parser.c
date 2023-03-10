#include "parser.h"
#include "lexer.h"
#include "tokenobject.h"
#include "../Include/symbolobject.h"
#include "../Include/vectorobject.h"
#include "../Include/pairobject.h"
#include "../Include/typeobject.h"
#include "interpreter.h"
#include "builtins.h"

static Object *syntax_error(TokenObject *t) {
	fprintf(stderr, "syntax error: unexpected ");
	Token_Print(t, stderr);
	fprintf(stderr, "\n");
	return NULL;
}

static Object *not_implemented(TokenObject *t) {
	fprintf(stderr, "not implemented yet");
	fprintf(stderr, "\n");
	DECREF(t);
	return NULL;
}

Object *ParseDatum(LexerObject *lexer) {
	TokenObject *t = NULL;
	begin:
	t = Lexer_PeekCurrentToken(lexer);
	if (t == NULL) return NULL;
	switch (t->kind) {
		case NOT_AVAILABLE:
			return syntax_error(t);
		case IDENTIFIER: {
			SymbolObject *symbol = Symbol_FromString(t->s);
			Lexer_ConsumeToken(lexer, 1);
			return AS_OBJECT(symbol);
		}
		case BOOLEAN:
		case NUMBER:
		case CHARACTER:
		case STRING: {
			Lexer_ConsumeToken(lexer, 1);
			return NewRef(t->ob_val);
		}
		case L_PAREN:
			return ParseList(lexer);
		case R_PAREN:
			return syntax_error(t);
		case SHARP_L_PAREN:
			return ParseVector(lexer);
		case SINGLE_QUOTE:
			return ParseList(lexer);
		case BACK_QUOTE:
		case COMMA:
		case COMMA_AT:
			return not_implemented(t);
		case DOT:
			return syntax_error(t);
		case COMMENT:
			Lexer_ConsumeToken(lexer, 1);
			goto begin;
	}
}


Object *ParseQuote() {
	return NULL;
}

Object *ParseList(LexerObject *lexer) {
	TokenObject *t = Lexer_PeekCurrentToken(lexer);
	Lexer_ConsumeToken(lexer, 1);
	if (t == NULL) return syntax_error(t);
	if (t->kind == L_PAREN) {
		int has_dot = 0;
		int has_r_paren = 0;
		Object *objs[20] = {NULL};
		size_t n_objs = 0;

		while (!has_r_paren) {
			assert(n_objs < 20);
			t = Lexer_PeekCurrentToken(lexer);
			if (t == NULL) return syntax_error(t);
			switch (t->kind) {
				case NOT_AVAILABLE:
					return syntax_error(t);
				case IDENTIFIER:
				case BOOLEAN:
				case NUMBER:
				case CHARACTER:
				case STRING:
					objs[n_objs++] = ParseDatum(lexer);
					break;
				case L_PAREN:
					objs[n_objs++] = ParseList(lexer);
					break;
				case R_PAREN:
					has_r_paren = 1;
					Lexer_ConsumeToken(lexer, 1);
					break;
				case SHARP_L_PAREN:
					objs[n_objs++] = ParseVector(lexer);
					break;
				case SINGLE_QUOTE:
					objs[n_objs++] = ParseList(lexer);
					break;
				case BACK_QUOTE:
				case COMMA:
				case COMMA_AT:
					return not_implemented(t);
				case DOT:
					if (has_dot != 0) return syntax_error(t);
					else has_dot = 1;

					Lexer_ConsumeToken(lexer, 1);

					objs[n_objs++] = ParseDatum(lexer);
					t = Lexer_PeekCurrentToken(lexer);

					if (t == NULL || t->kind != R_PAREN)
						syntax_error(t);

					Lexer_ConsumeToken(lexer, 1);
					has_r_paren = 1;
					break;
				case COMMENT:
					Lexer_ConsumeToken(lexer, 1);
					break;
			}
		}
		Object *cdr = NULL;
		if (has_dot) {
			--n_objs;
			cdr = objs[n_objs];
			objs[n_objs] = NULL;
		} else {
			cdr = NewRef(EMPTY_LIST);
		}
		while (n_objs--) {
			Object *tmp = NULL;
			MOVE_SET(tmp, cdr, CONS(objs[n_objs], cdr));
			DECREF(objs[n_objs]);
			objs[n_objs] = NULL;
		}
		return cdr;
	} else if (t->kind == SINGLE_QUOTE) {
		Object *datum = ParseDatum(lexer);
		Object *cdr = CONS(datum, EMPTY_LIST);
		Object *res = CONS(global_symbols.quote, cdr);
		DECREF(datum), DECREF(cdr);
		return res;
	} else {
		return not_implemented(t);
	}
}

Object *ParseVector(LexerObject *lexer) {
	TokenObject *t = Lexer_PeekCurrentToken(lexer);
	assert(t);
	assert(t->kind == SHARP_L_PAREN);
	Lexer_ConsumeToken(lexer, 1);
	Object *objs[20] = {0};
	int has_r_paren = 0;
	size_t n_objs = 0;
	while (!has_r_paren) {
		assert(n_objs < 20);
		t = Lexer_PeekCurrentToken(lexer);
		if (t == NULL) return syntax_error(t);
		switch (t->kind) {
			case NOT_AVAILABLE:
				return syntax_error(t);
			case R_PAREN:
				has_r_paren = 1;
				Lexer_ConsumeToken(lexer, 1);
				break;
			case COMMENT:
				Lexer_ConsumeToken(lexer, 1);
				break;
			default:
				objs[n_objs++] = ParseDatum(lexer);
				break;
		}
	}
	VectorObject *v = Vector_New(n_objs);
	for (size_t i = 0; i < n_objs; ++i)
		Vector_SetItem(v, i, objs[i]);
	return AS_OBJECT(v);
}
