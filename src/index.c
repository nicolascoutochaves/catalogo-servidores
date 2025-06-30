// index.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "sort_utils.h"
#include "preprocess.h"

// Se o .idx não existir, chama sort_by_field que já:
// 1) lê o binário
// 2) constroi o IndexEntry[] com calloc (zerando tudo)
// 3) ordena
// 4) salva via fwrite()
int build_index(const char *field,
                 const char *bin_filename,
                 const char *index_filename,
                 int descending)
{
    if (file_exists(index_filename)) {
        printf("Index file %s already exists. Skipping creation...\n", index_filename);
        return 1;
    }
    // sort_utils.c -> sort_by_field irá gravar o índice
    sort_by_field(field, descending, bin_filename, index_filename);
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