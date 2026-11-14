#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* 多进程合作追加文本：fcntl 建议锁作用于整个文件。 */
static int lock_file(int fd, short type, int command) {
    struct flock lock = {0};
    lock.l_type = type; lock.l_whence = SEEK_SET;
    while (fcntl(fd, command, &lock) < 0) {
        if (errno != EINTR) return -1;
    }
    return 0;
}
static int write_all(int fd, const char *s, size_t len) {
    while (len) {
        ssize_t n = write(fd, s, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { if (!n) errno = EIO; return -1; }
        s += n; len -= (size_t)n;
    }
    return 0;
}
int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "用法: %s 文件 一行内容\n", argv[0]); return 2; }
    if (strchr(argv[2], '\n') || strchr(argv[2], '\r')) { fprintf(stderr, "内容不能包含换行\n"); return 2; }
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) { perror("open"); return 1; }
    if (lock_file(fd, F_WRLCK, F_SETLKW) < 0) { perror("lock"); close(fd); return 1; }
    int failed = 0;
    if (write_all(fd, argv[2], strlen(argv[2])) < 0 || write_all(fd, "\n", 1) < 0) { perror("write"); failed = 1; }
    if (lock_file(fd, F_UNLCK, F_SETLK) < 0) { perror("unlock"); failed = 1; }
    if (close(fd) < 0) { perror("close"); failed = 1; }
    return failed;
}
