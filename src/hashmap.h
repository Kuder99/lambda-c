#pragma once

#include <lambda.h>

// A hashtable implementation to store free variables
// Linear probing is used to handle hash collisions
// The entries array is reallocated once the number of entries grows

struct HashMap {
	struct LambdaHandle *entries;

	size_t size;
	size_t capacity;
};

struct HashMap hashmap_create();		// Create an empty hashmap
void hashmap_destroy(struct HashMap hashmap);	// Deallocate all the memory stored inside the hashmap (including the terms stored inside it)

struct LambdaHandle hashmap_get(struct HashMap hashmap, struct Identifier identifier);		// Acess a term inside the hashmap
int hashmap_set(struct HashMap *hashmap, struct LambdaHandle lambda);				// Store a term inside the hashmap. Returns 0 upon failure and 1 upon success