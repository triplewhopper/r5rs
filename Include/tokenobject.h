#ifndef R5RS_TOKENOBJECT_H
#define R5RS_TOKENOBJECT_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "stringobject.h"

typedef struct {
	size_t line, column;
} SourceLocation;

typedef struct pos_ptr {
	SourceLocation pos;
//	const char *lookahead;
	size_t index;
} PosPtr;

typedef enum {
	NOT_AVAILABLE,
	IDENTIFIER, BOOLEAN, NUMBER,
	CHARACTER, STRING,
	L_PAREN, R_PAREN, SHARP_L_PAREN,
	SINGLE_QUOTE, BACK_QUOTE, COMMA, COMMA_AT, DOT, COMMENT
} TokenKind;

void TokenKind_Print(TokenKind, FILE *);

PosPtr consume(PosPtr, size_t);

// let j = n initially, and increase j repeatedly until last_loc.lookahead[j + 1] == '\0', space characters, '(', ')', '"', or ';'.
// return consume(last_loc, j).
//PosPtr consume_until_delimiters(PosPtr, size_t);

struct token_object {
	Object ob_base;
	TokenKind kind;
	SourceLocation start, end;
	StringObject *s;
	Object *ob_val;

#ifdef FLAG_TRACK_ALL_OBJS
	int ob_val_obj_index;
#endif
	// kind == STRING: ob_val is a StringObject
	// kind == NUMBER: ob_val is a number object
	// kind == CHARACTER: ob_val is a CharObject
	// kind == BOOLEAN: ob_val is a BooleanObject
	// otherwise ob_val == NULL.
};

TokenObject *Token_New(FILE *, size_t, TokenKind);

void Token_SetStartLoc(TokenObject *, SourceLocation);
void Token_SetEndLoc(TokenObject *, SourceLocation);

void Token_GetStartLoc(TokenObject *);

void Token_GetEndLoc(TokenObject *);

void Token_SetKind(TokenObject *, TokenKind);

void Token_SetVal(TokenObject *, Object *);

void Token_Print(TokenObject *, FILE *);

void Token_Dealloc(TokenObject *);


#endif //R5RS_TOKENOBJECT_H
