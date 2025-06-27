#include <stdio.h>
#include <string.h>
#include <time.h> 
#include "csv.h"
#include "preprocess.h"
#include "json_profile.h"

// função utilitária
double elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1e6;
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <input_csv> <profile_json> [output_bin]\n", argv[0]);
        return 1;
    }

    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    struct timespec t1, t2;

    printf("Loading profile: %s\n", argv[2]);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    Profile *profile = load_profile(argv[2]);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("✔️  Profile loaded in %.2f ms\n", elapsed_ms(t1, t2));

    if (!profile) {
        fprintf(stderr, "Error loading JSON profile.\n");
        return 1;
    }

    printf("Using encoding: %s\n", profile->aliases_encoding[0]);

    char filename[256];
    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (!preprocess(argv[1], filename, profile->aliases_encoding[0])) {
        fprintf(stderr, "Error preprocessing CSV.\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("✔️  Preprocessing done in %.2f ms\n\n", elapsed_ms(t1, t2));


    const char *output_file = (argc == 4) ? argv[3] : "bin/public_employees.dat";

    clock_gettime(CLOCK_MONOTONIC, &t1);
    if(process_csv(filename, argv[2], output_file) != 0) {
        fprintf(stderr, "Error processing CSV file.\n");
        free_profile(profile);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("\n✔️  CSV processed in %.2f ms\n", elapsed_ms(t1, t2));

    printf("Reading binary file and printing employees:\n");
    clock_gettime(CLOCK_MONOTONIC, &t1);
    read_binary_file_and_print(output_file);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("✔️  Binary file read in %.2f ms\n", elapsed_ms(t1, t2));

    clock_gettime(CLOCK_MONOTONIC, &total_end);
    printf("✅ Total execution time: %.2f ms\n", elapsed_ms(total_start, total_end));

    return 0;
}
