#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "kvs.h"
#include "hal.h"
#include "uthash.h"

typedef enum
{
    CACHE_CLEAN,
    CACHE_DIRTY,
    CACHE_DELETED
} cache_state_t;

typedef struct
{
    char key[256];
    uint8_t *value;
    uint16_t value_size;
    cache_state_t state;
    UT_hash_handle hh;
} cache_entry_t;

static cache_entry_t *cache_get(cache_entry_t *cache, const char *key)
{
    cache_entry_t *entry;
    HASH_FIND_STR(cache, key, entry);
    return entry;
}

static int cache_set(cache_entry_t **cache, const char *key, const uint8_t *value, uint16_t value_size)
{
    cache_entry_t *entry;
    HASH_FIND_STR(*cache, key, entry);
    if (entry == NULL)
    {
        entry = malloc(sizeof(cache_entry_t));
        if (entry == NULL)
            return -1;
        strncpy(entry->key, key, sizeof(entry->key) - 1);
        entry->key[sizeof(entry->key) - 1] = '\0';
        entry->value = NULL;
        HASH_ADD_STR(*cache, key, entry);
    }
    else
    {
        free(entry->value);
    }
    entry->value = malloc(value_size);
    if (entry->value == NULL)
    {
        HASH_DEL(*cache, entry);
        free(entry);
        return -1;
    }
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    entry->state = CACHE_CLEAN;
    return 0;
}

static void cache_delete(cache_entry_t **cache, const char *key)
{
    cache_entry_t *entry;
    HASH_FIND_STR(*cache, key, entry);
    if (entry != NULL)
    {
        HASH_DEL(*cache, entry);
        free(entry->value);
        free(entry);
    }
}

static void cache_clear(cache_entry_t **cache)
{
    cache_entry_t *entry, *tmp;
    HASH_ITER(hh, *cache, entry, tmp)
    {
        HASH_DEL(*cache, entry);
        free(entry->value);
        free(entry);
    }
}

static void cache_load_entry(const uint8_t *key, uint8_t key_size, const uint8_t *value, uint16_t value_size, void *ctx)
{
    char key_str[256];
    memcpy(key_str, key, key_size);
    key_str[key_size] = '\0';
    cache_set((cache_entry_t **)ctx, key_str, value, value_size);
}

struct kvs
{
    hal_t *hal;
    cache_entry_t *cache;
};

kvs_t *kvs_create(const char *filename)
{
    hal_t *hal = hal_create(filename);
    if (hal == NULL)
        return NULL;

    kvs_t *kvs = malloc(sizeof(kvs_t));
    if (kvs == NULL)
    {
        hal_destroy(hal);
        return NULL;
    };

    kvs->hal = hal;
    kvs->cache = NULL;
    hal_iterate(hal, cache_load_entry, &kvs->cache);

    return kvs;
}

int kvs_set(kvs_t *kvs, const char *key, const char *value)
{
    if (kvs == NULL || key == NULL || value == NULL)
        return -1;

    size_t key_size = strlen(key);
    if (key_size == 0 || key_size > UINT8_MAX)
        return -1;

    int result = cache_set(&kvs->cache, key, (const uint8_t *)value, (uint16_t)strlen(value));
    if (result == 0)
    {
        cache_entry_t *entry = cache_get(kvs->cache, key);
        entry->state = CACHE_DIRTY;
    }
    return result;
}

int kvs_get(kvs_t *kvs, const char *key, char *value, uint16_t *value_size)
{
    if (kvs == NULL || key == NULL || value == NULL || value_size == NULL)
        return -1;

    size_t key_size = strlen(key);
    if (key_size == 0 || key_size > UINT8_MAX)
        return -1;

    cache_entry_t *entry = cache_get(kvs->cache, key);
    if (entry == NULL || entry->state == CACHE_DELETED)
        return -1;
    *value_size = entry->value_size;
    memcpy(value, entry->value, entry->value_size);
    return 0;
}

int kvs_delete(kvs_t *kvs, const char *key)
{
    if (kvs == NULL || key == NULL)
        return -1;

    size_t key_size = strlen(key);
    if (key_size == 0 || key_size > UINT8_MAX)
        return -1;

    cache_entry_t *entry = cache_get(kvs->cache, key);
    if (entry == NULL || entry->state == CACHE_DELETED)
        return 0;
    entry->state = CACHE_DELETED;

    return 0;
}

void kvs_simulate_power_loss(kvs_t *kvs, size_t power_loss_after)
{
    hal_simulate_power_loss(kvs->hal, power_loss_after);
}

int kvs_sync(kvs_t *kvs)
{
    if (kvs == NULL)
        return -1;

    if (!hal_ready_check(kvs->hal))
        return -1;

    cache_entry_t *entry, *tmp;
    HASH_ITER(hh, kvs->cache, entry, tmp)
    {
        switch (entry->state)
        {
        case CACHE_DELETED:
            hal_delete(kvs->hal, (const uint8_t *)entry->key, (uint8_t)strlen(entry->key));
            cache_delete(&kvs->cache, entry->key);
            break;

        case CACHE_DIRTY:
            hal_delete(kvs->hal, (const uint8_t *)entry->key, (uint8_t)strlen(entry->key));
            if (hal_write(kvs->hal, (const uint8_t *)entry->key, (uint8_t)strlen(entry->key), entry->value, entry->value_size) != 0)
                return -1;
            entry->state = CACHE_CLEAN;
            break;

        default:
            break;
        }
    }

    return 0;
}

void kvs_destroy(kvs_t *kvs)
{
    if (kvs != NULL)
    {
        hal_destroy(kvs->hal);
        cache_clear(&kvs->cache);
        free(kvs);
    }
}