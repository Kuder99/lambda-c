#include <hashmap.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 16

static uint64_t hash_key(struct Identifier identifier);
static int identifier_comparison(struct Identifier left, struct Identifier right);

static void hashmap_scale(struct HashMap *hashmap);

struct HashMap hashmap_create()
{
	struct HashMap hashmap;

	hashmap.capacity = INITIAL_SIZE;
	hashmap.size = 0;

	// The function calloc() is used to initialize all pointers to NULL

	hashmap.entries = calloc(hashmap.capacity, sizeof(*hashmap.entries));

	if (hashmap.entries == NULL) {
		goto fatal_error;
	}

	return hashmap;
	
	fatal_error:

	printf("Fatal error: calloc() returned NULL in function hashmap_create().\n");

	exit(1);
}

void hashmap_destroy(struct HashMap hashmap)
{
	if (hashmap.entries == NULL) {
		return;
	}

	for (size_t index = 0; index < hashmap.capacity; index++) {
		lambda_free(hashmap.entries[index]);
	}

	free(hashmap.entries);
}

struct LambdaHandle hashmap_get(struct HashMap hashmap, struct Identifier identifier)
{
	uint64_t hash = hash_key(identifier);
	size_t index = hash % hashmap.capacity;

	// Linear probing

	struct LambdaHandle lambda = hashmap.entries[index];

	while (lambda.identifier.name != NULL) {
		if (identifier_comparison(lambda.identifier, identifier)) {
			return lambda;
		}

		index++;

		if (index == hashmap.capacity) {
			index = 0;
		}

		lambda = hashmap.entries[index];
	}

	// If no matching member is found, return an empty handle

	return (struct LambdaHandle){0};
}

int hashmap_set(struct HashMap *hashmap, struct LambdaHandle lambda)
{
	if (hashmap == NULL) {
		return 0;
	}

	uint64_t hash = hash_key(lambda.identifier);
	size_t index = hash % hashmap->capacity;

	if (hashmap->capacity == hashmap->size) {
		// Scaling the hashmap to fit new members
		hashmap_scale(hashmap);
	}

	struct LambdaHandle entry = hashmap->entries[index];

	while (entry.identifier.name != NULL) {
		if (identifier_comparison(entry.identifier, lambda.identifier)) {
			// Overwritting the current entry
			lambda_free(entry);
			hashmap->size--;
	
			break;
		}

		index++;

		if (index == hashmap->capacity) {
			index = 0;
		}

		entry = hashmap->entries[index];
	}

	hashmap->entries[index] = lambda;
	hashmap->size++;
	
	return 1;
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

uint64_t hash_key(struct Identifier identifier)
{
	uint64_t hash = FNV_OFFSET;

	for (const char *c = identifier.name; *c != '\0'; c++) {
		hash ^= (uint64_t)(unsigned char)*c;
		hash *= FNV_PRIME;
	}

	hash = 31 * hash + (uint64_t)identifier.subscript;

	return hash;
}

void hashmap_scale(struct HashMap *hashmap)
{
	size_t new_capacity = hashmap->capacity << 1;

	struct LambdaHandle *new_entries;

	new_entries = calloc(new_capacity, sizeof(*new_entries));

	if (new_entries == NULL) {
		goto fatal_error;
	}

	for (uint64_t index = 0; index < hashmap->capacity; index++) {
		struct LambdaHandle lambda = hashmap->entries[index];

		if (lambda.identifier.name == NULL) {
			continue;
		}

		uint64_t hash = hash_key(lambda.identifier);
		size_t new_index = hash % new_capacity;

		new_entries[new_index] = lambda;
	}

	free(hashmap->entries);
	
	hashmap->entries = new_entries;
	hashmap->capacity = new_capacity;

	return;

	fatal_error:

	printf("Fatal error: calloc() returned NULL in function hashmap_scale.");
	exit(1);
}

int identifier_comparison(struct Identifier left, struct Identifier right)
{
	if (left.name == NULL || right.name == NULL) {
		return 0;
	}

	if (left.subscript != right.subscript) {
		return 0;
	}

	if (strcmp(left.name, right.name) == 0) {
		return 1;
	}

	return 0;
}