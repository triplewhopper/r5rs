#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "lexer.h"
typedef struct{
	size_t length;
	char raw_data[0];
}String;

const char *readline() {
	static char tmp[10000];
	int c = getchar(), n = 0;
	while (c != EOF && c != '\n') {
		tmp[n++] = (char) c;
		c = getchar();
	}
	char *res = malloc(n * sizeof(char));
	assert(res);
	memcpy(res, tmp, n);
	return res;
}

int main() {
	int x = str_f_repr("he llo,\n\t\rworld!\n", stdout);
	printf("x=%d\n", x);
//	util_i32_vector *v = util_i32_vector_new();
//	for(size_t i = 0; i< 100;++i)
//		util_i32_vector_push_back(v, (int)i);
//	util_i32_vector_print(v);
//	util_i32_vector_delete(v);
//	FILE *fptr = fopen("../sample.scm", "r");
//	if (!fptr){
//		printf("%s\n", strerror(errno));
//		exit(0);
//	}
//	static char buf[10000];
//	fread(buf, sizeof(char), 10000, fptr);
//	printf("%s", buf);
//	fclose(fptr);

	return 0;
}
