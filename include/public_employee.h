#ifndef PUBLIC_EMPLOYEE_H
#define PUBLIC_EMPLOYEE_H

#define MAX_FIELD_LEN 256

typedef struct {
    char name[MAX_FIELD_LEN];
    int id;
    char position[MAX_FIELD_LEN];
    char department[MAX_FIELD_LEN];
    double gross_salary;
    double discounts;
    double net_salary;
} PublicEmployee;



void initialize_public_employee(PublicEmployee *e);
void print_public_employee(const PublicEmployee *e);

#endif // PUBLIC_EMPLOYEE_H
