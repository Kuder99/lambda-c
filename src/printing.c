#include <printing.h>
#include <stdio.h>

enum PrintSymbol {
	EMPTY,
	RIGHT_PARENTHESIS,
	LEFT_PARENTHESIS,
	SPACE
};

static void symbol_print(enum PrintSymbol symbol);

static void church_numeral_print(struct LambdaTerm *term);
static void variable_print(struct LambdaTerm *term);
static void abstraction_print(struct LambdaTerm *term);

static void application_symbols_push(struct LambdaTerm *function, struct LambdaTerm *argument, enum PrintSymbol *symbols, size_t *symbols_size);

void lambda_print(struct LambdaHandle lambda)
{
	// Minimal parenthesis printing

	if (lambda.term == NULL)
		return;
	
	// Terms stack

	struct LambdaTerm **terms;

	size_t terms_size = 1;
	size_t terms_capacity = 8;

	terms = malloc(sizeof(*terms) * terms_capacity);
	terms[0] = lambda.term;

	// Symbols stack

	enum PrintSymbol *symbols;

	size_t symbols_size = 1;
	size_t symbols_capacity = 8;

	symbols = malloc(sizeof(*symbols) * symbols_capacity);
	symbols[0] = EMPTY;

	// Traversing the AST

	while (terms_size > 0) {
		enum PrintSymbol symbol;

		while (symbols_size > 0) {
			// Popping the topmost symbol off the stack and printing it

			symbol = symbols[--symbols_size];
			symbol_print(symbol);

			if (symbol != RIGHT_PARENTHESIS) {
				// Once all right parenthesis are closed, go on
				break;
			}
		}

		struct LambdaTerm *term = terms[--terms_size];

		switch (term->type) {
		case CHURCH_NUMERAL:
			church_numeral_print(term);

			break;

		case FREE_VARIABLE:
		case BOUND_VARIABLE:
			variable_print(term);
			
			break;
		
		case ABSTRACTION:
			abstraction_print(term);

			symbols[symbols_size++] = EMPTY;
			terms[terms_size++] = term->expression.abstraction.body;

			break;
		
		case APPLICATION:
			struct LambdaTerm *function = term->expression.application.function;
			struct LambdaTerm *argument = term->expression.application.argument;

			terms[terms_size++] = argument;
			terms[terms_size++] = function;

			application_symbols_push(function, argument, symbols, &symbols_size);

			break;
		}

		// Scaling arrays

		if (terms_capacity - terms_size <= 1) {
			terms_capacity <<= 1;

			terms = realloc(terms, sizeof(*terms) * terms_capacity);
		}

		if (symbols_capacity - symbols_size <= 3) {
			symbols_capacity <<= 1;

			symbols = realloc(symbols, sizeof(*symbols) * symbols_capacity);
		}
	}

	// Printing remaining symbols

	while (symbols_size > 0) {
		enum PrintSymbol symbol;

		symbol = symbols[--symbols_size];

		symbol_print(symbol);
	}

	// Freeing memory before returning

	free(symbols);
	free(terms);
}

void application_symbols_push(struct LambdaTerm *function, struct LambdaTerm *argument, enum PrintSymbol *symbols, size_t *symbols_size)
{
	switch (argument->type) {
	case CHURCH_NUMERAL:
	case FREE_VARIABLE:
	case BOUND_VARIABLE:
		symbols[(*symbols_size)++] = SPACE;
		break;

	case ABSTRACTION:
		symbols[(*symbols_size)++] = SPACE;
		break;
				
	case APPLICATION:
		symbols[(*symbols_size)++] = RIGHT_PARENTHESIS;
		symbols[(*symbols_size)++] = LEFT_PARENTHESIS;
		
		break;
	}

	switch (function->type) {
	case CHURCH_NUMERAL:
	case FREE_VARIABLE:
	case BOUND_VARIABLE:
		symbols[(*symbols_size)++] = EMPTY;
		break;

	case ABSTRACTION:
		symbols[(*symbols_size)++] = RIGHT_PARENTHESIS;
		symbols[(*symbols_size)++] = LEFT_PARENTHESIS;
		break;

	case APPLICATION:
		if (function->expression.application.argument->type == ABSTRACTION) {
			symbols[(*symbols_size)++] = RIGHT_PARENTHESIS;
			symbols[(*symbols_size)++] = LEFT_PARENTHESIS;
		} else {
			symbols[(*symbols_size)++] = EMPTY;
		}

		break;
	}
}

void symbol_print(enum PrintSymbol symbol)
{
	switch (symbol) {
	case EMPTY:
		break;

	case RIGHT_PARENTHESIS:
		putchar(')');
		break;

	case LEFT_PARENTHESIS:
		putchar('(');
		break;

	case SPACE:
		putchar(' ');
		break;
	}
}

void church_numeral_print(struct LambdaTerm *term)
{
	int church_numeral = term->expression.church_numeral;

	printf("%d", church_numeral);
}

void variable_print(struct LambdaTerm *term)
{
	struct Identifier variable = term->expression.variable;

	if (variable.subscript < 0) {
		printf("%s", variable.name);
	} else {
		printf("%s%d", variable.name, variable.subscript);
	}
}

void abstraction_print(struct LambdaTerm *term)
{
	struct Identifier bound_variable = term->expression.abstraction.bound_variable;

	if (bound_variable.subscript < 0) {
		printf("λ%s.", bound_variable.name);
	} else {
		printf("λ%s%d.", bound_variable.name, bound_variable.subscript);
	}
}