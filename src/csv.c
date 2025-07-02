#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "csv.h"
#include "json_profile.h"
#include "preprocess.h"
#include "public_employee.h"
#include "file_utils.h"

static void convert_brazilian_number(char* input) {
    if (!strchr(input, ',')) return; // já está em formato internacional

    char temp[MAX_FIELD_LEN];
    int j = 0;
    for (int i = 0; input[i] != '\0' && j < MAX_FIELD_LEN - 1; i++) {
        if (input[i] == '.') continue;
        else if (input[i] == ',') temp[j++] = '.';
        else temp[j++] = input[i];
    }
    temp[j] = '\0';
    strncpy(input, temp, MAX_FIELD_LEN);
}

int map_header_index_any_list(const char* header_line, char** aliases, int alias_count) {
    char line_copy[MAX_LINE];
    strncpy(line_copy, header_line, MAX_LINE);
    line_copy[MAX_LINE - 1] = '\0';

    char* token = strtok(line_copy, ";");
    int index = 0;

    while (token != NULL) {
        if (token[0] == '"') {
            token[strlen(token) - 1] = '\0';
            token++;
        }

        for (int i = 0; i < alias_count; i++) {
            if (strcasecmp(token, aliases[i]) == 0) {
                return index;
            }
        }

        token = strtok(NULL, ";");
        index++;
    }

    return -1;
}



void parse_csv_line(char* line, char fields[MAX_FIELDS][MAX_FIELD_LEN]) {
    char* token = strtok(line, ";");
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

int process_csv(const char* filename, const char* profile_path, const char* output_path) {

    if(file_exists(output_path)) {
        puts("Output file already exists. Skipping creation...");
        return 1;
    }

    Profile* p = load_profile(profile_path);
    if (!p) {
        fprintf(stderr, "Error loading profile.\n");
        return 0;
    }

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening CSV file.\n");
        free_profile(p);
        return 0;
    }

    char line[MAX_LINE];
    char fields[MAX_FIELDS][MAX_FIELD_LEN];

    // Pular até achar o cabeçalho
    while (fgets(line, MAX_LINE, fp)) {
        if (strchr(line, ';')) break;
    }

    if (feof(fp)) {
        fprintf(stderr, "Header not found during file reading.\n");
        fclose(fp);
        free_profile(p);
        return 0;
    }

    // Mapeamento dinâmico
    int idx_id = map_header_index_any_list(line, p->aliases_matricula, p->count_matricula);
    int idx_name = map_header_index_any_list(line, p->aliases_nome, p->count_nome);
    int idx_position = map_header_index_any_list(line, p->aliases_cargo, p->count_cargo);
    int idx_department = map_header_index_any_list(line, p->aliases_departamento, p->count_departamento);
    int idx_bruto = map_header_index_any_list(line, p->aliases_bruto, p->count_bruto);
    int idx_disc = map_header_index_any_list(line, p->aliases_discounts, p->count_discounts);
    int idx_net = map_header_index_any_list(line, p->aliases_net, p->count_net);


    //Preenchimento da struct PublicEmployee e salvamento em binário
    FILE* out_fp = fopen(output_path, "wb");
    int i = 0;
    while (fgets(line, sizeof(line), fp) && i < MAX_EMPLOYEES) {
        PublicEmployee e;
        initialize_public_employee(&e);

        line[strcspn(line, "\r\n")] = '\0';
        parse_csv_line(line, fields);

        if (idx_id >= 0 && strlen(fields[idx_id]) > 0) e.id = atoi(fields[idx_id]);
        if (idx_name >= 0 && strlen(fields[idx_name]) > 0) strncpy(e.name, fields[idx_name], MAX_FIELD_LEN);
        if (idx_position >= 0 && strlen(fields[idx_position]) > 0) strncpy(e.position, fields[idx_position], MAX_FIELD_LEN);
        if (idx_department >= 0 && strlen(fields[idx_department]) > 0) strncpy(e.department, fields[idx_department], MAX_FIELD_LEN);
        if (idx_bruto >= 0 && strlen(fields[idx_bruto]) > 0) {
            convert_brazilian_number(fields[idx_bruto]);
            e.gross_salary = atof(fields[idx_bruto]);
        }
        if (idx_disc >= 0 && strlen(fields[idx_disc]) > 0) {
            convert_brazilian_number(fields[idx_disc]);
            e.discounts = atof(fields[idx_disc]);
        }

        if (p->use_direct_net && idx_net >= 0 && strlen(fields[idx_net]) > 0) {
            convert_brazilian_number(fields[idx_net]);
                e.net_salary = atof(fields[idx_net]);
        }
        else if (e.gross_salary >= 0 && e.discounts >= 0) {
            e.net_salary = e.gross_salary - e.discounts;
        }
        if(e.id < 0){
            e.id = i+1; // Atribui um ID sequencial se não estiver presente
        }

        if(fwrite(&e, sizeof(PublicEmployee), 1, out_fp) != 1) {
            fprintf(stderr, "Error writing to binary file.\n");
            fclose(out_fp);
            fclose(fp);
            free_profile(p);
            return 1;
        }

        i++;
    }

    fclose(fp);
    fclose(out_fp);
    free_profile(p);

    printf("CSV processed and saved to binary file: %s\n", output_path);
    return 1;
}
