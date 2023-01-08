#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "string2number.h"
#include "arrayobject.h"
#include "tokenobject.h"
#include "lexer.h"


enum LexerErrorCode {
	LEXER_ERR_UNEXPECTED_EOF = -1,
	LEXER_ERR_UNREACHABLE = -2,
	LEXER_ERR_NO_MORE_TOKENS = -3,
};

//int my_fgetc(FILE *f) {
//	static char *line;
//
//	int res = fgetc(f);
//	fprintf(stderr, "fgetc=%c (%d), ftell=%ld\n", (char) res, res, ftell(f));
//	return res;
//}
//
//#define fgetc(f) my_fgetc(f)
//PosPtr consume(PosPtr current, size_t n) {
//	while (n--) {
//		const char c = current.lookahead[0];
//		if (c) {
//			if (c != '\n') {
//				current.pos.column++;
//			} else {
//				current.pos.column = 1;
//				current.pos.line++;
//			}
//		} else {
//			break;
//		}
//		++current.lookahead;
//	}
//	return current;
//}

//PosPtr consume_until_delimiters(PosPtr current, size_t n) {
//	while (current.lookahead[n]) {
//		if (isspace(current.lookahead[n])) break;
//		if (current.lookahead[n] == '(') break;
//		if (current.lookahead[n] == ')') break;
//		if (current.lookahead[n] == '"') break;
//		if (current.lookahead[n] == ';') break;
//		n++;
//	}
//	assert(n > 0);
//	return consume(current, n - 1);
//}

#define update(c, loc) do{ if((c)=='\n') {(loc)->line++, (loc)->column=0;} else (loc)->column++; }while(0)

size_t consume_until_delimiters(FILE *f, FILE *scanned) {
	int n = 0, c = 0;
	while ((c = fgetc(f)) != EOF) {
		if (isspace(c) || c == '(' || c == ')' || c == '"' || c == ';') {
			ungetc(c, f);
			break;
		}
		fputc(c, scanned);
		n++;
	}
	return n;
}

static TokenObject *match_string_literal(FILE *lookahead, FILE *scanned) {
	int n = 2;
	int c = EOF;
	int n_backslashes = 0;
	while ((c = fgetc(lookahead)) != EOF) { // loop invariants: n>=1
		fputc(c, scanned);
		switch (c) {
			case '\\':
				n_backslashes++;
				break;
			case '\"':
				if (n_backslashes % 2 == 0) {
					return Token_New(lookahead, n, STRING);
				}
				break;
			default:
				n_backslashes = 0;
		}
		n++;
	}
	return NULL;
//	fprintf(stderr, "%zu:%zu: string literal has no terminating '\"'",
//			current.pos.line,
//			current.pos.column);
//	exit(EXIT_FAILURE);
}

// this function returns a PosPtr corresponding to the end of the # object matched.
static TokenObject *match_sharp_object(FILE *lookahead, FILE *scanned) {
	int c = fgetc(lookahead);
	if (c == EOF) {
		return NULL;
	}
	fputc(c, scanned);
	switch (c) {
		case '(': // #(
			return Token_New(lookahead, 2, SHARP_L_PAREN);
		case 't': // #t
		case 'f': // #f
			return Token_New(lookahead, 2, BOOLEAN);
		case '\\': {
			c = fgetc(lookahead);
			if (c == EOF) {
				return NULL;
			}
			fputc(c, scanned);
			if (isalpha(c)) { // e.g. #\a, #\b, #\space, #\newline, #\nul
				size_t len = consume_until_delimiters(lookahead, scanned);
				return Token_New(lookahead, 3 + len, CHARACTER);
			} else {
				// e.g. #\;, #\), #\>, etc.
				return Token_New(lookahead, 3, CHARACTER);
			}
		}
		default: {
			size_t len = consume_until_delimiters(lookahead, scanned);
			if (len == 0) {
				return NULL;
			}
			TokenObject *t = Token_New(lookahead, len, NUMBER);
			Object *val = String_ToNumber(t->s);
			if (val != NULL) {
				Token_SetVal(t, val);
				DECREF(val);
				return t;
			} else {
				DECREF(t);
				fprintf(stderr, "unknown sharp object '");
				String_Print(t->s, stderr);
				fprintf(stderr, "'\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// returns 0 on success, corresponding error code on failure.
// on success, *token is a valid TokenObject
// otherwise, *token is set to NULL
static int get_next_token(FILE *lookahead, FILE *scanned, TokenObject **token, int is_interactive) {
#define set_token_and_return(t, ret) *token=(t); return ret
	int c = 0;
	while ((c = fgetc(lookahead)) != EOF) {
		fputc(c, scanned);
		if(is_interactive && c == '\n') break;
		if (!isspace(c)) break;
	}

	if (c == '(') {
		set_token_and_return(Token_New(lookahead, 1, L_PAREN), 0);
	} else if (c == ')') {
		set_token_and_return(Token_New(lookahead, 1, R_PAREN), 0);
	} else if (c == '#') {
		TokenObject *t = match_sharp_object(lookahead, scanned);
		if (t == NULL) {
			set_token_and_return(NULL, LEXER_ERR_UNEXPECTED_EOF);
		}
		set_token_and_return(t, 0);
	} else if (c == '\'') {
		set_token_and_return(Token_New(lookahead, 1, SINGLE_QUOTE), 0);
	} else if (c == '`') {
		set_token_and_return(Token_New(lookahead, 1, BACK_QUOTE), 0);
	} else if (c == ',') {
		c = fgetc(lookahead);
		if (c == '@') {
			fputc(c, scanned);
			set_token_and_return(Token_New(lookahead, 2, COMMA_AT), 0);
		} else {
			ungetc(c, lookahead);
			set_token_and_return(Token_New(lookahead, 1, COMMA), 0);
		}
	} else if (c == '"') {
		TokenObject *t = match_string_literal(lookahead, scanned);
		if (t == NULL) {
			set_token_and_return(NULL, LEXER_ERR_UNEXPECTED_EOF);
		}
		set_token_and_return(t, 0);
	} else if (c == ';') {
		int n = 1;
		while ((c = fgetc(lookahead)) != EOF) {
			fputc(c, scanned);
			n++;
			if (c == '\n') break;
		}
		set_token_and_return(Token_New(lookahead, n, COMMENT), 0);
	} else {
		if (c == EOF || c == '\0') {
			set_token_and_return(NULL, LEXER_ERR_NO_MORE_TOKENS);
		}
		size_t len = consume_until_delimiters(lookahead, scanned);
		TokenObject *t = Token_New(lookahead, 1 + len, NOT_AVAILABLE);
		Object *val = String_ToNumber(t->s);
		if (val != NULL) {
			Token_SetKind(t, NUMBER);
			Token_SetVal(t, val);
			DECREF(val);
		} else {
			if (String_EqualCStyleStr(t->s, ".")) {
				Token_SetKind(t, DOT);
			} else {
				Token_SetKind(t, IDENTIFIER);
			}
		}
		set_token_and_return(t, 0);
	}

	set_token_and_return(NULL, LEXER_ERR_UNREACHABLE);
}

//// returns new ref
//StringObject *read_all(FILE *f) {
//	assert(f != NULL);
//	size_t size = 0;
//	char *s = NULL;
//	FILE *buf = open_memstream(&s, &size);
//	assert(buf != NULL);
//	for (int c = fgetc(f); c != EOF; c = fgetc(f)) {
//		fputc(c, buf);
//	}
//	fclose(buf);
//	StringObject *res = String_FromCStrN(s, size);
//	free(s);
//	return res;
//}

#define MAXN_TOKEN_BUF 100000
struct lexer_object {
	VarObject ob_base;
	FILE *lookahead;
//	ArrayObject *s;
	SourceLocation last_loc;
	size_t cur;
	ArrayObject *token_buffer;
//	ArrayObject *pos_ptr;
};

TypeObject Lexer_Type = {
		.tp_name="Lexer",
		.tp_basicsize=sizeof(LexerObject),
		.tp_itemsize=0,
		.tp_dealloc=(dealloc_proc) Lexer_Dealloc,
};

//LexerObject *Lexer_FromFile(const char *filename) {
//	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
//	assert(lexer);
//	SIZE(lexer) = 0;
//	lexer->f = fopen(filename, "r");
//	assert(lexer->f);
//	lexer->s = Array_New(0, sizeof(char));
//	lexer->last_loc = (PosPtr) {{1, 1}};
//	lexer->cur = 0;
//	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
//	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
//	return lexer;
//}

//static char lookahead(LexerObject *lexer, PosPtr current, int n) {
//	while (current.index + n >= SIZE(lexer->s)) {
//		int c = fgetc(lexer->f);
//		if (c != EOF) {
//			Array_Append(lexer->s, (char[]) {(char) c});
//		} else {
//			return EOF;
//		}
//	}
//	return Array_At(char, lexer->s, current.index + n);
//}

LexerObject *Lexer_FromStream(FILE *f, const char *name) {
	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
	assert(lexer);
	SIZE(lexer) = 0;
//	lexer->f = f;
	lexer->lookahead = f;
	assert(lexer->lookahead);
//	lexer->s = Array_New(0, sizeof(char));
	lexer->last_loc = (SourceLocation) {1, 0};
	lexer->cur = 0;
	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
//	size_t n = 0;
//	while (Lexer_PeekForwardTokenBy(lexer, n)) n++;
//	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
	return lexer;
}

//LexerObject *Lexer_FromCStr(const char *s) {
//	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
//	assert(lexer);
//	SIZE(lexer) = 0;
//	lexer->lookahead = fmemopen(s, strlen(s), "r");
////	lexer->f = fmemopen(s, strlen(s), "r");
////	lexer->s = Array_New(0, sizeof(char));
////	for (size_t i = 0; i < SIZE(lexer->s); ++i) {
////		Array_SetItem(lexer->s, i, s + i);
////	}
//	lexer->last_loc = (SourceLocation) {1, 0};
//	lexer->cur = 0;
//	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
////	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
//	return lexer;
//}

//LexerObject *Lexer_FromString(StringObject *s) {
//	LexerObject *lexer = CAST(LexerObject*, TypeGenericAlloc(&Lexer_Type, 0));
//	assert(lexer);
//	SIZE(lexer) = 0;
//	lexer->f = NULL;
//	lexer->s = Array_New(0, sizeof(char));
//	for (size_t i = 0; i < SIZE(lexer->s); ++i) {
//		Array_SetItem(lexer->s, i, s->ob_sval + i);
//	}
//	lexer->last_loc = (PosPtr) {{1, 1}};
//	lexer->cur = 0;
//	lexer->token_buffer = Array_New(0, sizeof(TokenObject *));
//	lexer->pos_ptr = Array_New(0, sizeof(PosPtr));
//	return lexer;
//
//}

#define TokenAt(lexer, index) Array_At(TokenObject *, (lexer)->token_buffer, (index))

//void Lexer_Extend(LexerObject *lexer, const char *s) {
//	StringObject *more = String_FromCStr(s);
//	StringObject *tmp;
//	MOVE_SET(tmp, lexer->s, String_Add(lexer->s, more));
//	DECREF(more);
//}

void Lexer_Dealloc(LexerObject *lexer) {
//	if (lexer->f != NULL) {
//		fclose(lexer->f);
//	}
	if (lexer->lookahead != NULL) {
		fclose(lexer->lookahead);
	}
	for (size_t i = 0; i < SIZE(lexer); ++i) {
		TokenObject *token = TokenAt(lexer, i);
		CLEAR(token);
	}
//	XDECREF(lexer->s);
	DECREF(lexer->token_buffer);
//	DECREF(lexer->pos_ptr);
}

static int Lexer_GetNextToken(LexerObject *lexer, TokenObject **next) {
	assert(next != NULL && *next == NULL);
	char *buf = NULL;
	size_t size = 0;
	FILE *scanned = open_memstream(&buf, &size);
	assert(scanned);
	int res = get_next_token(lexer->lookahead, scanned, next, 0);
	fclose(scanned);
//	printf("scanned=\"%s\"\n", buf ? buf : "");
	if (res == 0) {
		Array_Append(lexer->token_buffer, (TokenObject *[]) {NEW_REF(*next)});
		size_t i;
		for (i = 0; isspace(buf[i]); ++i) update(buf[i], &lexer->last_loc);
		update(buf[i], &lexer->last_loc);
		Token_SetStartLoc(*next, lexer->last_loc);
		for (++i; i < size; ++i) update(buf[i], &lexer->last_loc);
		Token_SetEndLoc(*next, lexer->last_loc);
//			Array_Append(lexer->pos_ptr, &lexer->last_loc);
//			lexer->cur++;
		SIZE(lexer)++;
		assert(SIZE(lexer) == SIZE(lexer->token_buffer));
//			assert(SIZE(lexer) == SIZE(lexer->pos_ptr));
	}
	free(buf);
	return res;

}

// borrowed ref.
TokenObject *Lexer_PeekForwardTokenBy(LexerObject *lexer, size_t n_lookahead) {
	const size_t cur = lexer->cur;
	while (SIZE(lexer) <= cur + n_lookahead) {
		TokenObject *t = NULL;
		int v = Lexer_GetNextToken(lexer, &t);
		assert(cur == lexer->cur);
		if (v != 0) {
//			fprintf(stderr, "no more tokens.\n");
//			lexer->cur = cur;
			return NULL;
		}
		DECREF(t);
	}
//	lexer->cur = cur;
//	lexer->last_loc = Array_At(PosPtr, lexer->pos_ptr, cur);
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

// consume n (>=0) token(s) starting from the last_loc position of lexer.
void Lexer_ConsumeToken(LexerObject *lexer, size_t n) {
	lexer->cur += n;
}

#undef update
#undef TokenAt
#undef MAXN_TOKEN_BUF
#undef set_token_and_return
