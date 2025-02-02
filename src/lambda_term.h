#pragma once

#include <stdlib.h>

enum ExpressionType {
	VARIABLE,
	ABSTRACTION,
	APPLICATION,
	INCOMPLETE_ABSTRACTION
};

// Tree lambda term data structure

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
};

struct LambdaHandle {
	struct LambdaTerm *term;
	char *name;
};

int lambda_is_valid(const char *str, const size_t size);

struct LambdaHandle lambda_parse(const char *expression, const size_t size);
void lambda_free(struct LambdaHandle lambda);

void lambda_print(struct LambdaHandle lambda);