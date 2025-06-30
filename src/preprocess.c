#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess.h"
#include "cJSON.h"

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>       // for _chsize_s, _fileno
    #define ftruncate _chsize_s
    #define fileno _fileno
#else
    #include <sys/sendfile.h>
    #include <sys/stat.h>
    #include <unistd.h>   // for ftruncate
    #include <fcntl.h>    // for fileno
#endif

#define LINE_BUF 8192

// Verifica se arquivo existe
int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void remove_extension(char* filename) {
    for (size_t i = 0; i < strlen(filename); i++) {
        if (strcmp(&filename[i], ".csv") == 0 ||
            strcmp(&filename[i], ".json") == 0 || strcmp(&filename[i], ".txt") == 0) {
            filename[i] = '\0';
            break;
        }
    }
}

// Cópia rápida usando syscalls (sendfile no Unix, CopyFileA no Windows)
int copy_file_syscall(const char *src, const char *dst) {
#ifdef _WIN32
    if (!CopyFileA(src, dst, FALSE)) {
        DWORD err = GetLastError();
        fprintf(stderr, "CopyFileA failed (error %lu)\n", err);
        return 0;
    }
    return 1;
#else
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0) { perror("open(src)"); return 0; }
    struct stat st;
    if (fstat(in_fd, &st) < 0) { perror("fstat"); close(in_fd); return 0; }
    int out_fd = open(dst, O_WRONLY|O_CREAT|O_TRUNC, st.st_mode);
    if (out_fd < 0) { perror("open(dst)"); close(in_fd); return 0; }

    off_t offset = 0;
    while (offset < st.st_size) {
        ssize_t sent = sendfile(out_fd, in_fd, &offset, st.st_size - offset);
        if (sent <= 0) { perror("sendfile"); close(in_fd); close(out_fd); return 0; }
    }
    close(in_fd);
    close(out_fd);
    return 1;
#endif
}

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
    // Gera nome de saída
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

    // Copia rápida
    if (!copy_file_syscall(input_file, output_file)) {
        fprintf(stderr, "Error copying file.\n");
        return 0;
    }

    // Abre para editar cabeçalho
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

    // Sanitiza e garante termina em nova linha
    char sanitized[LINE_BUF];
    sanitize_line(linha, sanitized, sizeof(sanitized), encoding);
    if (sanitized[strlen(sanitized)-1] != '\n') strcat(sanitized, "\n");

    // Lê resto do arquivo em memória
    fseek(f, after_header_pos, SEEK_SET);
    fseek(f, 0, SEEK_END);
    long file_end = ftell(f);
    size_t tail_size = file_end - after_header_pos;
    char *tail = malloc(tail_size);
    if (!tail) { perror("Memory error"); fclose(f); return 0; }
    fseek(f, after_header_pos, SEEK_SET);
    fread(tail, 1, tail_size, f);

    // Sobrepoe cabeçalho + resto
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
