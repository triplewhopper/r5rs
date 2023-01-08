#ifndef R5RS_LEXER_H
#define R5RS_LEXER_H

#include "typedefs.h"

extern TypeObject Lexer_Type;

//LexerObject *Lexer_FromFile(const char *);

//LexerObject *Lexer_FromCStr(const char *);

//LexerObject *Lexer_FromString(StringObject *s);
//void Lexer_Extend(LexerObject *lexer, const char *s);

void Lexer_Dealloc(LexerObject *lexer);

//int Lexer_GetNextToken(LexerObject *, TokenObject **);
LexerObject *Lexer_FromStream(FILE *, const char *);

TokenObject *Lexer_PeekForwardTokenBy(LexerObject *lexer, size_t n_lookahead);

TokenObject *Lexer_PeekNextToken(LexerObject *);

TokenObject *Lexer_PeekCurrentToken(LexerObject *);

void Lexer_ConsumeToken(LexerObject *, size_t);

size_t Lexer_GetNTokens();

#endif //R5RS_LEXER_H
