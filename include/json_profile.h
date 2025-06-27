#ifndef JSON_PROFILE_H
#define JSON_PROFILE_H

#include <stdbool.h>
#include "cJSON.h"

typedef struct {
    bool use_direct_net;
    char **aliases_nome;
    int count_nome;
    char **aliases_matricula;
    int count_matricula;
    char **aliases_cargo;
    int count_cargo;
    char **aliases_departamento;
    int count_departamento;
    char **aliases_bruto;
    int count_bruto;
    char **aliases_discounts;
    int count_discounts;
    char **aliases_net;
    int count_net;
    char **aliases_encoding;
    int count_encoding;
} Profile;

Profile *load_profile(const char *path);
void free_profile(Profile *p);

#endif // JSON_PROFILE_H
