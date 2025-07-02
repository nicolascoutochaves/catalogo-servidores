#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "file_utils.h"
#include "search.h"
#include "sort_utils.h"
#include "public_employee.h"
#include "index.h"
#include "stats.h"

#define MAX_FILENAME 256

IndexEntry* load_index(const char* index_filename, size_t* out_count) {
    FILE* f = fopen(index_filename, "rb");
    if (!f) {
        perror("Error opening index file");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);
    size_t count = file_size / sizeof(IndexEntry);
    IndexEntry* arr = malloc(count * sizeof(IndexEntry));
    if (!arr) {
        perror("Memory allocation for index");
        fclose(f);
        return NULL;
    }
    if (fread(arr, sizeof(IndexEntry), count, f) != count) {
        perror("Reading index file");
        free(arr);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_count = count;
    return arr;
}

static int detect_type(const char* s) {
    char* endptr;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 2;
    strtol(s, &endptr, 10);
    if (endptr != s && *endptr == '\0') return 0;
    strtod(s, &endptr);
    if (endptr != s && *endptr == '\0') return 1;
    return 2;
}
void prompt_and_search(char* input_file, char* output_file, int* opt) {
    char line[INPUT_BUF];
    SortKey key;
    KeyType kind;
    double eps = 0.1;  // margem padrão

    printf("Searching in binary file \"%s\".\n", output_file);
    printf("Enter search key (ID, Name, or Salary) [optional eps]: ");

    if (!fgets(line, sizeof(line), stdin)) {
        *opt = -1;
        return;
    }
    line[strcspn(line, "\r\n")] = '\0';

    if (strcmp(line, "-1") == 0 || (line[0] == '\x1B' && line[1] == '\0')) {
        *opt = -1;
        return;
    }

    // Identifica último espaço, se existir
    char* eps_str = strrchr(line, ' ');
    char term[INPUT_BUF];
    if (eps_str && isdigit(eps_str[1])) {
        // Tem eps fornecido: separa
        *eps_str = '\0'; // termina o termo de busca
        eps_str++;
        eps = atof(eps_str);
    }

    // term agora contém só o termo de busca (com espaços, se for nome)
    strncpy(term, line, sizeof(term));
    term[sizeof(term) - 1] = '\0';

    kind = detect_type(term);

    if (kind == KEY_INT && eps_str) {
        // Entrada era algo como: "5000 500" - força interpretação como float
        kind = KEY_FLOAT;
        key.float_key = atof(term);
        printf("→ Interpreted as float key: %.2f with eps = %.2f\n", key.float_key, eps);
    }
    else if (kind == KEY_INT) {
        key.int_key = atoi(term);
        printf("→ Detected integer key: %d\n", key.int_key);
        if (key.int_key == -1) {
            *opt = -1;
            return;
        }
    }
    else if (kind == KEY_FLOAT) {
        key.float_key = atof(term);
        printf("→ Detected float key: %.2f with eps = %.2f\n", key.float_key, eps);
    }
    else {
        strncpy(key.str_key, term, sizeof(key.str_key));
        key.str_key[sizeof(key.str_key) - 1] = '\0';
        for (char* p = key.str_key; *p; ++p)
            *p = toupper((unsigned char)*p);
        printf("→ Detected string key: \"%s\"\n", key.str_key);
    }

    int found = search(input_file, output_file, key, kind, eps);
    if (!found) {
        printf("No matching record found.\n");
    }
}


int search(char* input_file, char* output_file, SortKey key, KeyType type, double eps) {
    char* buffer = malloc(MAX_FILENAME);
    if (!buffer) {
        perror("Memory allocation failed");
        return 0;
    }
    int found = 0;

    if (type == KEY_INT) {
        int id = key.int_key;
        get_index_filename(input_file, "id", buffer);
        printf("Searching by ID: %d\n", id);
        found = search_by_id(output_file, buffer, id);
    }
    else if (type == KEY_FLOAT) {
        double salary = key.float_key;
        printf("Searching by Salary: %.2f\n", salary);

        get_index_filename(input_file, "net_salary", buffer);
        found += search_by_salary(output_file, buffer, salary, eps);

        puts("\n\n\nPRINTING GROSS SALARY\n\n\n");
        get_index_filename(input_file, "gross_salary", buffer);
        found += search_by_salary(output_file, buffer, salary, eps);
    }
    else {
        printf("Searching by Name: %s\n", key.str_key);
        get_index_filename(input_file, "name", buffer);
        found = search_by_name(output_file, buffer, key.str_key);
    }
    free(buffer);
    return found;
}

static int binary_search_id(IndexEntry* arr, size_t count, int id) {
    size_t left = 0, right = count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (arr[mid].key.int_key == id) return (int)mid;
        else if (arr[mid].key.int_key < id) left = mid + 1;
        else right = mid;
    }
    return -1;
}

int search_by_id(const char* bin_filename, const char* index_filename, int id) {
    size_t count;
    IndexEntry* index = load_index(index_filename, &count);
    if (!index) return 0;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    int pos = binary_search_id(index, count, id);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("Time taken to search: %.4f ms\n", elapsed_ms(t1, t2));
    if (pos < 0) {
        free(index);
        return 0;
    }
    FILE* bf = fopen(bin_filename, "rb");
    if (!bf) {
        perror("Error opening binary file");
        free(index);
        return 0;
    }
    fseek(bf, index[pos].offset, SEEK_SET);
    PublicEmployee e;
    fread(&e, sizeof(PublicEmployee), 1, bf);
    print_public_employee(&e);
    fclose(bf);
    free(index);
    return 1;
}

int binary_search_name(IndexEntry* arr, size_t count, const char* name) {
    size_t left = 0, right = count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = strcmp(arr[mid].key.str_key, name);
        if (cmp == 0) return (int)mid;
        else if (cmp < 0) left = mid + 1;
        else right = mid;
    }
    return -1;
}

char* strndup(const char* s, size_t n) {
    char* p = malloc(n + 1);
    if (!p) return NULL;
    strncpy(p, s, n);
    p[n] = '\0';
    return p;
}

int search_by_name(const char* bin_filename, const char* index_filename, const char* name) {
    clock_gettime(CLOCK_MONOTONIC, &t1);

    size_t count;
    IndexEntry* index = load_index(index_filename, &count);
    if (!index) return 0;

    FILE* bf = fopen(bin_filename, "rb");
    if (!bf) {
        perror("Error opening binary file");
        free(index);
        return 0;
    }
    PublicEmployee* e = malloc(sizeof(PublicEmployee) * count);
    if (!e) {
        perror("Error allocating memory for employees in search_by_name");
        fclose(bf);
        free(index);
        return 0;
    }

    int found = 0;
    int page_count = 0;


    // Caso especial: mostrar todos

    if (strcmp(name, "*") == 0 || strcmp(name, "**") == 0) {
        for (size_t i = 0; i < count; i++) {
            fseek(bf, index[i].offset, SEEK_SET);
            PublicEmployee e;
            fread(&e, sizeof(PublicEmployee), 1, bf);
            print_public_employee(&e);
            found++;
            page_count++;
            if (page_count == PAGE_SIZE) {
                if (!ask_continue_pagination()) break;
                page_count = 0;
            }
        }
        fclose(bf);
        free(index);
        return found;
    }

    // wildcards:

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
            const char* entry = index[i].key.str_key;
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
            clock_gettime(CLOCK_MONOTONIC, &t2);

            if (match) {
                fseek(bf, index[i].offset, SEEK_SET);
                fread(&e[found], sizeof(PublicEmployee), 1, bf);
                found++;
                if (found >= MAX_EMPLOYEES) {
                    printf("Maximum number of employees reached (%d). Stopping search.\n", MAX_EMPLOYEES);
                    break;
                }
            }
        }

        if (alloc) free(alloc);
    }
    else {
        int pos = binary_search_name(index, count, name);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        if (pos >= 0) {
            int start = pos;
            while (start > 0 && strcmp(index[start - 1].key.str_key, name) == 0) start--;
            for (size_t i = start; i < count && strcmp(index[i].key.str_key, name) == 0; i++) {
                fseek(bf, index[i].offset, SEEK_SET);
                fread(&e[found], sizeof(PublicEmployee), 1, bf);
                found++;
                if (found >= MAX_EMPLOYEES) {
                    printf("Maximum number of employees reached (%d). Stopping search.\n", MAX_EMPLOYEES);
                    break;
                }
            }
        }
    }
    if (found) {
        for (int i = 0; i < found; i++) {
            print_public_employee(&e[i]);
            page_count++;
            if (page_count == PAGE_SIZE) {
                if (!ask_continue_pagination()) break;
                page_count = 0;
            }
        }
    }
    printf("Time taken to search: %.2f ms\n", elapsed_ms(t1, t2));
    fclose(bf);
    free(index);
    return found;
}


int search_by_salary(const char* bin_filename, const char* index_filename, double salary, double eps) {
    clock_gettime(CLOCK_MONOTONIC, &t1);
    size_t count;
    IndexEntry* index = load_index(index_filename, &count);
    if (!index) return 0;

    PublicEmployee* employees = malloc(count * sizeof(PublicEmployee));
    if (!employees) {
        perror("Memory allocation for employees");
        free(index);
        return 0;
    }


    FILE* bf = fopen(bin_filename, "rb");
    if (!bf) {
        free(index);
        free(employees);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count && found < MAX_EMPLOYEES; i++) {
        if (fabs(index[i].key.float_key - salary) <= eps) {
            fseek(bf, index[i].offset, SEEK_SET);
            fread(&employees[found], sizeof(PublicEmployee), 1, bf);
            found++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    int printed = 0;

    for (int i = 0; i < found; i++) {
        print_public_employee(&employees[i]);
        printed++;
        if (printed == PAGE_SIZE) {
            printed = 0;
            if (!ask_continue_pagination()) break;
        }
    }
    printf("Time taken to search: %.2f ms\n", elapsed_ms(t1, t2));
    printf("Total entries found: %d\n", found);
    fclose(bf);
    free(employees);
    free(index);
    return found;
}
