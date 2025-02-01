#ifndef LAMBDA_H
#define LAMBDA_H

#include <stdlib.h>

typedef struct Lambda Lambda;

// TODO: implement hashtable

Lambda *lambda_parse(char *buffer, size_t size);	// Parses user input as a lambda term. Returns NULL upon error.
void lambda_free(Lambda *lambda);

void lambda_print(Lambda *lambda);

#endif