// sort_utils.c
#include "sort_utils.h"
#include "public_employee.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BYTE_RANGE 256
#define MAX_LENGTH_NAME 256


static int compare_float_asc(const void* a, const void* b) {
    double diff = ((IndexEntry*)a)->key.float_key - ((IndexEntry*)b)->key.float_key;
    return (diff < 0) ? -1 : (diff > 0);
}

static int compare_float_desc(const void* a, const void* b) {
    double diff = ((IndexEntry*)b)->key.float_key - ((IndexEntry*)a)->key.float_key;
    return (diff < 0) ? -1 : (diff > 0);
}

void sort_by_field(const char* field, int descending, const char* filename, const char* index_filename) {
    if (!field || !filename) {
        fprintf(stderr, "Invalid field or filename.\n");
        return;
    }

    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror("fopen"); return; }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    size_t count = filesize / sizeof(PublicEmployee);
    rewind(fp);

    IndexEntry* index = calloc(count, sizeof(IndexEntry));
    
    if (!index) { perror("malloc"); fclose(fp); return; }

    for (size_t i = 0; i < count; i++) {
        long offset = ftell(fp);
        PublicEmployee e;
        fread(&e, sizeof(e), 1, fp);
        index[i].offset = offset;
        if (strcmp(field, "id") == 0)
            index[i].key.int_key = e.id;
        else if (strcmp(field, "net_salary") == 0)
            index[i].key.float_key = e.net_salary;
        else if (strcmp(field, "gross_salary") == 0)
            index[i].key.float_key = e.gross_salary;
        else if (strcmp(field, "name") == 0) {
            strncpy(index[i].key.str_key, e.name, sizeof(index[i].key.str_key));
            index[i].key.str_key[sizeof(index[i].key.str_key) - 1] = '\0';
        }
    }
    fclose(fp);

    if (strcmp(field, "id") == 0) {
        if (descending) radix_sort_int_desc(index, count);
        else           radix_sort_int(index, count);
    }
    else if (!strcmp(field, "net_salary") || !strcmp(field, "gross_salary")) {
        qsort(index, count, sizeof(IndexEntry),
            descending ? compare_float_desc : compare_float_asc);
    }
    else if (!strcmp(field, "name")) {
        radix_sort_str(index, count, MAX_LENGTH_NAME, descending);
    }

    if (index_filename) {
        FILE* out = fopen(index_filename, "wb");
        if (!out) perror("fopen index");
        else {
            fwrite(index, sizeof(IndexEntry), count, out);
            fclose(out);
            printf("Index saved: %s\n", index_filename);
        }
    }
    else {
        fp = fopen(filename, "rb");
        if (!fp) { perror("fopen"); free(index); return; }
        for (size_t i = 0; i < count; i++) {
            fseek(fp, index[i].offset, SEEK_SET);
            PublicEmployee e;
            fread(&e, sizeof(e), 1, fp);
            print_public_employee(&e);
        }
        fclose(fp);
    }
    free(index);
}

void radix_sort_int(IndexEntry* arr, size_t n) {
    IndexEntry* output = malloc(n * sizeof(IndexEntry));
    if (!output) { perror("malloc"); return; }
    int count_buf[BYTE_RANGE];

    for (int shift = 0; shift < 32; shift += 8) {
        memset(count_buf, 0, sizeof(count_buf));
        for (size_t i = 0; i < n; i++) {
            int byte = (arr[i].key.int_key >> shift) & 0xFF;
            count_buf[byte]++;
        }
        for (int i = 1; i < BYTE_RANGE; i++) count_buf[i] += count_buf[i - 1];
        for (ssize_t i = n - 1; i >= 0; i--) {
            int byte = (arr[i].key.int_key >> shift) & 0xFF;
            output[--count_buf[byte]] = arr[i];
        }
        memcpy(arr, output, n * sizeof(IndexEntry));
    }
    free(output);
}

void radix_sort_int_desc(IndexEntry* arr, size_t n) {
    radix_sort_int(arr, n);
    for (size_t i = 0; i < n / 2; i++) {
        IndexEntry tmp = arr[i]; arr[i] = arr[n - 1 - i]; arr[n - 1 - i] = tmp;
    }
}

void radix_sort_str(IndexEntry* arr, size_t n, size_t maxlen, int descending) {
    IndexEntry* output = malloc(n * sizeof(IndexEntry));
    if (!output) { perror("malloc"); return; }
    int count_buf[BYTE_RANGE];

    for (int pos = (int)maxlen - 1; pos >= 0; pos--) {
        memset(count_buf, 0, sizeof(count_buf));
        for (size_t i = 0; i < n; i++) {
            const char* s = arr[i].key.str_key;
            size_t len = strlen(s);
            unsigned char c = (pos < (int)len ? (unsigned char)s[pos] : 0);
            count_buf[c]++;
        }
        if (descending) {
            for (int i = BYTE_RANGE - 2; i >= 0; i--) count_buf[i] += count_buf[i + 1];
        }
        else {
            for (int i = 1; i < BYTE_RANGE; i++) count_buf[i] += count_buf[i - 1];
        }
        for (ssize_t i = n - 1; i >= 0; i--) {
            const char* s = arr[i].key.str_key;
            size_t len = strlen(s);
            unsigned char c = (pos < (int)len ? (unsigned char)s[pos] : 0);
            output[--count_buf[c]] = arr[i];
        }
        memcpy(arr, output, n * sizeof(IndexEntry));
    }
    free(output);
}

void qsort_float(IndexEntry* arr, size_t n, int descending) {
    qsort(arr, n, sizeof(IndexEntry), descending ? compare_float_desc : compare_float_asc);
}
