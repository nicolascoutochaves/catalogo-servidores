#include <stdio.h>
#include <string.h>
#include "public_employee.h"

void initialize_public_employee(PublicEmployee *e) {
    e->id = -1;
    e->gross_salary = -1.0;
    e->discounts = -1.0;
    e->net_salary = -1.0;

    strcpy(e->name, "nao informado");
    strcpy(e->position, "nao informado");
    strcpy(e->department, "nao informado");
}



void print_public_employee(const PublicEmployee *e) {
    printf("=========== Servidor ===========\n");
    printf("Matrícula:       %d\n", e->id);
    printf("Nome:            %s\n", e->name);
    printf("Cargo:           %s\n", e->position);
    printf("Lotação:         %s\n", e->department);
    printf("Salário Bruto:   %.2f\n", e->gross_salary);
    printf("Descontos:       %.2f\n", e->discounts);
    printf("Salário Líquido: %.2f\n", e->net_salary);
    printf("=======================================\n\n");
}