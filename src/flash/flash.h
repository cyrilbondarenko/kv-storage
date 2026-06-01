#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>

static const size_t FLASH_SIZE = 1024 * 1024;
static const size_t PAGE_SIZE = 512;
static const size_t BLOCK_SIZE = 16 * 1024;

int flash_init(const char *filename);
int flash_read_page(const char *filename, size_t page, unsigned char* buffer);
int flash_write_page(const char *filename, size_t page, unsigned char *buffer);
int flash_erase_block(const char *filename, size_t block);

#endif