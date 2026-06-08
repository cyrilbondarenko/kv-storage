#include "hal.h"
#include "flash.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>

struct hal
{
    flash_t *flash;
    size_t next_page;
    uint32_t wear_count;
};

struct hal_header
{
    uint8_t status;
    size_t pages_count;
    uint8_t key_size;
    uint16_t value_size;
};

uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

hal_t* hal_create(const char *filename) {
    flash_t *flash = flash_create(filename);
    if (flash == NULL) return NULL;

    hal_t *hal = malloc(sizeof(hal_t));
    if (hal == NULL) {
        flash_destroy(flash);
        return NULL;
    };

    hal->flash = flash;
    hal->next_page = 0;
    hal->wear_count = 0;

    uint8_t buffer[PAGE_SIZE];
    for (size_t i = 0; i < TOTAL_PAGES; i++) {
        if (flash_read_page(flash, i, buffer) != 0) break;

        hal_header_t header;
        memcpy(&header, buffer, sizeof(hal_header_t));

        if (header.status == HAL_STATUS_EMPTY) {
            hal->next_page = i;
            break;
        }

        i += header.pages_count - 1;
    }

    return hal;
};

bool hal_ready_check(hal_t *hal) {
    (void)hal;
    return true;
};

bool hal_write_ready_check(hal_t *hal) {
    (void)hal;
    return true;
};

int hal_write(hal_t *hal, const uint8_t *key, uint8_t key_size, const uint8_t *value, uint16_t value_size) {
    if (!hal_write_ready_check(hal)) return -1;

    hal_header_t header;
    header.status = HAL_STATUS_VALID;
    header.pages_count = 1;
    header.key_size = key_size;
    header.value_size = value_size;

    uint8_t buffer[PAGE_SIZE];
    size_t offset = 0;

    memcpy(buffer + offset, &header, sizeof(hal_header_t));
    offset += sizeof(hal_header_t);

    memcpy(buffer + offset, key, key_size);
    offset += key_size;

    memcpy(buffer + offset, value, value_size);
    offset += value_size;

    uint32_t crc = crc32(buffer, offset);

    memcpy(buffer + offset, &crc, sizeof(uint32_t));

    if (flash_write_page(hal->flash, hal->next_page, buffer) != 0) return -1;

    hal->next_page = hal->next_page + 1;

    return 0;
};

int hal_read(hal_t *hal, const uint8_t *key, uint8_t key_size, uint8_t *value, uint16_t *value_size) {
    uint8_t buffer[PAGE_SIZE];
    size_t i = 0;

    while (i < TOTAL_PAGES)
    {
        flash_read_page(hal->flash, i, buffer);

        hal_header_t header;
        size_t offset = 0;

        memcpy(&header, buffer, sizeof(hal_header_t));
        offset += sizeof(hal_header_t);

        uint8_t page_key[header.key_size];
        memcpy(page_key, buffer + offset, header.key_size);
        offset += header.key_size;

        if (header.status == HAL_STATUS_EMPTY) {
            break;
        }

        if (header.status == HAL_STATUS_VALID && header.key_size == key_size && memcmp(key, page_key, key_size) == 0) {
            uint32_t page_crc;
            memcpy(&page_crc, buffer + offset + header.value_size, sizeof(uint32_t));
            uint32_t actual_crc = crc32(buffer, offset + header.value_size);
            if (page_crc != actual_crc) return -1;

            uint8_t page_value[header.value_size];
            memcpy(page_value, buffer + offset, header.value_size);

            *value_size = header.value_size;
            memcpy(value, page_value, header.value_size);
            
            return 0;
        }

        i += header.pages_count;
    }

    return -1;
};

int hal_delete(hal_t *hal, const uint8_t *key, uint8_t key_size) {
    uint8_t buffer[PAGE_SIZE];
    size_t i = 0;

    while (i < TOTAL_PAGES)
    {
        flash_read_page(hal->flash, i, buffer);

        hal_header_t header;
        size_t offset = 0;

        memcpy(&header, buffer, sizeof(hal_header_t));
        offset += sizeof(hal_header_t);

        uint8_t page_key[header.key_size];
        memcpy(page_key, buffer + offset, header.key_size);
        offset += header.key_size;

        if (header.status == HAL_STATUS_EMPTY) {
            break;
        }

        if (header.status == HAL_STATUS_VALID && memcmp(key, page_key, key_size) == 0) {
            header.status = HAL_STATUS_OBSOLETE;
            memcpy(buffer, &header, sizeof(hal_header_t));
            if (flash_write_page(hal->flash, i, buffer) != 0) return -1;
            return 0;
        }

        i += header.pages_count;
    }
    return -1;
};

void hal_destroy(hal_t *hal) {
    if (hal != NULL) {
        flash_destroy(hal->flash);
        free(hal);
    }
};