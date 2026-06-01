#include <stdio.h>
#include <string.h>
#include "flash/flash.h"

int main()
{
    if (flash_init("storage/flash.bin") != 0) {
        printf("Flash init failed\n");
        return 1;
    }

    printf("[1] Flash initialized\n");

    unsigned char write_buf[PAGE_SIZE];
    memset(write_buf, 'A', PAGE_SIZE);

    flash_write_page("storage/flash.bin", 2, write_buf);
    printf("[2] Page 2 written\n");

    unsigned char read_buf[PAGE_SIZE];
    flash_read_page("storage/flash.bin", 2, read_buf);

    printf("[3] Page 2 read: ");
    for (int i = 0; i < 10; i++) {
        printf("%c ", read_buf[i]);
    }
    printf("...\n");

    flash_erase_block("storage/flash.bin", 0);
    printf("[4] Block 0 erased\n");

    flash_read_page("storage/flash.bin", 2, read_buf);

    printf("[5] After erase: ");
    for (int i = 0; i < 10; i++) {
        printf("%x ", read_buf[i]);
    }
    printf("\n");

    return 0;
}