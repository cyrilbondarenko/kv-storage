CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wformat=2 -Wsign-conversion -Wcast-align -std=c11
INCLUDES = -Isrc/flash -Isrc/hal -Isrc/logger -Isrc/kvs

SRC = src/main.c src/flash/flash.c src/hal/hal.c src/logger/logger.c src/kvs/kvs.c
TARGET = kvs_demo

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET)

coverage:
	$(CC) $(CFLAGS) --coverage $(INCLUDES) $(SRC) -o $(TARGET)
	./$(TARGET)
	lcov --capture --directory . --output-file coverage.info --no-external
	genhtml coverage.info --output-directory coverage_html
	rm -f *.gcda *.gcno

clean:
	rm -f $(TARGET)
	rm -f coverage.info *.gcda *.gcno
	rm -rf coverage_html