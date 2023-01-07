#ifndef R5RS_LEXER_H
#define R5RS_LEXER_H

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "token.h"
#include "../Include/booleanobject.h"
#include "../Include/typedefs.h"

extern TypeObject Lexer_Type;

LexerObject *Lexer_FromFile(const char *);

LexerObject *Lexer_FromCStr(const char *);

void Lexer_Extend(LexerObject *lexer, const char *s);

void Lexer_Dealloc(LexerObject *lexer);

int Lexer_GetNextToken(LexerObject *, TokenObject **);

TokenObject *Lexer_PeekForwardTokenBy(LexerObject *lexer, size_t n_lookahead);

TokenObject *Lexer_PeekNextToken(LexerObject *);

TokenObject *Lexer_PeekCurrentToken(LexerObject *);

void Lexer_ConsumeToken(LexerObject *, size_t);

size_t Lexer_GetNTokens();

#endif //R5RS_LEXER_H
