CC = gcc
CFLAGS = -Wall -Wextra -std=c11
INCLUDES = -Isrc/flash -Isrc/hal -Isrc/logger -Isrc/kvs

SRC = src/main.c src/flash/flash.c src/hal/hal.c src/logger/logger.c src/kvs/kvs.c
TARGET = kvs_demo

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f storage/*
	rm -f logs/*