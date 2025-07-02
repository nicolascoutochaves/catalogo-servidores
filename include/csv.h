#ifndef CSV_H
#define CSV_H

#include "public_employee.h"

#define MAX_LINE 8192
#define MAX_FIELDS 500



// Variables declared global to be allocated in the heap
extern char line[MAX_LINE];
extern char fields[MAX_FIELDS][MAX_FIELD_LEN];

int process_csv(const char* filename, const char* profile_path, const char* output_path);

#endif // CSV_H
