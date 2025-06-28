// search.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search.h"
#include "sort_utils.h"
#include "public_employee.h"

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
