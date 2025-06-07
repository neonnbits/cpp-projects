CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/scheduler.c
TARGET = scheduler

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

debug: CFLAGS += -DDEBUG
debug: $(TARGET)

.PHONY: clean debug
