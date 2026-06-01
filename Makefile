CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRC = src/main.c src/flash/flash.c
TARGET = kvs_demo

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) storage/flash.bin