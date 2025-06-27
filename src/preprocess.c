
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h>  // para ftruncate
#include <fcntl.h>   // para fileno

#include "preprocess.h"
#include "cJSON.h" 

#define LINE_BUF 8192

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0; 
}


int copy_file_fopen(const char* src, const char* dst) {
    FILE* fin = fopen(src, "rb");
    FILE* fout = fopen(dst, "wb");
    if (!fin || !fout) {
        perror("Error copying file");
        return 0;
    }

    char buffer[8192];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
        fwrite(buffer, 1, n, fout);
    }

    fclose(fin);
    fclose(fout);
    return 1;
}

void sanitize_line(const char* src, char* dst, size_t maxlen, const char* encoding) {
    size_t i = 0, j = 0;

    int is_utf8 = (strcmp(encoding, "UTF-8") == 0 || strcmp(encoding, "utf-8") == 0);

    while (src[i] && j < maxlen - 1) {
        unsigned char c = (unsigned char)src[i];

        if (c <= 127) {
            dst[j++] = src[i++];
        } else {
            dst[j++] = '?';

            if (is_utf8) {
                i++; // já leu o primeiro byte
                if ((c & 0xE0) == 0xC0) i += 1;
                else if ((c & 0xF0) == 0xE0) i += 2;
                else if ((c & 0xF8) == 0xF0) i += 3;
            } else {
                i++; // ISO-8859-1 ou outro 1-byte encoding: só pula 1
            }
        }
    }

    dst[j] = '\0';
}

int preprocess(char* input_file, char* output_file, const char* encoding) {


    // Gerar nome do arquivo de saída
    snprintf(output_file, MAX_FILENAME, "%s", input_file);
    for (int i = 0; i < MAX_FILENAME - 4; i++) {
        if (strncmp(output_file + i, ".csv", 4) == 0) {
            output_file[i] = '\0';
            break;
        }
    }
    strcat(output_file, "_preprocessed.csv");

    if (file_exists(output_file)) {
        printf("Preprocessed file already exists. Skipping creation...\n");
        return 1;
    }

    // 1. Copiar arquivo inteiro
    if (!copy_file_fopen(input_file, output_file)) {
        fprintf(stderr, "Error copying file.\n");
        return 0;
    }

    // 2. Reabrir a cópia com leitura/escrita binária
    FILE* f = fopen(output_file, "r+b");
    if (!f) {
        perror("Error reopening file for editing");
        return 0;
    }

    // 3. Localizar linha de cabeçalho
    char linha[LINE_BUF];
    long inicio = ftell(f);
    long fim = 0;

    while (fgets(linha, sizeof(linha), f)) {
        if (strchr(linha, ';')) {
            puts("Separating character found");
            fim = ftell(f); // posição após a linha
            break;
        }
        inicio = ftell(f);
    }

    if (fim == 0) {
        fprintf(stderr, "Header not found during preprocessing.\n");
        fclose(f);
        return 0;
    }

    // 4. Sanitizar a linha de cabeçalho
    char sanitized[LINE_BUF];
    sanitize_line(linha, sanitized, sizeof(sanitized), encoding);
    if (sanitized[strlen(sanitized) - 1] != '\n') strcat(sanitized, "\n");

    // 5. Mover conteúdo restante para memória
    fseek(f, fim, SEEK_SET);
    long after_header_pos = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);

    size_t tail_size = end - after_header_pos;
    char* tail = malloc(tail_size);
    if (!tail) {
        perror("Memory allocation error");
        fclose(f);
        return 0;
    }

    fseek(f, after_header_pos, SEEK_SET);
    fread(tail, 1, tail_size, f);

    // 6. Voltar e sobrescrever tudo corretamente
    fseek(f, inicio, SEEK_SET);
    fputs(sanitized, f);
    fwrite(tail, 1, tail_size, f);

    // 7. Truncar o arquivo se necessário
    long final_size = ftell(f);
    fflush(f);
    int fd = fileno(f);
    if (ftruncate(fd, final_size) != 0) {
        perror("Error truncating file");
        free(tail);
        fclose(f);
        return 0;
    }

    free(tail);
    fclose(f);
    printf("File converted successfully: ");
    printf("%s\n", output_file);
    return 1;
}
