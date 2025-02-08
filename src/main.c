#include <lambda.h>
#include <printing.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 65535

int main()
{
	char input[BUFFER_SIZE];
	struct LambdaHandle lambda;

	printf("λ-C: a λ-calculus abstraction and application interpreter.");

	while (1) {
		printf("\nλ> ");

		if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
			printf("Error!\n");
			return 1;
		}
		
		const size_t null_pos = strcspn(input, "\n");

		input[null_pos] = '\0';

		lambda = lambda_parse(input, null_pos + 1);

		lambda_print(lambda);

		printf("\n");

		lambda_free(lambda);
	}

	return 0;
}