# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude

# Directories
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin

# Detect OS
ifeq ($(OS),Windows_NT)
	SUBDIR    := windows
	EXEEXT    := .exe
else
	SUBDIR    := linux
	EXEEXT    :=
endif

SEP       := /
OBJ_SUB   := $(OBJ_DIR)/$(SUBDIR)
BIN       := $(OBJ_SUB)/app$(EXEEXT)

# Sources and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_SUB)/%.o, $(SRCS))

.PHONY: all

# Default target
all: $(BIN)

# Link final binary
$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Compile each object file
$(OBJ_SUB)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean only objects and binary for current OS
clean:
	rm -rf $(OBJ_SUB) $(BIN)
