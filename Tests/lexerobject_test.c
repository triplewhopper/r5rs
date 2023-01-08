#include <stdio.h>
#include "lexer.h"
#include "object.h"
#include "typeobject.h"
#include "arrayobject.h"

int main() {
	FILE *f = fopen("../samples/s18.scm", "r");
	LexerObject *l = Lexer_FromStream(f, NULL);
	TokenObject *t = Lexer_PeekCurrentToken(l);
	ArrayObject *data = Array_New(0, sizeof(TokenObject*));
	int tot = 1;
	while (t && tot < 300) {
//		int c = fgetc(stdin);
//		if(c == '\n'){
//		} else ungetc(c, stdin);
		Array_Append(data, &t);
		tot++;
		Lexer_ConsumeToken(l, 1);
		t = Lexer_PeekCurrentToken(l);
	}
	for (size_t i = 0; i < SIZE(data); ++i) {
		PRINT(Array_At(TokenObject * , data, i), stdout);
		printf("\n");
	}
	Array_Clear(data);
//	fclose(f);
	return 0;
}