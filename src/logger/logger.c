#include "logger.h"
#include "stdlib.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

struct logger
{
    FILE *file;
};

logger_t *logger_create(const char *filename)
{
    logger_t *logger = malloc(sizeof(logger_t));
    if (logger == NULL)
        return NULL;

    FILE *fp = fopen(filename, "a");
    if (fp == NULL)
    {
        free(logger);
        return NULL;
    }

    logger->file = fp;
    return logger;
}

bool logger_write(logger_t *logger, const char sign, const char *message)
{
    if (logger == NULL)
        return false;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time_str = asctime(tm);
    time_str[strlen(time_str) - 1] = '\0';

    char *type = "Unknown";
    switch (sign)
    {
    case 'E':
        type = "Error";
        break;
    case 'W':
        type = "Warning";
        break;
    case 'I':
        type = "Info";
        break;
    default:
        break;
    }

    int result = fprintf(logger->file, "%s - %s: %s", time_str, type, message);
    fflush(logger->file);

    return result >= 0;
}

void logger_destroy(logger_t *logger)
{
    if (logger == NULL)
        return;
    fclose(logger->file);
    free(logger);
}