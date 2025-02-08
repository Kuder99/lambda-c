#pragma once

#include <stdlib.h>

enum ExpressionType {
	FREE_VARIABLE,
	BOUND_VARIABLE,
	ABSTRACTION,
	APPLICATION,
	INCOMPLETE_ABSTRACTION,
	CHURCH_NUMERAL
};

// Abstract Syntax Tree lambda term data structure

struct Identifier {
	char *name;
	int subscript;
};

struct LambdaTerm {
	enum ExpressionType type;

	union {
		int church_numeral;

		struct Identifier variable;

		struct Abstraction {
			struct Identifier bound_variable;
			struct LambdaTerm *body;
		} abstraction;
		
		struct Application {
			struct LambdaTerm *function;
			struct LambdaTerm *argument;
		} application;
	} expression;
};

// free_variables_array is an array of all terms which aren't bound by an abstraction
// Church numerals aren't stored in free_variables_array

struct LambdaHandle {
	struct LambdaTerm *term;

	struct Identifier identifier;

	struct Identifier *free_variables;

	size_t free_variables_size;
	size_t free_variables_capacity;
};


struct LambdaHandle lambda_parse(const char *expression, const size_t size);	// Wraps a lambda term represented by a string in a AST
void lambda_free(struct LambdaHandle lambda);	// Frees the allocated memory of a lambda term