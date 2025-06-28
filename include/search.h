#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include "public_employee.h"
#include "sort_utils.h"

IndexEntry *load_index(const char *index_filename, size_t *out_count);
int search_by_id(const char *bin_filename, const char *index_filename, int id);
int search_by_name(const char *bin_filename, const char *index_filename, const char *name);

#endif // SEARCH_H