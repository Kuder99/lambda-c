#include <hashmap.h>
#include <lambda.h>
#include <printing.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 65535

int main()
{
	char input[BUFFER_SIZE];
	struct HashMap hashmap;

	hashmap = hashmap_create();

	printf("λ-C: a Lambda Calculus (λ-calculus) abstraction and application interpreter.\n");
	printf("Made by victorsavas (https://github.com/victorsavas/lambda-c)\n");

	while (1) {
		printf("\nλ> ");

		if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
			printf("Error!\n");
			return 1;
		}
		
		const size_t null_pos = strcspn(input, "\n");

		input[null_pos] = '\0';

		if (input[0] == ':') {
			break;
		}

		struct LambdaHandle lambda = lambda_parse(input, null_pos + 1);

		if (lambda.term == NULL) {
			continue;
		}

		lambda_print(lambda);

		if (lambda.identifier.name == NULL) {
			lambda_free(lambda);
		} else {
			hashmap_set(&hashmap, lambda);
		}
	}

	hashmap_destroy(hashmap);

	return 0;
}