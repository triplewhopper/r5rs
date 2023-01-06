#include "lexer.h"
char s[100000];
int main(){
	FILE *fptr = fopen("../sample.scm", "r");
	if(fptr==NULL){
		perror("failed to open file");
		exit(EXIT_FAILURE);
	}
	fread(s, sizeof(char), sizeof(s)/sizeof(char), fptr);
	fclose(fptr);
	PosPtr p = {1, 1, s};
	GetNextToken(p);
	return 0;
}