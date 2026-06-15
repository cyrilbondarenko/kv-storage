#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "kvs/kvs.h"
#include "logger/logger.h"

#define STORAGE_MAIN       "storage/flash.bin"
#define STORAGE_POWER_LOSS "storage/power_loss.bin"
#define STORAGE_CRC        "storage/crc_test.bin"
#define LOG_PATH           "logs/log.txt"

static logger_t *logger;
static char read_buf[65536];

static void test_set(kvs_t *kvs, const char *key, const char *value) {
    if (kvs_set(kvs, key, value) == 0)
        logger_write(logger, 'I', "SET OK\n");
    else
        logger_write(logger, 'E', "SET FAILED\n");
}

static void test_get(kvs_t *kvs, const char *key, const char *expected) {
    uint16_t size = 0;
    int result = kvs_get(kvs, key, read_buf, &size);

    if (result != 0) {
        if (expected == NULL)
            logger_write(logger, 'I', "GET error as expected: OK\n");
        else
            logger_write(logger, 'E', "GET FAILED\n");
        return;
    }

    read_buf[size] = '\0';

    if (expected == NULL)
        logger_write(logger, 'E', "GET should have failed: BAD\n");
    else if (strcmp(read_buf, expected) != 0)
        logger_write(logger, 'E', "GET MISMATCH\n");
    else
        logger_write(logger, 'I', "GET OK\n");
}

static void test_delete(kvs_t *kvs, const char *key) {
    if (kvs_delete(kvs, key) == 0)
        logger_write(logger, 'I', "DELETE OK\n");
    else
        logger_write(logger, 'E', "DELETE FAILED\n");
}

int main(void) {
    mkdir("storage", 0755);
    mkdir("logs", 0755);

    logger = logger_create(LOG_PATH);
    if (logger == NULL) {
        printf("Failed to create logger\n");
        return -1;
    }

    // Test: Happy flow

    logger_write(logger, 'I', "[1] Happy flow\n");
    kvs_t *kvs = kvs_create(STORAGE_MAIN);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to create KVS\n");
        logger_destroy(logger);
        return -1;
    }

    test_set(kvs, "name", "cyril");
    test_get(kvs, "name", "cyril");

    test_set(kvs, "city", "voronezh");
    test_set(kvs, "lang", "c");
    test_get(kvs, "city", "voronezh");
    test_get(kvs, "lang", "c");

    test_set(kvs, "name", "updated");
    test_get(kvs, "name", "updated");

    test_delete(kvs, "city");
    test_get(kvs, "city", NULL);

    test_get(kvs, "ghost", NULL);

    // Test: Value larger than page size

    logger_write(logger, 'I', "[2] Value larger than page size\n");
    
    char value[1000];
    memset(value, 'X', sizeof(value) - 1);
    value[sizeof(value) - 1] = '\0';

    test_set(kvs, "largeval", value);
    test_get(kvs, "largeval", value);

    // Test: Persistence

    logger_write(logger, 'I', "[3] Persistence\n");
    kvs_destroy(kvs);
    kvs = kvs_create(STORAGE_MAIN);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to reopen KVS\n");
        logger_destroy(logger);
        return -1;
    }
    test_get(kvs, "name", "updated");
    test_get(kvs, "lang", "c");
    test_get(kvs, "largeval", value);
    kvs_destroy(kvs);

    // Test: Power loss

    logger_write(logger, 'I', "[4] Power loss\n");
    kvs = kvs_create(STORAGE_POWER_LOSS);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to create KVS for power loss test\n");
        logger_destroy(logger);
        return -1;
    }

    kvs_simulate_power_loss(kvs, 1);

    if (kvs_set(kvs, "powerloss", value) != 0)
        logger_write(logger, 'I', "Write interrupted by power loss: OK\n");
    else
        logger_write(logger, 'E', "Write should have been interrupted: BAD\n");

    kvs_destroy(kvs);

    kvs = kvs_create(STORAGE_POWER_LOSS);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to reopen KVS after power loss\n");
        logger_destroy(logger);
        return -1;
    }
    test_get(kvs, "powerloss", NULL);
    kvs_destroy(kvs);

    // Test: CRC

    logger_write(logger, 'I', "[5] CRC detects external data corruption\n");
    kvs = kvs_create(STORAGE_CRC);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to create KVS for CRC test\n");
        logger_destroy(logger);
        return -1;
    }
    test_set(kvs, "important", "sensitive_data");
    kvs_destroy(kvs);

    FILE *f = fopen(STORAGE_CRC, "rb+");
    if (f != NULL) {
        fseek(f, 40L, SEEK_SET);
        uint8_t garbage = 0xAB;
        fwrite(&garbage, 1, 1, f);
        fclose(f);
    }

    kvs = kvs_create(STORAGE_CRC);
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to reopen KVS for CRC test\n");
        logger_destroy(logger);
        return -1;
    }
    test_get(kvs, "important", NULL);
    kvs_destroy(kvs);

    // Test: GC

    logger_write(logger, 'I', "[6] GC\n");
    kvs = kvs_create("storage/gc_test.bin");
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to create KVS for GC test\n");
        logger_destroy(logger);
        return -1;
    }

    for (int i = 0; i < 700; i++) {
        kvs_set(kvs, "gckey", value);
    }

    test_get(kvs, "gckey", value);
    kvs_destroy(kvs);

    logger_write(logger, 'I', "All tests done\n");
    logger_destroy(logger);
    return 0;
}