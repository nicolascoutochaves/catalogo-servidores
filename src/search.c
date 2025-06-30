// search.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "search.h"
#include "sort_utils.h"
#include "public_employee.h"
#include "index.h"

#define MAX_FILENAME 256

// Helper: load index file
IndexEntry *load_index(const char *index_filename, size_t *out_count) {
    FILE *f = fopen(index_filename, "rb");
    if (!f) {
        perror("Error opening index file");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);
    size_t count = file_size / sizeof(IndexEntry);
    IndexEntry *arr = malloc(count * sizeof(IndexEntry));
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



// Tenta detectar se 's' é inteiro, double ou string
// Retorna 0=int, 1=double, 2=string
static int detect_type(const char *s) {
    char *endptr;

    // pula espaços iniciais
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 2;

    // tenta inteiro
    long li = strtol(s, &endptr, 10);
    if (endptr != s && *endptr == '\0') {
        return 0;
    }
    // tenta double
    double d = strtod(s, &endptr);
    if (endptr != s && *endptr == '\0') {
        (void)d;
        return 1;
    }
    return 2;
}


void prompt_and_search(char *input_file, char *output_file) {
    char line[INPUT_BUF];
    SortKey key;
    KeyType kind;

    printf("Searching in binary file \"%s\".\n", output_file);
    printf("Enter search key (ID, Name, or Salary): ");
    fflush(stdin);
    if (!fgets(line, sizeof(line), stdin)) return;
    // remove newline
    line[strcspn(line, "\r\n")] = '\0';

    kind = detect_type(line);
    if (kind == 0) {
        // inteiro
        key.int_key = atoi(line);
        printf("→ Detected integer key: %d\n", key.int_key);
    }
    else if (kind == 1) {
        // double
        key.float_key = atof(line);
        printf("→ Detected float key: %.2f\n", key.float_key);
    }
    else {
        // string
        strncpy(key.str_key, line, sizeof(key.str_key));
        key.str_key[sizeof(key.str_key)-1] = '\0';
        printf("→ Detected string key: \"%s\"\n", key.str_key);
        //Converte para maiusculas
       for (char *p = key.str_key; *p; ++p) *p = toupper((unsigned char)*p);
    }

    // e agora chama a busca
    int found = search(input_file, output_file, key, kind);
    if (!found) {
        printf("No matching record found.\n");
    }
}

 int search(char *input_file, char *output_file, SortKey key, KeyType type) {

    char *buffer = malloc(MAX_FILENAME);
    if (!buffer) {
        perror("Memory allocation failed");
        return 0;
    }
    int found = 0;
    //Compare the key and verify if its a string, int or float
    if(type == KEY_INT) {
        // Search by ID
        int id = key.int_key;
        get_index_filename(input_file, "id", buffer);
        puts("\n\nBUFFER:\n");
        puts(buffer);
        puts("\n\n\n");
        printf("Searching by ID: %d\n", id);
        found = search_by_id(output_file, buffer, id);
    } else if (type == KEY_FLOAT) {
        // Search by salary
        double salary = key.float_key;
        printf("Searching by Salary: %.2f\n", salary);

        get_index_filename(input_file, "net_salary", buffer);
        puts("\n\nBUFFER:\n");
        puts(buffer);
        puts("\n\n\n");
        found += search_by_salary(output_file, buffer, salary);

        get_index_filename(input_file, "gross_salary", buffer);
        puts("\n\nBUFFER:\n");
        puts(buffer);
        puts("\n\n\n");
        found += search_by_salary(output_file, buffer, salary);
    }  else {
        
        printf("Searching by Name: %s\n", key.str_key);
        get_index_filename(input_file, "name", buffer);
        puts("\n\nBUFFER:\n");
        puts(buffer);
        puts("\n\n\n");
        found = search_by_name(output_file, buffer, key.str_key);
    } 
    free(buffer);
    return found; // Return 1 for success, 0 for failure
}

// Binary search for integer key
static int binary_search_id(IndexEntry *arr, size_t count, int id) {
    size_t left = 0, right = count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (arr[mid].key.int_key == id) return (int)mid;
        else if (arr[mid].key.int_key < id) left = mid + 1;
        else right = mid;
    }
    return -1;
}


int search_by_id(const char *bin_filename, const char *index_filename, int id) {
    size_t count;
    IndexEntry *index = load_index(index_filename, &count);
    if (!index) return 0;
    int pos = binary_search_id(index, count, id);
    if (pos < 0) {
        free(index);
        return 0;
    }
    FILE *bf = fopen(bin_filename, "rb");
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

// Binary search for exact name match
static int binary_search_name(IndexEntry *arr, size_t count, const char *name) {
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

char *strndup(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (!p) return NULL;
    strncpy(p, s, n);
    p[n] = '\0';
    return p;
}


int search_by_name(const char *bin_filename, const char *index_filename, const char *name) {
    size_t count;
    IndexEntry *index = load_index(index_filename, &count);
    if (!index) return 0;

    FILE *bf = fopen(bin_filename, "rb");
    if (!bf) {
        perror("Error opening binary file");
        free(index);
        return 0;
    }

    int found = 0;

    // Detect if user input contains '*' (wildcard)
    if (strchr(name, '*')) {
        // Prepare pattern (remove '*', and detect position)
        const char *pattern = name;
        int prefix = 0, suffix = 0;

        if (name[0] == '*' && name[strlen(name) - 1] == '*') {
            pattern = strndup(name + 1, strlen(name) - 2); // *abc*
            prefix = suffix = 1;
        } else if (name[0] == '*') {
            pattern = name + 1; // *abc
            suffix = 1;
        } else if (name[strlen(name) - 1] == '*') {
            pattern = strndup(name, strlen(name) - 1); // abc*
            prefix = 1;
        }

        for (size_t i = 0; i < count; i++) {
            const char *entry = index[i].key.str_key;
            int match = 0;

            if (prefix && suffix) {
                match = strstr(entry, pattern) != NULL;
            } else if (prefix) {
                match = strncmp(entry, pattern, strlen(pattern)) == 0;
            } else if (suffix) {
                size_t len_entry = strlen(entry);
                size_t len_pattern = strlen(pattern);
                match = len_entry >= len_pattern &&
                        strcmp(entry + len_entry - len_pattern, pattern) == 0;
            }

            if (match) {
                fseek(bf, index[i].offset, SEEK_SET);
                PublicEmployee e;
                fread(&e, sizeof(PublicEmployee), 1, bf);
                print_public_employee(&e);
                found++;
            }
        }

        // Libera strndup se usado
        if ((prefix && !suffix) || (prefix && suffix)) free((void*)pattern);

    } else {
        // Binary search for exact match
        int pos = binary_search_name(index, count, name);
        if (pos >= 0) {
            int start = pos;
            while (start > 0 && strcmp(index[start - 1].key.str_key, name) == 0) start--;
            for (size_t i = start; i < count && strcmp(index[i].key.str_key, name) == 0; i++) {
                fseek(bf, index[i].offset, SEEK_SET);
                PublicEmployee e;
                fread(&e, sizeof(PublicEmployee), 1, bf);
                print_public_employee(&e);
                found++;
            }
        }
    }

    fclose(bf);
    free(index);
    return found;
}


/* 
int search_by_name(const char *bin_filename, const char *index_filename, const char *name) {
    size_t count;
    IndexEntry *index = load_index(index_filename, &count);
    if (!index) return 0;
    int pos = binary_search_name(index, count, name);
    if (pos < 0) {
        free(index);
        return 0;
    }
    FILE *bf = fopen(bin_filename, "rb");
    if (!bf) {
        perror("Error opening binary file");
        free(index);
        return 0;
    }
    // Walk backwards to first match
    int start = pos;
    while (start > 0 && strcmp(index[start-1].key.str_key, name) == 0) start--;
    // Print all matches
    int found = 0;
    for (size_t i = start; i < count && strcmp(index[i].key.str_key, name) == 0; i++) {
        fseek(bf, index[i].offset, SEEK_SET);
        PublicEmployee e;
        fread(&e, sizeof(PublicEmployee), 1, bf);
        print_public_employee(&e);
        found++;
    }
    fclose(bf);
    free(index);
    return found;
}
 */
static int binary_search_float(IndexEntry *arr, size_t count, double salary) {
    size_t left = 0, right = count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (arr[mid].key.float_key == salary) return (int)mid;
        else if (arr[mid].key.float_key < salary) left = mid + 1;
        else right = mid;
    }
    return -1;
}

int search_by_salary(const char *bin_filename, const char *index_filename, double salary) {
    size_t count;
    IndexEntry *index = load_index(index_filename, &count);
    if (!index) return 0;
    int pos = binary_search_float(index, count, salary);
    if (pos < 0) {
        free(index);
        return 0;
    }
    FILE *bf = fopen(bin_filename, "rb");
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
