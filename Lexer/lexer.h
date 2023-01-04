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

LexerObject *Lexer_FromFile(const char *);

LexerObject *Lexer_FromCStr(const char *);

void Lexer_Finalize(LexerObject *);

int Lexer_GetNextToken(LexerObject *, TokenObject **);

TokenObject *Lexer_PeekNextToken(LexerObject *);

TokenObject *Lexer_PeekCurrentToken(LexerObject *);

void Lexer_ConsumeToken(LexerObject *, size_t);

size_t Lexer_GetNTokens();

#endif //R5RS_LEXER_H
