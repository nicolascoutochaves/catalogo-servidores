#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <stdio.h>
#define MAX_FILENAME 256

int file_exists(const char *filename);
int preprocess(char *input_file, char *output_file, const char *encoding);
void remove_extension(char*);

#endif // PREPROCESS_H