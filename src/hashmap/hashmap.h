#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>

typedef struct hashmap hashmap_t;
typedef bool (*hashmap_callback_t)(const char *key, void *value, void *ctx);

hashmap_t *hashmap_create(void);
void *hashmap_get(const hashmap_t *map, const char *key);
int hashmap_set(hashmap_t *map, const char *key, void *value);
void hashmap_delete(hashmap_t *map, const char *key, void (*free_value)(void *value));
bool hashmap_for_each(hashmap_t *map, hashmap_callback_t callback, void *ctx);
void hashmap_destroy(hashmap_t *map, void (*free_value)(void *value));

#endif
