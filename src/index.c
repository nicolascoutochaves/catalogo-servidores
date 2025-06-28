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
void build_index(const char *field,
                 const char *bin_filename,
                 const char *index_filename,
                 int descending)
{
    if (file_exists(index_filename)) {
        printf("Index file already exists. Skipping creation...\n");
        return;
    }
    // sort_utils.c -> sort_by_field irá gravar o índice
    sort_by_field(field, descending, bin_filename, index_filename);
}
