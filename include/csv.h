#ifndef CSV_H
#define CSV_H

#include "public_employee.h"

#define MAX_LINE 2048
#define MAX_FIELDS 20

int map_header_index(const char *header_line, const char *target_field);
void parse_csv_line(char *line, char fields[MAX_FIELDS][MAX_FIELD_LEN]);
int read_csv_and_print(const char *filename);

#endif
