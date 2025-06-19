#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

int map_header_index(const char *header_line, const char *target_field) {
    char line_copy[MAX_LINE];
    strncpy(line_copy, header_line, MAX_LINE);
    char *token = strtok(line_copy, ";");
    int index = 0;

    while (token != NULL) {
        if (token[0] == '"') token[strlen(token) - 1] = '\0', token++;
        if (strcmp(token, target_field) == 0) return index;
        token = strtok(NULL, ";");
        index++;
    }

    return -1; // not found
}

void parse_csv_line(char *line, char fields[MAX_FIELDS][MAX_FIELD_LEN]) {
    char *token = strtok(line, ";");
    int idx = 0;

    while (token != NULL && idx < MAX_FIELDS) {
        if (token[0] == '"') token[strlen(token) - 1] = '\0', token++;
        strncpy(fields[idx++], token, MAX_FIELD_LEN);
        token = strtok(NULL, ";");
    }
    while (idx < MAX_FIELDS) {
        strcpy(fields[idx++], ""); // preenche os vazios restantes
    }
}

int read_csv_and_print(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE];
    char fields[MAX_FIELDS][MAX_FIELD_LEN];

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Empty file.\n");
        fclose(fp);
        return 1;
    }

    // mapeia os campos dinamicamente
    int idx_id = map_header_index(line, "Funcion?rio");
    int idx_name = map_header_index(line, "Nome Funcion?rio");
    int idx_position = map_header_index(line, "Cargo");
    int idx_department = map_header_index(line, "Centro Custo");
    int idx_gross = map_header_index(line, "Sal?rio Bruto");
    int idx_discounts = map_header_index(line, "Descontos");

    printf("ID: %d, Name: %d, Position: %d, Department: %d, Gross Salary: %d, Discounts: %d\n",
           idx_id, idx_name, idx_position, idx_department, idx_gross, idx_discounts);

    while (fgets(line, sizeof(line), fp)) {
        PublicEmployee e;
        initialize_public_employee(&e);

        line[strcspn(line, "\r\n")] = '\0';
        parse_csv_line(line, fields);

        if (idx_id >= 0 && strlen(fields[idx_id]) > 0) e.id = atoi(fields[idx_id]);
        if (idx_name >= 0 && strlen(fields[idx_name]) > 0) strncpy(e.name, fields[idx_name], MAX_FIELD_LEN);
        if (idx_position >= 0 && strlen(fields[idx_position]) > 0) strncpy(e.position, fields[idx_position], MAX_FIELD_LEN);
        if (idx_department >= 0 && strlen(fields[idx_department]) > 0) strncpy(e.department, fields[idx_department], MAX_FIELD_LEN);
        if (idx_gross >= 0 && strlen(fields[idx_gross]) > 0) e.gross_salary = atof(fields[idx_gross]);
        if (idx_discounts >= 0 && strlen(fields[idx_discounts]) > 0) e.discounts = atof(fields[idx_discounts]);
        e.net_salary = e.gross_salary - e.discounts;

        print_public_employee(&e);
    }

    fclose(fp);
    return 0;
}
