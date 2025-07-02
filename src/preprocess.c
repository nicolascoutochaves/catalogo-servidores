#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess.h"
#include "file_utils.h"


// Sanitiza linha: converte não-ASCII em '?' pulando bytes conforme encoding
void sanitize_line(const char* src, char* dst, size_t maxlen, const char* encoding) {
    size_t i = 0, j = 0;
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
    if (!f) {
        perror("Error opening copied file");
        return 0;
    }

    char raw[LINE_BUF], sanitized[LINE_BUF];
    long write_pos = 0;         // posição para sobrescrever
  

    while (fgets(raw, sizeof(raw), f)) {
        sanitize_line(raw, sanitized, sizeof(sanitized), encoding);
        if (sanitized[strlen(sanitized) - 1] != '\n') strcat(sanitized, "\n");

        long after_read = ftell(f);

        fseek(f, write_pos, SEEK_SET);
        fputs(sanitized, f);
        write_pos = ftell(f);

        fseek(f, after_read, SEEK_SET);
    }

    fflush(f);
    if (ftruncate(fileno(f), write_pos) != 0) {
        perror("Error truncating file");
        fclose(f);
        return 0;
    }

    fclose(f);
    printf("✔️ File successfully sanitized and overwritten: %s\n", output_file);
    return 1;
}
