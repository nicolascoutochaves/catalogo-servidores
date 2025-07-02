#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

#endif

#include "csv.h"
#include "preprocess.h"
#include "json_profile.h"
#include "sort_utils.h"
#include "index.h"
#include "search.h"
#include "stats.h"
#include "file_utils.h"

enum {
    OPTION_LOOP = 0,
    OPTION_PREPROCESS = 1,
    OPTION_PROCESS_CSV,
    OPTION_CREATE_INDEX,
    OPTION_SEARCH,
    OPTION_READ_BIN_AND_PRINT,
    OPTION_SORT_BY_FIELD,
    OPTION_PRINT_MAX_SALARY,
    OPTION_PRINT_MIN_SALARY,
    OPTION_DATA_ANALYSIS,
    OPTION_CLEAR_INDEX,
    OPTION_CLEAR_PREPROCESSED,
    OPTION_CLEAR_BIN,
    OPTION_CLEAR_ALL,
    OPTION_READ_NEW_DATA,
    OPTION_EXIT
};


struct timespec total_start, total_end;


int select_program(int opt, char* input_file, char* profile_path, char* output_file, char* encoding);
double elapsed_ms(struct timespec start, struct timespec end);
int is_valid_encoding(char* encoding);

int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif


    if (argc < 3 || argc > 5) {
        fprintf(stderr, "Usage: %s <input_csv> <profile_json> [output_bin] [encoding]\n", argv[0]);
        return 1;
    }

    char input_file_buf[256];
    char profile_path_buf[256];
    char output_file_buf[256];
    char encoding_buf[16];
    strcpy(input_file_buf, argv[1]);
    strcpy(profile_path_buf, argv[2]);
    strcpy(output_file_buf, (argc >= 4) ? argv[3] : "bin/public_employees.dat");
    if (argc == 5) {
        strcpy(encoding_buf, argv[4]);
        if (is_valid_encoding(encoding_buf)) {
            printf("Encoding getted by terminal: %s", encoding_buf);
        }
        else {
            fprintf(stderr, "Invalid encoding. Use 'utf8' or 'latin1'.\n");
            return 1;
        }
    }
    else {

        Profile* profile = load_profile(argv[2]);
        if (!profile) {
            fprintf(stderr, "Error loading profile from %s\n", argv[2]);
            return 1;
        }
        strcpy(encoding_buf, profile->aliases_encoding[0]);
        printf("Using encoding from profile: %s\n", encoding_buf);


    }

    printf("\n\nCATALOGO DE SERVIDORES PUBLICOS\nPara consultar campos com acentuação, use '?' no lugar do acento.\n(para nomes de funcionários geralmente os órgãos governamentais omitem acentos)\n\n\n");


    int opt = OPTION_LOOP;

    clock_gettime(CLOCK_MONOTONIC, &total_start);
    while (select_program(opt, input_file_buf, profile_path_buf, output_file_buf, encoding_buf) != OPTION_EXIT);
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
    printf("4. Search by ID, Name or Salary\n");
    printf("5. Read binary file and print employees\n");
    printf("6. Sort by field\n");
    printf("7. Print max salary\n");
    printf("8. Print min salary\n");
    printf("9. Data analysis by class\n");
    printf("10. Clear indices\n");
    printf("11. Clear preprocessed files\n");
    printf("12. Clear binary files\n");
    printf("13. Clear all files\n");
    printf("14. Read new data\n");
    printf("15. Exit\n");
    printf("Select an option: ");
    
    char input[MAX_FIELD_LEN];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "❌ Error reading input.\n");
        return OPTION_LOOP;
    }
    input[strlen(input) - 1] = '\0'; // Remove newline character

    opt = atoi(input);


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
            return OPTION_LOOP;
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

    case OPTION_SEARCH:
        if (!file_exists(output_file)) {
            fprintf(stderr, "❌ Binary file does not exist. Please process CSV first.\n");
            return OPTION_LOOP;
        }
        while (opt != -1) {
            prompt_and_search(input_file, output_file, &opt);
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

    case OPTION_SORT_BY_FIELD:
        printf("Insert the field to sort by (ID, Name, Gross_Salary, Department, Position, Net_Salary):\n");
        char field[MAX_FIELD_LEN];
        if ((fgets(field, sizeof(field), stdin)) == NULL) {
            fprintf(stderr, "❌ Error reading field input.\n");
            return OPTION_LOOP;
        }
        field[strlen(field) - 1] = '\0'; // Remove newline character

        for (int i = 0; field[i]; i++) {
            field[i] = tolower(field[i]); // Convert to lowercase
        }

        printf("Select sorting order (1 for ascending, 0 for descending):\n");
        int descending;
        if(!fgets(input, sizeof(input), stdin)) {
            fprintf(stderr, "❌ Error reading sorting order input.\n");
            return OPTION_LOOP;
        }

        input[strlen(input) - 1] = '\0'; // Remove newline character
        descending = atoi(input);

        if (descending != 0 && descending != 1) {
            fprintf(stderr, "❌ Invalid input for sorting order. Use 1 for ascending or 0 for descending.\n");
            return OPTION_LOOP;
        }

        printf("\n\nSorting by field: %s\n\n", field);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        sort_by_field(field, descending, output_file, input_file);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Sorting completed.\n");
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_PRINT_MAX_SALARY:
        printf("Printing max salary from binary file: %s\n", output_file);
        if (!file_exists(output_file)) {
            fprintf(stderr, "❌ Binary file does not exist. Please process CSV first.\n");
            return OPTION_LOOP;
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        print_max_salary(input_file, output_file, "net_salary");
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_PRINT_MIN_SALARY:
        printf("Printing min salary from binary file: %s\n", output_file);
        if (!file_exists(output_file)) {
            fprintf(stderr, "❌ Binary file does not exist. Please process CSV first.\n");
            return OPTION_LOOP;
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        print_min_salary(input_file, output_file, "net_salary");
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_DATA_ANALYSIS:

        int data_analysis_option;
        printf("Select the field to analyze (1 for Position, 2 for Department):\n");
        if (scanf("%d", &data_analysis_option) != 1 || (data_analysis_option != 1 && data_analysis_option != 2)) {
            fprintf(stderr, "❌ Invalid input for data analysis option. Use 1 for Position or 2 for Department.\n");
            return OPTION_LOOP;
        }

        printf("Insert the class to search in the field:\n");
        char class_name[MAX_FIELD_LEN];
        while (getchar() != '\n'); // Clear input buffer
        if ((fgets(class_name, sizeof(class_name), stdin)) == NULL) {
            fprintf(stderr, "❌ Error reading class name input.\n");
            return OPTION_LOOP;
        }
        class_name[strlen(class_name) - 1] = '\0'; // Remove newline character
    

        for (int i = 0; class_name[i]; i++) {
            class_name[i] = toupper(class_name[i]); // Convert to uppercase
        }
        if (data_analysis_option == 1)
            data_analisys_by_class("position", class_name, input_file, output_file);
        else
            data_analisys_by_class("department", class_name, input_file, output_file);
        break;

    case OPTION_CLEAR_INDEX:
        printf("Clearing indices...\n");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Deleted %d index files.\n", delete_index_files());
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_CLEAR_PREPROCESSED:
        printf("Clearing preprocessed files...\n");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Deleted %d preprocessed files.\n", delete_preprocessed_csv());
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_CLEAR_BIN:
        printf("Clearing binary files...\n");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Deleted %d binary files.\n", delete_binary_files());
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_CLEAR_ALL:
        printf("Clearing all files...\n");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("Deleted %d files.\n", delete_all_files());
        clock_gettime(CLOCK_MONOTONIC, &t2);
        printf("Time taken: %.2f ms\n", elapsed_ms(t1, t2));
        break;

    case OPTION_READ_NEW_DATA:

        printf("\nInsert the new paths: <input_csv> <profile_json> [output_bin] [encoding]\n'0' or none for default output bin and encoding.\n\n");
        printf("example: data/servidores.csv profile/profile.json bin/public_employees.dat utf8\nor:      data/servidores.csv profile/profile.json 0 0\n\n");

        char temp[1024];
        while (getchar() != '\n'); // Clear input buffer
        fgets(temp, sizeof(temp), stdin);
        temp[strcspn(temp, "\n")] = '\0';

        char tmp_enconding[16];
        strcpy(tmp_enconding, encoding); // Default encoding

        int parsed = sscanf(temp, "%255s %255s %255s %15s", input_file, profile_path, output_file, tmp_enconding);

        if (parsed < 3 || strcmp(output_file, "0") == 0) {
            strcpy(output_file, "bin/public_employees.dat");
        }
        if (parsed < 4 || strcmp(tmp_enconding, "0") == 0) {
            Profile *profile = load_profile(profile_path);
            strcpy(tmp_enconding, profile->aliases_encoding[0]);
            printf("Using encoding from profile: %s\n", tmp_enconding);
        }   
        strcpy(encoding, tmp_enconding);

        printf("New paths set:\n");
        printf("Input CSV: %s\n", input_file);
        printf("Profile JSON: %s\n", profile_path);
        printf("Output Binary: %s\n", output_file);
        printf("Encoding: %s\n", encoding);
        return OPTION_LOOP;

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
int is_valid_encoding(char* encoding) {
    if (strcmp(encoding, "utf8") == 0) {
        strcpy(encoding, "UTF-8");
    }
    else if (strcmp(encoding, "latin1") == 0) {
        strcpy(encoding, "ISO-8859-1");
    }
    else {
        encoding[0] = '\0'; // Invalid encoding
    }

    return (!strcmp(encoding, "UTF-8") || !strcmp(encoding, "ISO-8859-1"));
}
