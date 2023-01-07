#ifndef R5RS_IO_H
#define R5RS_IO_H

#include <stdio.h>
#include "stringobject.h"

StringObject *readline(FILE *input) {
	char *buf = NULL;
	size_t size = 0;
	FILE *f = open_memstream(&buf, &size);
	assert(f);
	int c = fgetc(input);
	while (c != '\n' && c != EOF){
		fputc(c, f);
		c = fgetc(input);
	}
	fclose(f);
	StringObject *res = String_FromCStrN(buf, size);
	free(buf);
	return res;
}

Object *read_as_datum(FILE *input){

}
StringObject *get_next_line(FILE *f) {

}

StringObject *read_file(FILE *f) {

}

StringObject *read_line_stdin()

StringObject *read_file_by_name()

#endif //R5RS_IO_H
