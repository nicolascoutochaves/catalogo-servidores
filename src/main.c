#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "csv.h"
#include "preprocess.h"
#include "json_profile.h"
#include "sort_utils.h"
#include "index.h"
#include "search.h"

enum {
    OPTION_LOOP = 0,
    OPTION_PREPROCESS,
    OPTION_PROCESS_CSV,
    OPTION_SEARCH,
    OPTION_CREATE_INDEX,
    OPTION_READ_BIN_AND_PRINT,
    OPTION_EXIT
};


struct timespec total_start, total_end;
struct timespec t1, t2;

int select_program(int, char*, char*, char*, char*);
double elapsed_ms(struct timespec start, struct timespec end);

int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif


    if (argc < 3 || argc > 5) {
        fprintf(stderr, "Usage: %s <input_csv> <profile_json> [output_bin] [encoding]\n", argv[0]);
        return 1;
    }

    char* output_file = (argc == 4) ? argv[3] : "bin/public_employees.dat";
    int opt = OPTION_LOOP;
    Profile* profile = load_profile(argv[2]);
    char* encoding = (argc == 5) ? argv[4] : profile->aliases_encoding[0];
    if (argc == 5 && strcmp(encoding, "utf8") && strcmp(encoding, "latin1")) {
        fprintf(stderr, "Invalid encoding. Use 'utf8' or 'latin1'.\n");
        return 1;
    }

    if (!profile) {
        fprintf(stderr, "Error loading profile from %s\n", argv[2]);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &total_start);
    while (select_program(opt, argv[1], argv[2], output_file, encoding) != OPTION_EXIT) {
        
    }
    clock_gettime(CLOCK_MONOTONIC, &total_end);
    printf("Total time taken: %.2f ms\n", elapsed_ms(total_start, total_end));


    return 0;
}

int select_program(int opt, char* input_file, char* profile_path, char* output_file, char* encoding) {
    char filename_preprocessed[MAX_FILENAME];
    strcpy(filename_preprocessed, input_file);
    remove_extension(filename_preprocessed); // Remove a extensão do nome do arquivo de entrada
    strcat(filename_preprocessed, "_preprocessed.csv"); // Adiciona sufixo _preprocessed.csv
    
    printf("1. Preprocess CSV\n");
    printf("2. Process CSV to binary\n");
    printf("3. Create indices\n");
    printf("4. Search by ID or Name\n");
    printf("5. Read binary file and print employees\n");
    printf("6. Exit\n");
    printf("Select an option: ");
    scanf("%d", &opt);

    switch (opt) {


        case OPTION_PREPROCESS:
            printf("Preprocessing file: %s\n", input_file);
            clock_gettime(CLOCK_MONOTONIC, &t1);
           
            if (preprocess(input_file, filename_preprocessed, encoding)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  Preprocessing done successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error during preprocessing.\n");
            }
            break;

        case OPTION_PROCESS_CSV:
            if (!file_exists(filename_preprocessed)) {
                fprintf(stderr, "❌ Preprocessed file does not exist. Please preprocess first.\n");
                return OPTION_LOOP;
            }

            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (process_csv(filename_preprocessed, profile_path, output_file)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  CSV processed successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error during CSV processing.\n");
            }
            break;

        case OPTION_READ_BIN_AND_PRINT:
            printf("Reading binary file \"%s\" and printing employees:\n", output_file);
            if (!file_exists(output_file)) {
                fprintf(stderr, "❌ Binary file does not exist. Please process CSV first.\n");
                return OPTION_LOOP;
            }
            clock_gettime(CLOCK_MONOTONIC, &t1);
            read_binary_file_and_print(output_file);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            break;

        case OPTION_SEARCH:
            if(!file_exists(output_file)) {
                fprintf(stderr, "❌ Binary file does not exist. Please process CSV first.\n");
                return OPTION_LOOP;
            }
            prompt_and_search(input_file, output_file);

            break;

        case OPTION_CREATE_INDEX:
            printf("Creating indices...\n");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (create_index(input_file, output_file)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  Indices created successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error creating indices.\n");
            }
            break;

        case OPTION_CLEAR_INDEX:
            printf("Clearing indices...\n");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (clear_index(output_file)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  Indices cleared successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error clearing indices.\n");
            }
            break;
        case OPTION_CLEAR_BINS:
            printf("Clearing binary files...\n");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (clear_binary(output_file)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  Binary files cleared successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error clearing binary files.\n");
            }
            break;
        case OPTION_CLEAR_PREPROCESSED:
            printf("Clearing preprocessed files...\n");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (clear_preprocessed(filename_preprocessed)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  Preprocessed files cleared successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error clearing preprocessed files.\n");
            }
            break;
        case OPTION_CLEAR_ALL:
            printf("Clearing all files...\n");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            if (clear_all(input_file, output_file, filename_preprocessed)) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                printf("✔️  All files cleared successfully.\n");
                printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
            }
            else {
                fprintf(stderr, "❌ Error clearing all files.\n");
            }
            break;

        case OPTION_EXIT:
            printf("Exiting...\n");
            return OPTION_EXIT;
            break;

        default:
            printf("Invalid option.\n");
            opt = OPTION_LOOP;
        }

    return opt;
}

// função utilitária
double elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
        (end.tv_nsec - start.tv_nsec) / 1e6;
}
