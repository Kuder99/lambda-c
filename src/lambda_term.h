#pragma once

#include <stdlib.h>

enum ExpressionType {
	VARIABLE,
	ABSTRACTION,
	APPLICATION,
	INCOMPLETE_ABSTRACTION
};

// Abstract Syntax Tree lambda term data structure

struct LambdaTerm {
	enum ExpressionType type;

	union {
		char *variable;
		struct {
			char *argument;
			struct LambdaTerm *body;
		} abstraction;
		struct {
			struct LambdaTerm *function;
			struct LambdaTerm *argument;
		} application;
	} expression;

	int subscript;
};

struct LambdaHandle {
	struct LambdaTerm *term;
	char *name;
};

int lambda_is_valid(const char *str, const size_t size);	// Determines whether a string represents a valid lambda term

struct LambdaHandle lambda_parse(const char *expression, const size_t size);	// Wraps a lambda term represented by a string in a AST
void lambda_free(struct LambdaHandle lambda);	// Frees the allocated memory of a lambda term

void lambda_print(struct LambdaHandle lambda);	// Pretty-printing for a lambda term