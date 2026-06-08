#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "kvs.h"
#include "hal.h"

struct kvs {
    hal_t *hal;
};

kvs_t* kvs_create(const char *filename) {
    hal_t *hal = hal_create(filename);
    if (hal == NULL) return NULL;

    kvs_t *kvs = malloc(sizeof(kvs_t));
    if (kvs == NULL) {
        hal_destroy(hal);
        return NULL;
    };

    kvs->hal = hal;

    return kvs;
};

int kvs_set(kvs_t *kvs, const char *key, const char *value) {
    if (kvs == NULL || key == NULL || value == NULL) return -1;
    if (!hal_ready_check(kvs->hal)) return -1;
    hal_delete(kvs->hal, (const uint8_t *)key, strlen(key));
    return hal_write(kvs->hal, (const uint8_t *)key, strlen(key), (const uint8_t *)value, strlen(value));
}

int kvs_get(kvs_t *kvs, const char *key, char *value, uint16_t *value_size) {
    if (kvs == NULL || key == NULL || value == NULL || value_size == NULL) return -1;
    if (!hal_ready_check(kvs->hal)) return -1;
    return hal_read(kvs->hal, (const uint8_t *)key, strlen(key), (uint8_t *)value, value_size);
};

int kvs_delete(kvs_t *kvs, const char *key) {
    if (kvs == NULL || key == NULL) return -1;
    if (!hal_ready_check(kvs->hal)) return -1;
    return hal_delete(kvs->hal, (const uint8_t *)key, strlen(key));
};

void kvs_destroy(kvs_t *kvs) {
    if (kvs != NULL) {
        hal_destroy(kvs->hal);
        free(kvs);
    }
};