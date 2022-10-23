#ifndef R5RS_LEXER_H
#define R5RS_LEXER_H

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

typedef enum {
	NOT_AVAILABLE,
	IDENTIFIER, BOOLEAN, NUMBER,
	CHARACTER, STRING,
	L_PAREN, R_PAREN, SHARP_L_PAREN,
	SINGLE_QUOTE, BACK_QUOTE, COMMA, COMMA_AT, DOT
} TokenKind;
typedef struct {
	size_t col, row;
} SourceLocation;
typedef struct {
	size_t len;
	TokenKind kind;
	SourceLocation loc;
	char data[0];
} Token;

void tokenize(const char *s) {
//	const char *ptr = s;
//	static int states[100000] = {0};
//	int stack_top = 0;
//	while (ptr >= s && *ptr) {
//		char head = *ptr;
//		if (states[stack_top] ==)
//	}
}

int is_letter(char);

int is_special_initial(char);

int is_initial(const char c) {
	return is_letter(c) || is_special_initial(c);
}

int is_letter(const char c) {
	return isalpha(c);
}

int is_special_initial(const char c) {
	static const char *y = "!$%&*/:<=>?^_~";
	for (const char *p = y; *p; ++p) {
		if (c == *p) return 1;
	}
	return 0;
}


int char_repr_impl(const char c, char *stream) {
	typedef struct {
		const char key;
		const char *value;
	} escape_character_table_t;
	static const escape_character_table_t table[] = {
			{'\a', "\\a"},
			{'\b', "\\b"},
			{'\f', "\\f"},
			{'\n', "\\n"},
			{'\r', "\\r"},
			{'\t', "\\t"},
			{'\v', "\\v"},
			{'\\', "\\\\"},
			{'\'', "\\\'"},
			{'\"', "\\\""},
			{'\?', "\\?"},
			{0, NULL}
	};

	if (isprint(c)) {
		return fputc(c, stream);
	} else {
		fputs("", stream);
		for (const escape_character_table_t *p = table; p->key; ++p) {
			if (c == p->key) {
				return sprintf(stream, "%s", p->value);
			}
		}
		char error_message[50] = "N/A";
		sprintf(error_message, "Unknown character (ascii code=%d)", (int) c);
		perror(error_message);
		exit(EXIT_FAILURE);
	}
}

int char_s_repr(const char c, char *str) {
	return char_repr_impl(c, str);
}

int char_f_repr(const char c, FILE *stream) {
	char buf[6] = {0};
	char_repr_impl(c, buf);
	return fprintf(stream, "%s", buf);
}

int str_f_repr(const char *s, FILE *stream) {
	int res = 0;
	for (; *s; ++s) {
		res += char_f_repr(*s, stream);
	}
	return res;
}

int str_s_repr(const char *s, char *stream) {
	int res = 0;
	for (; *s; ++s) {
		res += char_s_repr(*s, stream);
	}
	return res;
}

//const char *entrance(const char *str) {
//	int state = 0;
//	char error_message[100] = "N/A";
//	for (;;) {
//		switch (state) {
//			case 0:
//				switch (*str) {
//					case '\0':
//						break;
//					default:
//
//						sprintf(error_message, "Unknown transit at state %d via \'%c\'")
//
//				}
//				break;
//			default:
//				sprintf(error_message, "Uncaught state %d", state);
//				perror(error_message);
//				exit(EXIT_FAILURE);
//		}
//	}
//	while (*str && isspace(*str)) str++;
//	if (str[0] == ';') {
//		// <comment>
//		while (*str && *str != '\n') str++;
//		if (*str == '\n') str++;
//	} else {
//
//	}
//	if (strncmp(str, "...", 3) == 0) {
//		if (str[3]) {
//			return str + 3;
//		}
//	}
//	if (str[0] && str[0] == '+') {
//
//	}
//}

//int get_an_identifier(const char *str) {
//	if (str[0] == '_' || isalpha(str[0])) {
//		if (str[1] && str[2] &&)
//	}
//}

#endif //R5RS_LEXER_H
