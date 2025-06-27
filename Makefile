CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC_DIR = src
OBJ_DIR = build
BIN = $(OBJ_DIR)/app

# Lista de arquivos .c no diretório src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regra padrão
.PHONY: clean $(OBJ_DIR) $(BIN)

# Regra de linkagem final (agora com -lm!)
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lm

# Compilação dos .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -rf $(OBJ_DIR)

# Execução automática com arquivo de teste
run:
	./$(BIN) ./tests/test.csv ./profiles/gravatai.json

all: clean $(OBJ_DIR) $(BIN) run