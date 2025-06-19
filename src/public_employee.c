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
    printf("=========== Public Employee ===========\n");
    printf("ID:              %d\n", e->id);
    printf("Name:            %s\n", e->name);
    printf("Position:        %s\n", e->position);
    printf("Department:      %s\n", e->department);
    printf("Gross Salary:    %.2f\n", e->gross_salary);
    printf("Discounts:       %.2f\n", e->discounts);
    printf("Net Salary:      %.2f\n", e->net_salary);
    printf("=======================================\n\n");
}