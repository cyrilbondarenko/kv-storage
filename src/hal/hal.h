#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define HAL_STATUS_EMPTY 0xFF
#define HAL_STATUS_VALID 0xFE
#define HAL_STATUS_OBSOLETE 0x00

typedef struct hal hal_t;
typedef struct hal_header hal_header_t;

uint32_t crc32(const uint8_t *data, size_t length);
hal_t* hal_create(const char *filename);
bool hal_ready_check(hal_t *hal);
bool hal_write_ready_check(hal_t *hal);
int hal_write(hal_t *hal, const uint8_t *key, uint8_t key_size, const uint8_t *value, uint16_t value_size);
int hal_read(hal_t *hal, const uint8_t *key, uint8_t key_size, uint8_t *value, uint16_t *value_size);
int hal_delete(hal_t *hal, const uint8_t *key, uint8_t key_size);
void hal_destroy(hal_t *hal);

#endif