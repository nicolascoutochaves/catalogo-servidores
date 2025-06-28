# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude

# Directories
SRC_DIR := src
OBJ_DIR := build

# Detect OS
ifeq ($(OS),Windows_NT)
    EXEEXT    := .exe
    RM        := rm -rf
    LINK_LIBS := -lmingw32 -lmingwex
    SEP       := /
else
    EXEEXT    :=
    RM        := rm -rf
    LINK_LIBS :=
    SEP       := /
endif

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Final binary
BIN := $(OBJ_DIR)/app$(EXEEXT)

.PHONY: all clean run

# Default target
all: $(BIN)

# Link
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lm $(LINK_LIBS)

# Compile objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	$(RM) bin/*

reset:
	$(RM) data/*preprocessed.csv bin/*

# Run with sample arguments
run: clean all
	@echo "Running with sample CSV and profile..."
	$(BIN) tests$(SEP)test.csv profiles$(SEP)gravatai.json
