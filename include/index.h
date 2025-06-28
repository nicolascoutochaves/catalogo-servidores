#ifndef INDEX_H
#define INDEX_H


#include "sort_utils.h"
#include "public_employee.h"


void build_index(const char *field, const char* bin_filename, const char *index_filename, int descending);


double get_maximum(const char *field); //get maximum value of a field (net_salary, gross_salary, etc.)

double get_minimum(const char *field); //get minimum value of a field (net_salary, gross_salary, etc.)

#endif // INDEX_H