#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "stats.h"
#include "sort_utils.h"
#include "index.h"
#include "file_utils.h"
#include "public_employee.h"
#include "search.h"

const GaussianRange ranges[] = {
    {"30%", 0.39}, {"50%", 0.67}, {"80%", 1.28}, {"90%", 1.64}, {"95%", 1.96}
};
const int range_count = sizeof(ranges) / sizeof(ranges[0]);

void calculate_stats(double* data, int size, Stats* stats) {
    double sum = 0.0, sq = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i]; sq += data[i] * data[i];
    }
    stats->mean = sum / size;
    stats->variance = (sq / size) - (stats->mean * stats->mean);
    stats->standard_deviation = sqrt(stats->variance);
}

void print_stats(const Stats* s) {
    printf("Mean: %.2f\nVariance: %.2f\nStandard Deviation: %.2f\n",
        s->mean, s->variance, s->standard_deviation);
}

void set_gaussian_parameters(Gaussian* g, const Stats* s) {
    g->mu = s->mean;
    g->sigma = s->standard_deviation;
}

double calculate_gaussian(const Gaussian* g) {
    double z = (g->x - g->mu) / g->sigma;
    return (1.0 / (g->sigma * sqrt(2 * M_PI))) * exp(-0.5 * z * z);
}

double normal_cdf(double x, double mu, double sigma) {
    return 0.5 * (1.0 + erf((x - mu) / (sigma * sqrt(2.0))));
}

double p_less(const Gaussian* g, double x) {
    return normal_cdf(x, g->mu, g->sigma);
}

double p_more(const Gaussian* g, double x) {
    return 1.0 - normal_cdf(x, g->mu, g->sigma);
}

double p_interval(const Gaussian* g, double a, double b) {
    return normal_cdf(b, g->mu, g->sigma) - normal_cdf(a, g->mu, g->sigma);
}

void read_binary_file_and_print(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening binary file.\n");
        return;
    }

    int page_count = 0;

    PublicEmployee e;
    while (fread(&e, sizeof(PublicEmployee), 1, fp) == 1) {
        print_public_employee(&e);
        page_count++;
        if (page_count == PAGE_SIZE) {
            if (!ask_continue_pagination()) break;
            page_count = 0;
        }
    }

    fclose(fp);
}


void data_analisys_by_class(char* field, char* data_searching, char* input_file, char* output_file) {
    if (!field || !output_file || !input_file) {
        fprintf(stderr, "Invalid field, output file, or input file.\n");
        return;
    }

    if (strcmp(field, "department") && strcmp(field, "position")) {
        fprintf(stderr, "Invalid field for data analysis. Only 'department' or 'position' are supported.\n");
        return;
    }

    char idxfile[MAX_FILENAME];
    get_index_filename((char*)input_file, field, idxfile);

    size_t count;
    IndexEntry* idx = load_index(idxfile, &count); //Carrega o índice do arquivo .idx (Ja ordenado em build_index)

    if (!idx || count == 0) {
        fprintf(stderr, "Não foi possível carregar índice '%s'\n", idxfile);
        free(idx);
        return;
    }

    FILE* fp = fopen(output_file, "rb");
    if (!fp) { perror("fopen"); return; }


    double* data = calloc(MAX_EMPLOYEES, sizeof(double));
    if (!data) {
        perror("error allocating memory for data in sort_by_field");
        fclose(fp);
        free(idx);
        return;
    }

    int j = 0; // Index for data array
    int found = 0; // Count of found entries
    // wildcards:
    char name[MAX_FIELD_LEN];
    int start = 0; // Start position for searching
    strcpy(name, data_searching);
    PublicEmployee* e = malloc(sizeof(PublicEmployee) * MAX_EMPLOYEES);
    if (!e) {
        perror("error allocating memory for PublicEmployee in data_analisys_by_class");
        fclose(fp);
        free(idx);
        free(data);
        return;
    }

    if (strchr(name, '*')) {
        const char* pattern = name;
        int prefix = 0, suffix = 0;
        char* alloc = NULL;


        size_t len = strlen(name);

        if (len >= 2 && name[0] == '*' && name[len - 1] == '*') {
            alloc = strndup(name + 1, len - 2);
            pattern = alloc;
            prefix = suffix = 1;
        }
        else if (name[0] == '*') {
            pattern = name + 1;
            suffix = 1;
        }
        else if (name[len - 1] == '*') {
            alloc = strndup(name, len - 1);
            pattern = alloc;
            prefix = 1;
        }
        
        for (size_t i = 0; i < count; i++) {
            for(int j = 0; idx[i].key.str_key[j]; j++) {
                idx[i].key.str_key[j] = toupper(idx[i].key.str_key[j]); // Convert to uppercase
            }
            const char* entry = idx[i].key.str_key;
            int match = 0;

            if (prefix && suffix) {
                match = strstr(entry, pattern) != NULL;
            }
            else if (prefix) {
                match = strncmp(entry, pattern, strlen(pattern)) == 0;
            }
            else if (suffix) {
                size_t len_entry = strlen(entry);
                size_t len_pattern = strlen(pattern);
                match = len_entry >= len_pattern &&
                    strcmp(entry + len_entry - len_pattern, pattern) == 0;
            }

            if (match) {
                fseek(fp, idx[i].offset, SEEK_SET);
                fread(&e[j], sizeof(PublicEmployee), 1, fp);
                data[j] = e[j].net_salary; // Store net salary for data analysis
                found++;
                j++;
                if (j >= MAX_EMPLOYEES) {
                    fprintf(stderr, "Warning: Maximum number of employees reached (%d). Data may be incomplete.\n", MAX_EMPLOYEES);
                    break; // Prevent overflow
                }
            }
        }

        if (alloc) free(alloc);
    }
    else {
        int pos = binary_search_name(idx, count, data_searching);
        start = pos;
        if (pos >= 0) {
            while (start > 0 && strcmp(idx[start - 1].key.str_key, data_searching) == 0) start--;
            for (size_t i = start; i < count && !strcmp(idx[i].key.str_key, data_searching) && j < MAX_EMPLOYEES; i++) {
                fseek(fp, idx[i].offset, SEEK_SET);
                fread(&e[j], sizeof(PublicEmployee), 1, fp);
                data[j] = e[j].net_salary; // Store net salary for data analysis
                found++;
                j++;
            }
        }
        else printf("No entries were found");

    }
    if (found) plot_gaussian_terminal(data, j);
    else {
        printf("No entries were found for field '%s' with value '%s'.\n", field, data_searching);
        free(data);
        fclose(fp);
        free(idx);
        free(e);
        return;
    }



    printf("\n\nData analysis completed for field '%s' with value '%s'.\n\n", field, data_searching);


    int printed = 0;
    printf("Do you want to print the employees found? ");
    if (ask_continue_pagination()) {
        {
            for (int i = 0; i < found; i++) {
                print_public_employee(&e[i]);
                printed++;
                if (printed == PAGE_SIZE) {
                    if (!ask_continue_pagination()) break;
                    printed = 0; // Reset printed count for pagination
                }
            }
        }
    }
        free(data);
        fclose(fp);
        free(idx);
        free(e);
        printf("Total entries found: %d\n", found);

    }


    void plot_gaussian_terminal(double* data, int size) {
        if (size <= 0) {
            printf("No data to plot.\n");
            return;
        }

        Stats s;
        Gaussian g;
        calculate_stats(data, size, &s);
        set_gaussian_parameters(&g, &s);

        printf("\nGaussian Distribution (approx):\nMean (µ): %.2f\nStandard Deviation (σ): %.2f\n", g.mu, g.sigma);
        for (int i = 0; i < range_count; i++) {
            double a = g.mu - ranges[i].z * g.sigma;
            double b = g.mu + ranges[i].z * g.sigma;
            printf("%s range: [%.2f ; %.2f]\n", ranges[i].label, a, b);
        }
        double a = 4000, b = 8000;
        printf("P(<= %.2f) = %.2f%%\n", a, p_less(&g, a) * 100);
        printf("P(>  %.2f) = %.2f%%\n", a, p_more(&g, a) * 100);
        printf("P(%.2f - %.2f) = %.2f%%\n", a, b, p_interval(&g, a, b) * 100);
    }
