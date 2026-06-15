#include "flash.h"
#include "stdlib.h"
#include <stdio.h>

struct flash
{
    FILE *file;
};

flash_t* flash_create(const char *filename)
{
    flash_t *flash = malloc(sizeof(flash_t));
    if (flash == NULL) return NULL;
    FILE *fp = fopen(filename, "rb+");

    if (fp != NULL) {
        flash->file = fp;
        return flash;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        free(flash);
        return NULL;
    }

    for (size_t i = 0; i < FLASH_SIZE; i++) {
        fputc(0xFF, fp);
    }

    fclose(fp);
    fp = fopen(filename, "rb+");
    if (fp == NULL) {
        free(flash);
        return NULL;
    }

    flash->file = fp;
    return flash;
}

int flash_read_page(flash_t *flash, size_t page, unsigned char* buffer)
{   
    size_t offset = page * PAGE_SIZE;
    if (fseek(flash->file, (long)offset, SEEK_SET) != 0) return -1;

    size_t read = fread(buffer, 1, PAGE_SIZE, flash->file);
    if (read != PAGE_SIZE) return -1;

    return 0;
}

int flash_write_page(flash_t *flash, size_t page, unsigned char *buffer)
{
    size_t offset = page * PAGE_SIZE;
    if (fseek(flash->file, (long)offset, SEEK_SET) != 0) return -1;

    size_t written = fwrite(buffer, 1, PAGE_SIZE, flash->file);
    if (written != PAGE_SIZE) {
        return -1;
    }

    return 0;
}

int flash_erase_block(flash_t *flash, size_t block)
{
    size_t offset = block * BLOCK_SIZE;
    if (fseek(flash->file, (long)offset, SEEK_SET) != 0) return -1;

    unsigned char buffer[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        buffer[i] = 0xFF;
    }
    
    size_t written = fwrite(buffer, 1, BLOCK_SIZE, flash->file);
    if (written != BLOCK_SIZE) {
        return -1;
    }

    return 0;
}

void flash_destroy(flash_t *flash) {
    if (flash == NULL) return;
    fclose(flash->file);
    free(flash);
}