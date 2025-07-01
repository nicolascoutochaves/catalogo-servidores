#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "sort_utils.h"
#include "file_utils.h"

int build_index(const char *field,
                const char *bin_filename,
                const char *index_filename,
                int descending)
{
    if (file_exists(index_filename)) {
        printf("Index file %s already exists. Skipping creation...\n", index_filename);
        return 1;
    }

    FILE *fp = fopen(bin_filename, "rb");
    if (!fp) {
        perror("Error opening binary file");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    size_t count = filesize / sizeof(PublicEmployee);
    rewind(fp);

    IndexEntry *index = calloc(count, sizeof(IndexEntry));
    if (!index) {
        perror("calloc");
        fclose(fp);
        return 0;
    }

    for (size_t i = 0; i < count; i++) {
        long offset = ftell(fp);
        PublicEmployee e;
        fread(&e, sizeof(PublicEmployee), 1, fp);
        index[i].offset = offset;

        if (strcmp(field, "id") == 0)
            index[i].key.int_key = e.id;
        else if (strcmp(field, "net_salary") == 0)
            index[i].key.float_key = e.net_salary;
        else if (strcmp(field, "gross_salary") == 0)
            index[i].key.float_key = e.gross_salary;
        else if (strcmp(field, "name") == 0) {
            strncpy(index[i].key.str_key, e.name, sizeof(index[i].key.str_key));
            index[i].key.str_key[sizeof(index[i].key.str_key) - 1] = '\0';
        }
    }

    fclose(fp);

    // Ordena os índices
    if (strcmp(field, "id") == 0) {
        if (descending) radix_sort_int_desc(index, count);
        else            radix_sort_int(index, count);
    }
    else if (!strcmp(field, "net_salary") || !strcmp(field, "gross_salary")) {
        qsort_float(index, count, descending);
    }
    else if (!strcmp(field, "name")) {
        radix_sort_str(index, count, 256, descending);
    }

    // Salva o índice no arquivo .idx
    FILE *out = fopen(index_filename, "wb");
    if (!out) {
        perror("fopen index");
        free(index);
        return 0;
    }

    fwrite(index, sizeof(IndexEntry), count, out);
    fclose(out);
    free(index);

    printf("✔️ Index for field '%s' saved in: %s\n", field, index_filename);
    return 1;
}



void get_index_filename(char* filename, const char* field, char* index_filename) {
    snprintf(index_filename, MAX_FILENAME, "index/%s", filename);
    remove_extension(index_filename); // Remove a extensão do nome do arquivo de entrada
    strcat(index_filename, "_");
    strcat(index_filename, field); // Adiciona o campo como sufixo
    strcat(index_filename, ".idx\0"); // Adiciona a extensão .idx
}

int create_index(char* filename, const char* output_file) {
    
    char *id = malloc(MAX_FILENAME);
    char *name = malloc(MAX_FILENAME);
    char *gross = malloc(MAX_FILENAME);
    char *net = malloc(MAX_FILENAME);

    get_index_filename(filename, "id", id);
    get_index_filename(filename, "name", name);
    get_index_filename(filename, "gross_salary", gross);
    get_index_filename(filename, "net_salary", net);

    puts("Building indices...");

    return (build_index("id", output_file, id, 0) &&
        build_index("name", output_file, name, 0) &&
        build_index("gross_salary", output_file, gross, 0) &&
        build_index("net_salary", output_file, net, 0));

}