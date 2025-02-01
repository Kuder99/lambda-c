#include <lambda.h>
#include <stdio.h>

#define INPUT_BUFFER_SIZE 65535

int main()
{
	char input[INPUT_BUFFER_SIZE];

	Lambda *lambda;

	printf("λ-C: A λ-calculus application and abstraction interpreter.");

	while (1) {
		printf("\nλ> ");

		lambda = lambda_parse(input, INPUT_BUFFER_SIZE);

		lambda_print(lambda);

		lambda_free(lambda);
	}

	return 0;
}