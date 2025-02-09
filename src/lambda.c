#include <lambda.h>
#include <stdio.h>
#include <string.h>

#define LAMBDA_CHARACTER 'λ'
#define NO_SUBSCRIPT -1

// lambda_is_valid and lambda_parse subroutines

static void print_error_at(const char *error, const char *str, int position);

static const char *skip_whitespace(const char *str, const char *end);
static const char *skip_name(const char *str, const char *end);
static const char *skip_digits(const char *str, const char *end);

static int char_is_valid(char c);
static int char_is_name(char c);
static int char_is_digit(char c);

enum LambdaType {
	INVALID = 0,
	DEFINITION = 1,
	TEMPORARY = 2
};

static enum LambdaType lambda_is_valid(const char* str, const size_t size)
{
	if (str == NULL || size == 0) {
		return INVALID;
	}

	const char *end = str + size - 1;
	const char *current = str;

	// If the passed string is a valid lambda term, lambda_type will be set to either DEFINITION it has been assigned an identifier or TEMPORARY otherwise.

	enum LambdaType lambda_type = INVALID;

	int expression_expected = 1;
	size_t parenthesis_nesting = 0;

	current = skip_whitespace(current, end);

	while (*current != '\0' && current < end) {
		if (!char_is_valid(*current))
			goto error_invalid_character;

		if (char_is_digit(*current)) {
			current = skip_digits(current + 1, end);
			current = skip_whitespace(current, end);

			expression_expected = 0;

			if (lambda_type == INVALID) {
				lambda_type = TEMPORARY;
			}

			continue;
		}

		if (char_is_name(*current)) {
			expression_expected = 0;
			current = skip_name(current + 1, end);

			if (char_is_digit(*current)) {
				current = skip_digits(current + 1, end);
			}

			current = skip_whitespace(current, end);

			if (lambda_type != INVALID) {
				continue;
			}

			if (*current == '=') {
				lambda_type = DEFINITION;
				expression_expected = 1;

				current = skip_whitespace(current + 1, end);

				continue;
			}

			lambda_type = TEMPORARY;
		}

		switch (*current & 0XFF) {
		case '.':
			goto error_unexpected_operator;
		
		case '=':
			goto error_unexpected_operator;

		case ((LAMBDA_CHARACTER >> 8) & 0XFF):
			current++;

			if ((*current & 0XFF) != (LAMBDA_CHARACTER & 0XFF)) {
				goto error_invalid_character;
			}

		case '\\':
			current = skip_whitespace(current + 1, end);

			if (!char_is_name(*current)) {
				goto error_expected_argument;
			}

			current = skip_name(current + 1, end);
			current = skip_whitespace(current, end);

			if (char_is_digit(*current)) {
				current = skip_digits(current + 1, end);
			}

			if (*current != '.') {
				goto error_expected_dot;
			}

			expression_expected = 1;

			if (lambda_type == INVALID) {
				lambda_type = TEMPORARY;
			}

			current = skip_whitespace(current + 1, end);

			break;

		case ')':
			if (parenthesis_nesting-- == 0) {
				goto error_invalid_parenthesis;
			}

			if (expression_expected) {
				goto error_expression_expected;
			}

			current = skip_whitespace(current + 1, end);

			break;
		case '(':
			parenthesis_nesting++;
			
			if (lambda_type == INVALID) {
				lambda_type = TEMPORARY;
			}

			current = skip_whitespace(current + 1, end);

			break;
		}
	}

	if (parenthesis_nesting != 0) {
		goto error_invalid_parenthesis;
	}

	if (expression_expected) {
		goto error_expression_expected;
	}

	return lambda_type;

	// Error handling

	error_invalid_character:
	print_error_at("invalid character.", str, current - str);
	return INVALID;

	error_expected_argument:
	print_error_at("expected argument after lambda operator.", str, current - str);
	return INVALID;

	error_expected_dot:
	print_error_at("expected dot after lambda operator and argument.", str, current - str);
	return INVALID;

	error_expression_expected:
	print_error_at("expression expected.", str, current - str);
	return INVALID;

	error_unexpected_operator:
	print_error_at("unexpected operator.", str, current - str);
	return INVALID;

	error_invalid_parenthesis:
	print_error_at("invalid parenthesis.", str, current - str);
	return INVALID;
}

// lambda_parse subroutines

static struct Identifier identifier_parse(const char **current, const char *end);

static struct LambdaTerm *church_numeral_parse(const char **current, const char *end);
static struct LambdaTerm *variable_parse(
	struct Identifier **free_variables, size_t *free_variables_size, size_t *free_variables_capacity,
	struct Identifier *bound_variables, size_t bound_variables_size,
	const char **current, const char *end
);
static struct LambdaTerm *abstraction_parse(
	struct Identifier **bound_variables, size_t *bound_variables_size, size_t *bound_variables_capacity,
	const char **current, const char *end
);

static void terms_push(struct LambdaTerm *term, struct LambdaTerm ***terms, size_t *terms_size, size_t *terms_capacity);
static void terms_bind(struct LambdaTerm **terms, size_t *terms_size, size_t *bound_variables_size);

#ifdef STACK_DEBUG
static void stack_print(struct LambdaTerm **terms, size_t terms_size)
{
	if (terms == NULL || terms_size == 0) {
		printf("Stack: {}\n");
		return;
	}

	struct LambdaTerm **end = terms + terms_size - 1;
	struct LambdaTerm **i = terms;

	printf("Stack: {");

	while (i < end) {
		if (*i == NULL) {
			printf("LEFT_PARENTHESIS, ");
			i++;
			continue;
		}

		switch ((*i)->type) {
		case FREE_VARIABLE:
			printf("FREE_VARIABLE, ");
			break;


		case BOUND_VARIABLE:
			printf("BOUND_VARIABLE, ");
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

		case CHURCH_NUMERAL:
			printf("CHURCH_NUMERAL, ");
			break;
		}

		i++;
	}
	
	if (*i == NULL) {
		printf("LEFT_PARENTHESIS}\n");
		return;
	}

	switch ((*i)->type) {
	case FREE_VARIABLE:
		printf("FREE_VARIABLE}\n");
		break;

	case BOUND_VARIABLE:
		printf("BOUND_VARIABLE}\n");
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

	case CHURCH_NUMERAL:
		printf("CHURCH_NUMERAL}\n");
		break;
	}
}
#else

#define stack_print(stack, stack_count);

#endif

struct LambdaHandle lambda_parse(const char *expression, const size_t size)
{
	struct LambdaHandle lambda = {0};

	enum LambdaType lambda_type = lambda_is_valid(expression, size);

	if (lambda_type == INVALID) {
		// Returns an empty handle upon receiving invalid expression
		return lambda;
	}

	// Stack structure for storing lambda term nodes
	// The Stack is initialized with a NULL term used for binding the remaining incomplete abstraction once the string terminates

	// Every right parenthesis introduces a NULL pointer to the terms as a signal for binding
	// Every left parenthesis calls terms_bind() to bind abstractions, preserving right-associativity of abstraction

	struct LambdaTerm **terms;

	size_t terms_size = 0;
	size_t terms_capacity = 8;

	terms = malloc(sizeof(*terms) * terms_capacity);

	terms_push(NULL, &terms, &terms_size, &terms_capacity);

	// Free variables in lambda term
	// Church numerals aren't stored in this array
	// This array will be stored in lambda struct for handling future operations

	struct Identifier *free_variables;

	size_t free_variables_size = 0;
	size_t free_variables_capacity = 8;

	free_variables = malloc(sizeof(*free_variables) * free_variables_capacity);

	// Bound variables in lambda term
	// This array is deinitialized before returning

	struct Identifier *bound_variables;

	size_t bound_variables_size = 0;
	size_t bound_variables_capacity = 8;

	bound_variables = malloc(sizeof(*bound_variables) * bound_variables_capacity);

	// *end represents the last byte in the buffer

	const char *end = expression + size - 1;
	const char *current = expression;
	
	if (lambda_type == DEFINITION) {
		// Parse lambda identifier, iterating current pointer until the next character after the equals sign operator
		lambda.identifier = identifier_parse(&current, end);

		// Skipping equals sign
		current = skip_whitespace(current + 1, end);
	}

	current = skip_whitespace(current, end);

	while (*current != '\0' && current < end) {
		struct LambdaTerm *term;

		if (char_is_digit(*current)) {
			// Parsing Church numeral

			term = church_numeral_parse(&current, end);
			terms_push(term, &terms, &terms_size, &terms_capacity);

			continue;
		}

		if (char_is_name(*current)) {
			// Parse variable

			term = variable_parse(
				&free_variables, &free_variables_size, &free_variables_capacity,
				bound_variables, bound_variables_size,
				&current, end
			);
			terms_push(term, &terms, &terms_size, &terms_capacity);

			continue;
		}

		switch (*current & 0XFF) {
		case '(':
			// Add NULL member representing a left parenthesis signal for terms_bind().

			terms_push(NULL, &terms, &terms_size, &terms_capacity);

			current = skip_whitespace(current + 1, end);

			break;

		case ((LAMBDA_CHARACTER >> 8) & 0XFF):
			// Skipping the additional byte of the λ character
			current++;

		case '\\':
			// Parse abstraction

			term = abstraction_parse(
				&bound_variables, &bound_variables_size, &bound_variables_capacity,
				&current, end
			);
			terms_push(term, &terms, &terms_size, &terms_capacity);

			break;

		case ')':
			// Binds terms until NULL member, completing incomplete abstractions

			terms_bind(terms, &terms_size, &bound_variables_size);

			current = skip_whitespace(current + 1, end);

			break;
		}
	}

	// Binding the remaining incomplete abstractions

	terms_bind(terms, &terms_size, &bound_variables_size);

	// Setting up the handle

	lambda.term = terms[0];

	lambda.free_variables = free_variables;
	lambda.free_variables_size = free_variables_size;
	lambda.free_variables_capacity = free_variables_capacity;

	// Freeing memory and returning

	free(bound_variables);
	free(terms);

	return lambda;
}

void lambda_free(struct LambdaHandle lambda)
{
	if (lambda.term == NULL)
		return;

	free(lambda.identifier.name);

	// Freeing memory stored in variables

	for (size_t i = 0; i < lambda.free_variables_size; i++)
		free(lambda.free_variables[i].name);

	free(lambda.free_variables);

	// A stack-based pre-order traversal deallocation function

	size_t terms_size = 1;
	size_t terms_capacity = 8;
	
	struct LambdaTerm **terms;

	terms = malloc(sizeof(*terms) * terms_capacity);

	terms[0] = lambda.term;

	stack_print(terms, terms_size);

	while (terms_size > 0) {
		// Popping the topmost terms member
		
		struct LambdaTerm *top_term = terms[--terms_size];

		if (top_term == NULL)
			// This should never happen
			continue;

		// Pushing the current term's inner terms

		switch (top_term->type) {
		case ABSTRACTION:
			char *name = top_term->expression.abstraction.bound_variable.name;
			struct LambdaTerm *body = top_term->expression.abstraction.body;
			
			free(name);

			terms[terms_size++] = body;

			break;
		
		case APPLICATION:
			struct LambdaTerm *function = top_term->expression.application.function;
			struct LambdaTerm *argument = top_term->expression.application.argument;

			terms[terms_size++] = function;
			terms[terms_size++] = argument;

			break;
		}

		// Freeing the popped term

		free(top_term);

		// Resizing the terms stack to fit new members

		if (terms_capacity - terms_size <= 1) {
			// Scaling factor of 2

			terms_capacity <<= 1;

			// Reallocate the terms stack

			terms = realloc(terms, terms_capacity * sizeof(*terms));
		}

		stack_print(terms, terms_size);
	}

	free(terms);
}

void terms_push(struct LambdaTerm *term, struct LambdaTerm ***terms, size_t *terms_size, size_t *terms_capacity)
{
	// This function assumes there is at least one member in the terms

	// Peek the top, without popping.

	struct LambdaTerm *top = (*terms)[*terms_size - 1];

	if (top == NULL || term == NULL) {
		(*terms)[(*terms_size)++] = term;

		goto end;
	}

	if (term->type == INCOMPLETE_ABSTRACTION || top->type == INCOMPLETE_ABSTRACTION) {
		// This creates a terms of incomplete abstractions over incomplete abstractions, which will be eventually flattened.

		(*terms)[(*terms_size)++] = term;

		goto end;
	}

	struct LambdaTerm *application;

	application = malloc(sizeof(*application));

	application->type = APPLICATION;

	application->expression.application.argument = term;
	application->expression.application.function = top;

	// Pops the top term off of the terms and pushes the application

	(*terms)[*terms_size - 1] = application;

	end:

	if (*terms_capacity == *terms_size) {
		// Scaling factor of 2

		*terms_capacity <<= 1;

		// Reallocate the terms

		*terms = realloc(*terms, *terms_capacity * sizeof(**terms));
	}

	stack_print(*terms, *terms_size);
}
void terms_bind(struct LambdaTerm **terms, size_t *terms_size, size_t *bound_variables_size)
{
	// This function does not increase terms_size.
	// This function assumes there is a NULL pointer stored down the terms which represents a left parenthesis.
	// This function assumes that terms_size >= 2

	struct LambdaTerm *push = terms[--(*terms_size)];
	struct LambdaTerm *top = terms[--(*terms_size)];

	while (top != NULL) {
		if (top->type == INCOMPLETE_ABSTRACTION) {
			// End of abstraction body reached
			// Changing the type to a normal abstraction

			top->expression.abstraction.body = push;
			top->type = ABSTRACTION;

			push = top;

			top = terms[--(*terms_size)];

			// End of abstraction scope reached. Then, a bound variable is popped.

			(*bound_variables_size)--;

			continue;
		}
		
		struct LambdaTerm *application;

		application = malloc(sizeof(*application));

		application->type = APPLICATION;

		application->expression.application.argument = push;
		application->expression.application.function = top;

		push = application;

		top = terms[--(*terms_size)];
	}

	// Left associativity of application handling after flattening

	if (*terms_size == 0) {
		goto end;
	}

	top = terms[*terms_size - 1];

	if (top != NULL) {
		if (top->type == INCOMPLETE_ABSTRACTION) {
			goto end;
		}

		(*terms_size)--;

		struct LambdaTerm *application;

		application = malloc(sizeof(*application));

		application->type = APPLICATION;

		application->expression.application.argument = push;
		application->expression.application.function = top;

		push = application;
	}

	end:

	terms[(*terms_size)++] = push;

	stack_print(terms, *terms_size);
}

struct Identifier identifier_parse(const char **current, const char *end)
{
	struct Identifier identifier;

	size_t name_length;

	const char *name_begin;
	const char *name_end;

	// Parse name

	*current = skip_whitespace(*current, end);

	name_begin = *current;

	*current = skip_name(*current + 1, end);
	name_end = *current;

	name_length = name_end - name_begin + 1;

	identifier.name = malloc(sizeof(*identifier.name) * name_length);

	strncpy(identifier.name, name_begin, name_length - 1);

	// Insert null terminator

	identifier.name[name_length - 1] = '\0';

	// Parse subscript

	if (char_is_digit(**current)) {
		identifier.subscript = (int)strtol(*current, (char**)current, 10);
	} else {
		identifier.subscript = NO_SUBSCRIPT;
	}

	// Skipping white space before returning

	*current = skip_whitespace(*current, end);

	// Returning

	return identifier;
}

struct LambdaTerm *church_numeral_parse(const char **current, const char *end)
{
	int church_numeral = (int)strtol(*current, (char**)current, 10);

	struct LambdaTerm *term = malloc(sizeof(*term));

	term->type = CHURCH_NUMERAL;
	term->expression.church_numeral = church_numeral;

	*current = skip_whitespace(*current, end);

	return term;
}
struct LambdaTerm *variable_parse(
	struct Identifier **free_variables, size_t *free_variables_size, size_t *free_variables_capacity,
	struct Identifier *bound_variables, size_t bound_variables_size,
	const char **current, const char *end
)
{
	// Parsing variable name
	struct Identifier variable = identifier_parse(current, end);

	// Default type 
	enum ExpressionType type = FREE_VARIABLE;

	// Search for a matching identifier
	// Search for a matching bound variable
	// Search is done from the top of the terms to the bottom, because internal binds precede outer binds 
	// The terms is ordered such that outer bound variables remain at the bottom and inner bound variables at the top
	// For example, \x. \x. x is alpha-equivalent to \x. \y. y, and not \x. \y. x
	
	for (
		struct Identifier *bound_variable = bound_variables + bound_variables_size - 1;
		bound_variable >= bound_variables;
		bound_variable--
	) {
		if (bound_variable->subscript != variable.subscript) {
			continue;
		}

		if (strcmp(variable.name, bound_variable->name) == 0) {
			// Found a matching bound variable
			// Free the string allocated in variable.name

			type = BOUND_VARIABLE;

			free(variable.name);

			variable.name = bound_variable->name;

			// Jump straight to return

			goto end;
		}
	}

	// If no matching bound variable has been found, search for matching free variables

	for (
		struct Identifier *free_variable = *free_variables;
		free_variable < *free_variables + *free_variables_size;
		free_variable++
	) {
		if (free_variable->subscript != variable.subscript) {
			continue;
		}

		if (strcmp(variable.name, free_variable->name) == 0) {
			// Found a matching bound variable
			// Free the string allocated in variable.name

			type = BOUND_VARIABLE;

			free(variable.name);

			variable.name = free_variable->name;

			// Jump straight to return

			goto end;
		}
	}

	// If no match has been found, push a new free variable

	// Scaling the array to insert new member, if necessary

	if (*free_variables_capacity == *free_variables_size) {
		*free_variables_capacity <<= 1;

		*free_variables = realloc(*free_variables, *free_variables_capacity);
	}

	(*free_variables)[(*free_variables_size)++] = variable;

	end:

	struct LambdaTerm *term;

	term = malloc(sizeof(*term));

	term->type = type;
	term->expression.variable = variable;

	return term;
}
struct LambdaTerm *abstraction_parse(
	struct Identifier **bound_variables, size_t *bound_variables_size, size_t *bound_variables_capacity,
	const char **current, const char *end
)
{
	// This function is called once a backslash or a lambda operator has been encountered.
	// This is why one is added to (*current) before calling indentifier_parse()

	*current = skip_whitespace(*current + 1, end);

	struct Identifier bound_variable = identifier_parse(current, end);

	struct LambdaTerm *term;

	// Abstractions are initially initialized as INCOMPLETE_ABSTRACTIONS.
	// Once terms_bind() is called, it turns into a complete abstraction.
	// The abstraction body will be set during binding.

	term = malloc(sizeof(*term));

	term->type = INCOMPLETE_ABSTRACTION;

	term->expression.abstraction.bound_variable = bound_variable;
	term->expression.abstraction.body = NULL;

	// Scaling the array to fit a new member if necessary

	if (*bound_variables_capacity == *bound_variables_size) {
		// Scaling factor of 2
		*bound_variables_capacity <<= 1;

		*bound_variables = realloc(*bound_variables, sizeof(**bound_variables) * *bound_variables_capacity);
	}

	// Pushing the identifier pointer to the top of the bound variables array

	(*bound_variables)[(*bound_variables_size)++] = bound_variable;

	// Skipping the dot

	*current = skip_whitespace(*current, end) + 1;
	*current = skip_whitespace(*current, end);

	// Returning

	return term;
}


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
const char *skip_digits(const char *str, const char *end)
{
	while (char_is_digit(*str) && (end > str)) {
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
	const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	const char *it = table;

	while (*it != '\0') {
		if (*it++ == c)
			return 1;
	}

	return 0;
}
int char_is_digit(char c)
{
	const char *table = "0123456789";

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