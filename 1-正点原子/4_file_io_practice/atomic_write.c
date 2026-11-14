#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 在目标目录创建临时文件，写入并同步后通过 rename 替换目标。 */
int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "用法: %s 目标文件 文本\n", argv[0]); return 2; }
    size_t length = strlen(argv[1]);
    char *temp = malloc(length + sizeof ".tmp.XXXXXX");
    if (!temp) { perror("malloc"); return 1; }
    snprintf(temp, length + sizeof ".tmp.XXXXXX", "%s.tmp.XXXXXX", argv[1]);
    int fd = mkstemp(temp);
    if (fd < 0) { perror("mkstemp"); free(temp); return 1; }
    size_t total = strlen(argv[2]), offset = 0; int failed = 0;
    while (offset < total) {
        ssize_t n = write(fd, argv[2] + offset, total - offset);
        if (n > 0) offset += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else { perror("write"); failed = 1; break; }
    }
    if (!failed && fsync(fd) < 0) { perror("fsync"); failed = 1; }
    if (close(fd) < 0) { perror("close"); failed = 1; }
    if (!failed && rename(temp, argv[1]) < 0) { perror("rename"); failed = 1; }
    if (failed) unlink(temp);
    free(temp);
    return failed;
}
