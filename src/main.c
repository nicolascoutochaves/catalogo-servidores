#include <stdio.h>
#include <string.h>
#include "csv.h"
#include "preprocess.h"
#include "json_profile.h"

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Uso: %s <input_csv> <perfil_json> [output_bin]\n", argv[0]);
        return 1;
    }
    
    printf("Carregando perfil JSON: %s\n", argv[2]);
    Profile *profile = load_profile(argv[2]);
    if (!profile) {
        fprintf(stderr, "Erro ao carregar perfil JSON.\n");
        return 1;
    }
    char filename[256];
    printf("Usando encoding: %s\n", profile->aliases_encoding[0]);
    
    if (!preprocess(argv[1], filename, profile->aliases_encoding[0])) {
        fprintf(stderr, "Erro no pré-processamento do CSV.\n");
        return 1;
    }


    const char *profile_json = argv[2];
    const char *output_file = (argc == 4) ? argv[3] : "bin/public_employees.dat";

    printf("Arquivo convertido com sucesso.\n");
    printf("Reading CSV: %s\n", filename);
    printf("Saving binary to: %s\n", output_file);

    int result = read_csv_and_print(filename, profile_json);
    if (result != 0) {
        fprintf(stderr, "Erro ao processar o arquivo.\n");
        return 1;
    }

    // Futuro: save_to_binary(employees, output_file);

    return 0;
}
