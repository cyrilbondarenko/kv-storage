#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>

#define FLASH_SIZE (1024 * 1024)
#define PAGE_SIZE 512
#define BLOCK_SIZE (16 * 1024)
#define TOTAL_PAGES ((1024 * 1024) / 512)

typedef struct flash flash_t;

flash_t *flash_create(const char *filename);
int flash_read_page(flash_t *flash, size_t page, unsigned char *buffer);
int flash_write_page(flash_t *flash, size_t page, unsigned char *buffer);
int flash_erase_block(flash_t *flash, size_t block);
void flash_destroy(flash_t *flash);

#endif