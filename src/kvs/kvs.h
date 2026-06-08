#ifndef KVS_H
#define KVS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct kvs kvs_t;

kvs_t* kvs_create(const char *filename);
int kvs_set(kvs_t *kvs, const char *key, const char *value);
int kvs_get(kvs_t *kvs, const char *key, char *value, uint16_t *value_size);
int kvs_delete(kvs_t *kvs, const char *key);
void kvs_destroy(kvs_t *kvs);

#endif