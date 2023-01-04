#include "lexer.h"
#include "../library/string2number.h"

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

static void *resize(void *s, size_t *capacity, size_t size, size_t nitems) {
	assert(*capacity >= 1);
	while (nitems > *capacity) {
		void *tmp = realloc(s, size * (*capacity *= 2));
		if (tmp != NULL) {
			s = tmp;
		} else {
			free(s);
			perror("no memory");
			exit(EXIT_FAILURE);
		}
	}
	return s;
}

// returns new ref
static StringObject *read(FILE *f) {
#define buf_size 40
	static char buf[buf_size];
	size_t size = 0;
	size_t capacity = 1;
	char *s = malloc(capacity * sizeof(char));
	memset(s, 0, sizeof(char) * capacity);
	while (!feof(f)) {
		size_t n_bytes = fread(buf, sizeof(char), buf_size / sizeof(char), f) * sizeof(char);
		s = resize(s, &capacity, sizeof(char), size + n_bytes);
		memcpy(s + size, buf, n_bytes);
		size += n_bytes;
	}
	StringObject *res = String_FromCStrN(s, size);
	free(s);
	return res;
#undef buf_size
}

#define MAXN_TOKEN_BUF 100000
struct lexer_object {
	FILE *f;
	StringObject *s;
	PosPtr current;
	size_t size;
	size_t capacity;
	size_t cur;
	TokenObject **token_buffer;
	PosPtr *pos_ptr;
};

LexerObject *Lexer_FromFile(const char *filename) {
	LexerObject *lexer = calloc(1, sizeof(LexerObject));
	assert(lexer);
	lexer->f = fopen(filename, "r");
	assert(lexer->f);
	lexer->s = read(lexer->f);
//	String_Repr(lexer->s, stdout);
//	printf("\nlen(s)=%zu\n", String_GetSize(lexer->s));
	lexer->current = (PosPtr) {1, 1, lexer->s->ob_sval};
	lexer->size = 0;
	lexer->capacity = 1;
	lexer->cur = 0;
	lexer->token_buffer = calloc(lexer->capacity, sizeof(TokenObject *));
	lexer->pos_ptr = calloc(lexer->capacity, sizeof(PosPtr));
	assert(lexer->token_buffer);
	assert(lexer->pos_ptr);
	return lexer;
}

LexerObject *Lexer_FromCStr(const char *s) {
	LexerObject *lexer = calloc(1, sizeof(LexerObject));
	assert(lexer);
	lexer->f = NULL;
	lexer->s = String_FromCStr(s);
//	String_Repr(lexer->s, stdout);
//	printf("\nlen(s)=%zu\n", String_GetSize(lexer->s));
	lexer->current = (PosPtr) {1, 1, lexer->s->ob_sval};
	lexer->size = 0;
	lexer->capacity = 1;
	lexer->cur = 0;
	lexer->token_buffer = calloc(lexer->capacity, sizeof(TokenObject *));
	lexer->pos_ptr = calloc(lexer->capacity, sizeof(PosPtr));
	assert(lexer->token_buffer);
	assert(lexer->pos_ptr);
	return lexer;
}

void Lexer_Finalize(LexerObject *lexer) {
	if (lexer->f != NULL) {
		fclose(lexer->f);
	}
	while (lexer->size--) {
		DECREF(lexer->token_buffer[lexer->size]);
	}
	XDECREF(lexer->s);
	free(lexer->token_buffer);
	free(lexer->pos_ptr);
	free(lexer);
}

int Lexer_GetNextToken(LexerObject *lexer, TokenObject **next) {
	assert(next != NULL && *next == NULL);
	if (lexer->cur + 1 < lexer->size) {
		*next = NEW_REF(lexer->token_buffer[lexer->cur + 1]);
		lexer->current = lexer->pos_ptr[lexer->cur + 1];
		lexer->cur++;
		return 0;
	} else {
		if (lexer->size == MAXN_TOKEN_BUF) {
			fprintf(stderr, "token buffer is full\n");
			exit(EXIT_FAILURE);
		}
		if (lexer->size + 1 > lexer->capacity) {
			size_t capacity = lexer->capacity;
			lexer->token_buffer = resize(lexer->token_buffer, &capacity, sizeof(TokenObject *), lexer->size + 1);
			capacity = lexer->capacity;
			lexer->pos_ptr = resize(lexer->pos_ptr, &capacity, sizeof(PosPtr), lexer->size + 1);
			lexer->capacity = capacity;
		}
		int res = get_next_token(lexer->current, next);
		if (res != -1) {
			lexer->token_buffer[lexer->size] = NEW_REF(*next);
			while (lexer->current.pos.column != (*next)->end.column || lexer->current.pos.line != (*next)->end.line)
				lexer->current = consume(lexer->current, 1);
			lexer->current = consume(lexer->current, 1);
			lexer->pos_ptr[lexer->size] = lexer->current;
			lexer->cur++;
			lexer->size++;
		}
		return res;
	}
}
// borrowed ref.
TokenObject *Lexer_PeekForwardTokenBy(LexerObject *lexer, size_t n_lookahead) {
	const size_t cur = lexer->cur;
	while (lexer->size <= cur + n_lookahead) {
		TokenObject *t = NULL;
		int v = Lexer_GetNextToken(lexer, &t);
		if (v != 0) {
			fprintf(stderr, "no more tokens.\n");
			return NULL;
		}
		DECREF(t);
	}
	lexer->cur = cur;
	lexer->current = lexer->pos_ptr[cur];
	TokenObject *res = lexer->token_buffer[lexer->cur + n_lookahead];
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

size_t Lexer_GetNTokens(LexerObject *lexer) {
	return lexer->size;
}

#undef MAXN_TOKEN_BUF
#undef set_token_and_return_0
#undef LA
