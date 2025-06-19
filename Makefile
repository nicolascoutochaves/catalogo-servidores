CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC_DIR = src
OBJ_DIR = build
BIN = $(OBJ_DIR)/app

# Lista de arquivos .c no diretório src
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Gera os nomes dos arquivos .o correspondentes no diretório build
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


# Regra de ligação final
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN)

# Regra para compilar arquivos .c em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos binários e objetos
clean:
	rm -rf $(OBJ_DIR)

run:
	./$(BIN) ./tests/test.csv
.PHONY: all clean

# Regra padrão
all: clean $(BIN) run