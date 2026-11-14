#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/* 文件 IO：循环处理短读、短写和 EINTR。目的文件必须不存在。 */
static int write_all(int fd, const char *buf, size_t len) {
    while (len) {
        ssize_t n = write(fd, buf, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { if (!n) errno = EIO; return -1; }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}
int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "用法: %s 源文件 新文件\n", argv[0]); return 2; }
    int in = open(argv[1], O_RDONLY);
    if (in < 0) { perror(argv[1]); return 1; }
    struct stat st;
    if (fstat(in, &st) < 0) { perror("fstat"); close(in); return 1; }
    if (!S_ISREG(st.st_mode)) { fprintf(stderr, "源文件必须是普通文件\n"); close(in); return 1; }
    int out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, st.st_mode & 0777);
    if (out < 0) { perror(argv[2]); close(in); return 1; }
    int failed = 0;
    char buffer[16384];
    for (;;) {
        ssize_t n = read(in, buffer, sizeof buffer);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) { perror("read"); failed = 1; break; }
        if (!n) break;
        if (write_all(out, buffer, (size_t)n) < 0) { perror("write"); failed = 1; break; }
    }
    if (close(in) < 0) { perror("close source"); failed = 1; }
    if (close(out) < 0) { perror("close destination"); failed = 1; }
    if (failed) fprintf(stderr, "复制未完成，已创建的目标文件需要手动检查。\n");
    return failed;
}
