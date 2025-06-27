#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_profile.h"

static char **parse_alias_array(cJSON *arr, int *count) {
    if (!cJSON_IsArray(arr)) { *count = 0; return NULL; }
    int n = cJSON_GetArraySize(arr);
    char **list = malloc(n * sizeof(char*));
    for (int i = 0; i < n; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        list[i] = cJSON_IsString(item) ? strdup(item->valuestring) : NULL;
    }
    *count = n;
    return list;
}

Profile *load_profile(const char *path) {
    FILE *fp = fopen(path, "r"); if (!fp) return NULL;
    fseek(fp, 0, SEEK_END); long sz = ftell(fp); rewind(fp);
    char *txt = malloc(sz + 1); fread(txt, 1, sz, fp); txt[sz] = '\0'; fclose(fp);
    cJSON *json = cJSON_Parse(txt); free(txt);
    if (!json) return NULL;

    Profile *p = calloc(1, sizeof(Profile));
    p->use_direct_net = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "usa_liquido_direto"));
    p->aliases_nome       = parse_alias_array(cJSON_GetObjectItem(json, "nome"), &p->count_nome);
    p->aliases_matricula  = parse_alias_array(cJSON_GetObjectItem(json, "matricula"), &p->count_matricula);
    p->aliases_cargo      = parse_alias_array(cJSON_GetObjectItem(json, "cargo"), &p->count_cargo);
    p->aliases_departamento = parse_alias_array(cJSON_GetObjectItem(json, "departamento"), &p->count_departamento);
    p->aliases_bruto      = parse_alias_array(cJSON_GetObjectItem(json, "salario_bruto"), &p->count_bruto);
    p->aliases_discounts  = parse_alias_array(cJSON_GetObjectItem(json, "descontos"), &p->count_discounts);
    p->aliases_net        = parse_alias_array(cJSON_GetObjectItem(json, "salario_liquido"), &p->count_net);
    p->aliases_encoding  = parse_alias_array(cJSON_GetObjectItem(json, "encoding"), &p->count_encoding);
   


    cJSON_Delete(json);
    return p;
}

void free_profile(Profile *p) {
    #define FREE_LIST(field) for (int i = 0; i < p->count_##field; i++) free(p->aliases_##field[i]); free(p->aliases_##field)
    FREE_LIST(nome); FREE_LIST(matricula); FREE_LIST(cargo);
    FREE_LIST(departamento); FREE_LIST(bruto); FREE_LIST(discounts); FREE_LIST(net); FREE_LIST(encoding);
    free(p);
}
    