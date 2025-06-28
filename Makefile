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

.PHONY: all clean reset run

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

# Reset entire build: binaries + preprocessed CSVs
reset:
	rm -rf bin/*.dat data/*preprocessed.csv index/data/*.idx index/tests/*.idx

# Run example
run: clean all
	@echo "Running with sample CSV and profile..."
	$(BIN) tests$(SEP)test.csv profiles$(SEP)gravatai.json

bench:
	gcc -Wall -Iinclude src/search.c src/sort_utils.c src/public_employee.c main_with_index.c -o build/with_index -lm
	gcc -Wall -Iinclude src/sort_utils.c src/public_employee.c main_without_index.c -o build/without_index -lm
	./build/with_index
	./build/without_index

