#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
    #include <windows.h>
#endif

#include "search.h"  // <--- função de busca por índice

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    const char *binary_file = "bin/public_employees.dat";
    
    const char *name_idx = "index/data/estado_sao_paulo_name.idx";

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);


    char name_to_find[256] = "TANIA MARTINS LEWY"; // Substitua por um nome válido do seu dataset
    for(int i = 0; i < strlen(name_to_find); i++){
        name_to_find[i] = toupper(name_to_find[i]);
    }

    if(!search_by_name(binary_file, name_idx, name_to_find)) {
        printf("Name not found in index.\n");
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("⏱ Busca com índice levou %.2f ms\n", 
        (t2.tv_sec - t1.tv_sec) * 1000.0 + 
        (t2.tv_nsec - t1.tv_nsec) / 1e6);
    return 0;
}
