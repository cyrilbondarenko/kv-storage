#include <stdio.h>
#include "flash.h"

int flash_init(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp != NULL) {
        fclose(fp);
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return -1;
    }

    for (size_t i = 0; i < FLASH_SIZE; i++) {
        fputc(0xFF, fp);
    }

    fclose(fp);
    return 0;
}

int flash_read_page(const char *filename, size_t page, unsigned char* buffer)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;
    
    size_t offset = page * PAGE_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) return -1;

    size_t read = fread(buffer, 1, PAGE_SIZE, fp);
    if (read != PAGE_SIZE) return -1;

    fclose(fp);
    return 0;
}

int flash_write_page(const char *filename, size_t page, unsigned char *buffer)
{
    FILE *fp = fopen(filename, "rb+");
    if (!fp) return -1;
    
    size_t offset = page * PAGE_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) return -1;

    size_t written = fwrite(buffer, 1, PAGE_SIZE, fp);
    if (written != PAGE_SIZE) {
        return -1;
    }

    fclose(fp);
    return 0;
}

int flash_erase_block(const char *filename, size_t block)
{
    FILE *fp = fopen(filename, "rb+");
    if (!fp) return -1;
    
    size_t offset = block * BLOCK_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) return -1;

    unsigned char buffer[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        buffer[i] = 0xFF;
    }
    
    size_t written = fwrite(buffer, 1, BLOCK_SIZE, fp);
    if (written != BLOCK_SIZE) {
        return -1;
    }

    fclose(fp);
    return 0;
}