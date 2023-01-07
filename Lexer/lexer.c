#include "lexer.h"
#include "../library/string2number.h"
#include "../Include/arrayobject.h"

#define LA(k) (current.lookahead[k])

static TokenObject *match_string_literal(PosPtr current) {
	assert(LA(0) == '"');
	int n = 1;
	while (LA(n)) { // loop invariants: n>=1
		if (LA(n) == '\"') {
			int n_backslash = 0;
			while (n - 1 - n_backslash > 0 && LA(n - 1 - n_backslash) == '\\') {
				n_backslash++;
			}
			if (n_backslash % 2 == 0) {
				return Token_New(current, consume(current, n), STRING);
			}
		}
		n++;
	}
	fprintf(stderr, "%zu:%zu: string literal has no terminating '\"'",
			current.pos.line,
			current.pos.column);
	exit(EXIT_FAILURE);
}

// this function returns a PosPtr corresponding to the end of the # object matched.
static TokenObject *match_sharp_object(PosPtr current) {
	assert(LA(0) == '#');
	switch (LA(1)) {
		case '(': // #(
			return Token_New(current, consume(current, 1), SHARP_L_PAREN);
		case 't': // #t
		case 'f': // #f
			return Token_New(current, consume(current, 1), BOOLEAN);
		case '\\': {
			if (isalpha(LA(2))) { // e.g. #\a, #\b, #\space, #\newline, #\nul
				PosPtr end = consume_until_delimiters(current, 2);
				return Token_New(current, end, CHARACTER);
			} else {
				// e.g. #\;, #\), #\>, etc.
				return Token_New(current, consume(current, 2), CHARACTER);
			}
		}
		default: {
			PosPtr end = consume_until_delimiters(current, 0);
			TokenObject *t = Token_New(current, end, NUMBER);
			Object *val = String_ToNumber(t->s);
			if (!IS_FALSE(val)) {
				Token_SetVal(t, val);
				DECREF(val);
				return t;
			} else {
				DECREF(t);
				DECREF(val);
				fprintf(stderr, "unknown sharp object '");
				String_Print(t->s, stderr);
				fprintf(stderr, "'\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// returns 0 on success, -1 on failure.
// on success, *token is a valid TokenObject
// otherwise, *token is set to NULL
static int get_next_token(PosPtr current, TokenObject **token) {
#define set_token_and_return_0(t) *token=(t); return 0
	while (LA(0)) {
		if (isspace(LA(0))) {
			current = consume(current, 1);
		} else if (LA(0) == '(') {
			set_token_and_return_0(Token_New(current, current, L_PAREN));
		} else if (LA(0) == ')') {
			set_token_and_return_0(Token_New(current, current, R_PAREN));
		} else if (LA(0) == '#') {
			set_token_and_return_0(match_sharp_object(current));
		} else if (LA(0) == '\'') {
			set_token_and_return_0(Token_New(current, current, SINGLE_QUOTE));
		} else if (LA(0) == '`') {
			set_token_and_return_0(Token_New(current, current, BACK_QUOTE));
		} else if (LA(0) == ',') {
			if (LA(1) == '@') {
				set_token_and_return_0(Token_New(current, consume(current, 1), COMMA_AT));
			} else {
				set_token_and_return_0(Token_New(current, current, COMMA));
			}
		} else if (LA(0) == '"') {
			set_token_and_return_0(match_string_literal(current));
		} else if (LA(0) == ';') {
			int n = 1;
			while (LA(n) && LA(n) != '\n') {
				n++;
			}
			set_token_and_return_0(Token_New(current, consume(current, n - 1), COMMENT));
		} else {
			PosPtr end = consume_until_delimiters(current, 0);
			TokenObject *t = Token_New(current, end, NOT_AVAILABLE);
			Object *val = String_ToNumber(t->s);
			if (!IS_FALSE(val)) {
				Token_SetKind(t, NUMBER);
				Token_SetVal(t, val);
			} else {
				if (String_EqualCStyleStr(t->s, ".")) {
					Token_SetKind(t, DOT);
				} else {
					Token_SetKind(t, IDENTIFIER);
				}
			}
			DECREF(val);
			set_token_and_return_0(t);
		}
	}
	*token = NULL;
	return -1;
}

// returns new ref
StringObject *read_all(FILE *f) {
	assert(f != NULL);
	size_t size = 0;
	char *s = NULL;
	FILE *buf = open_memstream(&s, &size);
	assert(buf != NULL);
	for (int c = fgetc(f); c != EOF; c = fgetc(f)) {
		fputc(c, buf);
	}
	fclose(buf);
	StringObject *res = String_FromCStrN(s, size);
	free(s);
	return res;
}

#define MAXN_TOKEN_BUF 100000
struct lexer_object {
	VarObject ob_base;
	FILE *f;
	StringObject *s;
	PosPtr current;
	size_t cur;
	ArrayObject *token_buffer;
	ArrayObject *pos_ptr;
};

TypeObject Lexer_Type = {
		.tp_name="Lexer",
		.tp_basicsize=sizeof(LexerObject),
		.tp_itemsize=0,
		.tp_dealloc=(dealloc_proc) Lexer_Dealloc,
};

LexerObject *Lexer_FromFile(const char *filename) {
	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
	assert(lexer);
	SIZE(lexer) = 0;
	lexer->f = fopen(filename, "r");
	assert(lexer->f);
	lexer->s = read_all(lexer->f);
	lexer->current = (PosPtr) {{1, 1}, lexer->s->ob_sval};
	lexer->cur = 0;
	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
	return lexer;
}

LexerObject *Lexer_FromCStr(const char *s) {
	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
	assert(lexer);
	SIZE(lexer) = 0;
	lexer->f = NULL;
	lexer->s = String_FromCStr(s);
	lexer->current = (PosPtr) {{1, 1}, lexer->s->ob_sval};
	lexer->cur = 0;
	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
	return lexer;
}
LexerObject *Lexer_FromString(StringObject *s){
	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
	assert(lexer);
	SIZE(lexer) = 0;
	lexer->f = NULL;
	lexer->s = NEW_REF(s);
	lexer->current = (PosPtr) {{1, 1}, lexer->s->ob_sval};
	lexer->cur = 0;
	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
	return lexer;

}
#define TokenAt(lexer, index) Array_At(TokenObject *, (lexer)->token_buffer, (index))

void Lexer_Extend(LexerObject *lexer, const char *s){
	StringObject *more = String_FromCStr(s);
	StringObject *tmp;
	MOVE_SET(tmp, lexer->s, String_Add(lexer->s, more));
	DECREF(more);
}
void Lexer_Dealloc(LexerObject *lexer) {
	if (lexer->f != NULL) {
		fclose(lexer->f);
	}
	for (size_t i = 0; i < SIZE(lexer); ++i) {
		TokenObject *token = TokenAt(lexer, i);
		CLEAR(token);
	}
	XDECREF(lexer->s);
	DECREF(lexer->token_buffer);
	DECREF(lexer->pos_ptr);
}

int Lexer_GetNextToken(LexerObject *lexer, TokenObject **next) {
	assert(next != NULL && *next == NULL);
	if (lexer->cur + 1 < SIZE(lexer->token_buffer)) {
		*next = NEW_REF(TokenAt(lexer, lexer->cur + 1));
		lexer->current = Array_At(PosPtr, lexer->pos_ptr, lexer->cur + 1);
		lexer->cur++;
		return 0;
	} else {
		if (SIZE(lexer) == MAXN_TOKEN_BUF) {
			fprintf(stderr, "token buffer is full\n");
			exit(EXIT_FAILURE);
		}
		int res = get_next_token(lexer->current, next);
		if (res != -1) {
			Array_Append(lexer->token_buffer, (TokenObject *[]) {NEW_REF(*next)});
			while (lexer->current.pos.column != (*next)->end.column || lexer->current.pos.line != (*next)->end.line)
				lexer->current = consume(lexer->current, 1);
			lexer->current = consume(lexer->current, 1);
			Array_Append(lexer->pos_ptr, &lexer->current);
			lexer->cur++;
			SIZE(lexer)++;
			assert(SIZE(lexer) == SIZE(lexer->token_buffer));
			assert(SIZE(lexer) == SIZE(lexer->pos_ptr));
		}
		return res;
	}
}

// borrowed ref.
TokenObject *Lexer_PeekForwardTokenBy(LexerObject *lexer, size_t n_lookahead) {
	const size_t cur = lexer->cur;
	while (SIZE(lexer) <= cur + n_lookahead) {
		TokenObject *t = NULL;
		int v = Lexer_GetNextToken(lexer, &t);
		if (v != 0) {
//			fprintf(stderr, "no more tokens.\n");
			lexer->cur = cur;
			return NULL;
		}
		DECREF(t);
	}
	lexer->cur = cur;
	lexer->current = Array_At(PosPtr, lexer->pos_ptr, cur);
	TokenObject *res = TokenAt(lexer, lexer->cur + n_lookahead);
//	INCREF(res);
	return res;
}

TokenObject *Lexer_PeekNextToken(LexerObject *lexer) {
	return Lexer_PeekForwardTokenBy(lexer, 1);
}

// return borrowed ref.
TokenObject *Lexer_PeekCurrentToken(LexerObject *lexer) {
	return Lexer_PeekForwardTokenBy(lexer, 0);
}

void Lexer_ConsumeToken(LexerObject *lexer, size_t n) {
	lexer->cur += n;
}

#undef TokenAt
#undef MAXN_TOKEN_BUF
#undef set_token_and_return_0
#undef LA
