#include "iotools.h"

int fpeek(FILE *stream){
	int buff = fgetc(stream);

	if (buff == EOF){
		return EOF;
	}

	// Error in ungetc
	if (ungetc(buff, stream) == EOF){
		perror("[Lexing] \033[31mError\033[0m Reading error");
		return EOF;
	}

	return buff;
}
