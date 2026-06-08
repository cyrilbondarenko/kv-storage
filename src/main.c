#include <stdio.h>
#include <string.h>
#include "kvs/kvs.h"
#include "logger/logger.h"

static logger_t *logger;

void test_set(kvs_t *kvs, const char *key, const char *value) {
    if (kvs_set(kvs, key, value) == 0)
        logger_write(logger, 'I', "SET OK\n");
    else
        logger_write(logger, 'E', "SET FAILED\n");
}

void test_get(kvs_t *kvs, const char *key, const char *expected) {
    char buf[256];
    uint16_t size = 0;

    if (kvs_get(kvs, key, buf, &size) != 0) {
        logger_write(logger, 'E', "GET FAILED\n");
        return;
    }

    buf[size] = '\0';

    if (expected != NULL && strcmp(buf, expected) != 0)
        logger_write(logger, 'E', "GET MISMATCH\n");
    else
        logger_write(logger, 'I', "GET OK\n");
}

void test_delete(kvs_t *kvs, const char *key) {
    if (kvs_delete(kvs, key) == 0)
        logger_write(logger, 'I', "DELETE OK\n");
    else
        logger_write(logger, 'E', "DELETE FAILED\n");
}

int main() {
    logger = logger_create("logs/log.txt");
    if (logger == NULL) {
        printf("Failed to create logger\n");
        return -1;
    }

    kvs_t *kvs = kvs_create("storage/flash.bin");
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to create KVS\n");
        logger_destroy(logger);
        return -1;
    }

    logger_write(logger, 'I', "[1] Basic write and read\n");
    test_set(kvs, "name", "cyril");
    test_get(kvs, "name", "cyril");

    logger_write(logger, 'I', "[2] Multiple keys\n");
    test_set(kvs, "city", "voronezh");
    test_set(kvs, "lang", "c");
    test_get(kvs, "city", "voronezh");
    test_get(kvs, "lang", "c");

    logger_write(logger, 'I', "[3] Rewriting\n");
    test_set(kvs, "name", "updated");
    test_get(kvs, "name", "updated");

    logger_write(logger, 'I', "[4] Delete\n");
    test_delete(kvs, "city");
    test_get(kvs, "city", NULL); // expected error

    logger_write(logger, 'I', "[5] Read nonexistent key\n");
    test_get(kvs, "ghost", NULL); // expected error

    logger_write(logger, 'I', "[6] Persistence\n");
    kvs_destroy(kvs);
    kvs = kvs_create("storage/flash.bin");
    if (kvs == NULL) {
        logger_write(logger, 'E', "Failed to reopen KVS\n");
        logger_destroy(logger);
        return -1;
    }
    test_get(kvs, "name", "updated");
    test_get(kvs, "lang", "c");

    kvs_destroy(kvs);
    logger_destroy(logger);
    return 0;
}