#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
    #include <windows.h>
#endif

#include "public_employee.h"

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    const char *binary_file = "bin/public_employees.dat";

    char name_to_find[MAX_FIELD_LEN] = "TANIA MARTINS LEWY"; 

    for(int i = 0; i < strlen(name_to_find); i++){
        name_to_find[i] = toupper(name_to_find[i]);
    }
    FILE *fp = fopen(binary_file, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo binário");
        return 1;
    }

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    PublicEmployee e;
    int found = 0;
    while (fread(&e, sizeof(PublicEmployee), 1, fp) == 1) {
        if(!strcmp(e.name, name_to_find)) {
            print_public_employee(&e);
            found = 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    fclose(fp);

    
    printf("⏱ Busca sem índice levou %.2f ms\n",
        (t2.tv_sec - t1.tv_sec) * 1000.0 + 
        (t2.tv_nsec - t1.tv_nsec) / 1e6);
        
    if (!found) {
        printf("Nome '%s' não encontrado no arquivo binário.\n", name_to_find);
    }
    
    return 0;
}
