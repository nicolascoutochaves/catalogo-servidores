#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess.h"
#include "cJSON.h"
#include "file_utils.h"


// Sanitiza linha: converte não-ASCII em '?' pulando bytes conforme encoding
void sanitize_line(const char* src, char* dst, size_t maxlen, const char* encoding) {
    size_t i = 0, j = 0;
    printf("Using encoding: %s\n", encoding);
    int is_utf8 = (strcasecmp(encoding, "UTF-8") == 0);
    while (src[i] && j < maxlen - 1) {
        unsigned char c = (unsigned char)src[i];
        if (c <= 127) {
            dst[j++] = src[i++];
        } else {
            dst[j++] = '?';
            if (is_utf8) {
                i++;
                if ((c & 0xE0) == 0xC0) i += 1;
                else if ((c & 0xF0) == 0xE0) i += 2;
                else if ((c & 0xF8) == 0xF0) i += 3;
            } else {
                i++;
            }
        }
    }
    dst[j] = '\0';
}

int preprocess(char* input_file, char* output_file, const char* encoding) {

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

    if (!copy_file_syscall(input_file, output_file)) {
        fprintf(stderr, "Error copying file.\n");
        return 0;
    }

    FILE* f = fopen(output_file, "r+b");
    if (!f) { perror("Error reopening file"); return 0; }

    char linha[LINE_BUF];
    long start_pos = ftell(f);
    long after_header_pos = 0;
    while (fgets(linha, sizeof(linha), f)) {
        if (strchr(linha, ';')) {
            after_header_pos = ftell(f);
            break;
        }
        start_pos = ftell(f);
    }
    if (!after_header_pos) {
        fprintf(stderr, "Header not found.\n");
        fclose(f);
        return 0;
    }

    char sanitized[LINE_BUF];
    sanitize_line(linha, sanitized, sizeof(sanitized), encoding);
    if (sanitized[strlen(sanitized)-1] != '\n') strcat(sanitized, "\n");

    fseek(f, after_header_pos, SEEK_SET);
    fseek(f, 0, SEEK_END);
    long file_end = ftell(f);
    size_t tail_size = file_end - after_header_pos;
    char *tail = malloc(tail_size);
    if (!tail) { perror("Memory error"); fclose(f); return 0; }
    fseek(f, after_header_pos, SEEK_SET);
    fread(tail, 1, tail_size, f);

    fseek(f, start_pos, SEEK_SET);
    fputs(sanitized, f);
    fwrite(tail, 1, tail_size, f);

    // Trunca após escrita
    long final_pos = ftell(f);
    fflush(f);
    if (ftruncate(fileno(f), final_pos) != 0) {
        perror("Error truncating file"); free(tail); fclose(f); return 0;
    }

    free(tail);
    fclose(f);
    printf("File converted successfully: %s\n", output_file);
    return 1;
}
