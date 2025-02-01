#include <lambda.h>
#include <stdio.h>
#include <string.h>

#define LAMBDA_CHARACTER 'λ'

static int is_valid_character(char c);
static int is_name_character(char c);

static void print_error_at(const char *text, const char *buffer, int position);

struct Lambda {
	char *name;
	char *expression;

	int temporary;
	int maximally_reduced;
};

void lambda_print(Lambda *lambda)
{
	if (lambda == NULL) {
		return;
	}

	if (!lambda->temporary) {
		printf("name: %s\nexpression: %s\n", lambda->name, lambda->expression);
		return;
	}

	printf("expression: %s\n", lambda->expression);

}

Lambda *lambda_parse(char *buffer, size_t size)
{
	if (buffer == NULL || size == 0) {
		printf("Invalid buffer.\n");
		return NULL;
	}

	if (fgets(buffer, size, stdin) == NULL) {
		printf("fgets failed.\n");
                return NULL;
        }

	const size_t str_length = strcspn(buffer, "\n"); 
        buffer[str_length] = '\0';

	const char *buffer_null = buffer + str_length;
	char *buffer_iterator = buffer;

	int parenthesis_nesting = 0;
	int expression_expected = 0;

	char *lambda_expression_beginning = NULL;

	int lambda_temporary = 0;

	char *lambda_name_beginning = NULL;
	char *lambda_name_ending = NULL;

	char c;

	while (buffer_iterator < buffer_null) {
		c = *buffer_iterator;

		if (!is_valid_character(c)) {
			// Only ascii letters, numbers, parenthesis, backslash, dots and white spaces are allowed.
			goto error_invalid_character;
		}

		switch (c) {
		case '=':
			goto error_unexpected_attribution;

			break;

		case ' ':
			break;

		case '\\':
			buffer_iterator++;
			c = *buffer_iterator;

			if (c != 'l') {
				// Backslashes may only be used to define lambda operators.
				goto error_invalid_backslash;
			}
			
			if (lambda_expression_beginning == NULL) {
				lambda_expression_beginning = buffer_iterator - 1;

				
				if (lambda_name_beginning == NULL) {
					lambda_temporary = 1;
				}
			}

			*(buffer_iterator - 1) = (LAMBDA_CHARACTER >> 8) & 0xFF;
			*buffer_iterator = LAMBDA_CHARACTER & 0xFF;

			int bound_name_begun = 0;
			int bound_name_ended = 0;

			// Search for bound variable and dot operator
			while (1) {
				buffer_iterator++;
				c = *buffer_iterator;

				if (!is_valid_character(c)) {
					goto error_invalid_character;
				}

				if (c == ' ') {
					if (bound_name_begun) {
						bound_name_ended = 1;
					}

					// Skipping white space
					continue;
				}
				
				if (c == '.' && bound_name_begun) {
					break;
				} else if (bound_name_ended) {
					goto error_missing_dot_operator;
				}

				if (is_name_character(c)) {
					bound_name_begun = 1;
					continue;
				}

				if (!bound_name_begun) {
					goto error_missing_abstraction_bind;
				} else {
					goto error_missing_dot_operator;
				}
			}

			expression_expected = 1;

			break;

		case '(':
			parenthesis_nesting++;

			if (lambda_expression_beginning == NULL) {
				lambda_expression_beginning = buffer_iterator;
				

				if (lambda_name_beginning == NULL) {
					lambda_temporary = 1;
				}
			}

			break;
		
		case ')':
			if (parenthesis_nesting == 0) {
				// Missing left parenthesis.
				goto error_invalid_parenthesis;
			}

			if (expression_expected) {
				goto error_missing_expression;
			}

			parenthesis_nesting--;
			break;

		case '.':
			goto error_unexpected_dot;

		default:
			expression_expected = 0;

			if (lambda_temporary || lambda_name_beginning != NULL) {
				break;
			}

			lambda_name_beginning = buffer_iterator;

			do {
				buffer_iterator++;
				c = *buffer_iterator;
			} while (is_name_character(c));

			lambda_name_ending = buffer_iterator;

			do {
				buffer_iterator++;
				c = *buffer_iterator;

				if (!is_valid_character(c)) {
					goto error_invalid_character;
				}
			} while(c == ' ');

			switch (c) {
			case '=':
				lambda_expression_beginning = buffer_iterator + 1;
				expression_expected = 1;
				
				break;
			case ')':
				goto error_invalid_parenthesis;
			case '.':
				goto error_unexpected_dot;
			default:
				lambda_temporary = 1;

				lambda_expression_beginning = lambda_name_beginning;
				
				lambda_name_beginning = NULL;
				lambda_name_ending = NULL;
				
				break;
			}

			break;
		}

		buffer_iterator++;
	}

	if (parenthesis_nesting != 0) {
		// Missing right parenthesis.
		goto error_invalid_parenthesis;
	}

	if (expression_expected) {
		goto error_missing_expression;
	}

	Lambda *lambda;

	lambda = malloc(sizeof(*lambda));

	lambda->temporary = lambda_temporary;
	lambda->maximally_reduced = 0;

	lambda->expression = malloc(buffer_null - lambda_expression_beginning + 1);

	strcpy(lambda->expression, lambda_expression_beginning);
	
	if (!lambda_temporary) {
		size_t lambda_name_length = lambda_name_ending - lambda_name_beginning + 1;

		lambda->name = malloc(lambda_name_length);

		strncpy(lambda->name, lambda_name_beginning, lambda_name_length);

		lambda->name[lambda_name_length - 1] = '\0';
	} else {
		lambda->name = NULL;
	}


	return lambda;


	// Error messages

	error_invalid_character:
	print_error_at("Invalid character.", buffer, buffer_iterator - buffer);
	return NULL;

	error_invalid_backslash:
	print_error_at("Invalid backslash symbol.", buffer, buffer_iterator - buffer);
	return NULL;

	error_invalid_parenthesis:
	print_error_at("Invalid parenthesis.", buffer, buffer_iterator - buffer);
	return NULL;

	error_unexpected_dot:
	print_error_at("Unexpected dot \".\" operator.", buffer, buffer_iterator - buffer);
	return NULL;

	error_missing_abstraction_bind:
	print_error_at("Missing bound variable at abstraction.", buffer, buffer_iterator - buffer - 1);
	return NULL;

	error_missing_dot_operator:
	print_error_at("Missing dot \".\" operator at abstraction.", buffer, buffer_iterator - buffer - 2);
	return NULL;

	error_missing_expression:
	print_error_at("Missing expression.", buffer, buffer_iterator - buffer - 1);
	return NULL;

	error_unexpected_attribution:
	print_error_at("Unexpected attribution operator \"=\".", buffer, buffer_iterator - buffer - 1);
	return NULL;
}

void lambda_free(Lambda *lambda)
{
	if (lambda == NULL) {
		return;
	}

	if (!lambda->temporary) {
		free(lambda->name);
	}

	free(lambda->expression);

	free(lambda);
}

int is_valid_character(char c)
{
	char characters_table[] = " .\\=()0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	
	const char *p = characters_table;

	while (*p != '\0') {
		if (*p++ == c) {
			return 1;
		}
	}

	return 0;
}

int is_name_character(char c)
{
	char name_char_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	const char *p = name_char_table;

	while (*p != '\0') {
		if (*p++ == c) {
			return 1;
		}
	}

	return 0;
}

void print_error_at(const char *text, const char *buffer, int position)
{
	printf("ERROR: %s\n\t%s\n\t", text, buffer);

	while (position-- > 0) {
		printf(" ");
	}

	printf("^");
}