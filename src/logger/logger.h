#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

typedef struct logger logger_t;

logger_t* logger_create(const char *filename);
bool logger_write(logger_t *logger, const char sign, const char* message);
void logger_destroy(logger_t *logger);

#endif