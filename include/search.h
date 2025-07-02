#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include "public_employee.h"
#include "sort_utils.h"


char* strndup(const char* s, size_t n);

int search(char *input_file, char *output_file, SortKey key, KeyType type, double eps);
int search_by_id(const char *bin_filename, const char *index_filename, int id);
int search_by_name(const char *bin_filename, const char *index_filename, const char *name);
int search_by_salary(const char *bin_filename, const char *index_filename, double salary, double eps);
int binary_search_name(IndexEntry* arr, size_t count, const char* name);
void prompt_and_search(char *input_file, char *bin_file, int *opt);

#endif // SEARCH_H