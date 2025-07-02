#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>       // for _chsize_s, _fileno
    #include <conio.h>
    #define ftruncate _chsize_s
    #define fileno _fileno
#else
    #include <sys/sendfile.h>
    #include <sys/stat.h>
    #include <unistd.h>   // for ftruncate
    #include <fcntl.h>    // for fileno
    #include <termios.h>
    #include <unistd.h>
#endif

#define LINE_BUF 8192 

#define MAX_FILENAME 256
#define PAGE_SIZE 3


int file_exists(const char *filename);
void remove_extension(char *filename);
int copy_file_syscall(const char *src, const char *dst);
int ask_continue_pagination();

// Funções de exclusão
int delete_files_matching(const char *dirpath, const char *pattern);
int delete_index_files();
int delete_preprocessed_csv();
int delete_binary_files();
int delete_all_files(); // Deleta todos os arquivos gerados pelo programa   

#endif // FILE_UTILS_H
