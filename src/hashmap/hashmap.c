#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

#define HASHMAP_BUCKET_COUNT 256
#define HASHMAP_KEY_MAX 256

typedef struct hashmap_entry hashmap_entry_t;
struct hashmap_entry
{
    char key[HASHMAP_KEY_MAX];
    void *value;
    hashmap_entry_t *next;
};

struct hashmap
{
    hashmap_entry_t *buckets[HASHMAP_BUCKET_COUNT];
};

static uint32_t hash_str(const char *str)
{
    uint32_t hash = 2166136261u; /* FNV-1a offset */
    while (*str != '\0')
    {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u; /* FNV prime */
    }
    return hash;
}

static hashmap_entry_t *find_entry(hashmap_entry_t *entry, const char *key)
{
    while (entry != NULL && strcmp(entry->key, key) != 0)
        entry = entry->next;
    return entry;
}

hashmap_t *hashmap_create(void)
{
    return calloc(1, sizeof(hashmap_t));
}

void *hashmap_get(const hashmap_t *map, const char *key)
{
    hashmap_entry_t *entry = find_entry(map->buckets[hash_str(key) % HASHMAP_BUCKET_COUNT], key);
    return entry != NULL ? entry->value : NULL;
}

int hashmap_set(hashmap_t *map, const char *key, void *value)
{
    hashmap_entry_t **bucket = &map->buckets[hash_str(key) % HASHMAP_BUCKET_COUNT];
    hashmap_entry_t *entry = find_entry(*bucket, key);

    if (entry == NULL)
    {
        entry = malloc(sizeof(hashmap_entry_t));
        if (entry == NULL)
            return -1;
        strncpy(entry->key, key, sizeof(entry->key) - 1);
        entry->key[sizeof(entry->key) - 1] = '\0';
        entry->next = *bucket;
        *bucket = entry;
    }
    entry->value = value;
    return 0;
}

void hashmap_delete(hashmap_t *map, const char *key, void (*free_value)(void *value))
{
    hashmap_entry_t **bucket = &map->buckets[hash_str(key) % HASHMAP_BUCKET_COUNT];
    hashmap_entry_t *entry = *bucket;
    hashmap_entry_t *prev = NULL;
    while (entry != NULL && strcmp(entry->key, key) != 0)
    {
        prev = entry;
        entry = entry->next;
    }
    if (entry == NULL)
        return;

    if (prev == NULL)
        *bucket = entry->next;
    else
        prev->next = entry->next;

    if (free_value != NULL)
        free_value(entry->value);
    free(entry);
}

bool hashmap_for_each(hashmap_t *map, hashmap_callback_t callback, void *ctx)
{
    for (size_t i = 0; i < HASHMAP_BUCKET_COUNT; i++)
    {
        hashmap_entry_t *entry = map->buckets[i];
        while (entry != NULL)
        {
            hashmap_entry_t *next = entry->next;
            if (!callback(entry->key, entry->value, ctx))
                return false;
            entry = next;
        }
    }
    return true;
}

void hashmap_destroy(hashmap_t *map, void (*free_value)(void *value))
{
    if (map == NULL)
        return;

    for (size_t i = 0; i < HASHMAP_BUCKET_COUNT; i++)
    {
        hashmap_entry_t *entry = map->buckets[i];
        while (entry != NULL)
        {
            hashmap_entry_t *next = entry->next;
            if (free_value != NULL)
                free_value(entry->value);
            free(entry);
            entry = next;
        }
    }
    free(map);
}
