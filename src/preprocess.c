#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int preprocess(char *argv[]) {
    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    char output_file[256];
    snprintf(output_file, sizeof(output_file), "%s", argv[1]);
    for(int i = 0; i < 256; i++){
        if(strncmp(output_file + i, ".csv", 4) == 0) {
            output_file[i] = '\0'; // corta a extensão .csv
            break;
        }
    }
    strcat(output_file, "_preprocessed.csv");
    FILE *fout = fopen(output_file, "wb");

    if (!fout) {
        perror("Erro ao abrir arquivo de saída");
        fclose(fin);
        return 1;
    }

    int c;
    while ((c = fgetc(fin)) != EOF) {
        if (c >= 0 && c <= 127) {
            // ASCII válido, escreve diretamente
            fputc(c, fout);
        } else {
            // caractere UTF8 fora do ASCII, substitui por '?'
            fputc('?', fout);

            // pula bytes adicionais do caractere UTF-8 multibyte
            // Detectar quantos bytes tem o caractere pelo primeiro byte
            // UTF-8:
            // 0xxxxxxx - 1 byte (ASCII)
            // 110xxxxx - 2 bytes
            // 1110xxxx - 3 bytes
            // 11110xxx - 4 bytes

            int skip_bytes = 0;

            if ((c & 0xE0) == 0xC0)       // 2 bytes
                skip_bytes = 1;
            else if ((c & 0xF0) == 0xE0)  // 3 bytes
                skip_bytes = 2;
            else if ((c & 0xF8) == 0xF0)  // 4 bytes
                skip_bytes = 3;

            // pula os bytes de continuação 10xxxxxx
            for (int i = 0; i < skip_bytes; i++) {
                int skip_c = fgetc(fin);
                if (skip_c == EOF) break;
            }
        }
    }

    fclose(fin);
    fclose(fout);

    printf("Arquivo convertido com sucesso.\n");
    return 0;
}
