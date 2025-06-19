#include <stdio.h>
#include <string.h>
#include "csv.h"
#include "preprocess.h"

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input_csv> [output_bin]\n", argv[0]);
        return 1;
    }
    if (preprocess(argv) != 0) {
        fprintf(stderr, "Error preprocessing file.\n");
        return 1;
    }

    char *input_file = argv[1];
    input_file[strlen(input_file) - 4] = '\0'; // remove .csv
    input_file = strcat(input_file, "_preprocessed.csv");

    const char *output_file = (argc == 3) ? argv[2] : "bin/public_employees.dat";

    printf("Reading CSV: %s\n", input_file);
    printf("Saving binary to: %s\n", output_file);

    int result = read_csv_and_print(input_file); // substituiremos por função que salva binário
    if (result != 0) {
        fprintf(stderr, "Error processing file.\n");
        return 1;
    }

    // Futuramente: save_to_binary(employees, output_file);

    return 0;
}
