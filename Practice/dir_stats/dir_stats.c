#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct { uintmax_t files, dirs, links, bytes, errors; } Stats;

/* lstat 避免跟随符号链接，也就不会因为链接环无限递归。 */
static void visit(const char *path, Stats *s) {
    struct stat st;
    if (lstat(path, &st) != 0) { fprintf(stderr, "%s: %s\n", path, strerror(errno)); ++s->errors; return; }
    if (S_ISLNK(st.st_mode)) { ++s->links; return; }
    if (S_ISREG(st.st_mode)) { ++s->files; s->bytes += (uintmax_t)st.st_size; return; }
    if (!S_ISDIR(st.st_mode)) return;
    ++s->dirs;
    DIR *dir = opendir(path);
    if (!dir) { fprintf(stderr, "%s: %s\n", path, strerror(errno)); ++s->errors; return; }
    struct dirent *entry;
    for (;;) {
        errno = 0;
        entry = readdir(dir);
        if (!entry) { if (errno) { fprintf(stderr, "%s: %s\n", path, strerror(errno)); ++s->errors; } break; }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        size_t n = strlen(path) + strlen(entry->d_name) + 2;
        char *child = malloc(n);
        if (!child) { perror("malloc"); ++s->errors; break; }
        snprintf(child, n, "%s/%s", path, entry->d_name);
        visit(child, s);
        free(child);
    }
    if (closedir(dir) != 0) { perror("closedir"); ++s->errors; }
}
int main(int argc, char **argv) {
    if (argc > 2) { fprintf(stderr, "用法: %s [目录或文件]\n", argv[0]); return 2; }
    Stats s = {0}; visit(argc == 2 ? argv[1] : ".", &s);
    printf("目录: %ju\n文件: %ju\n符号链接: %ju\n文件字节数: %ju\n错误: %ju\n", s.dirs, s.files, s.links, s.bytes, s.errors);
    return s.errors ? 1 : 0;
}
