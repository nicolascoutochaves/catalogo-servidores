#ifndef INDEX_H
#define INDEX_H


#include "sort_utils.h"
#include "public_employee.h"



int build_index(const char *field, const char* bin_filename, const char *index_filename, int descending);

void get_index_filename(char* filename, const char* field, char* index_filename); //get index filename based on field and input file

int create_index(char* filename, const char* output_file); //create index for the given file and output file

double get_maximum(const char *field); //get maximum value of a field (net_salary, gross_salary, etc.)

double get_minimum(const char *field); //get minimum value of a field (net_salary, gross_salary, etc.)

#endif // INDEX_H