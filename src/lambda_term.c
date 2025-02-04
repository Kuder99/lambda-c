#include <lambda_term.h>
#include <stdio.h>
#include <string.h>

#define LAMBDA_CHARACTER 'λ'
#define NO_SUBSCRIPT -1

static void print_error_at(const char *error, const char *str, int position);

static const char *skip_whitespace(const char *str, const char *end);
static const char *skip_name(const char *str, const char *end);

static int char_is_valid(char c);
static int char_is_name(char c);

const char *skip_whitespace(const char *str, const char *end)
{
	while ((*str == ' ') && (end > str))
		str++;

	return str;
}

const char *skip_name(const char *str, const char *end)
{
	while (char_is_name(*str) && (end > str)) {
		str++;
	}

	return str;
}

int char_is_valid(char c)
{
	const char *table = "λ\\.()=0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	const char *it = table;

	while (*it != '\0') {
		if (*it++ == c)
			return 1;
	}

	return 0;
}

int char_is_name(char c)
{
	const char *table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	const char *it = table;

	while (*it != '\0') {
		if (*it++ == c)
			return 1;
	}

	return 0;
}

void print_error_at(const char *error, const char *str, int position)
{
	printf("ERROR: %s\n\t%s\n\t", error, str);

	while (position-- > 0)
		printf(" "); 
	
	printf("^");
}

enum Ternary {
	TERNARY_FALSE=0,
	TERNARY_TRUE=1,
	TERNARY_UNKNOWN=2
};

int lambda_is_valid(const char* str, const size_t size)
{
	if (str == NULL || size == 0) {
		return 0;
	}

	const char *end = str + size - 1;
	const char *current = str;

	// A ternary logic is used to handle parsing the name of the lambda expression, if it has been assigned one.

	enum Ternary temporary = TERNARY_UNKNOWN;

	int expression_expected = 1;
	size_t parenthesis_nesting = 0;

	current = skip_whitespace(current, end);

	while (*current != '\0' && current < end) {
		if (!char_is_valid(*current))
			goto error_invalid_character;

		switch (*current & 0XFF) {
		case '.':
			goto error_unexpected_operator;
		
		case '=':
			goto error_unexpected_operator;

		case '\\':
			current = skip_whitespace(current + 1, end);

			if (!char_is_name(*current)) {
				goto error_expected_argument;
			}

			current = skip_name(current + 1, end);
			current = skip_whitespace(current, end);

			if (*current != '.') {
				goto error_expected_dot;
			}

			expression_expected = 1;

			if (temporary == TERNARY_UNKNOWN) {
				temporary = TERNARY_TRUE;
			}

			current++;

			break;

		case ((LAMBDA_CHARACTER >> 8) & 0XFF):
			current++;

			if ((*current & 0XFF) != (LAMBDA_CHARACTER & 0XFF)) {
				goto error_invalid_character;
			}

			current = skip_whitespace(current + 1, end);

			if (!char_is_name(*current)) {
				goto error_expected_argument;
			}

			current = skip_name(current + 1, end);
			current = skip_whitespace(current, end);

			if (*current != '.') {
				goto error_expected_dot;
			}

			expression_expected = 1;

			if (temporary == TERNARY_UNKNOWN) {
				temporary = TERNARY_TRUE;
			}

			current++;

			break;

		case ')':
			if (parenthesis_nesting-- == 0) {
				goto error_invalid_parenthesis;
			}

			if (expression_expected) {
				goto error_expression_expected;
			}

			current++;

			break;
		case '(':
			parenthesis_nesting++;
			
			if (temporary == TERNARY_UNKNOWN) {
				temporary = TERNARY_TRUE;
			}

			current++;

			break;

		default:
			expression_expected = 0;
			current = skip_name(current + 1, end);

			if (temporary != TERNARY_UNKNOWN) {
				break;
			}

			current = skip_whitespace(current, end);

			if (*current != '=') {
				temporary = TERNARY_FALSE;
				break;
			}

			temporary = TERNARY_TRUE;
			expression_expected = 1;

			current++;

			break;
		}

		current = skip_whitespace(current, end);
	}

	if (parenthesis_nesting != 0) {
		goto error_invalid_parenthesis;
	}

	if (expression_expected) {
		goto error_expression_expected;
	}

	return 1;

	// Error handling

	error_invalid_character:
	print_error_at("invalid character.", str, current - str);
	return 0;

	error_expected_argument:
	print_error_at("expected argument after lambda operator.", str, current - str);
	return 0;

	error_expected_dot:
	print_error_at("expected dot after lambda operator and argument.", str, current - str);
	return 0;

	error_expression_expected:
	print_error_at("expression expected.", str, current - str);
	return 0;

	error_unexpected_operator:
	print_error_at("unexpected operator.", str, current - str);
	return 0;

	error_invalid_parenthesis:
	print_error_at("invalid parenthesis.", str, current - str);
	return 0;
}

#ifdef STACK_DEBUG
static void stack_print(struct LambdaTerm **stack, size_t stack_count)
{
	if (stack == NULL || stack_count == 0) {
		printf("Stack: {}\n");
		return;
	}

	struct LambdaTerm **end = stack + stack_count - 1;
	struct LambdaTerm **i = stack;

	printf("Stack: {");

	while (i < end) {
		if (*i == NULL) {
			printf("NULL, ");
			i++;
			continue;
		}

		switch ((*i)->type) {
		case VARIABLE:
			printf("VARIABLE, ");
			break;

		case ABSTRACTION:
			printf("ABSTRACTION, ");
			break;

		case INCOMPLETE_ABSTRACTION:
			printf("INCOMPLETE_ABSTRACTION, ");
			break;

		case APPLICATION:
			printf("APPLICATION, ");
			break;
		}

		i++;
	}
	
	if (*i == NULL) {
		printf("NULL}\n");
		return;
	}

	switch ((*i)->type) {
	case VARIABLE:
		printf("VARIABLE}\n");
		break;

	case ABSTRACTION:
		printf("ABSTRACTION}\n");
		break;

	case INCOMPLETE_ABSTRACTION:
		printf("INCOMPLETE_ABSTRACTION}\n");
		break;

	case APPLICATION:
		printf("APPLICATION}\n");
		break;
	}
}
#else

#define stack_print(stack, stack_count);

#endif

static size_t stack_push(struct LambdaTerm **stack, size_t stack_count, struct LambdaTerm *term)
{
	// This function assumes there is enough memory for pushing new terms
	// This function assumes there is at least one member in the stack

	// Peek the top, without popping.

	struct LambdaTerm *top = stack[stack_count - 1];

	if (top == NULL || term == NULL) {
		stack[stack_count++] = term;

		stack_print(stack, stack_count);

		return stack_count;
	}

	if (term->type == INCOMPLETE_ABSTRACTION || top->type == INCOMPLETE_ABSTRACTION) {
		// This creates a stack of incomplete abstractions over incomplete abstractions, which will be eventually flattened.

		stack[stack_count++] = term;

		stack_print(stack, stack_count);

		return stack_count;
	}

	struct LambdaTerm *application;

	application = malloc(sizeof(*application));

	application->type = APPLICATION;
	application->subscript = NO_SUBSCRIPT;

	application->expression.application.argument = term;
	application->expression.application.function = top;

	// Pops the top term off of the stack and pushes the application

	stack[stack_count - 1] = application;

	stack_print(stack, stack_count);

	return stack_count;
}

static size_t stack_flatten(struct LambdaTerm **stack, size_t stack_count)
{
	// This function does not increase stack_count.
	// This function assumes there is a NULL pointer stored down the stack which represents a left parenthesis.
	// This function assumes that stack_count >= 2

	struct LambdaTerm *push = stack[--stack_count];
	struct LambdaTerm *top = stack[--stack_count];

	while (top != NULL) {
		if (push->type == INCOMPLETE_ABSTRACTION) {
			push->type = ABSTRACTION;
		}

		if (top->type == INCOMPLETE_ABSTRACTION) {
			top->expression.abstraction.body = push;
			push = top;

			top = stack[--stack_count];

			continue;
		}
		
		struct LambdaTerm *application;

		application = malloc(sizeof(*application));

		application->type = APPLICATION;
		application->subscript = NO_SUBSCRIPT;

		application->expression.application.argument = push;
		application->expression.application.function = top;

		push = application;

		top = stack[--stack_count];
	}

	if (push->type == INCOMPLETE_ABSTRACTION) {
		push->type = ABSTRACTION;
	}

	// Left associativity of application handling after flattening

	if (stack_count == 0) {
		goto end;
	}

	top = stack[stack_count - 1];

	if (top != NULL) {
		if (top->type == INCOMPLETE_ABSTRACTION) {
			goto end;
		}

		stack_count--;

		struct LambdaTerm *application;

		application = malloc(sizeof(*application));

		application->type = APPLICATION;
		application->subscript = NO_SUBSCRIPT;

		application->expression.application.argument = push;
		application->expression.application.function = top;

		push = application;
	}

	end:

	stack[stack_count++] = push;

	stack_print(stack, stack_count);

	return stack_count;
}

struct LambdaHandle lambda_parse(const char *expression, const size_t size)
{
	struct LambdaHandle lambda = (struct LambdaHandle){NULL, NULL};

	int is_valid = lambda_is_valid(expression, size);

	if (!is_valid) {
		// Returns an empty handle upon receiving invalid expression
		return lambda;
	}

	// Wrap a lambda string inside a tree LambdaTerm data structure.

	struct LambdaTerm **stack;

	size_t stack_count = 1;
	size_t stack_capacity = 8;

	stack = malloc(sizeof(*stack) * stack_capacity);

	stack[0] = NULL;

	// *end represents the last byte in the buffer

	const char *end = expression + size - 1;
	const char *current = expression;

	enum Ternary temporary = TERNARY_UNKNOWN;

	stack_print(stack, stack_count);

	current = skip_whitespace(current, end);

	while (*current != '\0' && current < end) {
		struct LambdaTerm *term;

		size_t name_length;

		const char *name_begin;
		const char *name_end;

		char *name;

		switch (*current & 0XFF) {
		case '(':
			// Add NULL member representing a left parenthesis signal.

			stack_count = stack_push(stack, stack_count, NULL);

			current++;

			if (temporary == TERNARY_UNKNOWN) {
				temporary = TERNARY_TRUE;
			}

			break;

		case ((LAMBDA_CHARACTER >> 8) & 0XFF):
			current++;

		case '\\':
			// Parse abstraction

			current = skip_whitespace(current + 1, end);

			name_begin = current;

			current = skip_name(current + 1, end);
			name_end = current;

			name_length = name_end - name_begin + 1;

			name = malloc(sizeof(*name) * name_length);

			strncpy(name, name_begin, name_length - 1);
			name[name_length - 1] = '\0';

			// Skip dot

			current = skip_whitespace(current, end);
			current++;

			// Push abstraction

			term = malloc(sizeof(*term));

			term->type = INCOMPLETE_ABSTRACTION;
			term->subscript = NO_SUBSCRIPT;

			term->expression.abstraction.argument = name;
			term->expression.abstraction.body = NULL;

			stack_count = stack_push(stack, stack_count, term);

			break;

		case ')':
			// Flatten stack until NULL member

			stack_count = stack_flatten(stack, stack_count);

			current++;

			break;

		default:
			// Parse name

			name_begin = current;

			current = skip_name(current + 1, end);
			name_end = current;

			name_length = name_end - name_begin + 1;

			name = malloc(sizeof(*name) * name_length);

			strncpy(name, name_begin, name_length - 1);
			name[name_length - 1] = '\0';

			if (temporary != TERNARY_UNKNOWN) {
				term = malloc(sizeof(*term));

				term->type = VARIABLE;
				term->subscript = NO_SUBSCRIPT;

				term->expression.variable = name;

				stack_count = stack_push(stack, stack_count, term);

				break;
			}

			current = skip_whitespace(current, end);

			if (*current != '=') {
				term = malloc(sizeof(*term));

				term->type = VARIABLE;
				term->subscript = NO_SUBSCRIPT;

				term->expression.variable = name;

				stack_count = stack_push(stack, stack_count, term);

				temporary = TERNARY_TRUE;
				break;
			}

			lambda.name = name;
			current++;

			temporary = TERNARY_FALSE;

			break;
		}

		// Resizing the stack array to fit new members

		if (stack_capacity - stack_count <= 1) {
			// Scaling factor of 2

			stack_capacity <<= 1;

			// Reallocate the stack

			stack = realloc(stack, stack_capacity * sizeof(*stack));
		}

		current = skip_whitespace(current, end);
	}

	while (stack_count > 1) {
		stack_count = stack_flatten(stack, stack_count);
	}

	lambda.term = stack[0];

	free(stack);

	return lambda;
}

void lambda_free(struct LambdaHandle lambda)
{
	if (lambda.name != NULL)
		free(lambda.name);

	if (lambda.term == NULL)
		// This should never happen
		return;

	// A stack-based pre-order traversal deallocation function

	size_t stack_count = 1;
	size_t stack_capacity = 8;
	
	struct LambdaTerm **stack;

	stack = malloc(sizeof(*stack) * stack_capacity);

	stack[0] = lambda.term;

	stack_print(stack, stack_count);

	while (stack_count > 0) {
		// Popping the topmost stack member
		
		struct LambdaTerm *top_term = stack[--stack_count];

		if (top_term == NULL)
			// This should never happen
			continue;

		// Pushing the current term's inner terms

		switch (top_term->type) {
		case VARIABLE:
		{	
			char *variable = top_term->expression.variable;

			if (variable != NULL)
				free(top_term->expression.variable);
		}
			break;

		case ABSTRACTION:
		{
			char *argument = top_term->expression.abstraction.argument;
			struct LambdaTerm *body = top_term->expression.abstraction.body;
			
			if (argument != NULL)
				free(argument);
			
			if (body != NULL)
				stack[stack_count++] = top_term->expression.abstraction.body;
		}
			break;
		
		case APPLICATION:
		{
			struct LambdaTerm *function = top_term->expression.application.function;
			struct LambdaTerm *argument = top_term->expression.application.argument;

			if (function != NULL)
				stack[stack_count++] = function;
			
			if (argument != NULL)
				stack[stack_count++] = argument;

		}
			break;

		case INCOMPLETE_ABSTRACTION:
		default:
			// This should never happen
			break;
		}

		free(top_term);

		// Resizing the stack array to fit new members

		if (stack_capacity - stack_count <= 1) {
			// Scaling factor of 2

			stack_capacity <<= 1;

			// Reallocate the stack

			stack = realloc(stack, stack_capacity * sizeof(*stack));
		}

		stack_print(stack, stack_count);
	}

	free(stack);
}

void lambda_print(struct LambdaHandle lambda)
{
	if (lambda.term == NULL)
		return;

	if (lambda.name != NULL) {
		printf("%s = ", lambda.name);
	}

	struct LambdaTerm **stack;

	size_t stack_count = 1;
	size_t stack_capacity = 16;

	stack = malloc(sizeof(*stack) * stack_capacity);

	stack[0] = lambda.term;

	struct LambdaTerm space_signal;

	struct LambdaTerm left_parenthesis_signal;
	struct LambdaTerm right_parenthesis_signal;

	while (stack_count > 0) {
		struct LambdaTerm *top = stack[--stack_count];

		if (top == NULL) {
			// This is never reached
			return;
		}

		if (top == &space_signal) {
			putchar(' ');
			continue;
		}

		if (top == &right_parenthesis_signal) {
			putchar(')');
			continue;
		}

		if (top == &left_parenthesis_signal) {
			putchar('(');
			continue;
		}

		switch (top->type) {
		case VARIABLE:
			printf("%s", top->expression.variable);

			if (top->subscript >= 0) {
				printf("_%d", top->subscript);
			}

			break;

		case ABSTRACTION:
			printf("λ%s. ", top->expression.abstraction.argument);

			if (top->subscript >= 0) {
				printf("_%d", top->subscript);
			}

			stack[stack_count++] = top->expression.abstraction.body;

			break;

		case APPLICATION:

			struct LambdaTerm *function = top->expression.application.function;
			struct LambdaTerm *argument = top->expression.application.argument;

			// Formatting

			int function_parenthesis = 0;
			int argument_parenthesis = 0;

			int space = 1;

			if (function->type == ABSTRACTION)
				function_parenthesis = 1;

			if (function->type == APPLICATION)
				if (function->expression.application.argument->type == ABSTRACTION)
					function_parenthesis = 1;

			if (argument->type == APPLICATION) {
				argument_parenthesis = 1;
				space = 0;
			}

			// Stack pushes

			if (argument_parenthesis)
				stack[stack_count++] = &right_parenthesis_signal;

			stack[stack_count++] = argument;

			if (argument_parenthesis)
				stack[stack_count++] = &left_parenthesis_signal;

			if (space)
				stack[stack_count++] = &space_signal;

			if (function_parenthesis)
				stack[stack_count++] = &right_parenthesis_signal;

			stack[stack_count++] = function;

			if (function_parenthesis)
				putchar('(');

			break;

		default:
			// This is never reached
			break;
		}

		if (stack_capacity - stack_count <= 8) {
			stack_capacity <<= 1;

			stack = realloc(stack, sizeof(*stack) * stack_capacity);
		}
	}

	free(stack);
}