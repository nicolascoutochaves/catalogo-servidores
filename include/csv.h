#ifndef CSV_H
#define CSV_H

#include "public_employee.h"

#define MAX_LINE 8192
#define MAX_FIELDS 500
#define MAX_EMPLOYEES 5000000


// Variables declared global to be allocated in the heap
extern char line[MAX_LINE];
extern char fields[MAX_FIELDS][MAX_FIELD_LEN];




int map_header_index_any_list(const char *header_line, char **aliases, int alias_count);

void parse_csv_line(char *line, char fields[MAX_FIELDS][MAX_FIELD_LEN]);
int read_csv_and_print(const char *filename, const char *profile_path);

#endif // CSV_H
