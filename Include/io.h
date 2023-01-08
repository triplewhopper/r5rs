#ifndef R5RS_IO_H
#define R5RS_IO_H

#include <stdio.h>
#include "stringobject.h"

StringObject *readline(FILE *input) {
	char *buf = NULL;
	size_t size = 0;
	if (getline(&buf, &size, input) > 0) {
		StringObject *res = String_FromCStrN(buf, size);
		free(buf);
		return res;
	} else {
		perror("readline");
		exit(EXIT_FAILURE);
	}
}

Object *read_as_datum(FILE *input) {
	return NULL;
}

StringObject *get_next_line(FILE *f) {
	return NULL;
}

StringObject *read_file(FILE *f) {
	return NULL;
}

StringObject *read_line_stdin();

StringObject *read_file_by_name();

#endif //R5RS_IO_H
