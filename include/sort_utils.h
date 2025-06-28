#ifndef SORT_UTILS_H
#define SORT_UTILS_H

#include <stddef.h>

#define BYTE_RANGE 256

typedef union {
    int int_key;
    double float_key;
    char str_key[128];
} SortKey;

typedef struct {
    SortKey key;
    long offset;
} IndexEntry;

void sort_by_field(const char *field, int descending, const char* filename, const char *index_filename);
void radix_sort_int(IndexEntry *arr, size_t n);
void radix_sort_int_desc(IndexEntry *arr, size_t n);
void radix_sort_str(IndexEntry *arr, size_t n, size_t maxlen, int descending);
void radix_sort_str_desc(IndexEntry *arr, size_t n, size_t maxlen);

void qsort_float(IndexEntry *arr, size_t n, int descending);

#endif // SORT_UTILS_H
