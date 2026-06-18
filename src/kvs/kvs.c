#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "kvs.h"
#include "hal.h"
#include "hashmap.h"

typedef enum
{
    CACHE_CLEAN,
    CACHE_DIRTY,
    CACHE_DELETED
} cache_state_t;

typedef struct
{
    uint8_t *value;
    uint16_t value_size;
    cache_state_t state;
} cache_entry_t;

struct kvs
{
    hal_t *hal;
    hashmap_t *cache;
};

static void cache_free(void *value)
{
    cache_entry_t *entry = value;
    free(entry->value);
    free(entry);
}

static void cache_load_entry(const uint8_t *key, uint8_t key_size, const uint8_t *value, uint16_t value_size, void *ctx)
{
    char key_str[256];
    memcpy(key_str, key, key_size);
    key_str[key_size] = '\0';

    cache_entry_t *entry = malloc(sizeof(cache_entry_t));
    if (entry == NULL)
        return;
    entry->value = malloc(value_size);
    if (entry->value == NULL)
    {
        free(entry);
        return;
    }
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    entry->state = CACHE_CLEAN;

    if (hashmap_set((hashmap_t *)ctx, key_str, entry) != 0)
        cache_free(entry);
}

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
    }

    kvs->cache = hashmap_create();
    if (kvs->cache == NULL)
    {
        hal_destroy(hal);
        free(kvs);
        return NULL;
    }

    kvs->hal = hal;
    hal_iterate(hal, cache_load_entry, kvs->cache);

    return kvs;
}

int kvs_set(kvs_t *kvs, const char *key, const char *value)
{
    if (kvs == NULL || key == NULL || value == NULL)
        return -1;

    size_t key_size = strlen(key);
    if (key_size == 0 || key_size > UINT8_MAX)
        return -1;

    uint16_t value_size = (uint16_t)strlen(value);
    uint8_t *value_copy = malloc(value_size);
    if (value_copy == NULL)
        return -1;
    memcpy(value_copy, value, value_size);

    cache_entry_t *entry = hashmap_get(kvs->cache, key);
    if (entry == NULL)
    {
        entry = malloc(sizeof(cache_entry_t));
        if (entry == NULL)
        {
            free(value_copy);
            return -1;
        }
        if (hashmap_set(kvs->cache, key, entry) != 0)
        {
            free(value_copy);
            free(entry);
            return -1;
        }
    }
    else
    {
        free(entry->value);
    }

    entry->value = value_copy;
    entry->value_size = value_size;
    entry->state = CACHE_DIRTY;
    return 0;
}

int kvs_get(kvs_t *kvs, const char *key, char *value, uint16_t *value_size)
{
    if (kvs == NULL || key == NULL || value == NULL || value_size == NULL)
        return -1;

    size_t key_size = strlen(key);
    if (key_size == 0 || key_size > UINT8_MAX)
        return -1;

    cache_entry_t *entry = hashmap_get(kvs->cache, key);
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

    cache_entry_t *entry = hashmap_get(kvs->cache, key);
    if (entry == NULL || entry->state == CACHE_DELETED)
        return 0;
    entry->state = CACHE_DELETED;

    return 0;
}

void kvs_simulate_power_loss(kvs_t *kvs, size_t power_loss_after)
{
    hal_simulate_power_loss(kvs->hal, power_loss_after);
}

static bool sync_entry(const char *key, void *value, void *ctx)
{
    kvs_t *kvs = ctx;
    cache_entry_t *entry = value;

    switch (entry->state)
    {
    case CACHE_DELETED:
        hal_delete(kvs->hal, (const uint8_t *)key, (uint8_t)strlen(key));
        hashmap_delete(kvs->cache, key, cache_free);
        break;

    case CACHE_DIRTY:
        hal_delete(kvs->hal, (const uint8_t *)key, (uint8_t)strlen(key));
        if (hal_write(kvs->hal, (const uint8_t *)key, (uint8_t)strlen(key), entry->value, entry->value_size) != 0)
            return false;
        entry->state = CACHE_CLEAN;
        break;

    default:
        break;
    }
    return true;
}

int kvs_sync(kvs_t *kvs)
{
    if (kvs == NULL)
        return -1;

    if (!hal_ready_check(kvs->hal))
        return -1;

    return hashmap_for_each(kvs->cache, sync_entry, kvs) ? 0 : -1;
}

void kvs_destroy(kvs_t *kvs)
{
    if (kvs != NULL)
    {
        hal_destroy(kvs->hal);
        hashmap_destroy(kvs->cache, cache_free);
        free(kvs);
    }
}
