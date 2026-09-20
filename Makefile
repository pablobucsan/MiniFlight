
CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -g -O0
CPPFLAGS = -Iinclude
LDFLAGS  =
LDLIBS   =

BUILD_DIR = build
TARGET    = $(BUILD_DIR)/program

SRCS = $(shell find src -name '*.c')
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean directories

all: directories $(TARGET)

directories:
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
