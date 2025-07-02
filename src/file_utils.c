#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#ifdef __linux__
#include <sys/sendfile.h>
#endif
#include "file_utils.h"

int file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}
void remove_extension(char* filename) {
    size_t n = strlen(filename);
    for (size_t i = 0; i + 4 <= n; ++i) {
        if (strncmp(&filename[i], ".csv", 4) == 0 ||
            strncmp(&filename[i], ".json", 5) == 0 ||
            strncmp(&filename[i], ".txt", 4) == 0) {
            filename[i] = '\0';
            return;
        }
    }
}

int ends_with(const char* str, const char* suffix) {
    size_t ls = strlen(str), lx = strlen(suffix);
    return (ls >= lx && strcmp(str + ls - lx, suffix) == 0);
}

int copy_file_syscall(const char *src, const char *dst) {
#ifdef _WIN32
    if (!CopyFileA(src, dst, FALSE)) {
        fprintf(stderr, "CopyFileA failed (error %lu)\n", GetLastError());
        return 0;
    }
    return 1;
#else
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0) { perror("open(src)"); return 0; }
    struct stat st;
    if (fstat(in_fd, &st) < 0) { perror("fstat"); close(in_fd); return 0; }
    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (out_fd < 0) { perror("open(dst)"); close(in_fd); return 0; }
    off_t off = 0;
    while (off < st.st_size) {
        ssize_t sent = sendfile(out_fd, in_fd, &off, st.st_size - off);
        if (sent <= 0) { perror("sendfile"); close(in_fd); close(out_fd); return 0; }
    }
    close(in_fd);
    close(out_fd);
    return 1;
#endif
}

int delete_files_matching(const char *dirpath, const char *pattern) {
    const char *suffix = (pattern[0] == '*') ? pattern + 1 : pattern;
    DIR *d = opendir(dirpath);
    if (!d) { perror("opendir"); return 0; }

    struct dirent *ent;
    char path[MAX_FILENAME];
    int count = 0;
    while ((ent = readdir(d)) != NULL) {
        if (ends_with(ent->d_name, suffix)) {
            snprintf(path, sizeof(path), "%s/%s", dirpath, ent->d_name);
            if (unlink(path) == 0) {
                printf("Deleted: %s\n", path);
                count++;
            } else {
                perror("unlink");
            }
        }
    }
    closedir(d);
    return count;
}

int delete_index_files() {
    return delete_files_matching("index/data", "*.idx");
}

int delete_preprocessed_csv() {
    return delete_files_matching("data", "*_preprocessed.csv");
}

int delete_binary_files() {
    return delete_files_matching("bin", "*");
}

int delete_all_files() {
    int c = 0;
    c += delete_index_files();
    c += delete_preprocessed_csv();
    c += delete_binary_files();
    return c;
}


int ask_continue_pagination() {
    printf("Show more? (any key to continue or type ESC to stop): ");
    int ch;

#ifdef _WIN32
    ch = getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    puts("\n");
    if (ch == 27) return 0;     // ESC
    if (ch == '\n' || ch == '\r') return 1;   // ENTER
    return 1;
}
